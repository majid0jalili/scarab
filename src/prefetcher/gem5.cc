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


#include "debug/debug_print.h"
#include "globals/global_defs.h"
#include "globals/global_types.h"
#include "globals/global_vars.h"


#include "globals/utils.h"
#include "op.h"

#include "core.param.h"
#include "dcache_stage.h"
#include "debug/debug.param.h"

#include "libs/cache_lib.h"
#include "libs/hash_lib.h"
#include "libs/list_lib.h"

#include "prefetcher/l2l1pref.h"
#include "prefetcher/pref.param.h"
#include "prefetcher/pref_common.h"
#include "statistics.h"


}

/**************************************************************************************/
/* Macros */

#define DEBUG(proc_id, args...) _DEBUG(proc_id, DEBUG_GEM5, ##args)

/**************************************************************************************/


using namespace Gem5Prefetchers;

ScarabWrapperGem5* wrapper_gem5 = NULL;



void pref_gem5_init(HWP* hwp);
void pref_gem5_ul1_train(uns8 proc_id, Addr lineAddr, Addr loadPC, Flag ul1_hit);
void pref_gem5_ul1_miss(uns8 proc_id, Addr lineAddr, Addr loadPC,
                       uns32 global_hist);
void pref_gem5_ul1_prefhit(uns8 proc_id, Addr lineAddr, Addr loadPC,
                          uns32 global_hist);
						  

void stats_callback_gem5(int coreid, int type);
void pref_gem5_per_core_done(uns proc_id);


void pref_gem5_init(HWP* hwp) {
  
  printf("******pref_gem5_init \n");
  if(!PREF_GEM5_ON)
    return;
  wrapper_gem5 = new ScarabWrapperGem5(&stats_callback_gem5);
  hwp->hwp_info->enabled = TRUE;
  printf("******pref_gem5_init ScarabWrapperGem5 created \n");

}

void stats_callback_gem5(int coreid, int type) {
  printf("******stats_callback_gem5 \n");
}

void pref_gem5_ul1_train(uns8 proc_id, Addr lineAddr, Addr loadPC, Flag ul1_hit)
{
	printf("******pref_gem5_ul1_train \n");
}

void pref_gem5_ul1_miss(uns8 proc_id, Addr lineAddr, Addr loadPC,
                       uns32 global_hist)
{
	printf("******pref_gem5_ul1_miss \n");
}

void pref_gem5_ul1_prefhit(uns8 proc_id, Addr lineAddr, Addr loadPC,
                          uns32 global_hist)
{
	printf("******pref_gem5_ul1_prefhit \n");
}

void pref_gem5_per_core_done(uns proc_id)
{
  printf("******pref_stream_per_core_done \n");
  wrapper_gem5->finish();

  delete wrapper_gem5;
}