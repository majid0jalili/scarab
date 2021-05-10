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

#ifndef __MEM_CACHE_PREFETCH_DELTA_CORRELATING_PREDICTION_TABLES_HH__
#define __MEM_CACHE_PREFETCH_DELTA_CORRELATING_PREDICTION_TABLES_HH__

#include <vector>
#include "./libs/circular_queue.hh"
#include "./libs/tagged_entry.hh"
#include "./libs/associative_set.hh"
#include <cstdint> // include this header for uint64_t





using namespace ReplacementPolicy;

namespace Prefetcher {
class DeltaCorrelatingPredictionTables
{
    /** Number of bits of each delta */
    unsigned int deltaBits;
    /** Number of lower bits to ignore from the deltas */
    unsigned int deltaMaskBits;

    /** DCPT Table entry datatype */
    struct DCPTEntry : public TaggedEntry
    {
        /** Last accessed address */
        uint64_t lastAddress;
        /** Stored deltas */
        CircularQueue<uint64_t> deltas;

        /**
         * Constructor
         * @param num_deltas number of deltas stored in the entry
         */
        DCPTEntry(unsigned int num_deltas)
          : TaggedEntry(), lastAddress(0), deltas(num_deltas)
        {
        }

        void invalidate();

        /**
         * Adds an address to the entry, if the entry already existed, a delta
         * will be generated
         * @param address address to add
         * @param delta_num_bits number of bits of the delta
         */
        void addAddress(uint64_t address, unsigned int delta_num_bits);

        /**
         * Attempt to generate prefetch candidates using the two most recent
         * deltas. Prefetch candidates are added to the provided vector.
         * @param pfs reference to a vector where candidates will be added
         * @param mask_bits the number of lower bits that should be masked
         *        (ignored) when comparing deltas
         */
        void getCandidates(std::vector<uint64_t> &pfs,
                           unsigned int mask_bits) const;

    };
    /** The main table **/
	// LFU* lfu;
    AssociativeSet<DCPTEntry> table;

  public:
    DeltaCorrelatingPredictionTables();
    ~DeltaCorrelatingPredictionTables() = default;

    /**
     * Computes the prefetch candidates given a prefetch event.
     * @param pfi The prefetch event information
     * @param addresses prefetch candidates generated
     */
    void calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresses);

};



class DCPT 
{
  private:
      int degree;
	  DeltaCorrelatingPredictionTables *dcpt;
  public:

  public:
    DCPT ();
    ~DCPT () = default;

    void calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresses);
};

} // namespace Prefetcher

#endif // __MEM_CACHE_PREFETCH_DELTA_CORRELATING_PREDICTION_TABLES_HH__