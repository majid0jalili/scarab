#include "tagged.hh"
#include <iostream>


using namespace std;


namespace Prefetcher {

Tagged::Tagged()
{
	cout<<"Tagged::Tagged"<<endl;
	degree= 2;
}

void
Tagged::calculatePrefetch(std::vector<uint64_t> &addresses)
{
    cout<<"Tagged::calculatePrefetch"<<endl;
}

} // namespace Prefetcher