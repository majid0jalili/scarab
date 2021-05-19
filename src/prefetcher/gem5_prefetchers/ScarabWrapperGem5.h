/* Copyright 2020 HPS/SAFARI Research Groups
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ScarabWrapperGem5_H
#define __ScarabWrapperGem5_H

#include <string>
#include <vector>
#include "tagged.hh"
#include "bop.hh"
#include "delta_correlating_prediction_tables.hh"
#include "indirect_memory.hh"
#include "irregular_stream_buffer.hh"
#include "access_map_pattern_matching.hh"
#include "stride.hh"

using namespace std;
using namespace Prefetcher;

namespace Gem5Prefetchers
{



class ScarabWrapperGem5 
{
private:
	Tagged * tagged;
	BOP * bop;
	DCPT * dcpt;
	IndirectMemory * imp;
	IrregularStreamBuffer * isb;
	AMPM * ampm;
	Stride * stride;
public:
    //double tCK;
    ScarabWrapperGem5(void (* stats_callback)(int, int));
    ~ScarabWrapperGem5();
    void tick();
    bool send();
    void finish(void);

    void init();


	vector<uint64_t> train_miss_L1(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist);
	vector<uint64_t> train_hit_L1(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist);
					   
	vector<uint64_t> train_miss_L2(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist);
	vector<uint64_t> train_hit_L2(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist);
					   
    vector<uint64_t> train_miss_L3(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist);
	vector<uint64_t> train_hit_L3(uint8_t proc_id, uint64_t lineAddr, uint64_t loadPC,
                       uint32_t global_hist);

	int info();

};

} /*namespace ramulator*/


 
#endif /*__SCARAB_WRAPPER_H*/

