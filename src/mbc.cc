
/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors
may be used to endorse or promote products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

/**********************************************************************************************
 * File         : mbc.cc
 * Author       : HPArch
 * Date         : 11/1/2020
 * Description  : Memory Bounds Check
 *********************************************************************************************/
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>

/* macsim */
#include "all_knobs.h"
#include "assert_macros.h"
#include "core.h"
#include "debug_macros.h"
#include "frontend.h"
#include "memory.h"
#include "mbc.h"
#include "statistics.h"

#include "all_knobs.h"
#include "statistics.h"


using namespace std;

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MMU, ##args)
#define DEBUG_CORE(m_core_id, args...)                        \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) { \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MMU, ##args);      \
  }

mbc_c::mbc_c(int core_id, macsim_c *simBase)
{
  m_core_id = core_id; 
  m_simBase = simBase; 

/* init hash map rbt, cache rbt_l0_cache, cache rbt_l1_cache */ 
  m_l0_cache = new cache_c (
  "rbt_l1_cache", *KNOB(KNOB_BOUNDS_L0_CACHE_ENTRY), *KNOB(KNOB_BOUNDS_L0_CACHE_ENTRY),
    *KNOB(KNOB_LLC_LINE_SIZE), sizeof(dcache_data_s), *KNOB(KNOB_BOUNDS_L0_CACHE_ENTRY),
    false, core_id, CACHE_MBC_L0, false, 0, 0 /*ideal interleaving */ , m_simBase);
  
  // printf("MBC_L0_CACHE core_id:%d m_l0_cache:%p is created\n", core_id, m_l0_cache);
// port_c *rbt_l0_port; 
// cache_c *rbt_l1_cache; 
// port_c *rbt_l1_port; 

}

mbc_c::~mbc_c() {

}

bool mbc_c::bounds_checking(uop_c *cur_uop)
{
  
  if (*KNOB(KNOB_PERFECT_BOUNDS_CACHE)== true) {

    cur_uop->m_bounds_check_status = BOUNDS_L0_HIT; 
    return true; 
  }
  bool l0_cache_hit = false; 
 // int appl_id =  m_simBase->m_core_pointers[cur_uop->m_core_id]->get_appl_id(cur_uop->m_thread_id);
  int appl_id = 0;
  dcache_data_s* line = NULL; 
  Addr region_id = cur_uop->m_pc;  // weill be replaced with reading a separate file 
  Addr line_addr;
  Addr victim_line_addr; 
  cur_uop->m_bounds_check_status = BOUNDS_L0_HIT; 
 
  line = (dcache_data_s*)m_l0_cache->access_cache(region_id, &line_addr, true,
                                               appl_id);
  l0_cache_hit = (line)? true: false; 

  // port latency modeling? 

  if (!line) {
    // ideal insert of l0 or not ? 
    bounds_insert(cur_uop->m_core_id, region_id, 1, 1000);
    line = (dcache_data_s*)m_l0_cache->insert_cache(region_id, &line_addr, &victim_line_addr, appl_id, false);
  }

  cur_uop->m_bounds_check_status = (l0_cache_hit ? BOUNDS_L0_HIT : BOUNDS_L1_HIT); 

  return l0_cache_hit; 
  // return true; 
}

bool mbc_c::bounds_insert(int core_id, Addr id, Addr min_addr, Addr max_addr)
{
 
//unordered_map<Addr, bounds_info_s> m_rbt;  

auto rbt_iter = m_rbt.find(id);

if (rbt_iter != m_rbt.end()){
  // founds the bounds info 
  STAT_CORE_EVENT(core_id, BOUNDS_INFO_HASH_HIT);
}
else {
  
  // not found the bounds info 
  /*
  bounds_info_s bounds_info; 
  bounds_info.id = id; 
  bounds_info.min_addr = min_addr; 
  bounds_info.max_addr = max_addr; 

  m_rbt.insert(std::make_pair <Addr, bounds_info_s> (id, bounds_info)); 
  */
  m_rbt[id]= id; 

  STAT_CORE_EVENT(core_id, BOUNDS_INFO_INSERT);

}


return true; 
}