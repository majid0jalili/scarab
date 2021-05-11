#ifndef __MEM_CACHE_PREFETCH_ACCESS_MAP_PATTERN_MATCHING_HH__
#define __MEM_CACHE_PREFETCH_ACCESS_MAP_PATTERN_MATCHING_HH__


#include <vector>
#include "./libs/circular_queue.hh"
#include "./libs/tagged_entry.hh"
#include "./libs/associative_set.hh"
#include <cstdint> // include this header for uint64_t
#include "./libs/sat_counter.hh"



namespace Prefetcher {
class AccessMapPatternMatching
{
    /** Cacheline size used by the prefetcher using this object */
    unsigned blkSize;
    /** Limit the stride checking to -limitStride/+limitStride */
    unsigned limitStride;
    /** Maximum number of prefetch generated */
    unsigned startDegree;
    /** Amount of memory covered by a hot zone */
    uint64_t hotZoneSize;
    /** A prefetch coverage factor bigger than this is considered high */
    double highCoverageThreshold;
    /** A prefetch coverage factor smaller than this is considered low */
    double lowCoverageThreshold;
    /** A prefetch accuracy factor bigger than this is considered high */
    double highAccuracyThreshold;
    /** A prefetch accuracy factor smaller than this is considered low */
    double lowAccuracyThreshold;
    /** A cache hit ratio bigger than this is considered high */
    double highCacheHitThreshold;
    /** A cache hit ratio smaller than this is considered low */
    double lowCacheHitThreshold;
    /** Cycles in an epoch period */
    uint64_t epochCycles;
    /** Off chip memory latency to use for the epoch bandwidth calculation */
    uint64_t offChipMemoryLatency;

    /** Data type representing the state of a cacheline in the access map */
    enum AccessMapState
    {
        AM_INIT,
        AM_PREFETCH,
        AM_ACCESS,
        AM_INVALID
    };

    /** AccessMapEntry data type */
    struct AccessMapEntry : public TaggedEntry
    {
        /** vector containing the state of the cachelines in this zone */
        std::vector<AccessMapState> states;

        AccessMapEntry(size_t num_entries)
          : TaggedEntry(), states(num_entries, AM_INIT)
        {
        }

        void
        invalidate() override
        {
            TaggedEntry::invalidate();
            for (auto &entry : states) {
                entry = AM_INIT;
            }
        }
    };
    /** Access map table */
    AssociativeSet<AccessMapEntry> accessMapTable;

    /**
     * Number of good prefetches
     * - State transitions from PREFETCH to ACCESS
     */
    uint64_t numGoodPrefetches;
    /**
     * Number of prefetches issued
     * - State transitions from INIT to PREFETCH
     */
    uint64_t numTotalPrefetches;
    /**
     * Number of raw cache misses
     * - State transitions from INIT or PREFETCH to ACCESS
     */
    uint64_t numRawCacheMisses;
    /**
     * Number of raw cache hits
     * - State transitions from ACCESS to ACCESS
     */
    uint64_t numRawCacheHits;
    /** Current degree */
    unsigned degree;
    /** Current useful degree */
    unsigned usefulDegree;
	
	//Majid
	//Added this to replace the tick interval
	uint64_t numCalls;

    /**
     * Given a target cacheline, this function checks if the cachelines
     * that follow the provided stride have been accessed. If so, the line
     * is considered a good candidate.
     * @param states vector containing the states of three contiguous hot zones
     * @param current target block (cacheline)
     * @param stride access stride to obtain the reference cachelines
     * @return true if current is a prefetch candidate
     */
    inline bool checkCandidate(std::vector<AccessMapState> const &states,
                        uint64_t current, int stride) const
    {
        enum AccessMapState tgt   = states[current - stride];
        enum AccessMapState s     = states[current + stride];
        enum AccessMapState s2    = states[current + 2 * stride];
        enum AccessMapState s2_p1 = states[current + 2 * stride + 1];
        return (tgt != AM_INVALID &&
                ((s == AM_ACCESS && s2 == AM_ACCESS) ||
                (s == AM_ACCESS && s2_p1 == AM_ACCESS)));
    }

    /**
     * Obtain an AccessMapEntry  from the AccessMapTable, if the entry is not
     * found a new one is initialized and inserted.
     * @param am_addr address of the hot zone
     * @param is_secure whether the address belongs to the secure memory area
     * @return the corresponding entry
     */
    AccessMapEntry *getAccessMapEntry(uint64_t am_addr, bool is_secure);

    /**
     * Updates the state of a block within an AccessMapEntry, also updates
     * the prefetcher metrics.
     * @param entry AccessMapEntry to update
     * @param block cacheline within the hot zone
     * @param state new state
     */
    void setEntryState(AccessMapEntry &entry, uint64_t block,
        enum AccessMapState state);

    /**
     * This event constitues the epoch of the statistics that keep track of
     * the prefetcher accuracy, when this event triggers, the prefetcher degree
     * is adjusted and the statistics counters are reset.
     */
    void processEpochEvent();


  public:
    AccessMapPatternMatching();
    ~AccessMapPatternMatching() = default;


    void calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresse);
};

class AMPM
{
    AccessMapPatternMatching *ampm;
  public:
    AMPM();
    ~AMPM() = default;

    void calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresse);
};	
}

#endif//__MEM_CACHE_PREFETCH_ACCESS_MAP_PATTERN_MATCHING_HH__