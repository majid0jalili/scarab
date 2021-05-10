/*
 * Copyright (c) 2005 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * Describes a tagged prefetcher.
 */

#ifndef __MEM_CACHE_PREFETCH_BOP_HH__
#define __MEM_CACHE_PREFETCH_BOP_HH__

#include <vector>
#include <cstdint> // include this header for uint64_t
#include <queue> // include this header for uint64_t





namespace Prefetcher {


class BOP
{
    private:

        enum RRWay {
            Left,
            Right
        };
		unsigned int degree;
        /** Learning phase parameters */
        unsigned int scoreMax;
        unsigned int roundMax;
        unsigned int badScore;
        /** Recent requests table parameteres */
        unsigned int rrEntries;
        unsigned int tagMask;
        /** Delay queue parameters */
        bool         delayQueueEnabled;
        unsigned int delayQueueSize;
        unsigned int delayTicks;

        std::vector<uint64_t> rrLeft;
        std::vector<uint64_t> rrRight;

        /** Structure to save the offset and the score */
        typedef std::pair<int16_t, uint8_t> OffsetListEntry;
        std::vector<OffsetListEntry> offsetsList;

        /** In a first implementation of the BO prefetcher, both banks of the
         *  RR were written simultaneously when a prefetched line is inserted
         *  into the cache. Adding the delay queue tries to avoid always
         *  striving for timeless prefetches, which has been found to not
         *  always being optimal.
         */
        struct DelayQueueEntry
        {
            uint64_t baseAddr;
            uint64_t processTick;

            DelayQueueEntry(uint64_t x, uint64_t t) : baseAddr(x), processTick(t)
            {}
        };

        std::deque<DelayQueueEntry> delayQueue;

        /** Event to handle the delay queue processing */
        void delayQueueEventWrapper();
        // EventFunctionWrapper delayQueueEvent;

        /** Hardware prefetcher enabled */
        bool issuePrefetchRequests;
        /** Current best offset to issue prefetches */
        uint64_t bestOffset;
        /** Current best offset found in the learning phase */
        uint64_t phaseBestOffset;
        /** Current test offset index */
        std::vector<OffsetListEntry>::iterator offsetsListIterator;
        /** Max score found so far */
        unsigned int bestScore;
        /** Current round */
        unsigned int round;

        /** Generate a hash for the specified address to index the RR table
         *  @param addr: address to hash
         *  @param way:  RR table to which is addressed (left/right)
         */
        unsigned int hash(uint64_t addr, unsigned int way) const;

        /** Insert the specified address into the RR table
         *  @param addr: address to insert
         *  @param way: RR table to which the address will be inserted
         */
        void insertIntoRR(uint64_t addr, unsigned int way);

        /** Insert the specified address into the delay queue. This will
         *  trigger an event after the delay cycles pass
         *  @param addr: address to insert into the delay queue
         */
        void insertIntoDelayQueue(uint64_t addr);

        /** Reset all the scores from the offset list */
        void resetScores();

        /** Generate the tag for the specified address based on the tag bits
         *  and the block size
         *  @param addr: address to get the tag from
        */
        uint64_t tag(uint64_t addr) const;

        /** Test if @X-O is hitting in the RR table to update the
            offset score */
        bool testRR(uint64_t) const;

        /** Learning phase of the BOP. Update the intermediate values of the
            round and update the best offset if found */
        void bestOffsetLearning(uint64_t);
		uint64_t curTick();

        

    public:

        BOP();
        ~BOP() = default;

       void calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresses);
};

   

} // namespace Prefetcher

#endif // __MEM_CACHE_PREFETCH_BOP_HH__