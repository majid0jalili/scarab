#ifndef __MEM_CACHE_PREFETCH_IRREGULAR_STREAM_BUFFER_HH__
#define __MEM_CACHE_PREFETCH_IRREGULAR_STREAM_BUFFER_HH__






#include <vector>
#include "./libs/circular_queue.hh"
#include "./libs/tagged_entry.hh"
#include "./libs/associative_set.hh"
#include <cstdint> // include this header for uint64_t
#include "./libs/sat_counter.hh"

struct IrregularStreamBufferPrefetcherParams;

namespace Prefetcher {

class IrregularStreamBuffer
{
    /** Size in bytes of a temporal stream */
    uint64_t chunkSize;
    /** Number of prefetch candidates per Physical-to-Structural entry */
    unsigned prefetchCandidatesPerEntry;
    /** Number of maximum prefetches requests created when predicting */
    unsigned degree;

    /**
     * Training Unit Entry datatype, it holds the last accessed address and
     * its secure flag
     */
    struct TrainingUnitEntry : public TaggedEntry {
        uint64_t lastAddress;
        bool lastAddressSecure;
    };
    /** Map of PCs to Training unit entries */
    AssociativeSet<TrainingUnitEntry> trainingUnit;

    /** Address Mapping entry, holds an address and a confidence counter */
    struct AddressMapping {
        uint64_t address;
        SatCounter8 counter;
        AddressMapping(unsigned bits) : address(0), counter(bits)
        {}
    };

    /**
     * Maps a set of contiguous addresses to another set of (not necessarily
     * contiguos) addresses, with their corresponding confidence counters
     */
    struct AddressMappingEntry : public TaggedEntry
    {
        std::vector<AddressMapping> mappings;
        AddressMappingEntry(size_t num_mappings, unsigned counter_bits)
          : TaggedEntry(), mappings(num_mappings, counter_bits)
        {
        }

        void
        invalidate() override
        {
            TaggedEntry::invalidate();
            for (auto &entry : mappings) {
                entry.address = 0;
                entry.counter.reset();
            }
        }
    };

    /** Physical-to-Structured mappings table */
    AssociativeSet<AddressMappingEntry> psAddressMappingCache;
    /** Structured-to-Physical mappings table */
    AssociativeSet<AddressMappingEntry> spAddressMappingCache;
    /**
     * Counter of allocated structural addresses, increased by "chunkSize",
     * each time a new structured address is allocated
     */
    uint64_t structuralAddressCounter;

    /**
     * Add a mapping to the Structured-to-Physica mapping table
     * @param structuralAddress structural address
     * @param is_secure whether this page is inside the secure memory area
     * @param physical_address corresponding physical address
     */
    void addStructuralToPhysicalEntry(uint64_t structuralAddress, bool is_secure,
                                      uint64_t physical_address);

    /**
     * Obtain the Physical-to-Structured mapping entry of the given physical
     * address. If the entry does not exist a new one is allocated, replacing
     * an existing one if needed.
     * @param paddr physical address
     * @param is_secure whether this page is inside the secure memory area
     * @result reference to the entry
     */
    AddressMapping& getPSMapping(uint64_t paddr, bool is_secure);
  public:
    IrregularStreamBuffer();
    ~IrregularStreamBuffer() = default;

    void calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresse);
};

} // namespace Prefetcher

#endif//__MEM_CACHE_PREFETCH_IRREGULAR_STREAM_BUFFER_HH__