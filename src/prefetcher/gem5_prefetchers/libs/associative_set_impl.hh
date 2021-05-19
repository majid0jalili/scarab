/**
 * Copyright (c) 2018 Metempsy Technology Consulting
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

#ifndef __CACHE_PREFETCH_ASSOCIATIVE_SET_IMPL_HH__
#define __CACHE_PREFETCH_ASSOCIATIVE_SET_IMPL_HH__

#include <iostream>
using namespace std;


#include "associative_set.hh"

// table(128, 128, new SetAssociative(128, 128, 1), new LFU(), DCPTEntry(20)


template<class Entry>
AssociativeSet<Entry>::AssociativeSet(int assoc, int num_entries,
        BaseIndexingPolicy *idx_policy, ReplacementPolicy::Base *rpl_policy,
        Entry const &init_value)
  : associativity(assoc), numEntries(num_entries), indexingPolicy(idx_policy),
    replacementPolicy(rpl_policy), entries(numEntries, init_value)
{

	cout<<"assoc "<<assoc<<endl;
	cout<<"num_entries "<<num_entries<<endl;
	cout<<"assoc "<<assoc<<endl;
	cout<<"assoc "<<assoc<<endl;
	
	
    for (unsigned int entry_idx = 0; entry_idx < numEntries; entry_idx += 1) {
		// cout<<"entry_idx "<<entry_idx<<endl;
        Entry* entry = &entries[entry_idx];
        indexingPolicy->setEntry(entry, entry_idx);
        entry->replacementData = replacementPolicy->instantiateEntry();
    }
}

template<class Entry>
Entry*
AssociativeSet<Entry>::findEntry(uint64_t addr, bool is_secure) const
{

    uint64_t tag = indexingPolicy->extractTag(addr);
	// cout<<"findEntry -- add "<<addr<<" tag "<<tag<<endl;
    const std::vector<ReplaceableEntry*> selected_entries =
        indexingPolicy->getPossibleEntries(addr);

    for (const auto& location : selected_entries) {
        Entry* entry = static_cast<Entry *>(location);
        if ( (entry) && (entry->getTag() == tag) && entry->isValid() &&
            entry->isSecure() == is_secure) {
			// cout<<"findEntry hit"<<endl;
            return entry;
        }
    }
	// cout<<"findEntry 5"<<endl;
    return nullptr;
}

template<class Entry>
void
AssociativeSet<Entry>::accessEntry(Entry *entry)
{
    replacementPolicy->touch(entry->replacementData);
}

template<class Entry>
Entry*
AssociativeSet<Entry>::findVictim(uint64_t addr)
{
    // Get possible entries to be victimized
	// cout<<"findVictim 1"<<endl;
    const std::vector<ReplaceableEntry*> selected_entries =
        indexingPolicy->getPossibleEntries(addr);
	// cout<<"findVictim 2"<<endl;
    Entry* victim = static_cast<Entry*>(replacementPolicy->getVictim(
                            selected_entries));
	// cout<<"findVictim 3"<<endl;
    // There is only one eviction for this replacement
    // if(victim)
	invalidate(victim);
	
	// cout<<"findVictim 4"<<endl;
    return victim;
}


template<class Entry>
std::vector<Entry *>
AssociativeSet<Entry>::getPossibleEntries(const uint64_t addr) const
{
    std::vector<ReplaceableEntry *> selected_entries =
        indexingPolicy->getPossibleEntries(addr);
    std::vector<Entry *> entries(selected_entries.size(), nullptr);

    unsigned int idx = 0;
    for (auto &entry : selected_entries) {
        entries[idx++] = static_cast<Entry *>(entry);
    }
    return entries;
}

template<class Entry>
void
AssociativeSet<Entry>::insertEntry(uint64_t addr, bool is_secure, Entry* entry)
{
   entry->insert(indexingPolicy->extractTag(addr), is_secure);
   replacementPolicy->reset(entry->replacementData);
}

template<class Entry>
void
AssociativeSet<Entry>::invalidate(Entry* entry)
{
    entry->invalidate();
    replacementPolicy->invalidate(entry->replacementData);
}

#endif//__CACHE_PREFETCH_ASSOCIATIVE_SET_IMPL_HH__