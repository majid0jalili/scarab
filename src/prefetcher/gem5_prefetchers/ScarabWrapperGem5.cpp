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

#include "tagged.hh"

using namespace Gem5Prefetchers;
using namespace Prefetcher;
using namespace std;




ScarabWrapperGem5::ScarabWrapperGem5( void (*stats_callback)(int,int)) {
  cout<<"ScarabWrapperGem5"<<endl;
  Tagged * tagged =  new Tagged();
  vector<uint64_t> address;
  address.push_back(100);
  address.push_back(100);
  tagged->calculatePrefetch(address);
  
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
	
}

void 
ScarabWrapperGem5::init()
{
	
}



int  
ScarabWrapperGem5::train_miss(uint64_t a)
{
	return 0;
}


void 
ScarabWrapperGem5::train_hit()
{
	
}


int 
ScarabWrapperGem5::info()
{
	return 0;	
}
	
