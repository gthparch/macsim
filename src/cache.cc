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
 * File         : cache.cc
 * Author       : Harsh Murarka 
 * Date         : 03/03/2010 
 * CVS          : $Id: cache.cc,
 * Description  : cache structure (based on cache_lib at scarab) 
 *********************************************************************************************/


/*
 * Summary: Cache library
 */


#include "assert_macros.h"
#include "cache.h"
#include "utils.h"

#include "debug_macros.h"

#include "all_knobs.h"

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_CACHE_LIB, ## args)
#define DEBUG_MEM(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM_TRACE, ## args)


///////////////////////////////////////////////////////////////////////////////////////////////


cache_entry_c::cache_entry_c()
  : m_valid(false), m_last_access_time(0)
{
}


cache_set_c::cache_set_c(int assoc)
{
  m_entry = new cache_entry_c[assoc];
  m_assoc = assoc;
}

cache_set_c::~cache_set_c()
{
  delete m_entry;
}


///////////////////////////////////////////////////////////////////////////////////////////////


// cache_c constructor
cache_c::cache_c(string name, int num_set, int assoc, int line_size, 
    int data_size, int bank_num, bool cache_by_pass, int core_id, Cache_Type cache_type_info,
    bool enable_partition, macsim_c* simBase) 
{
  m_simBase = simBase;

  DEBUG("Initializing cache called '%s'.\n", name.c_str());

  // Setting Basic Parameters sent via caller
  m_name        = name;
  m_data_size  = data_size;
  m_assoc      = assoc;
  m_num_sets   = num_set;
  m_line_size  = line_size;
  m_cache_type = cache_type_info; 

  // Setting some fields to make indexing quick
  m_set_bits    = log2_int(num_set);
  m_shift_bits  = log2_int(line_size); /* use for shift amt. */
  m_set_mask    = N_BIT_MASK(log2_int(num_set)); /* use after shifting */
  m_tag_mask    = ~m_set_mask;  /* use after shifting */
  m_offset_mask = N_BIT_MASK(m_shift_bits); /* use before shifting */
  m_bank_num    = bank_num; 

  // Allocating memory for all the sets (pointers to line arrays)
  m_core_id = core_id; 

  // Allocating memory for all of the lines in each set
  m_set = new cache_set_c* [m_num_sets];  

  for (int ii = 0; ii < m_num_sets; ++ii) {
    m_set[ii] = new cache_set_c(m_assoc); 

    // Allocating memory for all of the data elements in each line
    for (int jj = 0; jj < assoc; ++jj) {
      m_set[ii]->m_entry[jj].m_valid          = false;
      m_set[ii]->m_entry[jj].m_access_counter = false;
      if (data_size > 0) {
        m_set[ii]->m_entry[jj].m_data = (void *)malloc(data_size);
        memset(m_set[ii]->m_entry[jj].m_data, 0, data_size);
      } 
      else {
        m_set[ii]->m_entry[jj].m_data = INIT_CACHE_DATA_VALUE;
      }
    }
  }

  // Initializing Last update count
  m_cache_by_pass = cache_by_pass;

  m_num_cpu_line = 0;
  m_num_gpu_line = 0;
  m_insert_count = 0;

  m_enable_partition = enable_partition;
}


cache_c::~cache_c()
{
  for (int ii = 0; ii < m_num_sets; ++ii) {
    for (int jj = 0; jj < m_assoc; ++jj) {
      if (m_data_size > 0) {
        free(m_set[ii]->m_entry[jj].m_data);
      }
    }
    delete m_set[ii];
  }
  delete[] m_set;
}


// parse tag address and set index from an address
void cache_c::find_tag_and_set(Addr addr, Addr *tag, int *set) 
{
  *tag = addr >> m_shift_bits & m_tag_mask;
  *set = addr >> m_shift_bits & m_set_mask;
}


// access the cache
void* cache_c::access_cache(Addr addr, Addr *line_addr, bool update_repl, int appl_id) 
{
  // Check if cache by pass is set. If so return NULL.
  if (m_cache_by_pass)
    return NULL;

  Addr tag;
  int set;

  // Get Tag and set to check if the addr exists in cache
  find_tag_and_set(addr, &tag, &set);
  *line_addr = base_cache_line (addr);

  if (update_repl)
    update_cache_on_access(*line_addr, set, appl_id);

  // Walk through the set
  for (int ii = 0; ii < m_assoc; ++ii) {
    // For each line in based on associativity
    cache_entry_c * line = &(m_set[set]->m_entry[ii]);

    // Check for matching tag and validity
    if (line->m_valid && line->m_tag == tag) {
      // If hit, then return  
      assert(line->m_data);

      if (update_repl) {
        // If prefetch is set mark it as used  
        if (line->m_pref) {
          line->m_pref = false;
        }
        update_line_on_hit(line, set, appl_id);
      }   

      return line->m_data;
    }
  }

  if (update_repl)
    update_cache_on_miss(set, appl_id);

  return NULL;
}

