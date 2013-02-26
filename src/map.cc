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
 * File         : map.cc 
 * Author       : Hyesoon Kim 
 * Date         : 1/8/2008 
 * CVS          : $Id: map.cc 894 2009-11-10 22:37:36Z hyesoon $:
 * Description  : map  (based on scarab) 
 *********************************************************************************************/


#include <typeinfo>
#include <iostream>
#include <sys/types.h>

#include "assert_macros.h"
#include "global_types.h"
#include "uop.h"
#include "trace_read.h"
#include "core.h"
#include "map.h"
#include "utils.h"

#include "knob.h"
#include "debug_macros.h"
#include "statistics.h"
#include "statsEnums.h"

#include "all_knobs.h"

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MAP_STAGE, ## args)

#define MEM_MAP_KEY(va, off_path)(((va >> 3) << 1) | off_path)

/* Are these defined somewhere else? */
#define xsetbit(x,y)   ((x) |=  (((__typeof__ (x))1) << (y)))
#define xclearbit(x,y) ((x) &= ~(((__typeof__ (x))1) << (y)))
#define invertbit(x,y) ((x) ^=  (((__typeof__ (x))1) << (y)))
#define xtestbit(x,y)  (((x) >> (y)) & ((__typeof__ (x))1))

/* do addresses 0 and 1 overlap?  */
#define BYTE_OVERLAP(a0, s0, a1, s1)	(!((a0)  >= (a1) + (s1) || (a1) >= (a0) + (s0)))


///////////////////////////////////////////////////////////////////////////////////////////////


// map data constructor
map_data_c::map_data_c(macsim_c* simBase)
{
  /* initialize the memory dependence hash table */
  m_simBase = simBase;
  m_oracle_mem_hash = new hash_c<mem_map_entry_c>(simBase->m_mem_map_entry_pool);
}


// initialize map data
void map_data_c::initialize()
{
  for (int jj = 0; jj < NUM_REG_IDS *2; ++jj) {
    m_reg_map[jj].m_uop        = m_simBase->m_invalid_uop;
    m_reg_map[jj].m_uop_num    = 0;
    m_reg_map[jj].m_unique_num = 0; 
    m_reg_map[jj].m_thread_id  = 0;
    m_map_flags[jj]            = false; 

  }

  m_last_store[0].m_uop     = m_simBase->m_invalid_uop;
  m_last_store[0].m_uop_num = 0; 
  m_last_store[1].m_uop     = m_simBase->m_invalid_uop;
  m_last_store[1].m_uop_num = 0; 

  m_last_store_flag = false;

  m_oracle_mem_hash->clear();
}


///////////////////////////////////////////////////////////////////////////////////////////////


// constructor
map_c::map_c(macsim_c* simBase)
{
  m_simBase = simBase;
  m_core_map_data = new hash_c<map_data_c>("core_map_data"); 
}


// destructor
map_c::~map_c()
{
  delete m_core_map_data;
}


// set source N not ready 
void map_c::set_not_rdy_bit (uop_c *uop, int bit)
{
  ASSERT(uop);
  ASSERT(bit < uop->m_num_srcs);

  uop->m_srcs_not_rdy_vector |= (0x1 << bit);
}


