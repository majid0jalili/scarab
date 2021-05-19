/*
 * Copyright (c) 2018 Inria
 * Copyright (c) 2012-2013, 2015 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
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
 * Describes a strided prefetcher.
 */

#ifndef __MEM_CACHE_PREFETCH_STRIDE_HH__
#define __MEM_CACHE_PREFETCH_STRIDE_HH__

#include <string>
#include <unordered_map>
#include <vector>


#include <vector>
#include "./libs/circular_queue.hh"
#include "./libs/tagged_entry.hh"
#include "./libs/associative_set.hh"
#include <cstdint> // include this header for uint64_t
#include "./libs/sat_counter.hh"



namespace Prefetcher {


class Stride
{
  protected:
    /** Initial confidence counter value for the pc tables. */
    SatCounter8 initConfidence;

    /** Confidence threshold for prefetch generation. */
    double threshConf;

    bool useRequestorId;

    int degree;

    /**
     * Information used to create a new PC table. All of them behave equally.
     */
    const struct PCTableInfo
    {
        const int assoc;
        const int numEntries;

        BaseIndexingPolicy* const indexingPolicy;
        ReplacementPolicy::Base* const replacementPolicy;

        PCTableInfo(int assoc, int num_entries,
            BaseIndexingPolicy* indexing_policy,
            ReplacementPolicy::Base* replacement_policy)
          : assoc(assoc), numEntries(num_entries),
            indexingPolicy(indexing_policy),
            replacementPolicy(replacement_policy)
        {
        }
    } pcTableInfo;

    /** Tagged by hashed PCs. */
    struct StrideEntry : public TaggedEntry
    {
        StrideEntry(const SatCounter8& init_confidence);

        void invalidate() override;

        uint64_t lastAddr;
        int stride;
        SatCounter8 confidence;
    };
    typedef AssociativeSet<StrideEntry> PCTable;
    std::unordered_map<int, PCTable> pcTables;

    /**
     * Try to find a table of entries for the given context. If none is
     * found, a new table is created.
     *
     * @param context The context to be searched for.
     * @return The table corresponding to the given context.
     */
    PCTable* findTable(int context);

    /**
     * Create a PC table for the given context.
     *
     * @param context The context of the new PC table.
     * @return The new PC table
     */
    PCTable* allocateNewContext(int context);

  public:
    Stride();

    void calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresses);
};

} // namespace Prefetcher

#endif // __MEM_CACHE_PREFETCH_STRIDE_HH__