/**
 * Copyright (c) 2018-2020 Inria
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

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_BASE_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_BASE_HH__

#include <memory>
#include <vector>

#include "replaceable_entry.hh"


/**
 * Replacement candidates as chosen by the indexing policy.
 */
typedef std::vector<ReplaceableEntry*> ReplacementCandidates;

namespace ReplacementPolicy {

/**
 * A common base class of cache replacement policy objects.
 */
class Base
{
  public:

    Base(){}
    virtual ~Base() = default;

    /**
     * Invalidate replacement data to set it as the next probable victim.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    virtual void invalidate(const std::shared_ptr<ReplacementData>&
                                                replacement_data) const = 0;

    /**
     * Update replacement data.
     *
     * @param replacement_data Replacement data to be touched.
     */
    virtual void touch(const std::shared_ptr<ReplacementData>&
                                                replacement_data) const = 0;

    /**
     * Reset replacement data. Used when it's holder is inserted/validated.
     *
     * @param replacement_data Replacement data to be reset.
     */
    virtual void reset(const std::shared_ptr<ReplacementData>&
                                                replacement_data) const = 0;

    /**
     * Find replacement victim among candidates.
     *
     * @param candidates Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    virtual ReplaceableEntry* getVictim(
                           const ReplacementCandidates& candidates) const = 0;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    virtual std::shared_ptr<ReplacementData> instantiateEntry() = 0;
};

} // namespace ReplacementPolicy

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_BASE_HH__