// set dependent-source information
void map_c::add_src_from_map_entry(uop_c *uop, int src_num, map_entry_c *map_entry, 
    Dep_Type type)
{
  src_info_c *info = &(uop->m_map_src_info[src_num]); 

  ASSERT(uop);
  ASSERT(map_entry);
  ASSERT(map_entry->m_uop);
  ASSERT(type < NUM_DEP_TYPES);

  DEBUG("core_id:%d thread_id:%d Added dep uop_num:%s inst_num:%s src_uop_num:%s src_num:%d\n",
        uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), unsstr64(uop->m_inst_num), 
        unsstr64(map_entry->m_uop_num), src_num);

  ASSERTM(src_num < MAX_UOP_SRC_DEPS, "src_num:%d MAX_UOP_SRC_DEPS:%d \n", 
      src_num, MAX_UOP_SRC_DEPS); 

  ASSERTM(!(uop->m_uop_num) || ((map_entry->m_uop_num) < uop->m_uop_num) || uop->m_off_path, 
      "core_id:%d thread_id:%d map_entry->uop_num:%lld uop_num:%lld \n",  
      uop->m_core_id, uop->m_thread_id, map_entry->m_uop_num, uop->m_uop_num);;

  // set source information
  info->m_type       = type;
  info->m_uop        = map_entry->m_uop;
  info->m_uop_num    = map_entry->m_uop_num;
  info->m_unique_num = map_entry->m_unique_num; 

  // set source bit not ready
  set_not_rdy_bit(uop, src_num); 

  DEBUG("core_id:%d thread_id:%d Added dep uop_num:%s src_uop_num:%s "
      "dep inst_num:%s src_inst_num:%s src_num:%d dep pc:0x%s src pc:0x%s\n", 
      uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), 
      unsstr64(map_entry->m_uop_num), unsstr64(uop->m_inst_num), 
      unsstr64(map_entry->m_inst_num), src_num, hexstr64s(uop->m_pc),
      hexstr64s(map_entry->m_pc));
}


// add dependent source uop from another uop
void map_c::add_src_from_uop (uop_c *uop, uop_c *src_uop, Dep_Type type) 
{
  // check whether we src_uop is already in the op or not. 
  for (int jj = 0 ; jj < uop->m_num_srcs ; ++jj) {
    src_info_c *info = &uop->m_map_src_info[jj];
    uop_c  *uop_src_uop = info->m_uop;
    if (uop_src_uop->m_uop_num == src_uop->m_uop_num) return; 
  }

  // we want to use this function only for memory dependencies 
  ASSERT(type != REG_DATA_DEP); 

  uns src_num	     = uop->m_num_srcs++;
  src_info_c *info = &(uop->m_map_src_info[src_num]);

  ASSERT(uop);
  ASSERT(src_uop);
  ASSERT(type < NUM_DEP_TYPES);
  ASSERTM(src_num < MAX_UOP_SRC_DEPS, "src_num=%d MAX_UOP_SRC_DEPS=%d\n", 
      src_num, MAX_UOP_SRC_DEPS);

  ASSERTM(src_uop->m_uop_num < uop->m_uop_num ||
      (uop->m_thread_id != -1 && src_uop->m_thread_id != -1) ||
      (uop->m_off_path && src_uop->m_thread_id != -1),
      "uop:%s  src_uop:%s\n", 
      unsstr64(uop->m_uop_num), unsstr64(src_uop->m_uop_num));

  info->m_type	     = type;
  info->m_uop	       = src_uop;
  info->m_uop_num    = src_uop->m_uop_num;
  info->m_unique_num = src_uop->m_unique_num;

  // for memory dependencies, derived_from_prog_input incremented in track_addr
  set_not_rdy_bit(uop, src_num);

  DEBUG("core_id:%d thread_id:%d Added dep uop_num:%s src_uop_num:%s src_num:%d dep_type:%d "
      "pc:0x%s\n", uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), 
      unsstr64(src_uop->m_uop_num), src_num, type, hexstr64s(uop->m_pc));
}


// add memory dependence information
void map_c::map_mem_dep(uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_MEM_OBEY_STORE_DEP) 
    return; 

  // update store dependence
  if ((uop->m_mem_type == MEM_ST) || (uop->m_mem_type == MEM_ST_LM)) 
    update_store_hash(uop);

  // add store dependence
  if ((uop->m_mem_type == MEM_LD) || (uop->m_mem_type == MEM_LD_LM)) 
    add_store_deps(uop); 
}


// delete entire dependence data for terminated thread
void map_c::delete_map(int tid)
{
  m_core_map_data->hash_table_access_delete(tid);
}


