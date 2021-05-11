



#include <iostream>
#include <assert.h>
#include <limits>

#include "access_map_pattern_matching.hh"
#include "./libs/associative_set_impl.hh"
#include "./libs/set_associative.hh"
#include "./libs/lfu_rp.hh"


using namespace std;
using namespace IndexingPolicyPolicy;
using namespace ReplacementPolicy;


namespace Prefetcher {

AccessMapPatternMatching::AccessMapPatternMatching()
   :
    // (int assoc, int num_entries,
        // BaseIndexingPolicy *idx_policy, 
			// ReplacementPolicy::Base *rpl_policy,
        // Entry const &init_value)
    // prefetchTable(16, 16, new SetAssociative(16, 16, 1), new LFU(), PrefetchTableEntry(3))
	// uint32_t size, unsigned assoc, unsigned entry_size)
      accessMapTable(8, 256,
					new SetAssociative(256, 8, 1), 
					new LFU(), 
					AccessMapEntry((2048)/64)),
      numGoodPrefetches(0), numTotalPrefetches(0), numRawCacheMisses(0),
      numRawCacheHits(0), degree(startDegree), usefulDegree(startDegree)
{
    blkSize=64;
	limitStride=0;
	startDegree=4; //FIXE ME MAJID
	hotZoneSize=2*1024; //FIX ME 
	highCoverageThreshold=0.25;
	lowCoverageThreshold=0.125;
	highAccuracyThreshold=0.5;
	lowAccuracyThreshold=0.25;
	highCacheHitThreshold=0.875;
	lowCacheHitThreshold=0.75;
	// epochCycles=256000;
	epochCycles=256;
	offChipMemoryLatency=30; //FIXE ME MAJID ns
}



void
AccessMapPatternMatching::processEpochEvent()
{
    double prefetch_accuracy =
        ((double) numGoodPrefetches) / ((double) numTotalPrefetches);
    double prefetch_coverage =
        ((double) numGoodPrefetches) / ((double) numRawCacheMisses);
    double cache_hit_ratio = ((double) numRawCacheHits) /
        ((double) (numRawCacheHits + numRawCacheMisses));
    double num_requests = (double) (numRawCacheMisses - numGoodPrefetches +
        numTotalPrefetches);
    // double memory_bandwidth = num_requests * offChipMemoryLatency /
        // clockEdge(epochCycles);
	//Majid: Approximating the BW
	//FIXME
	double memory_bandwidth = num_requests * offChipMemoryLatency /
        (epochCycles);
    if (prefetch_coverage > highCoverageThreshold &&
        (prefetch_accuracy > highAccuracyThreshold ||
        cache_hit_ratio < lowCacheHitThreshold)) {
        usefulDegree += 1;
    } else if ((prefetch_coverage < lowCoverageThreshold &&
               (prefetch_accuracy < lowAccuracyThreshold ||
                cache_hit_ratio > highCacheHitThreshold)) ||
               (prefetch_accuracy < lowAccuracyThreshold &&
                cache_hit_ratio > highCacheHitThreshold)) {
        usefulDegree -= 1;
    }
    degree = std::min((unsigned) memory_bandwidth, usefulDegree);
    // reset epoch stats
    numGoodPrefetches = 0.0;
    numTotalPrefetches = 0.0;
    numRawCacheMisses = 0.0;
    numRawCacheHits = 0.0;
}

AccessMapPatternMatching::AccessMapEntry *
AccessMapPatternMatching::getAccessMapEntry(uint64_t am_addr,
                bool is_secure)
{
    AccessMapEntry *am_entry = accessMapTable.findEntry(am_addr, is_secure);
    if (am_entry != nullptr) {
        accessMapTable.accessEntry(am_entry);
    } else {
        am_entry = accessMapTable.findVictim(am_addr);
        assert(am_entry != nullptr);

        accessMapTable.insertEntry(am_addr, is_secure, am_entry);
    }
    return am_entry;
}

void
AccessMapPatternMatching::setEntryState(AccessMapEntry &entry,
    uint64_t block, enum AccessMapState state)
{
    enum AccessMapState old = entry.states[block];
    entry.states[block] = state;

    //do not update stats when initializing
    if (state == AM_INIT) return;

    switch (old) {
        case AM_INIT:
            if (state == AM_PREFETCH) {
                numTotalPrefetches += 1;
            } else if (state == AM_ACCESS) {
                numRawCacheMisses += 1;
            }
            break;
        case AM_PREFETCH:
            if (state == AM_ACCESS) {
                numGoodPrefetches += 1;
                numRawCacheMisses += 1;
            }
            break;
        case AM_ACCESS:
            if (state == AM_ACCESS) {
                numRawCacheHits += 1;
            }
            break;
        default:
            // panic("Impossible path\n");
            break;
    }
}

void
AccessMapPatternMatching::calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresse)
{
    assert(addresse.empty());
	numCalls +=1;
	if(numCalls > epochCycles){
		numCalls = 0;
		processEpochEvent();
	}

    bool is_secure = true;
    uint64_t am_addr = (lineAddr) / hotZoneSize;
    uint64_t current_block = ((lineAddr) % hotZoneSize) / blkSize; //FIXME
    uint64_t lines_per_zone = hotZoneSize / blkSize; //FIXME

    // Get the entries of the curent block (am_addr), the previous, and the
    // following ones
    AccessMapEntry *am_entry_curr = getAccessMapEntry(am_addr, is_secure);
    AccessMapEntry *am_entry_prev = (am_addr > 0) ?
        getAccessMapEntry(am_addr-1, is_secure) : nullptr;
    AccessMapEntry *am_entry_next = (am_addr < ((std::numeric_limits<uint64_t>::max())/hotZoneSize)) ?
        getAccessMapEntry(am_addr+1, is_secure) : nullptr;
	// cout<<"am_addr:"<<am_addr<<" am_addr+1:"<<am_addr+1<<" am_addr-1:"<<am_addr-1<<endl;
    assert(am_entry_curr != am_entry_prev);
    assert(am_entry_curr != am_entry_next);
	// cout<<"max "<<(std::numeric_limits<uint64_t>::max())<<" hotZoneSize"<<hotZoneSize<<endl;
	// std::cout<<"am_entry_prev "<<am_entry_prev<<" am_entry_next "<<am_entry_next<<std::endl;
    assert(am_entry_prev != am_entry_next);
    assert(am_entry_curr != nullptr);

    //Mark the current access as Accessed
    setEntryState(*am_entry_curr, current_block, AM_ACCESS);

    /**
     * Create a contiguous copy of the 3 entries states.
     * With this, we avoid doing boundaries checking in the loop that looks
     * for prefetch candidates, mark out of range positions with AM_INVALID
     */
    std::vector<AccessMapState> states(3 * lines_per_zone);
    for (unsigned idx = 0; idx < lines_per_zone; idx += 1) {
        states[idx] =
            am_entry_prev != nullptr ? am_entry_prev->states[idx] : AM_INVALID;
        states[idx + lines_per_zone] = am_entry_curr->states[idx];
        states[idx + 2 * lines_per_zone] =
            am_entry_next != nullptr ? am_entry_next->states[idx] : AM_INVALID;
    }

    /**
     * am_entry_prev->states => states[               0 ..   lines_per_zone-1]
     * am_entry_curr->states => states[  lines_per_zone .. 2*lines_per_zone-1]
     * am_entry_next->states => states[2*lines_per_zone .. 3*lines_per_zone-1]
     */

    // index of the current_block in the new vector
    uint64_t states_current_block = current_block + lines_per_zone;
    // consider strides 1..lines_per_zone/2
    int max_stride = limitStride == 0 ? lines_per_zone / 2 : limitStride + 1;
    for (unsigned int stride = 1; stride < (unsigned int)max_stride; stride += 1) {
        // Test accessed positive strides
        if (checkCandidate(states, states_current_block, stride)) {
            // candidate found, current_block - stride
            uint64_t pf_addr;
            if (stride > current_block) {
                // The index (current_block - stride) falls in the range of
                // the previous zone (am_entry_prev), adjust the address
                // accordingly
                uint64_t blk = states_current_block - stride;
                pf_addr = (am_addr - 1) * hotZoneSize + blk * blkSize;
                setEntryState(*am_entry_prev, blk, AM_PREFETCH);
            } else {
                // The index (current_block - stride) falls within
                // am_entry_curr
                uint64_t blk = current_block - stride;
                pf_addr = am_addr * hotZoneSize + blk * blkSize;
                setEntryState(*am_entry_curr, blk, AM_PREFETCH);
            }
			pf_addr = (pf_addr)>>6;
			assert(proc_id == (pf_addr >> (58 - 6)));
			addresse.push_back(pf_addr);
            if (addresse.size() == degree) {
                break;
            }
        }

        // Test accessed negative strides
        if (checkCandidate(states, states_current_block, -stride)) {
            // candidate found, current_block + stride
            uint64_t pf_addr;
            if (current_block + stride >= lines_per_zone) {
                // The index (current_block + stride) falls in the range of
                // the next zone (am_entry_next), adjust the address
                // accordingly
                uint64_t blk = (states_current_block + stride) % lines_per_zone;
                pf_addr = (am_addr + 1) * hotZoneSize + blk * blkSize;
                setEntryState(*am_entry_next, blk, AM_PREFETCH);
            } else {
                // The index (current_block + stride) falls within
                // am_entry_curr
                uint64_t blk = current_block + stride;
                pf_addr = am_addr * hotZoneSize + blk * blkSize;
                setEntryState(*am_entry_curr, blk, AM_PREFETCH);
            }
            pf_addr = (pf_addr)>>6;
			assert(proc_id == (pf_addr >> (58 - 6)));
			addresse.push_back(pf_addr);
            if (addresse.size() == degree) {
                break;
            }
        }
    }
}

AMPM::AMPM()
{
	ampm =  new AccessMapPatternMatching();
}

void
AMPM::calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresse)
{
	ampm->calculatePrefetch(proc_id, lineAddr, loadPC, global_hist, addresse);
}

}
