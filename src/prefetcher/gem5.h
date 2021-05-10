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
 * File         : gem5.h
 * Author       : SAFARI research group
 * Date         : 6/12/2018
 * Description  : Header file defining an interface to gem5's prefetcher
 ***************************************************************************************/

#ifndef __GEM5_H__
#define __GEM5_H__

#include "globals/global_types.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/* HWP Interface */
HWP* hwp_gem5;
EXTERNC void pref_gem5_init(HWP* hwp);
EXTERNC void pref_gem5_ul1_train(uns8 proc_id, Addr lineAddr, Addr loadPC, Flag ul1_hit);
EXTERNC void pref_gem5_ul1_miss(uns8 proc_id, Addr lineAddr, Addr loadPC,
                       uns32 global_hist);
EXTERNC void pref_gem5_ul1_prefhit(uns8 proc_id, Addr lineAddr, Addr loadPC,
                          uns32 global_hist);
						  
EXTERNC void pref_gem5_per_core_done(uns proc_id);				  

// EXTERNC void gem5_init();
// EXTERNC void gem5_finish();

// EXTERNC int  gem5_train_miss(Addr a);
// EXTERNC void gem5_train_hit();

// EXTERNC int gem5_info();

#undef EXTERNC

#endif  // __RAMULATOR_H__
