#include <iostream>
#include <assert.h>


#include "irregular_stream_buffer.hh"
#include "./libs/associative_set_impl.hh"
#include "./libs/set_associative.hh"
#include "./libs/lfu_rp.hh"


using namespace std;
using namespace IndexingPolicyPolicy;
using namespace ReplacementPolicy;


namespace Prefetcher {
IrregularStreamBuffer::IrregularStreamBuffer()
  :
    trainingUnit(128, 128,
				new SetAssociative(128, 128, 1),
                new LFU()),
	psAddressMappingCache(128,
                          128,
                          new SetAssociative(128, 128, 1),
						  new LFU(),
                          AddressMappingEntry(16, 2)),
    spAddressMappingCache(128,
                          128,
                          new SetAssociative(128, 128, 1),
						  new LFU(),
                          AddressMappingEntry(16, 2))
{
    // assert(isPowerOf2(prefetchCandidatesPerEntry));
	chunkSize=256;
	prefetchCandidatesPerEntry=16;
	degree=1;
	structuralAddressCounter=0;
	
}

void
IrregularStreamBuffer::calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresse)
{
    // This prefetcher requires a PC
    if (loadPC==0) {
        return;
    }
    bool is_secure = true;
    uint64_t pc = loadPC;
    uint64_t addr = lineAddr>>6;

    // Training, if the entry exists, then we found a correlation between
    // the entry lastAddress (named as correlated_addr_A) and the address of
    // the current access (named as correlated_addr_B)
    TrainingUnitEntry *entry = trainingUnit.findEntry(pc, is_secure);
    bool correlated_addr_found = false;
    uint64_t correlated_addr_A = 0;
    uint64_t correlated_addr_B = 0;
    if (entry != nullptr && entry->lastAddressSecure == is_secure) {
        trainingUnit.accessEntry(entry);
        correlated_addr_found = true;
        correlated_addr_A = entry->lastAddress;
        correlated_addr_B = addr;
    } else {
        entry = trainingUnit.findVictim(pc);
        assert(entry != nullptr);

        trainingUnit.insertEntry(pc, is_secure, entry);
    }
    // Update the entry
    entry->lastAddress = addr;
    entry->lastAddressSecure = is_secure;

    if (correlated_addr_found) {
        // If a correlation was found, update the Physical-to-Structural
        // table accordingly
        AddressMapping &mapping_A = getPSMapping(correlated_addr_A, is_secure);
        AddressMapping &mapping_B = getPSMapping(correlated_addr_B, is_secure);
        if (mapping_A.counter > 0 && mapping_B.counter > 0) {
            // Entry for A and B
            if (mapping_B.address == (mapping_A.address + 1)) {
                mapping_B.counter++;
            } else {
                if (mapping_B.counter == 1) {
                    // Counter would hit 0, reassign address while keeping
                    // counter at 1
                    mapping_B.address = mapping_A.address + 1;
                    addStructuralToPhysicalEntry(mapping_B.address, is_secure,
                            correlated_addr_B);
                } else {
                    mapping_B.counter--;
                }
            }
        } else {
            if (mapping_A.counter == 0) {
                // if A is not valid, generate a new structural address
                mapping_A.counter++;
                mapping_A.address = structuralAddressCounter;
                structuralAddressCounter += chunkSize;
                addStructuralToPhysicalEntry(mapping_A.address,
                        is_secure, correlated_addr_A);
            }
            mapping_B.counter.reset();
            mapping_B.counter++;
            mapping_B.address = mapping_A.address + 1;
            // update SP-AMC
            addStructuralToPhysicalEntry(mapping_B.address, is_secure,
                    correlated_addr_B);
        }
    }

    // Use the PS mapping to predict future accesses using the current address
    // - Look for the structured address
    // - if it exists, use it to generate prefetches for the subsequent
    //   addresses in ascending order, as many as indicated by the degree
    //   (given the structured address S, prefetch S+1, S+2, .. up to S+degree)
    uint64_t amc_address = addr / prefetchCandidatesPerEntry;
    uint64_t map_index   = addr % prefetchCandidatesPerEntry;
    AddressMappingEntry *ps_am = psAddressMappingCache.findEntry(amc_address,
                                                                 is_secure);
    if (ps_am != nullptr) {
        AddressMapping &mapping = ps_am->mappings[map_index];
        if (mapping.counter > 0) {
            uint64_t sp_address = mapping.address / prefetchCandidatesPerEntry;
            uint64_t sp_index   = mapping.address % prefetchCandidatesPerEntry;
            AddressMappingEntry *sp_am =
                spAddressMappingCache.findEntry(sp_address, is_secure);
            if (sp_am == nullptr) {
                // The entry has been evicted, can not generate prefetches
                return;
            }
            for (unsigned d = 1;
                    d <= degree && (sp_index + d) < prefetchCandidatesPerEntry;
                    d += 1)
            {
                AddressMapping &spm = sp_am->mappings[sp_index + d];
                //generate prefetch
                if (spm.counter > 0) {
                    uint64_t pf_addr = spm.address;
					assert(proc_id == (pf_addr >> (58 - 6)));
					addresse.push_back(pf_addr);
                }
            }
        }
    }
}

IrregularStreamBuffer::AddressMapping&
IrregularStreamBuffer::getPSMapping(uint64_t paddr, bool is_secure)
{
    uint64_t amc_address = paddr / prefetchCandidatesPerEntry;
    uint64_t map_index   = paddr % prefetchCandidatesPerEntry;
    AddressMappingEntry *ps_entry =
        psAddressMappingCache.findEntry(amc_address, is_secure);
    if (ps_entry != nullptr) {
        // A PS-AMC line already exists
        psAddressMappingCache.accessEntry(ps_entry);
    } else {
        ps_entry = psAddressMappingCache.findVictim(amc_address);
        assert(ps_entry != nullptr);

        psAddressMappingCache.insertEntry(amc_address, is_secure, ps_entry);
    }
    return ps_entry->mappings[map_index];
}

void
IrregularStreamBuffer::addStructuralToPhysicalEntry(
    uint64_t structural_address, bool is_secure, uint64_t physical_address)
{
    uint64_t amc_address = structural_address / prefetchCandidatesPerEntry;
    uint64_t map_index   = structural_address % prefetchCandidatesPerEntry;
    AddressMappingEntry *sp_entry =
        spAddressMappingCache.findEntry(amc_address, is_secure);
    if (sp_entry != nullptr) {
        spAddressMappingCache.accessEntry(sp_entry);
    } else {
        sp_entry = spAddressMappingCache.findVictim(amc_address);
        assert(sp_entry != nullptr);

        spAddressMappingCache.insertEntry(amc_address, is_secure, sp_entry);
    }
    AddressMapping &mapping = sp_entry->mappings[map_index];
    mapping.address = physical_address;
    mapping.counter.reset();
    mapping.counter++;
}
	
	
}
	