void cache_c::update_cache_on_access(Addr line_addr, int set, int appl_id)
{
}


void cache_c::update_line_on_hit(cache_entry_c* line, int set, int appl_id)
{
  line->m_last_access_time = CYCLE;
}


void cache_c::update_cache_on_miss(int set_id, int appl_id)
{
  // do nothing
}


// find an entry to be replaced based on the policy
cache_entry_c* cache_c::find_replacement_line(int set, int appl_id) 
{
  if (*m_simBase->m_knobs->KNOB_CACHE_USE_PSEUDO_LRU) {
    while (1) {
      for (int ii = 0; ii < m_assoc; ++ii) {
        cache_entry_c* line = &(m_set[set]->m_entry[ii]);
        if (!line->m_valid || line->m_last_access_time == 0) {
          return &(m_set[set]->m_entry[ii]);
        }
      }

      for (int ii = 0; ii < m_assoc; ++ii) {
        cache_entry_c* line = &(m_set[set]->m_entry[ii]);
        line->m_last_access_time = 0;
      }
    }
  }
  else {
    int i = 0;
    int lru_ind = 0;
    Counter lru_time = MAX_INT;
    while (i < m_assoc) {
      cache_entry_c* line = &(m_set[set]->m_entry[i]);
      // If free entry found, return it
      if (!line->m_valid) {
        lru_ind = i;
        break;
      }

      // Check if this is the LRU entry encountered
      if (line->m_last_access_time < lru_time) {
        lru_ind  = i;
        lru_time = line->m_last_access_time;
      }
      ++i;
    }
    return &(m_set[set]->m_entry[lru_ind]);
  }
}


cache_entry_c* cache_c::find_replacement_line_from_same_type(int set, int appl_id, 
    bool gpuline) 
{
  int current_type_count; 
  int current_type_max;

  if (gpuline) {
    current_type_count = m_set[set]->m_num_gpu_line;
    current_type_max   = m_assoc - *m_simBase->m_knobs->KNOB_HETERO_STATIC_CPU_PARTITION;
  }
  else {
    current_type_count = m_set[set]->m_num_cpu_line;
    current_type_max   = *m_simBase->m_knobs->KNOB_HETERO_STATIC_CPU_PARTITION;
  }
    
  int lru_index = -1;
  Counter lru_time = ULLONG_MAX;
  for (int ii = 0; ii < m_assoc; ++ii) {
    cache_entry_c* line = &(m_set[set]->m_entry[ii]);
    if (!line->m_valid && current_type_count < current_type_max) {
      lru_index = ii;
      break;
    }

    if (line->m_valid && line->m_gpuline == gpuline && line->m_last_access_time < lru_time) {
      lru_index = ii;
      lru_time = line->m_last_access_time;
    }
  }

  if (lru_index == -1) {
    for (int ii = 0; ii < m_assoc; ++ii) {
      cache_entry_c* line = &(m_set[set]->m_entry[ii]);
      report("valid:" << line->m_valid << " gpu:" << line->m_gpuline << " lru:" << line->m_last_access_time); 
    }
    ASSERTM(lru_index != -1, "assoc:%d count:%d max:%d gpu:%d\n", 
            m_assoc, current_type_count, current_type_max, gpuline);
  }
  return &(m_set[set]->m_entry[lru_index]);
}


// initialize a cache line
void cache_c::initialize_cache_line(cache_entry_c *ins_line, Addr tag, Addr addr, int appl_id,
    bool gpuline, int set_id, bool skip) 
{
  ins_line->m_valid            = true;
  ins_line->m_tag              = tag;
  ins_line->m_base             = (addr & ~m_offset_mask);
  ins_line->m_access_counter   = 0;
  ins_line->m_last_access_time = CYCLE;
  ins_line->m_pref             = false;
  ins_line->m_skip             = skip;

  // for heterogeneous simulation
  ins_line->m_appl_id          = appl_id;
  ins_line->m_gpuline          = gpuline;
  if (ins_line->m_gpuline) { 
    ++m_num_gpu_line;
    ++m_set[set_id]->m_num_gpu_line;
  }
  else {
    ++m_num_cpu_line;
    ++m_set[set_id]->m_num_cpu_line;
  }
}


void *cache_c::insert_cache(Addr addr, Addr *line_addr, Addr *updated_line, int appl_id,
    bool gpuline) 
{
  return insert_cache(addr, line_addr, updated_line, appl_id, gpuline, false);
}


