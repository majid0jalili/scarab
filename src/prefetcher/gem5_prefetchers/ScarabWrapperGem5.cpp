/* Copyright 2020 HPS/SAFARI Research Groups
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <map>
#include <iostream>
#include "ScarabWrapperGem5.h"



using namespace Gem5Prefetchers;
using namespace Prefetcher;
using namespace std;




ScarabWrapperGem5::ScarabWrapperGem5( void (*stats_callback)(int,int)) {
  cout<<"****ScarabWrapperGem5"<<endl;
  tagged	=  new Tagged();
  bop 		=  new BOP();
  dcpt 		=  new DCPT();
  imp 		=  new IndirectMemory();
  // vector<uint64_t> address;
  // tagged->calculatePrefetch(address);
  // for(unsigned int i = 0 ; i < address.size(); i++){
	  // cout<<"address["<<i<<"] "<<address[i]<<endl;
  // }
  
}


ScarabWrapperGem5::~ScarabWrapperGem5() {
  
}


void 
ScarabWrapperGem5::tick()
{
	
}

bool 
ScarabWrapperGem5::send()
{
	return false;
}
    
void 
ScarabWrapperGem5::finish(void)
{
	printf("ScarabWrapperGem5::finish\n");
}

void 
ScarabWrapperGem5::init()
{
	
}



vector<uint64_t>  
ScarabWrapperGem5::train_miss(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist)
{
	// cout<<"train_miss proc_id:"<<hex<<proc_id<<" lineAddr:"<<lineAddr<<" loadPC:"<<loadPC<<" global_hist:"<<global_hist<<endl;
	vector<uint64_t> address;
	imp->calculatePrefetch(proc_id, lineAddr, loadPC, global_hist, address);
	
	return address;
}


vector<uint64_t> 
ScarabWrapperGem5::train_hit(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist)
{
	// cout<<"train_hit proc_id:"<<hex<<proc_id<<" lineAddr:"<<lineAddr<<" loadPC:"<<loadPC<<" global_hist:"<<global_hist<<endl;
	vector<uint64_t> address;
	imp->calculatePrefetch(proc_id, lineAddr, loadPC, global_hist, address);
	
	return address;
	
}


int 
ScarabWrapperGem5::info()
{
	return 0;	
}
	
