

#include <iostream>
#include <assert.h>

#include "delta_correlating_prediction_tables.hh"
#include "./libs/associative_set_impl.hh"
#include "./libs/set_associative.hh"
#include "./libs/lfu_rp.hh"


using namespace std;
using namespace IndexingPolicyPolicy;
using namespace ReplacementPolicy;




namespace Prefetcher {
	// SetAssociative(uint32_t size, unsigned assocs, unsigned entry_size);

DeltaCorrelatingPredictionTables::DeltaCorrelatingPredictionTables():table(256, 256, new SetAssociative(256, 256, 1), new LFU(), DCPTEntry(20))
{
	cout<<"Creating Table"<<endl;
	deltaBits = 12;
	deltaMaskBits = 8;
	
	// table= new AssociativeSet<DCPTEntry>(4, 64);
	// cout<<"Creating Table Done!"<<endl;
}

void
DeltaCorrelatingPredictionTables::DCPTEntry::invalidate()
{
    TaggedEntry::invalidate();

    deltas.flush();
    while (!deltas.full()) {
        deltas.push_back(0);
    }
    lastAddress = 0;
}

void
DeltaCorrelatingPredictionTables::DCPTEntry::addAddress(uint64_t address,
    unsigned int delta_bits)
{
    if ((address - lastAddress) != 0) {
        uint64_t delta = address - lastAddress;
        // Account for the sign bit
        uint64_t max_positive_delta = (1 << (delta_bits-1)) - 1;
        if (address > lastAddress) {
            // check positive delta overflow
            if (delta > max_positive_delta) {
                delta = 0;
            }
        } else {
            // check negative delta overflow
            if (lastAddress - address > (max_positive_delta + 1)) {
                delta = 0;
            }
        }
		// cout<<"deltas.size() "<<deltas.size()<<endl;
        deltas.push_back(delta);
        lastAddress = address;
    }
}

void
DeltaCorrelatingPredictionTables::DCPTEntry::getCandidates(
    std::vector<uint64_t> &pfs, unsigned int mask) const
{
    assert(deltas.full());

    // Get the two most recent deltas
    const int delta_penultimate = *(deltas.end() - 2);
    const int delta_last = *(deltas.end() - 1);

    // a delta 0 means that it overflowed, we can not match it
    if (delta_last == 0 || delta_penultimate == 0) {
        return;
    }

    // Try to find the two most recent deltas in a previous position on the
    // delta circular array, if found, start issuing prefetches using the
    // remaining deltas (adding each delta to the last uint64_t to generate the
    // prefetched address.
    auto it = deltas.begin();
    for (; it != (deltas.end() - 2); ++it) {
        const int prev_delta_penultimate = *it;
        const int prev_delta_last = *(it + 1);
        if ((prev_delta_penultimate >> mask) == (delta_penultimate >> mask) &&
            (prev_delta_last >> mask) == (delta_last >> mask)) {
            // Pattern found. Skip the matching pair and issue prefetches with
            // the remaining deltas
            it += 2;
            uint64_t addr = lastAddress;
            while (it != deltas.end()) {
                const int pf_delta = *(it++);
				// cout<<"addr "<<addr<<" pf_delta "<<pf_delta<<" deltas.size() "<<deltas.size()<<endl;
				// getchar();
                addr += pf_delta;
				
                pfs.push_back(addr);
            }
            break;
        }
    }
}

void
DeltaCorrelatingPredictionTables::calculatePrefetch(
		uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
		uint32_t global_hist, std::vector<uint64_t> &addresses)
{
    if (loadPC==0) {
        return;
    }
	// cout<<"1) DeltaCorrelatingPredictionTables "<<loadPC<<endl;
    uint64_t address = lineAddr;
    uint64_t pc = loadPC;
    // Look up table entry, is_secure is unused in findEntry because we
    // index using the pc
    DCPTEntry *entry = table.findEntry(pc, false /* unused */);
	// cout<<"2"<<endl;
    if (entry != nullptr) {
        entry->addAddress(address, deltaBits);
        //Delta correlating
        entry->getCandidates(addresses, deltaMaskBits);
    } else {
		// cout<<"6"<<endl;
        entry = table.findVictim(pc);
	       table.insertEntry(pc, false /* unused */, entry);
	    entry->lastAddress = address;
		// cout<<"9"<<endl;
    }
	
}

DCPT::DCPT()
{
	cout<<"DCPT::DCPT"<<endl;
	degree= 2;
	dcpt = new DeltaCorrelatingPredictionTables();
	cout<<"DCPT::DCPT Done"<<endl;
}

void
DCPT::calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresses)
{
    // cout<<"****DCPT::calculatePrefetch 1"<<endl;
	dcpt->calculatePrefetch(proc_id, lineAddr, loadPC, global_hist, addresses);
	for (unsigned int d = 0; d < addresses.size(); d++) {
        addresses[d] = (addresses[d]>>6);
    }
}

} // namespace Prefetcher