// insert a cache line
void *cache_c::insert_cache(Addr addr, Addr *line_addr, Addr *updated_line, int appl_id,
    bool gpuline, bool skip) 
{
  Addr tag;
  int set;
  cache_entry_c *ins_line;
  *line_addr = base_cache_line (addr);

  // Get the set where the addr maps and tag to asssociate 
  // to the new cache line being returned
  find_tag_and_set(addr, &tag, &set);

  // Get the pointer to a line that should be replaced as per policy
  if (*m_simBase->m_knobs->KNOB_HETERO_STATIC_CACHE_PARTITION && m_enable_partition) {
    ins_line = find_replacement_line_from_same_type(set, appl_id, gpuline);
  }
  else {
    ins_line = find_replacement_line(set, appl_id);
  }

  // Populate the update_line variable if the present line was in use
  if (ins_line->m_valid) {
    *updated_line = ins_line->m_base;
    update_set_on_replacement(tag, ins_line->m_appl_id, set, ins_line->m_gpuline);
  }
  else {
    *updated_line = 0;
  }
  
  DEBUG("Replacing (set %u, tag 0x%s, base 0x%s, up:0x%s) in cache '%s' "
        "core_id:%d with base 0x%s\n",
        set, hexstr64s(ins_line->m_tag), hexstr64s(ins_line->m_base), 
        hexstr64s(*updated_line), m_name.c_str(), m_core_id, hexstr64s(*line_addr));
  
  // Initialize the other fileds of the cache line
  initialize_cache_line(ins_line, tag, addr, appl_id, gpuline, set, skip);

  // Check if prefetch flag was set and update the field accordingly
  ++m_insert_count;

  return ins_line->m_data;
}


void cache_c::update_set_on_replacement(Addr tag, int appl_id, int set, bool gpuline)
{
  if (gpuline) {
    --m_num_gpu_line;
    --m_set[set]->m_num_gpu_line;
  }
  else {
    --m_num_cpu_line;
    --m_set[set]->m_num_cpu_line;
  }
}


// initialize (nullify) a cache line
bool cache_c::null_cache_line_fields(cache_entry_c *line)
{
  line->m_tag   = 0;
  line->m_valid = false;
  line->m_base  = 0;
  memset(line->m_data, 0, m_data_size);
  if (line->m_dirty) {
    return true;
  }
  else {
    return false;
  }
}


// invalidate a cache line
bool cache_c::invalidate_cache_line(Addr addr)
{
  Addr tag;
  int set;

  // Get the set where the addr maps and tag to asssociate 
  // to the new cache line being returned
  find_tag_and_set(addr, &tag, &set);

  for (int ii = 0; ii < m_assoc; ++ii) {
    // For each line in based on associativity
    cache_entry_c *line = &(m_set[set]->m_entry[ii]);

    // Check for matching tag and validity
    if (line->m_valid && line->m_tag == tag) {
      // If hit, then erase the current line data and return 
      return null_cache_line_fields(line);
    }
  }

  return false;
}


// get a cache line address from an address
Addr cache_c::base_cache_line(Addr addr) 
{
  return (addr & ~m_offset_mask);
}


// invalidate all cache lines
void cache_c::invalidate_cache(void) 
{
  for (int ii = 0; ii < m_num_sets; ++ii) {
    for (int jj = 0; jj < m_assoc; ++jj) {
      cache_entry_c* line = &(m_set[ii]->m_entry[jj]);
      line->m_valid = false;
      line->m_tag   = 0;
      memset(line->m_data, 0, m_data_size);
    }
  }
}


// get bank id from an address
int cache_c::get_bank_num(Addr addr) 
{ 
  return addr >> m_shift_bits & N_BIT_MASK(log2_int(m_bank_num)); 
}


// find lru counter value
Counter cache_c::find_min_lru(int set) 
{
  Counter lru_time = MAX_INT;
  for (int ii = 0; ii < m_assoc; ++ii) {
    cache_entry_c *entry = &(m_set[set]->m_entry[ii]);
    if (entry->m_valid && entry->m_last_access_time < lru_time) {
      lru_time = m_set[set]->m_entry[ii].m_last_access_time;
    }   
  }

  if (lru_time == MAX_INT) {
    lru_time = CYCLE;
  }
  return lru_time;
}


// print cache information
void cache_c::print_info(int id)
{
  if (*m_simBase->m_knobs->KNOB_COLLECT_CACHE_INFO > 0 && 
      ++m_insert_count % *m_simBase->m_knobs->KNOB_COLLECT_CACHE_INFO == 0) {
    cout << "CACHE::L" << id << " cpu: " << m_num_cpu_line << " gpu: " << m_num_gpu_line << "\n";
  }
}