// update register dependence information (set destination register)
void map_c::update_map(uop_c *uop)
{
  map_data_c *map_data;
  map_data = m_core_map_data->hash_table_access(uop->m_thread_id);
  ASSERT(NULL != map_data);

  // update the register map if the uop produces a value
  for (int ii = 0; ii < uop->m_num_dests; ++ii) {
    uns16 id = uop->m_dest_info[ii]; 

    ASSERTM(id < NUM_REG_IDS, "id:%d \n", id); 

    uns ind = id << 1 | uop->m_off_path; 
    ASSERT(ind < NUM_REG_IDS*2); 
    map_entry_c *map_entry = &(map_data->m_reg_map[ind]); 

    DEBUG("core_id:%d thread_id:%d Writing map uop_num:%s inst_num:%s off_path:%d id:%d "
        "flag:%d ind:%d \n", uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), 
        unsstr64(uop->m_inst_num), uop->m_off_path, id, map_data->m_map_flags[id], ind); 

    // update dependence information
    map_entry->m_uop          = uop;
    map_entry->m_uop_num      = uop->m_uop_num;
    map_entry->m_unique_num   = uop->m_unique_num; 
    map_entry->m_inst_num     = uop->m_inst_num; 
    map_entry->m_pc           = uop->m_pc; 
    map_entry->m_mem_type     = uop->m_mem_type;
    map_data->m_map_flags[id] = uop->m_off_path; 
  }
}


// map_uop: involves two things: setting up the src array in op_info
// and updating the current map state based on the uop's output values.
// note that this function does nothing for memory dependencies.  you
// must call map_mem_dep after oracle_exec to properly handle them. */
void map_c::map_uop (uop_c *uop)
{
  ASSERT         (uop);
  read_reg_map   (uop);  /* set reg sources */ 
  read_store_map (uop);  /* set addr dependency on last store */ 
  update_map     (uop);  /* update reg and last store maps */ 
}


// read_reg_map: read and set srcs based on registers */
void map_c::read_reg_map (uop_c *uop)
{
  map_data_c *map_data;
  bool new_entry;
  map_data = m_core_map_data->hash_table_access_create(uop->m_thread_id, &new_entry, m_simBase);

  ASSERT(NULL != map_data);
  // new entry
  if (new_entry)
    map_data->initialize();

  for (int ii = 0; ii < uop->m_num_srcs; ++ii) {
    uns id = uop->m_src_info[ii];
    ASSERT(id < NUM_REG_IDS); 
    uns ind = id << 1 | (map_data->m_map_flags[id]); 
    ASSERT(ind < NUM_REG_IDS*2); 
    map_entry_c *map_entry = &(map_data->m_reg_map[ind]); 
    DEBUG("core_id:%d thread_id:%d Reading map uop_num:%s inst_num:%s "
          "off_path:%d id:%d flag:%d ind:%d ii:%d num_srcs:%d \n",
          uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), 
          unsstr64(uop->m_inst_num), uop->m_off_path, id, 
          map_data->m_map_flags[id], ind, ii, uop->m_num_srcs); 

    add_src_from_map_entry(uop, ii, map_entry, REG_DATA_DEP);
  }
}


//  read_store_map: used to make mem ops dependent on the last store
//   (no speculative loads)
void map_c::read_store_map(uop_c *uop)
{
  map_data_c *map_data;
  map_data = m_core_map_data->hash_table_access(uop->m_thread_id);
  ASSERT(NULL != map_data);
  if (!*KNOB(KNOB_MEM_OBEY_STORE_DEP) || *KNOB(KNOB_MEM_OOO_STORES)) 
    return; 

  if (uop->m_mem_type) {
    uns ind = map_data->m_last_store_flag; 

    ASSERT(ind < 2); 
    map_entry_c *map_entry = &(map_data->m_last_store[ind]); 

    DEBUG("core_id:%d thread_id:%d Reading store map uop_num:%s inst_num:%s off_path:%d "
        "flag:%d   ind:%d \n",
        uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), unsstr64(uop->m_inst_num), 
        uop->m_off_path, map_data->m_last_store_flag, ind); 

    add_src_from_map_entry(uop, uop->m_num_srcs++, map_entry, MEM_ADDR_DEP); 
  }
}


