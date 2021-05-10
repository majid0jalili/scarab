

#include <iostream>
#include <assert.h>


#include "indirect_memory.hh"
#include "./libs/associative_set_impl.hh"
#include "./libs/set_associative.hh"
#include "./libs/lfu_rp.hh"


using namespace std;
using namespace IndexingPolicyPolicy;
using namespace ReplacementPolicy;




	

namespace Prefetcher {
	
IndirectMemory::IndirectMemory():
    prefetchTable(16, 16, new SetAssociative(16, 16, 1), new LFU(), PrefetchTableEntry(3)),
					ipd(4, 4, new SetAssociative(4, 4, 1),
					 new LFU(),
					IndirectPatternDetectorEntry(4, 4)),
					ipdEntryTrackingMisses(nullptr) 
{
	// , byteOrder(p.sys->getGuestByteOrder()
	maxPrefetchDistance=16;
	shiftValues.push_back(2);
	shiftValues.push_back(3);
	shiftValues.push_back(4);
	shiftValues.push_back(-3);
	prefetchThreshold=2;
	streamCounterThreshold=4;
	streamingDistance=4;
	
}

void
IndirectMemory::allocateOrUpdateIPDEntry(
    const PrefetchTableEntry *pt_entry, int64_t index)
{
    // The address of the pt_entry is used to index the IPD
    uint64_t ipd_entry_addr = (uint64_t) pt_entry;
    IndirectPatternDetectorEntry *ipd_entry = ipd.findEntry(ipd_entry_addr,
                                                            false/* unused */);
    if (ipd_entry != nullptr) {
        ipd.accessEntry(ipd_entry);
        if (!ipd_entry->secondIndexSet) {
            // Second time we see an index, fill idx2
            ipd_entry->idx2 = index;
            ipd_entry->secondIndexSet = true;
            ipdEntryTrackingMisses = ipd_entry;
        } else {
            // Third access! no pattern has been found so far,
            // release the IPD entry
            ipd.invalidate(ipd_entry);
            ipdEntryTrackingMisses = nullptr;
        }
    } else {
        ipd_entry = ipd.findVictim(ipd_entry_addr);
        assert(ipd_entry != nullptr);
        ipd.insertEntry(ipd_entry_addr, false /* unused */, ipd_entry);
        ipd_entry->idx1 = index;
        ipdEntryTrackingMisses = ipd_entry;
    }
}

void
IndirectMemory::trackMissIndex1(uint64_t miss_addr)
{
    IndirectPatternDetectorEntry *entry = ipdEntryTrackingMisses;
    // If the second index is not set, we are just filling the baseAddr
    // vector
    assert(entry->numMisses < (int)entry->baseAddr.size());
    std::vector<uint64_t> &ba_array = entry->baseAddr[entry->numMisses];
    int idx = 0;
    for (int shift : shiftValues) {
        ba_array[idx] = miss_addr - (entry->idx1 << shift);
        idx += 1;
    }
    entry->numMisses += 1;
    if (entry->numMisses == (int)entry->baseAddr.size()) {
        // stop tracking misses once we have tracked enough
        ipdEntryTrackingMisses = nullptr;
    }
}
void
IndirectMemory::trackMissIndex2(uint64_t miss_addr)
{
    IndirectPatternDetectorEntry *entry = ipdEntryTrackingMisses;
    // Second index is filled, compare the addresses generated during
    // the previous misses (using idx1) against newly generated values
    // using idx2, if a match is found, fill the additional fields
    // of the PT entry
    for (int midx = 0; midx < entry->numMisses; midx += 1)
    {
        std::vector<uint64_t> &ba_array = entry->baseAddr[midx];
        int idx = 0;
        for (int shift : shiftValues) {
            if (ba_array[idx] == (miss_addr - (entry->idx2 << shift))) {
                // Match found!
                // Fill the corresponding pt_entry
                PrefetchTableEntry *pt_entry =
                    (PrefetchTableEntry *) entry->getTag();
                pt_entry->baseAddr = ba_array[idx];
                pt_entry->shift = shift;
                pt_entry->enabled = true;
                pt_entry->indirectCounter.reset();
                // Release the current IPD Entry
                ipd.invalidate(entry);
                // Do not track more misses
                ipdEntryTrackingMisses = nullptr;
                return;
            }
            idx += 1;
        }
    }
}

void
IndirectMemory::checkAccessMatchOnActiveEntries(uint64_t addr)
{
    for (auto &pt_entry : prefetchTable) {
        if (pt_entry.enabled) {
            if (addr == pt_entry.baseAddr +
                       (pt_entry.index << pt_entry.shift)) {
                pt_entry.indirectCounter++;
                pt_entry.increasedIndirectCounter = true;
            }
        }
    }
}


void
IndirectMemory::calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresses)
{
    // This prefetcher requires a PC
    if (loadPC==0) {
        return;
    }

    bool is_secure = true;
    uint64_t pc = loadPC;
    uint64_t addr = lineAddr;
    bool miss = true; //FIXME
	bool isWrite = false; //FIXME
	int getSize = 64;
	
    checkAccessMatchOnActiveEntries(addr);

    // First check if this is a miss, if the prefetcher is tracking misses
    if (ipdEntryTrackingMisses != nullptr && miss) {
        // Check if the entry tracking misses has already set its second index
        if (!ipdEntryTrackingMisses->secondIndexSet) {
            trackMissIndex1(addr);
        } else {
            trackMissIndex2(addr);
        }
    } else {
        // if misses are not being tracked, attempt to detect stream accesses
        PrefetchTableEntry *pt_entry =
            prefetchTable.findEntry(pc, false /* unused */);
        if (pt_entry != nullptr) {
            prefetchTable.accessEntry(pt_entry);

            if (pt_entry->address != addr) {
                // Streaming access found
                pt_entry->streamCounter += 1;
                if (pt_entry->streamCounter >= streamCounterThreshold) {
                    int64_t delta = addr - pt_entry->address;
                    for (unsigned int i = 1; i <= streamingDistance; i += 1) {
						uint64_t newAddr = (addr + delta * i)>>6;
						// assert(proc_id == (newAddr >> (58 - 6)));
						if(proc_id == (newAddr >> (58 - 6)))
							addresses.push_back((newAddr));
						else{
							cout<<"----0"<<endl;
						}
                        // addresses.push_back(newAddr);
                    }
                }
                pt_entry->address = addr;
                pt_entry->secure = is_secure;


                // if this is a read, read the data from the cache and assume
                // it is an index (this is only possible if the data is already
                // in the cache), also, only indexes up to 8 bytes are
                // considered

                if (!miss && !isWrite && getSize <= 8) {
                    int64_t index = 0;
                    bool read_index = true;
                    switch(getSize) {
                        case sizeof(uint8_t):
                            // index = pfi.get<uint8_t>(byteOrder);
                            index =0;
                            break;
                        case sizeof(uint16_t):
                            // index = pfi.get<uint16_t>(byteOrder);
							index =0;
                            break;
                        case sizeof(uint32_t):
                            // index = pfi.get<uint32_t>(byteOrder);
							index =0;
                            break;
                        case sizeof(uint64_t):
                            // index = pfi.get<uint64_t>(byteOrder);
							index =0;
                            break;
                        default:
                            // Ignore non-power-of-two sizes
                            read_index = false;
                    }
                    if (read_index && !pt_entry->enabled) {
                        // Not enabled (no pattern detected in this stream),
                        // add or update an entry in the pattern detector and
                        // start tracking misses
                        allocateOrUpdateIPDEntry(pt_entry, index);
                    } else if (read_index) {
                        // Enabled entry, update the index
                        pt_entry->index = index;
                        if (!pt_entry->increasedIndirectCounter) {
                            pt_entry->indirectCounter--;
                        } else {
                            // Set this to false, to see if the new index
                            // has any match
                            pt_entry->increasedIndirectCounter = false;
                        }

                        // If the counter is high enough, start prefetching
                        if (pt_entry->indirectCounter > prefetchThreshold) {
                            unsigned distance = maxPrefetchDistance *
                                pt_entry->indirectCounter.calcSaturation();
                            for (unsigned int delta = 1; delta < distance; delta += 1) {
                                uint64_t pf_addr = pt_entry->baseAddr +
                                    (pt_entry->index << pt_entry->shift);
								uint64_t newAddr1 = (pf_addr)>>6;
								// assert(proc_id == (newAddr1 >> (58 - 6)));
								if(proc_id == (newAddr1 >> (58 - 6)))
									addresses.push_back((newAddr1));
								else{
									cout<<"----1"<<endl;
								}
                            }
                        }
                    }
                }
            }
        } else {
            pt_entry = prefetchTable.findVictim(pc);
            assert(pt_entry != nullptr);
            prefetchTable.insertEntry(pc, false /* unused */, pt_entry);
            pt_entry->address = addr;
            pt_entry->secure = is_secure;
        }
    }

	
}

} // namespace Prefetcher