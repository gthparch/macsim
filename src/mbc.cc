
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

//static set<int> bounds_info; 

mbc_c::mbc_c(int core_id, macsim_c *simBase)
{
  m_core_id = core_id; 
  m_simBase = simBase; 

/* init hash map rbt, cache rbt_l0_cache, cache rbt_l1_cache */ 
  m_l0_cache = new cache_c (
  "rbt_l1_cache", *KNOB(KNOB_BOUNDS_L0_CACHE_ENTRY), *KNOB(KNOB_BOUNDS_L0_CACHE_ENTRY),
    *KNOB(KNOB_LLC_LINE_SIZE), sizeof(dcache_data_s), *KNOB(KNOB_BOUNDS_L0_CACHE_ENTRY),
    false, core_id, CACHE_MBC_L0, false, 0, 0 /*ideal interleaving */ , m_simBase);

/*
  if (*KNOB(KNOB_ENABLE_BOUNDS_IDS_FILE)) mbc_info_read()
    MEMDEP_OUT = file_tag_fopen(boundsid_filename, "r", m_simBase);	
  }
  */
  // printf("MBC_L0_CACHE core_id:%d m_l0_cache:%p is created\n", core_id, m_l0_cache);
// port_c *rbt_l0_port; 
// cache_c *rbt_l1_cache; 
// port_c *rbt_l1_port; 

}


void mbc_c::bounds_info_read(string file_name_base,  unordered_map <int, int> &m_bounds_info)
{

  ifstream bounds_info_file;
  string bounds_info = file_name_base + ".Binfo";
  // string bounds_info = "test.Binfo";
  cout <<" bounds file name is " << bounds_info << endl; 
  

  bounds_info_file.open(bounds_info.c_str());

  unsigned linenum = 0;
  while (bounds_info_file.good()) {
    string line;
    getline(bounds_info_file, line);
    // skip empty lines
    if (line.empty()) continue;
    // skip comment lines
    if (line[0] == '#') continue;
    // skip the first line (csv header)
    if (linenum == 0) {
      linenum++;
      continue;
    }
    int load_store_type; 
    int reg_id; 
    int region_id; 
    int ec_region_id; 
    int pointer_type; 
    string name;
    std::stringstream ss(line);
    ss >> load_store_type; 
    ss >> reg_id;
    ss >> region_id; 
    ss >> pointer_type; 
    
    ec_region_id = region_id * 64 + pointer_type; 
    std::cout << "reg_id:" << reg_id << " region_id : " << region_id <<  " ec_region_id: " << ec_region_id;
    std::cout << "load/store:" << load_store_type << "pointer_type: " << pointer_type << endl; 
    m_bounds_info[reg_id] = ec_region_id; 
    
    linenum++;
  }
  bounds_info_file.close();
  
}

mbc_c::~mbc_c() {

}

bool mbc_c::bounds_info_check_signed(int src1_id, unordered_map<int, int>  &m_bounds_info, int &region_id){ 

//  bool mbc_c::bounds_info_check_signed( set <int> &m_bounds_info){ 
//    int src1_id = 1; 
    /* check whether uop sources the destination or */ 
    if(m_bounds_info.find(src1_id) != m_bounds_info.end()){
      region_id = m_bounds_info[src1_id]; 
      return true;
    }
    else {
      region_id = src1_id*64+2; 
      return true;
    }

   /* else if (*KNOB(KNOB_ENABLE_BOUNDS_PROB_FILTER)) { 
       double x = rand()/static_cast<double>(RAND_MAX+1); 
      if (x*10 > *KNOB(KNOB_BOUNDS_PROB_TH)){
        region_id = src1_id*64+2;
        return true; 
      } else {
        region_id = src1_id*64+1; 
        return true; 
      }
    } */
   //   else 
    //    return false; 

    if(m_bounds_info.empty()) {
      region_id = src1_id *64 + 2; 
      return true; // if there is no bounds info, then all memory instructions are signed 
    }
}

bool mbc_c::bounds_checking(uop_c *cur_uop)
{
  
  if (*KNOB(KNOB_PERFECT_BOUNDS_CACHE)== true) {

    cur_uop->m_bounds_check_status = BOUNDS_L0_HIT; 
    return true; 
  }
  bool l0_cache_hit = false;  
  bool l1_cache_hit = false; 
 // int appl_id =  m_simBase->m_core_pointers[cur_uop->m_core_id]->get_appl_id(cur_uop->m_thread_id);
  int appl_id = 0;
  dcache_data_s* line = NULL; 
  Addr region_id;  
  Addr line_addr;
  Addr victim_line_addr; 
  bool bounds_insert_hw = false; 

  if (cur_uop->m_bounds_id) region_id = cur_uop->m_bounds_id; 
  else region_id = cur_uop->m_pc;  // when bounds id files are not available, we will just use pc 

  int pointer_type = (cur_uop->m_bounds_id)%64; 

  STAT_CORE_EVENT(cur_uop->m_core_id, DYN_BOUNDS_POINTER_TYPE__0 + MIN2(pointer_type, 3));
  cur_uop->m_bounds_check_status = BOUNDS_L0_HIT; 


 
  line = (dcache_data_s*)m_l0_cache->access_cache(region_id, &line_addr, true,
                                               appl_id);

  if (line) {
      if ((line->m_fetch_cycle +  *KNOB(KNOB_BOUNDS_L0_INSERT_LATENCY)) <= (m_simBase->m_core_cycle[cur_uop->m_core_id] )) { 
        l0_cache_hit = true;
      }
      else {
          l0_cache_hit = false;
          STAT_CORE_EVENT(cur_uop->m_core_id, BOUNDS_L0_INSERT_DELAY_MISS); 
      }
      DEBUG_CORE(cur_uop->m_core_id, "fetched_cycle:%lld ready_cycle:%lld curr_cycle:%lld l0_cache_hit:%d\n",
         line->m_fetch_cycle, ( line->m_fetch_cycle +  *KNOB(KNOB_BOUNDS_L0_INSERT_LATENCY)), (m_simBase->m_core_cycle[cur_uop->m_core_id]) , l0_cache_hit);
  }else {
    l0_cache_hit = false; 
  }
  // l0_cache_hit = (line)? true: false; 

  // port latency modeling? 

  if (!line) {
    // ideal insert of l0 or not ? 
    l1_cache_hit = bounds_insert(cur_uop->m_core_id, region_id, 1, 1000);
    line = (dcache_data_s*)m_l0_cache->insert_cache(region_id, &line_addr, &victim_line_addr, appl_id, false);
    line->m_fetch_cycle = m_simBase->m_core_cycle[cur_uop->m_core_id];
    bounds_insert_hw = true; 
  }

  cur_uop->m_bounds_check_status = (l0_cache_hit ? BOUNDS_L0_HIT : (l1_cache_hit ? BOUNDS_L1_HIT: BOUNDS_TABLE_INSERT)); 
  DEBUG_CORE(cur_uop->m_core_id, "pc:%lld region_id:%lld  bounds_id:%d l0_cache_hit:%d bounds_hw_insert:%d \n", 
         cur_uop->m_pc, region_id, cur_uop->m_bounds_id, l0_cache_hit, bounds_insert_hw);
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
  return true; 
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
  int pointer_type = (id)%64; 

  STAT_CORE_EVENT(core_id, DYN_BOUNDS_POINTER_TYPE__0 + MIN2(pointer_type, 3));
  m_rbt[id]= id; 

  STAT_CORE_EVENT(core_id, BOUNDS_INFO_INSERT);
  return false; 
}


}