#define MEM_GEN_OFF_PATH_VALS 0 
// update load-store dependence
void map_c::update_store_hash (uop_c *uop) 
{
  mem_map_entry_c * mem_map_p;
  Addr va = uop->m_vaddr; 
  int first_byte = va & 0x7; 
  bool off_path = uop->m_off_path;
  Quad old_data = 0;
  bool new_entry = false;
  map_data_c *map_data = m_core_map_data->hash_table_access(uop->m_thread_id);
  ASSERT(NULL != map_data);

  // If using new_oracle, hash_table entry should have been created 
  // in memory write function
  if (*m_simBase->m_knobs->KNOB_USE_NEW_ORACLE) { 
    if (off_path && !MEM_GEN_OFF_PATH_VALS) 
      mem_map_p = (map_data->m_oracle_mem_hash)->hash_table_access_create(
          MEM_MAP_KEY(va, off_path), &new_entry); 
    else {
      mem_map_p = (map_data->m_oracle_mem_hash)->hash_table_access(
          MEM_MAP_KEY(va, off_path));
    }

    DEBUG("add store_hash core_id:%d thread_id:%d fb:%d USH: (%s) inst:%s "
          "St(%d)[%s]: %s + %s => %s\n", 
          uop->m_core_id, uop->m_thread_id, first_byte, 
          unsstr64(uop->m_uop_num), unsstr64(uop->m_inst_num), uop->m_mem_size,
          hexstr64s(uop->m_vaddr), hexstr64s(old_data), "XXX", "XXX"); 
  } 
  else { 
    mem_map_p = (map_data->m_oracle_mem_hash)->hash_table_access_create(
        MEM_MAP_KEY(va, off_path), &new_entry);
  }


  if (new_entry) {
    ASSERT(!*KNOB(KNOB_USE_NEW_ORACLE) || (off_path && !MEM_GEN_OFF_PATH_VALS));
    DEBUG("core_id:%d thread_id:%d update_store_hash uop:%s inst:%s va:%s first_byte:%s\n",
          uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), 
          unsstr64(uop->m_inst_num), hexstr64s(va), hexstr64s(first_byte)); 

    mem_map_p->m_store_mask = 0;
  } 

  old_data = mem_map_p->m_data;
  xsetbit(mem_map_p->m_store_mask, first_byte);
  ASSERT(first_byte < BYTES_IN_QUADWORD);

  mem_map_p->m_uop[first_byte] = uop;
  DEBUG("core_id:%d thread_id:%d store_mask:%s first_byte:%s uop_num:%s vaddr:%s  \n", 
      uop->m_core_id, uop->m_thread_id, hexstr64s(mem_map_p->m_store_mask), 
      hexstr64s(first_byte), unsstr64(uop->m_uop_num), 
      hexstr64s((mem_map_p->m_uop[first_byte])->m_vaddr)); 
}


