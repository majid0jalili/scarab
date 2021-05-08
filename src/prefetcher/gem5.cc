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

/***************************************************************************************
 * File         : ramulator.cc
 * Author       : SAFARI research group
 * Date         : 6/12/2018
 * Description  : Defines an interface to Ramulator
 ***************************************************************************************/

#include <deque>
#include <list>
#include <map>



#include "gem5_prefetchers/ScarabWrapperGem5.h"

extern "C" {
#include "general.param.h"
#include "globals/assert.h"
#include "memory/memory.h"
#include "memory/memory.param.h"
#include "gem5.h"
#include "gem5.param.h"
#include "statistics.h"
}

/**************************************************************************************/
/* Macros */

#define DEBUG(proc_id, args...) _DEBUG(proc_id, DEBUG_GEM5, ##args)

/**************************************************************************************/


using namespace Gem5Prefetchers;

ScarabWrapperGem5* wrapper_gem5 = NULL;


void gem5_init();
void gem5_finish();

int  gem5_train_miss(Addr a);
void gem5_train_hiy();

int gem5_info();

void stats_callback_gem5(int coreid, int type);



void gem5_init() {
  

  wrapper_gem5 = new ScarabWrapperGem5(&stats_callback_gem5);

  DPRINTF("Initialized Ramulator. \n");
}

void gem5_finish() {
  wrapper_gem5->finish();

  delete wrapper_gem5;
  
}

void stats_callback_gem5(int coreid, int type) {
  
}

int  gem5_train_miss(Addr a)
{
	return 0;
}

void gem5_train_hit(){
	
}

