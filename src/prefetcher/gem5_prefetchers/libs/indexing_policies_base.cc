/*
 * Copyright (c) 2018 Inria
 * Copyright (c) 2012-2014,2017 ARM Limited
 * All rights reserved.
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
 * Copyright (c) 2003-2005,2014 The Regents of The University of Michigan
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
 * Definitions of a common framework for indexing policies.
 */

#include "indexing_policies_base.hh"
#include <cstdlib>
#include <math.h>
#include "replaceable_entry.hh"

namespace IndexingPolicyPolicy {
BaseIndexingPolicy::BaseIndexingPolicy(uint32_t size, unsigned assocs, unsigned entry_size)
      
{
	assoc = assocs;
	numSets = size / (entry_size * assocs);
	setShift = (int)log2(entry_size);
	setMask =(numSets - 1);
	sets.resize(numSets);
	tagShift = (setShift + (int)log2(numSets)); 
    // Make space for the entries
    for (uint32_t i = 0; i < numSets; ++i) {
        sets[i].resize(assoc);
    }
}

ReplaceableEntry*
BaseIndexingPolicy::getEntry(const uint32_t set, const uint32_t way) const
{
    return sets[set][way];
}

void
BaseIndexingPolicy::setEntry(ReplaceableEntry* entry, const uint64_t index)
{
    // Calculate set and way from entry index
    const std::lldiv_t div_result = std::div((long long)index, assoc);
    const uint32_t set = div_result.quot;
    const uint32_t way = div_result.rem;

    // Sanity check
    assert(set < numSets);

    // Assign a free pointer
    sets[set][way] = entry;

    // Inform the entry its position
    entry->setPosition(set, way);
}

uint64_t
BaseIndexingPolicy::extractTag(const uint64_t addr)
{
    return (addr >> tagShift);
}

uint64_t
BaseIndexingPolicy::regenerateAddr(uint64_t tag, ReplaceableEntry* entry)
                                                                        const 
{
    return 0;
}

std::vector<ReplaceableEntry*>
BaseIndexingPolicy::getPossibleEntries(const uint64_t addr) const
{
	cout<<"BaseIndexingPolicy::getPossibleEntries"<<endl;
	std::vector<ReplaceableEntry*> ss;
    return ss;
}

}