// add store dependence
uop_c* map_c::add_store_deps(uop_c * uop) 
{
  mem_map_entry_c  * mem_map_p = NULL;
  Addr va = uop->m_vaddr;
  int first_byte  = va & 0x7;
  bool off_path   = uop->m_off_path;
  uop_c * src_uop     = NULL;
  map_data_c *map_data = m_core_map_data->hash_table_access(uop->m_thread_id);
  ASSERT(NULL != map_data);

  mem_map_p = (map_data->m_oracle_mem_hash)->hash_table_access(MEM_MAP_KEY(va, off_path));

  if (mem_map_p == NULL) {
    STAT_EVENT(LD_NO_FORWARD);
    return NULL;
  } 

  // determine which store within this quadword I am dependent on, if any
  // I overlap the only pending store to this quadword */
  if (mem_map_p->m_store_mask == (0x1 << first_byte)) { 
    ASSERTM(mem_map_p->m_uop[first_byte]->m_vaddr == va, 
        "%s != %s  first_byte:%d uop_num:%s core_id:%d thread_id:%d core_id:%d \n", 
        hexstr64s(mem_map_p->m_uop[first_byte]->m_vaddr), hexstr64s(va), first_byte, 
        uop ? unsstr64(uop->m_uop_num): "-1", 
        uop ? uop->m_core_id : -1, uop? uop->m_thread_id: -1, uop ? uop->m_core_id: -1);

    if (*m_simBase->m_knobs->KNOB_MEM_OOO_STORES) {
      add_src_from_uop(uop, mem_map_p->m_uop[first_byte], MEM_DATA_DEP);
      STAT_EVENT(FORWARDED_LD);
    }
  } 
  // Find byte on whom I am dependent
  else {  
    int ii;
    unsigned int first_byte_op_num = 0;
    for (first_byte = -1, ii = 0; ii < BYTES_IN_QUADWORD; ++ii) {
      uop_c * src_uop = mem_map_p->m_uop[ii];

      /* ensure mem_map_p info is valid */
      if (!xtestbit(mem_map_p->m_store_mask, ii)) 
        continue;           

      if (!BYTE_OVERLAP(src_uop->m_vaddr, src_uop->m_mem_size, va, uop->m_mem_size)) 
        continue;

      DEBUG("src uop_num:%s va:%s mem_size:%d dest uop uop_num:%s va:%s mem_size:%d \n", 
          unsstr64(src_uop->m_uop_num), hexstr64s(src_uop->m_vaddr), src_uop->m_mem_size, 
          unsstr64(uop->m_uop_num), hexstr64s(uop->m_vaddr), uop->m_mem_size); 

      if (*m_simBase->m_knobs->KNOB_MEM_OOO_STORES) {
        add_src_from_uop(uop, mem_map_p->m_uop[ii], MEM_DATA_DEP);
        STAT_EVENT(FORWARDED_LD);
      }

      /* take latest store dependency only */
      if (first_byte_op_num < mem_map_p->m_uop[ii]->m_uop_num) {        
        first_byte = ii;
        first_byte_op_num = mem_map_p->m_uop[ii]->m_uop_num;
      } 
    }

    if (first_byte == -1) {
      STAT_EVENT(LD_NO_FORWARD);
      return NULL;  /* No dependency found */
    }
  }

  ASSERT(mem_map_p->m_uop[first_byte]->m_uop_num < uop->m_uop_num || 
      uop->m_thread_id != -1 || 
      uop->m_off_path);
  ASSERT(MEM_MAP_KEY(va, off_path) ==  
      MEM_MAP_KEY(mem_map_p->m_uop[first_byte]->m_vaddr, off_path));

  src_uop = mem_map_p->m_uop[first_byte];

  if (!*m_simBase->m_knobs->KNOB_MEM_OOO_STORES) {
    add_src_from_uop(uop, src_uop, MEM_DATA_DEP);
    STAT_EVENT(FORWARDED_LD);
  }

  return src_uop;
}


// delete store dependence information
void map_c::delete_store_hash_entry(uop_c *uop) 
{
  if (*m_simBase->m_knobs->KNOB_IGNORE_DEP)
    return ;

  mem_map_entry_c * mem_map_p;

  Addr va              = uop->m_vaddr;
  int first_byte       = va & 0x7;
  bool off_path        = uop->m_off_path;
  map_data_c *map_data = m_core_map_data->hash_table_access(uop->m_thread_id);

  if (map_data == NULL)
    return ;

  auto oracle_mem_hash = map_data->m_oracle_mem_hash;

  mem_map_p = oracle_mem_hash->hash_table_access(MEM_MAP_KEY(va, off_path));

  if (!(mem_map_p && mem_map_p->m_uop[first_byte] == uop)) 
    return;

  xclearbit(mem_map_p->m_store_mask, first_byte);

  DEBUG("clear store_hash first_byte:%d va:%s core_id:%d uop_num:%s "
        "thread_num:%d\n", 
        first_byte, hexstr64s(uop->m_vaddr), uop->m_core_id, 
        unsstr64(uop->m_uop_num), uop->m_thread_id); 

  mem_map_p->m_uop[first_byte] = NULL;

  if (!mem_map_p->m_store_mask) {
    oracle_mem_hash->hash_table_access_delete(MEM_MAP_KEY(va, off_path));
  }
}


// wrapper function to delete store dependence information
void delete_store_hash_entry_wrapper (map_c *map, uop_c *uop) {
  map->delete_store_hash_entry(uop);
}


