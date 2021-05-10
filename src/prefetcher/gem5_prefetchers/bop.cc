#include "bop.hh"
#include <iostream>
#include <assert.h>

#include <math.h>   

using namespace std;


namespace Prefetcher {


BOP::BOP()
{
	cout<<"BOP::BOP"<<endl;
	degree= 2;
	scoreMax=31;
	roundMax=100;
	badScore=10;
	rrEntries=64;
	tagMask=((1 << 12) - 1);
	delayQueueEnabled=true;
	delayQueueSize=15;
	delayTicks=60; //ticks
	rrLeft.resize(rrEntries);
    rrRight.resize(rrEntries);
	unsigned int offset_list_size  = 46;
	
	int factors[] = { 2, 3, 5 };
    unsigned int i = 0;
    int64_t offset_i = 1;
	bool negative_offsets_enable  = true; 
	
    while (i < offset_list_size)
    {
        int64_t offset = offset_i;

        for (int n : factors) {
            while ((offset % n) == 0) {
                offset /= n;
            }
        }

        if (offset == 1) {
            offsetsList.push_back(OffsetListEntry(offset_i, 0));
            i++;
            // If we want to use negative offsets, add also the negative value
            // of the offset just calculated
            if (negative_offsets_enable)  {
                offsetsList.push_back(OffsetListEntry(-offset_i, 0));
                i++;
            }
        }

        offset_i++;
    }

    offsetsListIterator = offsetsList.begin();
	
}

uint64_t
BOP::curTick(){
	return 0;
}
void
BOP::delayQueueEventWrapper()
{
    while (!delayQueue.empty() &&
            delayQueue.front().processTick <= curTick())
    {
        uint64_t addr_x = delayQueue.front().baseAddr;
        insertIntoRR(addr_x, RRWay::Left);
        delayQueue.pop_front();
    }

   
}

unsigned int
BOP::hash(uint64_t addr, unsigned int way) const
{
    uint64_t hash1 = addr >> way;
    uint64_t hash2 = hash1 >>(int)(log2(rrEntries));
    return (hash1 ^ hash2) & (uint64_t)(rrEntries - 1);
}

void
BOP::insertIntoRR(uint64_t addr, unsigned int way)
{
    switch (way) {
        case RRWay::Left:
            rrLeft[hash(addr, RRWay::Left)] = addr;
            break;
        case RRWay::Right:
            rrRight[hash(addr, RRWay::Right)] = addr;
            break;
    }
}

void
BOP::insertIntoDelayQueue(uint64_t x)
{
    if (delayQueue.size() == delayQueueSize) {
        return;
    }

    // Add the address to the delay queue and schedule an event to process
    // it after the specified delay cycles
    uint64_t process_tick = curTick() + delayTicks;

    delayQueue.push_back(DelayQueueEntry(x, process_tick));

    // if (!delayQueueEvent.scheduled()) {
        // schedule(delayQueueEvent, process_tick);
    // }
}

void
BOP::resetScores()
{
    for (auto& it : offsetsList) {
        it.second = 0;
    }
}

inline uint64_t
BOP::tag(uint64_t addr) const
{
    return (addr >> 6) & tagMask;
}


bool
BOP::testRR(uint64_t addr) const
{
    for (auto& it : rrLeft) {
        if (it == addr) {
            return true;
        }
    }

    for (auto& it : rrRight) {
        if (it == addr) {
            return true;
        }
    }

    return false;
}

void
BOP::bestOffsetLearning(uint64_t x)
{
    uint64_t offset_addr = (*offsetsListIterator).first;
    uint64_t lookup_addr = x - offset_addr;

    // There was a hit in the RR table, increment the score for this offset
    if (testRR(lookup_addr)) {
        (*offsetsListIterator).second++;
        if ((*offsetsListIterator).second > bestScore) {
            bestScore = (*offsetsListIterator).second;
            phaseBestOffset = (*offsetsListIterator).first;
        }
    }

    offsetsListIterator++;

    // All the offsets in the list were visited meaning that a learning
    // phase finished. Check if
    if (offsetsListIterator == offsetsList.end()) {
        offsetsListIterator = offsetsList.begin();
        round++;

        // Check if the best offset must be updated if:
        // (1) One of the scores equals SCORE_MAX
        // (2) The number of rounds equals ROUND_MAX
        if ((bestScore >= scoreMax) || (round == roundMax)) {
            bestOffset = phaseBestOffset;
            round = 0;
            bestScore = 0;
            phaseBestOffset = 0;
            resetScores();
            issuePrefetchRequests = true;
        } else if (phaseBestOffset <= badScore) {
            issuePrefetchRequests = false;
        }
    }
}


void
BOP::calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresses)
{
    uint64_t addr =lineAddr;
    uint64_t tag_x = tag(addr);

    if (delayQueueEnabled) {
        insertIntoDelayQueue(tag_x);
    } else {
        insertIntoRR(tag_x, RRWay::Left);
    }

    // Go through the nth offset and update the score, the best score and the
    // current best offset if a better one is found
    bestOffsetLearning(tag_x);

    // This prefetcher is a degree 1 prefetch, so it will only generate one
    // prefetch at most per access
    if (issuePrefetchRequests) {
        uint64_t prefetch_addr = (addr + (bestOffset << 6))>>6;
		assert(proc_id == (prefetch_addr >> (58 - 6)));
        addresses.push_back(prefetch_addr);
    }
}



} // namespace Prefetcher