#include "tagged.hh"
#include <iostream>
#include <assert.h>



using namespace std;


namespace Prefetcher {

Tagged::Tagged()
{
	cout<<"Tagged::Tagged"<<endl;
	degree= 2;
}

void
Tagged::calculatePrefetch(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist, std::vector<uint64_t> &addresses)
{
    uint64_t blkAddr = lineAddr >>6;

    for (int d = 1; d <= degree; d++) {
        uint64_t newAddr = blkAddr + d;
		assert(proc_id == (newAddr >> (58 - 6)));
        addresses.push_back(newAddr);
    }
}

} // namespace Prefetcher