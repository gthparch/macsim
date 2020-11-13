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
 * File         : memory.cc
 * Author       : Jaekyu Lee
 * Date         : 3/4/2011
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : memory system
 *********************************************************************************************/

#include <iostream>
#include <cmath>

#include "assert_macros.h"
#include "cache.h"
#include "core.h"
#include "debug_macros.h"
#include "dram.h"
#include "frontend.h"
#include "memory.h"
#include "network.h"
#include "port.h"
#include "uop.h"
#include "factory_class.h"
#include "bug_detector.h"
#include "mmu.h"

#include "config.h"

#include "all_knobs.h"
#include "statistics.h"

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM, ##args)
#define DEBUG_CORE(m_core_id, args...)                        \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) { \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM, ##args);      \
  }

#define ULINK 1
#define DLINK 1
#define ENABLE 0
#define DISABLE 1
#define HAS_ROUTER 1
#define NO_ROUTER 0
#define HAS_DONE_FUNC 1

///////////////////////////////////////////////////////////////////////////////////////////////
/// \page memory Memory system
/// memory system
///
// \image html memory.jpg (todo: add image)
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////

#define BIG_NUMBER 10000000000

int g_mem_priority[] = {
  10,  // IFETCH
  10,  // DFETCH
  10,  // DSTORE
  0,  // IPRF
  0,  // DPRF
  5,  // WB
  0,  // SW_DPRF
  0,  // SW_NTA
  0,  // SW_T0
  0,  // SW_T1
  0,  // SW_T2
};

///////////////////////////////////////////////////////////////////////////////////////////////

inline bool IsStore(Mem_Type type) {
  switch (type) {
    case MEM_ST:
    case MEM_ST_LM:
    case MEM_ST_SM:
    case MEM_ST_GM:
      return true;
    default:
      return false;
  }
}

inline bool IsLoad(Mem_Type type) {
  return !IsStore(type);
}

// dcache fill wrapper function
bool dcache_fill_line_wrapper(mem_req_s* req) {
  bool result = true;
  list<mem_req_s*> done_list;
  for (auto I = req->m_merge.begin(), E = req->m_merge.end(); I != E; ++I) {
    if ((*I)->m_done_func && !((*I)->m_done_func((*I)))) {
      result = false;
      continue;
    }
    done_list.push_back((*I));
  }

  // process merged requests
  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    req->m_merge.remove((*I));
    req->m_simBase->m_memory->free_req((*I)->m_core_id, (*I));
  }

  // process the request
  if (result && req->m_done_func && !req->m_simBase->m_memory->done(req)) {
    result = false;
  }

  return result;
}

bool dcache_write_ack_wrapper(mem_req_s* req) {
  assert(req->m_done_func && req->m_simBase->m_memory->write_done(req));
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// register base branch predictor
memory_c* default_mem(macsim_c* m_simBase) {
  string policy = m_simBase->m_knobs->KNOB_MEMORY_TYPE->getValue();
  memory_c* new_mem;
  if (policy == "llc_coupled_network") {
    new_mem = new llc_coupled_network_c(m_simBase);
  } else if (policy == "llc_decoupled_network") {
    new_mem = new llc_decoupled_network_c(m_simBase);
  } else if (policy == "l2_coupled_local") {
    new_mem = new l2_coupled_local_c(m_simBase);
  } else if (policy == "no_cache") {
    new_mem = new no_cache_c(m_simBase);
  } else if (policy == "l2_decoupled_network") {
    new_mem = new l2_decoupled_network_c(m_simBase);
  } else if (policy == "l2_decoupled_local") {
    new_mem = new l2_decoupled_local_c(m_simBase);
  } else if (policy == "igpu_network") {
    new_mem = new igpu_network_c(m_simBase);
  } else {
    ASSERT(0);
  }

  return new_mem;
}

// Default LLC constructor function
cache_c* default_llc(macsim_c* m_simBase) {
  string llc_type = KNOB(KNOB_LLC_TYPE)->getValue();
  assert(llc_type == "default");

  int num_tiles;
  int interleaving = -1;

  if (*KNOB(KNOB_DEFAULT_INTERLEAVING)) {
    num_tiles = 1;
    interleaving = 0;
  } else {
    num_tiles = *KNOB(KNOB_NUM_LLC);
    if (*KNOB(KNOB_NEW_INTERLEAVING_DIFF_GRANULARITY)) {
      interleaving = *m_simBase->m_knobs->KNOB_LLC_LINE_SIZE;
    } else if (*KNOB(KNOB_NEW_INTERLEAVING_SAME_GRANULARITY)) {
      interleaving = *m_simBase->m_knobs->KNOB_DRAM_ROWBUFFER_SIZE;
    }
  }

  cache_c* llc = new cache_c(
    "llc_default", *KNOB(KNOB_LLC_NUM_SET), *KNOB(KNOB_LLC_ASSOC),
    *KNOB(KNOB_LLC_LINE_SIZE), sizeof(dcache_data_s), *KNOB(KNOB_LLC_NUM_BANK),
    false, 0, CACHE_DL1, false, num_tiles, interleaving, m_simBase);
  return llc;
}

///////////////////////////////////////////////////////////////////////////////////////////////

bool queue_c::sort_func::operator()(mem_req_s* a, mem_req_s* b) {
  if (*m_simBase->m_knobs->KNOB_HETERO_MEM_PRIORITY_CPU) {
    if (a->m_acc != true && b->m_acc == true) {
      return true;
    } else if (a->m_acc == true && b->m_acc != true)
      return false;
  } else if (*m_simBase->m_knobs->KNOB_HETERO_MEM_PRIORITY_GPU) {
    if (a->m_acc != true && b->m_acc == true) {
      return false;
    } else if (a->m_acc == true && b->m_acc != true)
      return true;
  }

  return a->m_priority > b->m_priority;
}

queue_c::queue_c(macsim_c* simBase, int size) {
  m_simBase = simBase;
  m_size = size;
}

queue_c::~queue_c() {
}

// search an entry with address and size.
mem_req_s* queue_c::search(Addr addr, int size) {
  for (auto I = m_entry.begin(), E = m_entry.end(); I != E; ++I) {
    mem_req_s* req = (*I);

    // matching should be inclusive
    if (req->m_addr <= addr && req->m_addr + req->m_size >= addr + size) {
      return req;
    }
  }

  return NULL;
}

// search a request.
bool queue_c::search(mem_req_s* req) {
  for (auto I = m_entry.begin(), E = m_entry.end(); I != E; ++I) {
    if ((*I) == req) return true;
  }

  return false;
}

// delete an entry.
void queue_c::pop(mem_req_s* req) {
  m_entry.remove(req);
}

// insert en entry.
bool queue_c::push(mem_req_s* req) {
  if (m_entry.size() == m_size) return false;

  req->m_queue = this;
  m_entry.push_back(req);
  m_entry.sort(sort_func(m_simBase));

  return true;
}

// check buffer space.
bool queue_c::full() {
  if (m_entry.size() == m_size) return true;
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////

// data cache constructor.
dcu_c::dcu_c(int id, Unit_Type type, int level, memory_c* mem, int noc_id,
             dcu_c** next, dcu_c** prev, macsim_c* simBase) {
  m_simBase = simBase;

  CREATE_CACHE_CONFIGURATION();

  // instantiate queues
  m_in_queue = new queue_c(simBase, 1024);
  m_wb_queue = new queue_c(simBase, 2048);
  m_fill_queue = new queue_c(simBase, 1024);
  m_out_queue = new queue_c(simBase, 2048);

  m_id = id;
  m_noc_id = noc_id;
  m_type = type;
  m_level = level;
  m_memory = mem;
  m_next = next;
  m_prev = prev;
  m_bypass = false;

  // clock cycle
  m_cycle = 0;

  m_cache = NULL;
  m_port = NULL;
}

// dcu_c destructor.
dcu_c::~dcu_c() {
  if (m_disable) return;

  if (m_cache) delete m_cache;

  if (m_port) {
    for (int ii = 0; ii < m_banks; ++ii)
      if (m_port[ii]) delete m_port[ii];

    delete[] m_port;
  }
}

// initialize data cache.
void dcu_c::init(int next_id, int prev_id, bool done, bool coupled_up,
                 bool coupled_down, bool disable, bool has_router) {
  m_next_id = next_id;
  m_prev_id = prev_id;
  m_coupled_up = coupled_up;
  m_coupled_down = coupled_down;
  m_done = done;
  m_disable = disable;
  m_has_router = has_router;

  if (!m_disable) {
    if (m_level == MEM_LLC) {
      string llc_policy = *KNOB(KNOB_LLC_TYPE);
      m_cache = llc_factory_c::get()->allocate(llc_policy, m_simBase);
      m_cache->set_core_id(m_id);
    } else {
      m_cache =
        new cache_c("dcache", m_num_set, m_assoc, m_line_size,
                    sizeof(dcache_data_s), m_banks, false, m_id, CACHE_DL1,
                    m_level == MEM_L3 ? true : false, 1, 0, m_simBase);
    }

    // allocate port
    m_port = new port_c*[m_banks];
    for (int ii = 0; ii < m_banks; ++ii) {
      m_port[ii] = new port_c("dcache_port", m_num_read_port, m_num_write_port,
                              false, m_simBase);
    }
  } else {
    m_latency = 1;
  }

  if (m_bypass == true) {
    m_disable = true;
  }
}

// check input queue availability.
bool dcu_c::full(void) {
  return m_in_queue->full();
}

Addr dcu_c::base_addr(Addr addr) {
  // nbl - taking advantage of 2's complement representation
  return (addr & -m_line_size);
}

// get cache line size.
int dcu_c::line_size() {
  return m_line_size;
}

// get bank id.
int dcu_c::bank_id(Addr addr) {
  return BANK(addr, m_banks, 256);
}

// acquire read port.
bool dcu_c::get_read_port(int bank_id) {
  return m_port[bank_id]->get_read_port(m_cycle);
}

// access the cache.
dcache_data_s* dcu_c::access_cache(Addr addr, Addr* line_addr, bool update,
                                   int appl_id) {
  return (dcache_data_s*)m_cache->access_cache(addr, line_addr, update,
                                               appl_id);
}

// search a prefetch request in input queue.
mem_req_s* dcu_c::search_pref_in_queue() {
  mem_req_s* evict_req = NULL;
  for (auto I = m_in_queue->m_entry.rbegin(), E = m_in_queue->m_entry.rend();
       I != E; ++I) {
    if ((*I)->m_type == MRT_DPRF) {
      evict_req = (*I);
      break;
    }
  }

  if (evict_req) {
    m_in_queue->m_entry.remove(evict_req);
  }

  return evict_req;
}

// DCACHE (L1) access from the pipeline stage
// If miss in the cache, it will go thru the memory system.
int dcu_c::access(uop_c* uop) {
  ASSERT(m_level == MEM_L1);
  DEBUG_CORE(uop->m_core_id, "L%d[%d] uop_num:%lld access\n", m_level, m_id,
             uop->m_uop_num);

  if (*m_simBase->m_knobs->KNOB_ENABLE_PHYSICAL_MAPPING) {
    bool success = m_simBase->m_MMU->translate(uop);
    if (!success) return -1;  // treat a TLB miss as a longer latency cache miss
  } else
    uop->m_paddr = uop->m_vaddr;

  uop->m_state = OS_DCACHE_BEGIN;

  dcache_data_s* line = NULL;

  Addr vaddr = uop->m_paddr;
  Mem_Type type = uop->m_mem_type;
  Addr line_addr = base_addr(vaddr);
  int bank = BANK(vaddr, m_banks, 256);

  // -------------------------------------
  // DCACHE port access
  // -------------------------------------
  if (*m_simBase->m_knobs->KNOB_DCACHE_INFINITE_PORT || m_disable == true) {
    // do nothing
  } else if (IsStore(type) && !m_port[bank]->get_write_port(m_cycle - 1)) {
    // port busy
    STAT_EVENT(CACHE_BANK_BUSY);
    uop->m_dcache_bank_id = bank + 64;
    uop->m_state = OS_DCACHE_PORT_UNAVAILABLE;
    return 0;
  } else if (IsLoad(type) && !m_port[bank]->get_read_port(m_cycle - 1)) {
    // port busy
    STAT_EVENT(CACHE_BANK_BUSY);
    uop->m_dcache_bank_id = bank;
    uop->m_state = OS_DCACHE_PORT_UNAVAILABLE;
    return 0;
  }
  DEBUG_CORE(uop->m_core_id,
             "L%d[%d] uop_num:%llu addr:0x%llx port:%d acquired\n", m_level,
             m_id, uop->m_uop_num, line_addr, bank);

  // -------------------------------------
  // DCACHE access
  // -------------------------------------
  bool cache_hit = false;
  if (*m_simBase->m_knobs->KNOB_PERFECT_DCACHE) {
    cache_hit = true;
  } else if (m_disable == true) {
    cache_hit = false;
  } else {
    int appl_id =
      m_simBase->m_core_pointers[uop->m_core_id]->get_appl_id(uop->m_thread_id);
    line =
      (dcache_data_s*)m_cache->access_cache(vaddr, &line_addr, true, appl_id);
    cache_hit = (line) ? true : false;

    if (m_level != MEM_LLC) {
      POWER_CORE_EVENT(uop->m_core_id, POWER_DCACHE_R_TAG + (m_level - 1));
    } else {
      POWER_EVENT(POWER_LLC_R_TAG);
    }

    // prefetch cache should be here
  }

  // -------------------------------------
  // DCACHE hit
  // -------------------------------------
  if (cache_hit) {
    if (m_level != MEM_LLC) {
      POWER_CORE_EVENT(uop->m_core_id, POWER_DCACHE_R + (m_level - 1));
    } else {
      POWER_EVENT(POWER_LLC_R);
    }
    STAT_EVENT(L1_HIT_CPU + this->m_acc_sim);
    DEBUG_CORE(uop->m_core_id, "L%d[%d] uop_num:%lld cache hit\n", m_level,
               m_id, uop->m_uop_num);
    // stat
    uop->m_uop_info.m_dcmiss = false;

    if (line && IsStore(type)) line->m_dirty = true;

    // -------------------------------------
    // hardware prefetcher training
    // -------------------------------------
    m_simBase->m_core_pointers[uop->m_core_id]->train_hw_pref(
      MEM_L1, uop->m_thread_id, line_addr, uop->m_pc, uop, true);

    if (*m_simBase->m_knobs->KNOB_ENABLE_CACHE_COHERENCE) {
    }

    if (this->m_acc_sim &&
        *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
        type == MEM_ST) {
      // evict global data on write hit in L1
      m_cache->invalidate_cache_line(vaddr);

      int req_size;
      Addr req_addr;
      if (m_acc_sim && *m_simBase->m_knobs->KNOB_BYTE_LEVEL_ACCESS) {
        req_size = uop->m_mem_size;
        req_addr = vaddr;
      } else {
        req_size = m_line_size;
        req_addr = line_addr;
      }

      Mem_Req_Type req_type = MRT_DSTORE;

      function<bool(mem_req_s*)> done_func = dcache_write_ack_wrapper;

      int result = m_simBase->m_memory->new_mem_req(
        req_type, req_addr, req_size, cache_hit, true, m_latency, uop,
        done_func, uop->m_unique_num, NULL, m_id, uop->m_thread_id, m_acc_sim);

      if (!result) {
        uop->m_state = OS_DCACHE_MEM_ACCESS_DENIED;
        return 0;
      }

      return -1;
    } else {
      return m_latency;
    }
  }
  // -------------------------------------
  // DCACHE miss
  // -------------------------------------
  else {  // !cache_hit
    STAT_EVENT(L1_MISS_CPU + this->m_acc_sim);
    DEBUG_CORE(uop->m_core_id, "L%d[%d] uop_num:%lld cache miss\n", m_level,
               m_id, uop->m_uop_num);

    // -------------------------------------
    // hardware prefetcher training
    // -------------------------------------
    if (!m_disable) {
      m_simBase->m_core_pointers[uop->m_core_id]->train_hw_pref(
        MEM_L1, uop->m_thread_id, line_addr, uop->m_pc, uop, false);
    }

    // stat
    uop->m_uop_info.m_dcmiss = true;

    // set type;
    Mem_Req_Type req_type;
    switch (type) {
      case MEM_LD:
      case MEM_LD_LM:
      case MEM_LD_CM:
      case MEM_LD_TM:
        req_type = MRT_DFETCH;
        break;
      case MEM_ST:
      case MEM_ST_LM:
      case MEM_ST_GM:
        req_type = MRT_DSTORE;
        break;
      case MEM_SWPREF_NTA:
      case MEM_SWPREF_T0:
      case MEM_SWPREF_T1:
      case MEM_SWPREF_T2:
      case MEM_PF:
        req_type = MRT_DPRF;
        break;
      default:
        ASSERTM(0, "type:%d\n", type);
    }

    // -------------------------------------
    // set address and size
    // -------------------------------------
    int req_size;
    Addr req_addr;
    if (m_acc_sim && *m_simBase->m_knobs->KNOB_BYTE_LEVEL_ACCESS) {
      req_size = uop->m_mem_size;
      req_addr = vaddr;
    } else {
      req_size = m_line_size;
      req_addr = line_addr;
    }

    // FIXME (jaekyu, 10-26-2011)
    if (m_id == *m_simBase->m_knobs->KNOB_HETERO_GPU_CORE_DISABLE) {
      uop->m_bypass_llc = true;
    }

    if (m_id == *m_simBase->m_knobs->KNOB_HETERO_GPU_CORE_DISABLE + 1) {
      uop->m_skip_llc = true;
    }

    // -------------------------------------
    // Generate a new memory request (MSHR access)
    // -------------------------------------
    function<bool(mem_req_s*)> done_func = NULL;
    if (this->m_acc_sim &&
        *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
        (type == MEM_ST || type == MEM_ST_LM)) {
      done_func = dcache_write_ack_wrapper;
    } else {
      done_func = dcache_fill_line_wrapper;
    }

    int result;
    result = m_simBase->m_memory->new_mem_req(
      req_type, req_addr, req_size, cache_hit,
      (type == MEM_ST_GM || type == MEM_ST_LM), m_latency, uop, done_func,
      uop->m_unique_num, NULL, m_id, uop->m_thread_id, m_acc_sim);

    // -------------------------------------
    // MSHR full
    // -------------------------------------
    if (!result) {
      uop->m_state = OS_DCACHE_MEM_ACCESS_DENIED;
      return 0;
    }
    // -------------------------------------
    // In case of software prefetch, generate pref request and retire the instruction
    // -------------------------------------
    else if (req_type == MRT_DPRF) {
      return m_latency;
    }
  }  // !cache_hit

  return -1;  // cache miss
}

// fill a cache line (fill_queue)
bool dcu_c::fill(mem_req_s* req) {
  macsim_c* m_simBase = req->m_simBase;
  if (m_fill_queue->push(req)) {
    req->m_queue = m_fill_queue;
    req->m_state = MEM_FILL_NEW;
    req->m_rdy_cycle = m_cycle + 1;
    DEBUG_CORE(req->m_core_id, "L%d[%d] (->fill_queue) req:%d type:%s\n",
               m_level, m_id, req->m_id,
               mem_req_c::mem_req_type_name[req->m_type]);

    if (m_level != MEM_LLC) {
      POWER_CORE_EVENT(req->m_core_id,
                       POWER_DCACHE_LINEFILL_BUF_W + m_level - MEM_L1);
    } else {
      POWER_EVENT(POWER_LLC_LINEFILL_BUF_W);
    }
    return true;
  }

  if (req->m_type == MRT_WB)
    DEBUG_CORE(req->m_core_id, "L%d[%d] req:%d type:%s fill queue rejected\n",
               m_level, m_id, req->m_id,
               mem_req_c::mem_req_type_name[req->m_type]);

  return false;
}

// insert to the cache (in_queue)
bool dcu_c::insert(mem_req_s* req) {
  if (m_in_queue->push(req)) {
    req->m_queue = m_in_queue;
    req->m_rdy_cycle = m_cycle + m_latency;
    DEBUG_CORE(req->m_core_id, "L%d[%d] (->in_queue) req:%d type:%s\n", m_level,
               m_id, req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
    return true;
  } else {
    m_retry_queue.emplace_back(req);
  }

  return false;
}

// =======================================
// cache run_a_cycle
// =======================================
void dcu_c::run_a_cycle(bool pll_lock) {
  if (pll_lock) {
    ++m_cycle;
    return;
  }

  if (!m_retry_queue.empty()) {
    for (auto it = m_retry_queue.begin(); it != m_retry_queue.end();
         /* do nothing */) {
      if (m_in_queue->push(*it))
        it = m_retry_queue.erase(it);
      else
        ++it;
    }
  }

  process_wb_queue();
  process_fill_queue();
  process_out_queue();
  process_in_queue();

  if (*KNOB(KNOB_ENABLE_IRIS) || *KNOB(KNOB_ENABLE_NEW_NOC)) receive_packet();

  ++m_cycle;
}

// Main cache access function
// process requests in the input queue
// input queue:
//   1) to access the cache
//   2) requests originated from upper-level cache misses
//   3) will go to the output queue on cache misses
void dcu_c::process_in_queue() {
  list<mem_req_s*> done_list;
  int count = 0;
  for (auto I = m_in_queue->m_entry.begin(), E = m_in_queue->m_entry.end();
       I != E; ++I) {
    if (count == 4) break;

    mem_req_s* req = (*I);

    if (req->m_rdy_cycle > m_cycle) continue;

    if (req->m_type == MRT_IFETCH) {
      POWER_CORE_EVENT(req->m_core_id, POWER_ICACHE_MISS_BUF_R);
      POWER_CORE_EVENT(req->m_core_id, POWER_LOAD_BYPASS);
    } else if (req->m_type == MRT_DSTORE) {
      POWER_CORE_EVENT(req->m_core_id, POWER_DCACHE_MISS_BUF_R);
      POWER_CORE_EVENT(req->m_core_id, POWER_LOAD_BYPASS);
    } else {
      POWER_CORE_EVENT(req->m_core_id, POWER_DCACHE_MISS_BUF_R);
      POWER_CORE_EVENT(req->m_core_id, POWER_LOAD_BYPASS);
    }

    // -------------------------------------
    // Cache access
    // -------------------------------------
    Addr line_addr;
    dcache_data_s* line;
    bool cache_hit = false;
    if (m_level == MEM_LLC && req->m_bypass == true) {
      line = NULL;
      cache_hit = false;
    } else if (!m_disable) {
      // for wb request, do not update lru state in case of the hit
      line = (dcache_data_s*)m_cache->access_cache(
        req->m_addr, &line_addr, req->m_type == MRT_WB ? false : true,
        req->m_appl_id);
      cache_hit = (line) ? true : false;

      if (m_level != MEM_LLC) {
        POWER_CORE_EVENT(req->m_core_id, POWER_DCACHE_R_TAG + (m_level - 1));
      } else {
        POWER_EVENT(POWER_LLC_R_TAG);
      }
    }

    // -------------------------------------
    // Cache hit
    // -------------------------------------
    if (cache_hit) {
      if (m_level != MEM_LLC) {
        POWER_CORE_EVENT(req->m_core_id, POWER_DCACHE_R + (m_level - 1));
      } else {
        POWER_EVENT(POWER_LLC_R);
      }

      // -------------------------------------
      // hardware prefetcher training
      // -------------------------------------
      m_simBase->m_core_pointers[req->m_core_id]->train_hw_pref(
        m_level, req->m_thread_id, req->m_addr, req->m_pc,
        req->m_uop ? req->m_uop : NULL, true);

      STAT_EVENT(L1_HIT_CPU + (m_level - 1) * 4 + req->m_acc);

      if (line && req->m_type == MRT_DSTORE) {
        line->m_dirty = true;
      }

      //      handle_coherence(m_level, false, );

      // -------------------------------------
      // WB reqeust: the line should be changed to the dirty state and retire (no further act.)
      // -------------------------------------
      if (req->m_type == MRT_WB) {
        line->m_dirty = true;
        req->m_done = true;
      }
      // -------------------------------------
      // If done_func is enabled in this level, need to call done_func to fill lower levels
      // -------------------------------------
      else if (m_done) {
        DEBUG_CORE(
          req->m_core_id,
          "L%d[%d] (in_queue->done_func()) req:%d type:%s access hit\n",
          m_level, m_id, req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
        if (req->m_done_func && !req->m_done_func(req)) continue;
        req->m_done = true;
      }
      // -------------------------------------
      // Send a fill request to the upper level via direct path
      // -------------------------------------
      else if ((m_coupled_up && m_prev_id == req->m_cache_id[m_level - 1]) ||
               !m_has_router) {
        ASSERTM(m_level == MEM_LLC, "Level:%d\n", m_level);
        DEBUG_CORE(req->m_core_id,
                   "L%d[%d] (in_queue->L%d[%d]) req:%d type:%s access hit\n",
                   m_level, m_id, m_level - 1, req->m_cache_id[m_level - 1],
                   req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
        if (!m_prev[req->m_cache_id[m_level - 1]]->fill(req)) continue;
      }
      // LLC cache - decoupled
      // : send to l2 cache fill via NoC
      // -------------------------------------
      // Send a fill request to the upper level via direct path
      // -------------------------------------
      else {
        if (!m_out_queue->push(req)) continue;
        DEBUG_CORE(req->m_core_id,
                   "L%d[%d] (in_queue->out_queue) req:%d type:%s access hit\n",
                   m_level, m_id, req->m_id,
                   mem_req_c::mem_req_type_name[req->m_type]);
        req->m_state = MEM_OUT_FILL;
        req->m_rdy_cycle = m_cycle + 1;
      }

      done_list.push_back(req);
      ++count;
    }
    // -------------------------------------
    // Cache miss or Disabled cache
    // -------------------------------------
    else {
      // hardware prefetcher training
      if (!m_disable) {
        m_simBase->m_core_pointers[req->m_core_id]->train_hw_pref(
          m_level, req->m_thread_id, req->m_addr, req->m_pc,
          req->m_uop ? req->m_uop : NULL, false);
        // g_core_pointers[req->m_core_id]->m_hw_pref->train(m_level, req->m_thread_id,
        //    req->m_addr, req->m_pc, req->m_uop ? req->m_uop : NULL, false);
      }

      STAT_EVENT(L1_HIT_CPU + (m_level - 1) * 4 + 2 + req->m_acc);

      //      handle_coherence(m_level, false, );

      // -------------------------------------
      // If there is a direct link from current level and next lower level,
      // directly insert current request to the input queue of lower level
      // -------------------------------------
      if ((m_coupled_down && m_next_id == req->m_cache_id[m_level + 1]) ||
          !m_has_router) {
        ASSERT(
          m_level !=
          MEM_LLC);  // LLC is always connected to memory controllers via noc
        if (!m_next[req->m_cache_id[m_level + 1]]->insert(req)) {
          continue;
        }
        DEBUG_CORE(req->m_core_id,
                   "L%d[%d] (in_queue->L%d[%d]) req:%d type:%s access miss\n",
                   m_level, m_id, m_level + 1, req->m_cache_id[m_level + 1],
                   req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
      }
      // -------------------------------------
      // Because there is no direct link to the next level, send a request thru NoC
      // -------------------------------------
      else {
        if (!m_out_queue->push(req)) {
          continue;
        }
        DEBUG_CORE(req->m_core_id,
                   "L%d[%d] (in_queue->out_queue) req:%d type:%s access miss\n",
                   m_level, m_id, req->m_id,
                   mem_req_c::mem_req_type_name[req->m_type]);
        req->m_state = MEM_OUTQUEUE_NEW;
        req->m_rdy_cycle = m_cycle + 1;
      }

      done_list.push_back(req);
      ++count;
    }
  }

  // -------------------------------------
  // Delete processed requests from the queue
  // -------------------------------------
  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    m_in_queue->pop((*I));
    if ((*I)->m_done == true) {
      mem_req_s* req = *I;
      DEBUG_CORE(
        req->m_core_id,
        "L%d[%d] (in_queue) req:%d type:%s has been completed lat:%lld\n",
        m_level, m_id, req->m_id, mem_req_c::mem_req_type_name[req->m_type],
        m_cycle - req->m_in);

      if (req->m_acc && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
          req->m_type == MRT_DSTORE) {
        m_simBase->m_memory->free_write_req(req);
      } else {
        m_simBase->m_memory->free_req(req->m_core_id, req);
      }
    }
  }

  done_list.clear();
}

// receive a packet from the NoC
void dcu_c::receive_packet(void) {
  if (!m_has_router) return;

  mem_req_s* req = NULL;

  string topology = KNOB(KNOB_NOC_TOPOLOGY)->getValue();
  int num_rounds = 1;
  if (topology == "simple_noc") num_rounds = 2;

  for (int ii = 0; ii < num_rounds; ++ii) {
    req = NETWORK->receive(m_level, m_id);

    if (req != NULL) {
      req->m_state = MEM_NOC_DONE;
      bool insert_done = false;
      if (req->m_msg_type == NOC_FILL || req->m_msg_type == NOC_ACK) {
        insert_done = fill(req);
      } else if (req->m_msg_type == NOC_NEW ||
                 req->m_msg_type == NOC_NEW_WITH_DATA) {
        insert_done = insert(req);
      } else {
        assert(0);
      }

      if (insert_done) {
        NETWORK->receive_pop(m_level, m_id);

        if (*KNOB(KNOB_BUG_DETECTOR_ENABLE)) {
          m_simBase->m_bug_detector->deallocate_noc(req);
        }
      } else {
        if (*KNOB(KNOB_ENABLE_IRIS)) assert(0);
      }
    }
  }
}

// =======================================
// send a packet to the NoC
// =======================================
bool dcu_c::send_packet(mem_req_s* req, int msg_type, int dir) {
  req->m_msg_type = msg_type;

  bool packet_insert = false;
  packet_insert = NETWORK->send(req, m_level, m_id, m_level + dir,
                                req->m_cache_id[m_level + dir]);

  if (packet_insert) {
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE) &&
        (*KNOB(KNOB_ENABLE_IRIS) || *KNOB(KNOB_ENABLE_NEW_NOC))) {
      m_simBase->m_bug_detector->allocate_noc(req);
    }

    req->m_state = MEM_IN_NOC;
    return true;
  } else {
    return false;
  }
}

// process out queue
// output request
//   request that are waiting to be sent to the router
void dcu_c::process_out_queue() {
  list<mem_req_s*> done_list;
  int count = 0;
  for (auto I = m_out_queue->m_entry.begin(), E = m_out_queue->m_entry.end();
       I != E; ++I) {
    if (count == 4) break;

    mem_req_s* req = (*I);

    if (req->m_rdy_cycle > m_cycle) continue;

    ASSERT(m_has_router == true);

    // -------------------------------------
    // NEW request : send to lower level
    // -------------------------------------
    if (req->m_state == MEM_OUTQUEUE_NEW) {
      int msg_type;
      if (req->m_acc && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
          req->m_with_data && m_level != MEM_LLC) {
        // can change if to req->m_type == MRT_DSTORE
        msg_type = NOC_NEW_WITH_DATA;
      } else {
        msg_type = NOC_NEW;
      }
      if (!send_packet(req, msg_type, 1)) continue;
      DEBUG_CORE(req->m_core_id,
                 "L%d[%d]->L%d[%d] (out_queue->noc) req:%d type:%s (new)\n",
                 m_level, m_id, m_level + 1, req->m_cache_id[m_level + 1],
                 req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
      done_list.push_back(req);
      ++count;
    }
    // -------------------------------------
    // FILL request : send to upward
    // -------------------------------------
    else if (req->m_state == MEM_OUT_FILL) {
      int msg_type;
      if (req->m_acc && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
          req->m_with_data && m_level == MEM_LLC) {
        // can change if to req->m_type == MRT_DSTORE
        msg_type = NOC_ACK;
      } else {
        msg_type = NOC_FILL;
      }

      if (!send_packet(req, msg_type, -1)) continue;
      DEBUG_CORE(req->m_core_id,
                 "L%d[%d]->L%d[%d] (out_queue->noc) req:%d type:%s(fill)\n",
                 m_level, m_id, m_level - 1, req->m_cache_id[m_level - 1],
                 req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
      done_list.push_back(req);
      ++count;
    }
    // -------------------------------------
    // WB request : send to lower level
    // -------------------------------------
    else if (req->m_state == MEM_OUT_WB) {
      if (!send_packet(req, NOC_FILL, 1)) continue;
      DEBUG_CORE(req->m_core_id,
                 "L%d[%d]->L%d[%d] (out_queue->noc) req:%d type:%s(fill)\n",
                 m_level, m_id, m_level + 1, req->m_cache_id[m_level + 1],
                 req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
      done_list.push_back(req);
      ++count;
    } else
      ASSERT(0);
  }

  // -------------------------------------
  // Delete processed requests from the queue
  // -------------------------------------
  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    (*I)->m_queue = NULL;
    m_out_queue->pop((*I));
  }

  done_list.clear();
}

// process fill queue
// fill queue
//   request that come from the memory or write back from the upper-level cache
//   to fill a cache line
void dcu_c::process_fill_queue() {
  list<mem_req_s*> done_list;
  int count = 0;

  for (auto I = m_fill_queue->m_entry.begin(), E = m_fill_queue->m_entry.end();
       I != E; ++I) {
    if (count == 4) break;

    // if wb-queue is full, fill request cannot be made
    if (m_wb_queue->full()) break;

    mem_req_s* req = (*I);

    if (req->m_acc && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
        m_level == MEM_L1 && req->m_type == MRT_DSTORE) {
      ASSERTM(m_done && req->m_done_func && req->m_done_func(req),
              "done function failed\n");
      req->m_done = true;
      done_list.push_back(req);
      ++count;
      continue;
    }

    if (m_level != MEM_LLC) {
      POWER_CORE_EVENT(req->m_core_id,
                       POWER_DCACHE_LINEFILL_BUF_R_TAG + m_level - MEM_L1);
    } else {
      POWER_EVENT(POWER_LLC_LINEFILL_BUF_R_TAG);
    }

    if (req->m_rdy_cycle > m_cycle) continue;

    if (m_level != MEM_LLC) {
      POWER_CORE_EVENT(req->m_core_id,
                       POWER_DCACHE_LINEFILL_BUF_R + m_level - MEM_L1);
    } else {
      POWER_EVENT(POWER_LLC_LINEFILL_BUF_R);
    }

    // MEM_FILL_NEW
    // MEM_FILL_WAIT_DONE
    // MEM_FILL_WAIT_FILL
    switch (req->m_state) {
      // -------------------------------------
      // MEM_FILL_NEW : just inserted to the fill queue
      // -------------------------------------
      case MEM_FILL_NEW: {
        Addr line_addr, victim_line_addr;
        dcache_data_s* line = NULL;
        bool cache_hit = true;

        // Access cache to check whether there is the same line in the cache.
        if (!m_disable) {
          line = (dcache_data_s*)m_cache->access_cache(req->m_addr, &line_addr,
                                                       false, req->m_appl_id);
          cache_hit = (line) ? true : false;
        }

        if (!cache_hit) {  // !cache_hit
          // Access write ports
          int bank = m_cache->get_bank_num(req->m_addr);
          if (!m_port[bank]->get_write_port(m_cycle)) {
            STAT_EVENT(CACHE_BANK_BUSY);
            continue;
          }

          // -------------------------------------
          // Insert a cache line
          // -------------------------------------
          dcache_data_s* data;
          data = (dcache_data_s*)m_cache->insert_cache(
            req->m_addr, &line_addr, &victim_line_addr, req->m_appl_id,
            req->m_acc);

          if (m_level != MEM_LLC) {
            POWER_CORE_EVENT(req->m_core_id, POWER_DCACHE_W + (m_level - 1));
          } else {
            POWER_EVENT(POWER_LLC_W);
          }

          // -------------------------------------
          // If there is a victim line, we do the write-back.
          // -------------------------------------
          if (victim_line_addr) {
            if (data->m_dirty) {
              if (*(m_simBase->m_knobs->KNOB_USE_INCOMING_TID_CID_FOR_WB)) {
                data->m_core_id = req->m_core_id;
                data->m_tid = req->m_thread_id;
              }

              // new write-back request
              mem_req_s* wb = m_simBase->m_memory->new_wb_req(
                victim_line_addr, m_line_size, m_acc_sim, data, m_level);

              wb->m_rdy_cycle = m_cycle + 1;

              if (!m_wb_queue->push(wb)) ASSERT(0);

              if (m_level != MEM_LLC) {
                POWER_CORE_EVENT(req->m_core_id,
                                 POWER_DCACHE_WB_BUF_W + m_level - MEM_L1);
              } else {
                POWER_EVENT(POWER_LLC_WB_BUF_W);
              }

              DEBUG_CORE(req->m_core_id,
                         "L%d[%d] (fill_queue) new_wb_req:%d addr:0x%llx "
                         "type:%s by req:%d\n",
                         m_level, m_id, wb->m_id, victim_line_addr,
                         mem_req_c::mem_req_type_name[wb->m_type], req->m_id);
            }
          }

          // -------------------------------------
          // cache line setup
          // -------------------------------------
          data->m_dirty = req->m_dirty;
          data->m_fetch_cycle = m_cycle;
          data->m_core_id = req->m_core_id;
          data->m_pc = req->m_pc;
          data->m_tid = req->m_thread_id;
        } else if (line != NULL) {
          line->m_dirty |= req->m_dirty;
        }

        // L2: done function has been called in this level
        if (m_done == true) {
          ASSERTM(m_level != MEM_LLC, "req:%d type:%s", req->m_id,
                  mem_req_c::mem_req_type_name[req->m_type]);
          if (req->m_done_func && !req->m_done_func(req)) {
            req->m_state = MEM_FILL_WAIT_DONE;
            continue;
          }
          req->m_done = true;
          DEBUG_CORE(
            req->m_core_id,
            "L%d[%d] (fill_queue->done_func()) hit:%d req:%d type:%s filled\n",
            m_level, m_id, cache_hit, req->m_id,
            mem_req_c::mem_req_type_name[req->m_type]);
        } else if (req->m_type == MRT_WB) {
          req->m_done = true;
          DEBUG_CORE(
            req->m_core_id,
            "L%d[%d] (fill_queue->done_func()) hit:%d req:%d type:%s filled\n",
            m_level, m_id, cache_hit, req->m_id,
            mem_req_c::mem_req_type_name[req->m_type]);
        } else {
          // Disabled Cache
          if (m_disable) {
            if (m_coupled_up && m_prev_id == req->m_cache_id[m_level - 1]) {
              if (!m_prev[m_prev_id]->fill(req)) {
                req->m_state = MEM_FILL_WAIT_FILL;
                continue;
              }
              DEBUG_CORE(
                req->m_core_id,
                "L%d[%d] (fill_queue->L%d[%d]) hit:%d req:%d type:%s bypass\n",
                m_level, m_id, m_level - 1, req->m_cache_id[m_level - 1],
                cache_hit, req->m_id,
                mem_req_c::mem_req_type_name[req->m_type]);
            } else {
              if (!m_out_queue->push(req)) {
                req->m_state = MEM_FILL_WAIT_FILL;
                continue;
              }
              req->m_state = MEM_OUT_FILL;
              DEBUG_CORE(req->m_core_id,
                         "L%d[%d] (fill_queue->out_queue) hit:%d req:%d "
                         "type:%s filled\n",
                         m_level, m_id, cache_hit, req->m_id,
                         mem_req_c::mem_req_type_name[req->m_type]);
            }
          }
          // COUPLED LLC OR without router: fill upper level cache
          else if ((m_coupled_up &&
                    m_prev_id == req->m_cache_id[m_level - 1]) ||
                   !m_has_router) {
            if (!m_prev[req->m_cache_id[m_level - 1]]->fill(req)) {
              req->m_state = MEM_FILL_WAIT_FILL;
              continue;
            }
            DEBUG_CORE(
              req->m_core_id,
              "L%d[%d] (fill_queue->L%d[%d]) hit:%d req:%d type:%s filled\n",
              m_level, m_id, m_level - 1, req->m_cache_id[m_level - 1],
              cache_hit, req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
          }
          // DECOUPLED LLC: send to busout queue
          else {
            if (!m_out_queue->push(req)) {
              req->m_state = MEM_FILL_WAIT_FILL;
              continue;
            }
            req->m_state = MEM_OUT_FILL;
            DEBUG_CORE(
              req->m_core_id,
              "L%d[%d] (fill_queue->out_queue) hit:%d req:%d type:%s filled\n",
              m_level, m_id, cache_hit, req->m_id,
              mem_req_c::mem_req_type_name[req->m_type]);
          }
        }
        done_list.push_back(req);
        ++count;
        break;
      }

      // -------------------------------------
      // MEM_FILL_NEW -> MEM_FILL_WAIT_DONE : waiting done_func is successfully done
      // -------------------------------------
      case MEM_FILL_WAIT_DONE: {
        if (req->m_done_func && !req->m_done_func(req)) continue;

        req->m_done = true;
        done_list.push_back(req);
        ++count;
        break;
      }
      // -------------------------------------
      // MEM_FILL_NEW -> MEM_FILL_WAIT_FILL : waiting for sending to next/prev level
      // -------------------------------------
      case MEM_FILL_WAIT_FILL: {
        // COUPLED LLC OR without router: fill upper level cache
        if ((m_coupled_up && m_prev_id == req->m_cache_id[m_level - 1]) ||
            !m_has_router) {
          if (!m_prev[req->m_cache_id[m_level - 1]]->fill(req)) {
            req->m_state = MEM_FILL_WAIT_FILL;
            continue;
          }
          DEBUG_CORE(req->m_core_id,
                     "L%d[%d] (fill_queue->L%d[%d]) req:%d type:%s filled\n",
                     m_level, m_id, m_level - 1, req->m_cache_id[m_level - 1],
                     req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
        }
        // DECOUPLED LLC: send to busout queue
        else {
          if (!m_out_queue->push(req)) {
            req->m_state = MEM_FILL_WAIT_FILL;
            continue;
          }
          req->m_state = MEM_OUT_FILL;
          DEBUG_CORE(req->m_core_id,
                     "L%d[%d] (fill_queue->out_queue) req:%d type:%s filled\n",
                     m_level, m_id, req->m_id,
                     mem_req_c::mem_req_type_name[req->m_type]);
        }
        done_list.push_back(req);
        ++count;
        break;
      }
      default: {
        ASSERTM(0, "req_id:%d state:%d\n", req->m_id, req->m_state);
        assert(0);
        break;
      }
    }
  }

  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    m_fill_queue->pop((*I));
    if ((*I)->m_done == true) {
      mem_req_s* req = *I;
      DEBUG_CORE(
        req->m_core_id,
        "L%d[%d] fill_queue req:%d type:%s has been completed lat:%lld\n",
        m_level, m_id, req->m_id, mem_req_c::mem_req_type_name[req->m_type],
        m_cycle - req->m_in);

      if (req->m_acc && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
          req->m_type == MRT_DSTORE) {
        m_simBase->m_memory->free_write_req(req);
      } else {
        m_memory->free_req(req->m_core_id, req);
      }
    }
  }

  done_list.clear();
}

// process write-back queue
// write-back queue
//   destination would be either the output queue or the fill queue of the next-level cache
void dcu_c::process_wb_queue() {
  list<mem_req_s*> done_list;
  int count = 0;
  for (auto I = m_wb_queue->m_entry.begin(), E = m_wb_queue->m_entry.end();
       I != E; ++I) {
    if (count == 4) break;

    mem_req_s* req = (*I);

    if (m_level != MEM_LLC) {
      POWER_CORE_EVENT(req->m_core_id,
                       POWER_DCACHE_WB_BUF_R_TAG + m_level - MEM_L1);
    } else {
      POWER_EVENT(POWER_LLC_WB_BUF_R_TAG);
    }

    if (req->m_rdy_cycle > m_cycle) continue;

    if (m_level != MEM_LLC) {
      POWER_CORE_EVENT(req->m_core_id,
                       POWER_DCACHE_WB_BUF_R + m_level - MEM_L1);
    } else {
      POWER_EVENT(POWER_LLC_WB_BUF_R);
    }

    // L1 and L2 : insert next level's in_queue
    if (m_level != MEM_LLC && m_coupled_down == true &&
        m_next_id == req->m_cache_id[m_level + 1]) {
      // if (!m_next->insert(req))
      if (!m_next[m_next_id]->fill(req)) continue;
      DEBUG_CORE(req->m_core_id, "L%d[%d] req:%d type:%s inserted to L%d[%d]\n",
                 m_level, m_id, req->m_id,
                 mem_req_c::mem_req_type_name[req->m_type], m_level + 1,
                 req->m_cache_id[m_level + 1]);
    }
    // LLC : send to dram controller
    else {
      if (!m_out_queue->push(req)) continue;
      DEBUG_CORE(req->m_core_id,
                 "L%d[%d] req:%d type:%s send to busout queue\n", m_level, m_id,
                 req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
      req->m_state = MEM_OUT_WB;
    }

    done_list.push_back(req);
    ++count;
  }

  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    m_wb_queue->pop((*I));
  }
  done_list.clear();
}

// done function for DCACHE miss (look process_fill_queue() for L2 and LLC caches) request
// resolve DCACHE miss and set miss-uop ready
bool dcu_c::done(mem_req_s* req) {
  int bank;
  dcache_data_s* data;
  dcache_data_s* line = NULL;
  Addr addr;
  Addr line_addr;
  Addr repl_line_addr;

  if (m_wb_queue->full()) return false;

  if (m_disable) {
    // do nothing
  } else {
    addr = req->m_addr;
    bank = m_cache->get_bank_num(req->m_addr);

    // -------------------------------------
    // DCACHE write port access
    // -------------------------------------
    if (!m_disable && !m_port[bank]->get_write_port(m_cycle)) {
      STAT_EVENT(CACHE_BANK_BUSY);
      return false;
    }

    // for the safety check, do not insert duplicate blocks
    line = (dcache_data_s*)m_cache->access_cache(addr, &line_addr, false,
                                                 req->m_appl_id);

    if (!line) {
      // -------------------------------------
      // DCACHE insertion
      // -------------------------------------
      data = (dcache_data_s*)m_cache->insert_cache(
        addr, &line_addr, &repl_line_addr, req->m_appl_id, req->m_acc);

      if (m_level != MEM_LLC) {
        POWER_CORE_EVENT(req->m_core_id, POWER_DCACHE_W + (m_level - 1));
      } else {
        POWER_EVENT(POWER_LLC_W);
      }

      if (*m_simBase->m_knobs->KNOB_ENABLE_CACHE_COHERENCE) {
      }

      // -------------------------------------
      // evict a line
      // -------------------------------------
      if (repl_line_addr) {
        // STAT_CORE_EVENT(req->m_core_id, POWER_DCACHE_C);
        if (data->m_dirty == 1) {
          if (*(m_simBase->m_knobs->KNOB_USE_INCOMING_TID_CID_FOR_WB)) {
            data->m_core_id = req->m_core_id;
            data->m_tid = req->m_thread_id;
          }

          // new write back request
          mem_req_s* wb = m_simBase->m_memory->new_wb_req(
            repl_line_addr, m_line_size, m_acc_sim, data, m_level);

          wb->m_rdy_cycle = m_cycle + 1;

          // FIXME(jaekyu, 10-26-2011) - queue rejection
          if (!m_wb_queue->push(wb)) ASSERT(0);

          DEBUG_CORE(
            req->m_core_id,
            "L%d[%d] (done) new_wb_req:%d addr:0x%llx by req:%d type:%s\n",
            m_level, m_id, wb->m_id, repl_line_addr, req->m_id,
            mem_req_c::mem_req_type_name[MRT_WB]);

          if (m_level != MEM_LLC) {
            POWER_CORE_EVENT(req->m_core_id,
                             POWER_DCACHE_WB_BUF_W + m_level - MEM_L1);
          } else {
            POWER_EVENT(POWER_LLC_WB_BUF_W);
          }
        }
      }

      data->m_dirty = req->m_dirty;
      data->m_fetch_cycle = m_cycle;
      data->m_core_id = req->m_core_id;
      data->m_pc = req->m_pc;
      data->m_tid = req->m_thread_id;
    } else {
      line->m_dirty |= req->m_dirty;
    }
  }

  if (req->m_uop) {
    uop_c* uop = req->m_uop;
    DEBUG_CORE(req->m_core_id,
               "req_id:%d inst:%lld uop:%lld done in_cycle:%llu\n", req->m_id,
               uop->m_inst_num, uop->m_uop_num, req->m_in_global);
    uop->m_done_cycle = m_simBase->m_core_cycle[uop->m_core_id] + 1;
    uop->m_state = OS_SCHEDULED;
    if (m_acc_sim) {
      if (uop->m_parent_uop) {
        uop_c* puop = uop->m_parent_uop;
        ++puop->m_num_child_uops_done;
        if (puop->m_num_child_uops_done == puop->m_num_child_uops) {
          if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
            m_simBase->m_core_pointers[puop->m_core_id]
              ->get_frontend()
              ->set_load_ready(puop->m_thread_id, puop->m_uop_num);
          }

          puop->m_done_cycle = m_simBase->m_core_cycle[uop->m_core_id] + 1;
          puop->m_state = OS_SCHEDULED;
        }
      }  // uop->m_parent_uop
      else {
        if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
          m_simBase->m_core_pointers[uop->m_core_id]
            ->get_frontend()
            ->set_load_ready(uop->m_thread_id, uop->m_uop_num);
        }
      }
    }
  }

  return true;
}

bool dcu_c::write_done(mem_req_s* req) {
  uop_c* uop = req->m_uop;
  uop->m_done_cycle = m_simBase->m_core_cycle[uop->m_core_id] + 1;
  uop->m_state = OS_SCHEDULED;
  if (m_acc_sim || m_igpu_sim) {
    if (uop->m_parent_uop) {
      uop_c* puop = uop->m_parent_uop;
      ++puop->m_num_child_uops_done;
      if (puop->m_num_child_uops_done == puop->m_num_child_uops) {
        if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
          m_simBase->m_core_pointers[puop->m_core_id]
            ->get_frontend()
            ->set_load_ready(puop->m_thread_id, puop->m_uop_num);
        }

        puop->m_done_cycle = m_simBase->m_core_cycle[uop->m_core_id] + 1;
        puop->m_state = OS_SCHEDULED;
      }
    }  // uop->m_parent_uop
    else {
      if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
        m_simBase->m_core_pointers[uop->m_core_id]
          ->get_frontend()
          ->set_load_ready(uop->m_thread_id, uop->m_uop_num);
      }
    }
  }

  STAT_CORE_EVENT(uop->m_core_id, NUM_WRITE_ACKS);
  STAT_EVENT(TOTAL_WRITE_ACKS);

  return true;
}

void dcu_c::invalidate(Addr addr) {
  m_cache->invalidate_cache_line(addr);
}

///////////////////////////////////////////////////////////////////////////////////////////////

int memory_c::m_unique_id = 0;

// =======================================
// memory_c constructor
// =======================================
memory_c::memory_c(macsim_c* simBase) {
  m_simBase = simBase;
  REPORT("Memory system(%s) has been initialized.\n",
         KNOB(KNOB_MEMORY_TYPE)->getValue().c_str());

  m_num_core = *m_simBase->m_knobs->KNOB_NUM_SIM_CORES;
  m_num_l3 = *m_simBase->m_knobs->KNOB_NUM_L3;
  m_num_llc = *m_simBase->m_knobs->KNOB_NUM_LLC;
  m_num_mc = *m_simBase->m_knobs->KNOB_DRAM_NUM_MC;

  m_num_gpu = 0;
  m_num_cpu = 0;

  if ((KNOB(KNOB_LARGE_CORE_TYPE)->getValue() == "ptx") ||
      (KNOB(KNOB_LARGE_CORE_TYPE)->getValue() == "igpu"))
    m_num_gpu += *KNOB(KNOB_NUM_SIM_LARGE_CORES);
  else
    m_num_cpu += *KNOB(KNOB_NUM_SIM_LARGE_CORES);

  if ((KNOB(KNOB_MEDIUM_CORE_TYPE)->getValue() == "ptx") ||
      (KNOB(KNOB_LARGE_CORE_TYPE)->getValue() == "igpu"))
    m_num_gpu += *KNOB(KNOB_NUM_SIM_MEDIUM_CORES);
  else
    m_num_cpu += *KNOB(KNOB_NUM_SIM_MEDIUM_CORES);

  if ((KNOB(KNOB_CORE_TYPE)->getValue() == "ptx") ||
      (KNOB(KNOB_LARGE_CORE_TYPE)->getValue() == "igpu"))
    m_num_gpu += *KNOB(KNOB_NUM_SIM_SMALL_CORES);
  else
    m_num_cpu += *KNOB(KNOB_NUM_SIM_SMALL_CORES);

  //  ASSERT(m_num_core == m_num_llc);

  // allocate mshr
  m_mshr = new list<mem_req_s*>[m_num_core];
  m_mshr_free_list = new list<mem_req_s*>[m_num_core];

  for (int ii = 0; ii < m_num_core; ++ii) {
    for (int jj = 0; jj < *m_simBase->m_knobs->KNOB_MEM_MSHR_SIZE; ++jj) {
      mem_req_s* entry = new mem_req_s(simBase);
      m_mshr_free_list[ii].push_back(entry);
    }
  }

  m_mem_req_pool = new pool_c<mem_req_s>;

  int num_large_core = *m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES;
  int num_medium_core = *m_simBase->m_knobs->KNOB_NUM_SIM_MEDIUM_CORES;
  int num_small_core = *m_simBase->m_knobs->KNOB_NUM_SIM_SMALL_CORES;

  // allocate caches
  m_l1_cache = new dcu_c*[m_num_core];
  m_l2_cache = new dcu_c*[m_num_core];
  m_l3_cache = new dcu_c*[m_num_l3];
  m_llc_cache = new dcu_c*[m_num_llc];

  int id = 0;
  for (int ii = 0; ii < num_large_core; ++id, ++ii) {
    m_l1_cache[id] =
      new dcu_c(id, UNIT_LARGE, MEM_L1, this, id, m_l2_cache, NULL, simBase);
    m_l2_cache[id] = new dcu_c(id, UNIT_LARGE, MEM_L2, this, id + m_num_core,
                               m_llc_cache, m_l1_cache, simBase);
  }

  for (int ii = 0; ii < num_medium_core; ++id, ++ii) {
    m_l1_cache[id] =
      new dcu_c(id, UNIT_MEDIUM, MEM_L1, this, id, m_l2_cache, NULL, simBase);
    m_l2_cache[id] = new dcu_c(id, UNIT_MEDIUM, MEM_L2, this, id + m_num_core,
                               m_llc_cache, m_l1_cache, simBase);
  }

  for (int ii = 0; ii < num_small_core; ++id, ++ii) {
    m_l1_cache[id] =
      new dcu_c(id, UNIT_SMALL, MEM_L1, this, id, m_l2_cache, NULL, simBase);
    m_l2_cache[id] = new dcu_c(id, UNIT_SMALL, MEM_L2, this, id + m_num_core,
                               m_llc_cache, m_l1_cache, simBase);
  }

  // L3 cache
  id += m_num_core;
  for (int ii = 0; ii < m_num_l3; ++ii, ++id)
    m_l3_cache[ii] = new dcu_c(ii, UNIT_LARGE, MEM_L3, this, id, m_llc_cache,
                               m_l2_cache, simBase);

  // LLC cache
  for (int ii = 0; ii < m_num_llc; ++ii, ++id)
    m_llc_cache[ii] =
      new dcu_c(ii, UNIT_LARGE, MEM_LLC, this, id, NULL, m_l3_cache, simBase);

  m_noc_index_base[MEM_L1] = 0;
  m_noc_index_base[MEM_L2] = m_num_core;
  m_noc_index_base[MEM_L3] = m_num_core * 2;
  m_noc_index_base[MEM_LLC] = m_num_core * 2 + m_num_l3;
  m_noc_index_base[MEM_MC] = m_num_core * 2 + m_num_l3 + m_num_llc;

  // misc
  m_stop_prefetch = 0;
  m_cycle = 0;

  if (*m_simBase->m_knobs->KNOB_DEFAULT_INTERLEAVING) {
    m_l3_interleave_factor = log2_int(*m_simBase->m_knobs->KNOB_L3_NUM_SET) +
                             log2_int(*m_simBase->m_knobs->KNOB_L3_LINE_SIZE);
    m_l3_interleave_factor = static_cast<int>(pow(2, m_l3_interleave_factor));

    m_llc_interleave_factor = log2_int(*m_simBase->m_knobs->KNOB_LLC_NUM_SET) +
                              log2_int(*m_simBase->m_knobs->KNOB_LLC_LINE_SIZE);
    m_llc_interleave_factor = static_cast<int>(pow(2, m_llc_interleave_factor));

    m_dram_interleave_factor =
      log2_int(*m_simBase->m_knobs->KNOB_DRAM_ROWBUFFER_SIZE) +
      log2_int(*m_simBase->m_knobs->KNOB_DRAM_NUM_BANKS);
    m_dram_interleave_factor =
      static_cast<int>(pow(2, m_dram_interleave_factor));
  }
  // diff granularity for LLC and DRAM
  else if (*m_simBase->m_knobs->KNOB_NEW_INTERLEAVING_DIFF_GRANULARITY) {
    m_l3_interleave_factor = *m_simBase->m_knobs->KNOB_L3_LINE_SIZE;
    m_llc_interleave_factor = *m_simBase->m_knobs->KNOB_LLC_LINE_SIZE;
    m_dram_interleave_factor = *m_simBase->m_knobs->KNOB_DRAM_ROWBUFFER_SIZE;
  }
  // same granularity for LLC and DRAM - if #LLC == #mc, then each LLC slice sends request to only one mc
  else if (*m_simBase->m_knobs->KNOB_NEW_INTERLEAVING_SAME_GRANULARITY) {
    m_l3_interleave_factor = *m_simBase->m_knobs->KNOB_DRAM_ROWBUFFER_SIZE;
    m_llc_interleave_factor = *m_simBase->m_knobs->KNOB_DRAM_ROWBUFFER_SIZE;
    m_dram_interleave_factor = *m_simBase->m_knobs->KNOB_DRAM_ROWBUFFER_SIZE;
  } else {
    assert(0);
  }

  m_page_size = *m_simBase->m_knobs->KNOB_PAGE_SIZE;
  m_igpu_sim = false;
}

// memory class destructor
memory_c::~memory_c() {
  for (int ii = 0; ii < m_num_core; ++ii) {
    delete m_l1_cache[ii];
    delete m_l2_cache[ii];
    m_mshr_free_list[ii].clear();
    m_mshr[ii].clear();
  }

  for (int ii = 0; ii < m_num_l3; ++ii) delete m_l3_cache[ii];

  for (int ii = 0; ii < m_num_llc; ++ii) delete m_llc_cache[ii];

  delete[] m_mshr;
  delete[] m_mshr_free_list;
  delete[] m_l1_cache;
  delete[] m_l2_cache;
  delete[] m_l3_cache;
  delete[] m_llc_cache;
}

// =======================================
// initialize the memory system (noc interface)
// =======================================
void memory_c::init(void) {
}

// generate a new memory request
// called from 1) data cache, 2) instruction cache, and 3) prefetcher
bool memory_c::new_mem_req(Mem_Req_Type type, Addr addr, uns size,
                           bool cache_hit, bool with_data, uns delay,
                           uop_c* uop, function<bool(mem_req_s*)> done_func,
                           Counter unique_num, pref_req_info_s* pref_info,
                           int core_id, int thread_id, bool ptx) {
  DEBUG_CORE(core_id, "MSHR[%d] new_req type:%s (%d)\n", core_id,
             mem_req_c::mem_req_type_name[type], (int)m_mshr[core_id].size());

  if (m_stop_prefetch > m_cycle && type == MRT_DPRF) {
    DEBUG_CORE(core_id, "PREFETCHING blocked\n");
    return true;
  }

  // find a matching request
  // search other cores' MSHRs as well since only L1s have MSHRs
  mem_req_s* matching_req = NULL;
  for (int i = 0; i < *KNOB(KNOB_NUM_SIM_CORES); ++i) {
    matching_req = search_req(i, addr, size);
    if (matching_req != NULL) break;
  }

  if (type == MRT_IFETCH) {
    POWER_CORE_EVENT(core_id, POWER_ICACHE_MISS_BUF_R_TAG);
  } else {
    // cache hit will be true only for writes to global memory
    // (write-evict for global memory in L1)
    if (!cache_hit) {
      POWER_CORE_EVENT(core_id, POWER_DCACHE_MISS_BUF_R_TAG);
    }
  }

  if (ptx && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
      type == MRT_DSTORE) {
    STAT_CORE_EVENT(core_id, NUM_WRITES);
    STAT_EVENT(TOTAL_WRITES);
  }

  if (ptx && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
      matching_req && type == MRT_DSTORE) {
    // nbl: TBD dec-20-2012
    // store matching a load, we cannot have a load matching
    // a store because stores are not allocated mshrs
  } else if (matching_req) {
    ASSERT(type != MRT_WB);
    // redundant hardware prefetch request
    if (type == MRT_DPRF) {
      // adjust priority of matching entry
      return true;
    } else if (matching_req->m_type == MRT_DPRF) {
      // promotion from hardware prefetch to demand
      DEBUG_CORE(core_id, "req:%d has been promoted type:%s\n",
                 matching_req->m_id,
                 mem_req_c::mem_req_type_name[matching_req->m_type]);
      adjust_req(matching_req, type, addr, size, delay, uop, done_func,
                 unique_num, g_mem_priority[type], core_id, thread_id, ptx);
      return true;
    } else if (matching_req->m_type == MRT_WB) {
      ASSERT(0);
    }
  }

  // queue full
  if (m_l2_cache[core_id]->full()) {
    DEBUG_CORE(core_id, "QUEUE FULL\n");
    m_stop_prefetch = m_cycle + 500;
    flush_prefetch(core_id);
    if (m_l2_cache[core_id]->full()) {
      STAT_EVENT(MSHR_FULL);
      if (type == MRT_DPRF) {
        return true;
      }
    }
  }

  // allocate an entry
  mem_req_s* new_req = NULL;

  if (ptx && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
      type == MRT_DSTORE) {
    new_req = m_mem_req_pool->acquire_entry(m_simBase);
  } else {
    new_req = allocate_new_entry(core_id);

    // mshr full
    if (new_req == NULL) {
      DEBUG_CORE(core_id, "MSHR FULL\n");
      m_stop_prefetch = m_cycle + 500;
      flush_prefetch(core_id);
      if (type == MRT_DPRF) return true;

      new_req = allocate_new_entry(core_id);

      if (new_req == NULL) {
        STAT_EVENT(MSHR_FULL);
        return false;
      }
    }
  }

  if (type == MRT_IFETCH) {
    POWER_CORE_EVENT(core_id, POWER_ICACHE_MISS_BUF_W);
  } else if (type == MRT_DSTORE) {
    POWER_CORE_EVENT(core_id, POWER_DCACHE_MISS_BUF_W);
  } else {
    POWER_CORE_EVENT(core_id, POWER_DCACHE_MISS_BUF_W);
  }
  STAT_EVENT(TOTAL_MEMORY);
  POWER_EVENT(POWER_MC_R);

  Counter priority = g_mem_priority[type];

  // init new request
  init_new_req(new_req, type, addr, size, with_data, delay, uop, done_func,
               unique_num, priority, core_id, thread_id, ptx);

  // merge to existing request
  if (ptx && *m_simBase->m_knobs->KNOB_COMPUTE_CAPABILITY == 2.0f &&
      matching_req && type == MRT_DSTORE) {
    // nbl: TBD - dec-20-2012
    // store matching a load, we cannot have a load matching
    // a store because stores are not allocated mshrs
  } else if (matching_req) {
    STAT_EVENT(TOTAL_MEMORY_MERGE);
    DEBUG_CORE(
      core_id,
      "req:%d addr:0x%llx has matching entry req:%d addr:0x%llx type:%s\n",
      new_req->m_id, new_req->m_addr, matching_req->m_id, matching_req->m_addr,
      mem_req_c::mem_req_type_name[matching_req->m_type]);

    matching_req->m_merge.push_back(new_req);
    new_req->m_merged_req = matching_req;
    new_req->m_state = MEM_MERGED;

    // adjust priority
    if (matching_req->m_priority < priority)
      matching_req->m_priority = priority;

    return true;
  }

  // insert to queue
  m_l2_cache[core_id]->insert(new_req);

  return true;
}

// allocate a new memory request
mem_req_s* memory_c::allocate_new_entry(int core_id) {
  if (m_mshr_free_list[core_id].empty()) return NULL;

  mem_req_s* new_req = m_mshr_free_list[core_id].back();
  m_mshr_free_list[core_id].pop_back();
  m_mshr[core_id].push_back(new_req);

  return new_req;
}

// search matching request
mem_req_s* memory_c::search_req(int core_id, Addr addr, int size) {
  for (auto I = m_mshr[core_id].begin(), E = m_mshr[core_id].end(); I != E;
       ++I) {
    mem_req_s* req = (*I);
    if (req->m_addr <= addr && req->m_addr + req->m_size >= addr + size) {
      return req;
    }
  }

  return NULL;
}

// initialize a new request
void memory_c::init_new_req(mem_req_s* req, Mem_Req_Type type, Addr addr,
                            int size, bool with_data, int delay, uop_c* uop,
                            function<bool(mem_req_s*)> done_func,
                            Counter unique_num, Counter priority, int core_id,
                            int thread_id, bool ptx) {
  req->m_id = m_unique_id++;
  req->m_appl_id = m_simBase->m_core_pointers[core_id]->get_appl_id(thread_id);
  req->m_core_id = core_id;
  req->m_thread_id = thread_id;
  req->m_block_id = 0;
  req->m_state = MEM_NEW;
  req->m_type = type;
  req->m_priority = priority;
  req->m_addr = addr;
  req->m_size = size;
  req->m_with_data = with_data;
  req->m_rdy_cycle = m_cycle + delay;
  req->m_pc = uop ? uop->m_pc : 0;
  req->m_prefetcher_id = 0;
  req->m_pref_loadPC = 0;
  req->m_acc = ptx;
  req->m_done_func = done_func;
  req->m_uop = uop ? uop : NULL;
  if (type == MRT_DPRF) req->m_uop = NULL;
  req->m_in = m_cycle;
  req->m_core_in = m_simBase->m_core_cycle[core_id];
  req->m_in_global = CYCLE;
  req->m_dirty = false;
  req->m_done = false;
  req->m_merged_req = NULL;
  req->m_bypass = uop ? uop->m_bypass_llc : false;
  req->m_skip = uop ? uop->m_skip_llc : false;

  ASSERT(req->m_merge.empty());

  set_cache_id(req);
}

// adjust a new request
void memory_c::adjust_req(mem_req_s* req, Mem_Req_Type type, Addr addr,
                          int size, int delay, uop_c* uop,
                          function<bool(mem_req_s*)> done_func,
                          Counter unique_num, Counter priority, int core_id,
                          int thread_id, bool ptx) {
  req->m_appl_id = m_simBase->m_core_pointers[core_id]->get_appl_id(thread_id);
  ;
  req->m_core_id = core_id;
  req->m_thread_id = thread_id;
  req->m_block_id = 0;
  req->m_type = type;
  req->m_priority = priority;
  req->m_addr = addr;
  req->m_size = size;
  req->m_pc = uop ? uop->m_pc : 0;
  req->m_prefetcher_id = 0;
  req->m_pref_loadPC = 0;
  req->m_acc = ptx;
  req->m_done_func = done_func;
  req->m_uop = uop ? uop : NULL;
  if (type == MRT_DPRF) req->m_uop = NULL;
  req->m_in = m_cycle;
  req->m_core_in = m_simBase->m_core_cycle[core_id];
  req->m_in_global = CYCLE;
  req->m_dirty = false;
  req->m_done = false;
  req->m_merged_req = NULL;
  req->m_bypass = false;
  req->m_skip = false;

  set_cache_id(req);
}

// set each cache-level id : L1,L2: private to core, LLC: shared by cores
void memory_c::set_cache_id(mem_req_s* req) {
  req->m_cache_id[MEM_L1] = req->m_core_id;
  req->m_cache_id[MEM_L2] = req->m_core_id;
  req->m_cache_id[MEM_L3] = BANK(req->m_addr, m_num_l3, m_l3_interleave_factor);
  req->m_cache_id[MEM_LLC] =
    BANK(req->m_addr, m_num_llc, m_llc_interleave_factor);
  req->m_cache_id[MEM_MC] =
    BANK(req->m_addr, m_num_mc, *KNOB(KNOB_DRAM_INTERLEAVE_FACTOR));
}

// deallocate a memory request
void memory_c::free_req(int core_id, mem_req_s* req) {
  STAT_EVENT(AVG_MEMORY_LATENCY_BASE);
  STAT_EVENT_N(AVG_MEMORY_LATENCY, m_cycle - req->m_in);

  // when there are still merged requests, call done wrapper function
  if (!req->m_merge.empty()) {
    DEBUG_CORE(req->m_core_id, "req:%d has merged req type:%s\n", req->m_id,
               mem_req_c::mem_req_type_name[req->m_type]);
    dcache_fill_line_wrapper(req);
  }

  ASSERTM(req->m_merge.empty(), "type:%s\n",
          mem_req_c::mem_req_type_name[req->m_type]);

  if (req->m_type == MRT_WB) {
    delete req;
  } else {
    req->init();
    m_mshr[core_id].remove(req);
    m_mshr_free_list[core_id].push_back(req);
  }
}

void memory_c::free_write_req(mem_req_s* req) {
  STAT_EVENT(AVG_MEMORY_LATENCY_BASE);
  STAT_EVENT_N(AVG_MEMORY_LATENCY, m_cycle - req->m_in);

  m_mem_req_pool->release_entry(req);
}

// get number of available mshr entries
int memory_c::get_num_avail_entry(int core_id) {
  return m_mshr_free_list[core_id].size();
}

// access L1 cache from execution stage
int memory_c::access(uop_c* uop) {
  return m_l1_cache[uop->m_core_id]->access(uop);
}

Addr memory_c::base_addr(int core_id, Addr addr) {
  // nbl - taking advantage of 2's complement representation
  return (addr & -line_size(core_id));
}

// get cache line size
int memory_c::line_size(int core_id) {
  return m_l1_cache[core_id]->line_size();
}

// from level id, get destination noc id
int memory_c::get_dst_id(int level, int id) {
  return m_noc_index_base[level] + id;
}

// cache done function
bool memory_c::done(mem_req_s* req) {
  return m_l1_cache[req->m_cache_id[MEM_L1]]->done(req);
}

bool memory_c::write_done(mem_req_s* req) {
  return m_l1_cache[req->m_cache_id[MEM_L1]]->write_done(req);
}

// get cache bank id
int memory_c::bank_id(int core_id, Addr addr) {
  return m_l2_cache[core_id]->bank_id(addr);
}

// get a cache read port
bool memory_c::get_read_port(int core_id, int bank_id) {
  return m_l1_cache[core_id]->get_read_port(bank_id);
}

// access data cache
dcache_data_s* memory_c::access_cache(int core_id, Addr addr, Addr* line_addr,
                                      bool update, int appl_id) {
  return m_l1_cache[core_id]->access_cache(addr, line_addr, update, appl_id);
}

// memory run_a_cycle
void memory_c::run_a_cycle(bool pll_lock) {
  run_a_cycle_uncore(pll_lock);
  ++m_cycle;
}

void memory_c::run_a_cycle_core(int core_id, bool pll_lock) {
  m_l2_cache[core_id]->run_a_cycle(pll_lock);
  m_l1_cache[core_id]->run_a_cycle(pll_lock);
}

void memory_c::run_a_cycle_uncore(bool pll_lock) {
  int index = m_cycle % m_num_llc;
  for (int ii = index; ii < index + m_num_llc; ++ii)
    m_llc_cache[ii % m_num_llc]->run_a_cycle(pll_lock);

  index = m_cycle % m_num_l3;
  for (int ii = index; ii < index + m_num_l3; ++ii)
    m_l3_cache[ii % m_num_l3]->run_a_cycle(pll_lock);
}

// evict a prefetch request
mem_req_s* memory_c::evict_prefetch(int core_id) {
  mem_req_s* evict = m_l2_cache[core_id]->search_pref_in_queue();
  if (evict != NULL) {
    DEBUG_CORE(core_id, "pref_req:%d has been evicted.\n", evict->m_id);
  }

  return evict;
}

// new write-back request
#define GET_APPL_ID(xx, yy) \
  (m_simBase->m_core_pointers[(xx)]->get_appl_id((yy)))
mem_req_s* memory_c::new_wb_req(Addr addr, int size, bool ptx,
                                dcache_data_s* data, int level) {
  STAT_EVENT(TOTAL_WB);
  STAT_EVENT(L1_WB + (level - 1));
  mem_req_s* req = new mem_req_s(m_simBase);

  req->m_id = m_unique_id++;
  req->m_appl_id = GET_APPL_ID(data->m_core_id, data->m_tid);
  req->m_core_id = data->m_core_id;
  req->m_thread_id = data->m_tid;
  req->m_block_id = 0;
  req->m_state = MEM_NEW;
  req->m_type = MRT_WB;
  req->m_priority = g_mem_priority[MRT_WB];
  req->m_addr = addr;
  req->m_size = size;
  req->m_rdy_cycle = m_cycle + 1;
  req->m_pc = data->m_pc;
  req->m_prefetcher_id = 0;
  req->m_pref_loadPC = 0;
  req->m_acc = ptx;
  req->m_done_func = NULL;
  req->m_uop = NULL;
  req->m_in = m_cycle;
  req->m_in_global = CYCLE;
  req->m_core_in = m_simBase->m_core_cycle[data->m_core_id];
  req->m_dirty = true;
  req->m_done = false;

  set_cache_id(req);
  // req->m_cache_id[MEM_L1] = data->m_core_id;
  // req->m_cache_id[MEM_L2] = data->m_core_id;
  // req->m_cache_id[MEM_LLC] = BANK(addr, m_num_llc, m_llc_interleave_factor);
  // req->m_cache_id[MEM_MC] = BANK(addr, m_num_mc, *KNOB(KNOB_DRAM_INTERLEAVE_FACTOR));

  return req;
}

// print all mshr entries
void memory_c::print_mshr(void) {
  // print noc entries
  m_simBase->m_bug_detector->print_noc();

  // print mshr
  FILE* fp = fopen("bug_detect_mem.out", "w");
  for (int ii = 0; ii < m_num_core; ++ii) {
    fprintf(fp, "== Core %d ==\n", ii);
    fprintf(fp, "%-20s %-10s %-10s %-15s %-15s %-7s %-20s %-15s %-15s\n", "ID",
            "IN_TIME", "DELTA", "TYPE", "STATE", "MERGED", "MERGED_ID",
            "MERGED_TYPE", "MERGED_STATE");

    for (auto I = m_mshr[ii].begin(), E = m_mshr[ii].end(); I != E; ++I) {
      mem_req_s* req = (*I);
      fprintf(
        fp, "%-20d %-10llu %-10llu %-15s %-15s %-7d %-20d %-15s %-15s\n",
        req->m_id, req->m_in, m_cycle - req->m_in,
        mem_req_c::mem_req_type_name[req->m_type],
        mem_req_c::mem_state[req->m_state], (req->m_merged_req ? 1 : 0),
        (req->m_merged_req ? req->m_merged_req->m_id : -1),
        (req->m_merged_req
           ? mem_req_c::mem_req_type_name[req->m_merged_req->m_type]
           : "NULL"),
        (req->m_merged_req ? mem_req_c::mem_state[req->m_merged_req->m_state]
                           : NULL));
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

// flush all prefetches in the mshr
void memory_c::flush_prefetch(int core_id) {
  list<mem_req_s*> done_list;
  for (auto I = m_mshr[core_id].begin(), E = m_mshr[core_id].end(); I != E;
       ++I) {
    if ((*I)->m_type == MRT_DPRF && (*I)->m_merge.empty()) {
      done_list.push_back((*I));
    }
  }

  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    if ((*I)->m_queue != NULL) {
      (*I)->m_queue->pop((*I));
      free_req(core_id, (*I));
    }
  }
}

void memory_c::handle_coherence(int level, bool hit, bool store, Addr addr,
                                dcu_c* cache) {
  int state = I_STATE;

  // READ Hit
  if (!store && hit) {
    if (level == MEM_L1) {
      assert(state != I_STATE);
      // do nothing
    } else if (level == MEM_L2) {
      assert(state != I_STATE);
      if (state == M_STATE) {
        // L2 : Write-back to memory
        // L2 -> L1
        // Update TD L1:S L2:S
        // Lock TD // Free when L1 insert
      } else if (state == S_STATE) {
        // L2 -> L1
        // Update TD L1:S L2:S
        // Lock TD // Free when L1 insert
      }
    } else if (level == MEM_LLC) {
      assert(state != I_STATE);
      if (state == M_STATE) {
      } else if (state == S_STATE) {
      }
    }
  }

  // Write Hit
  if (store && hit) {
    if (level == MEM_L1) {
    } else if (level == MEM_L2) {
    } else if (level == MEM_LLC) {
    }
  }

  // Read Miss
  if (!store && !hit && level == MEM_LLC) {
  }

  // Write Miss
  if (store && !hit && level == MEM_LLC) {
  }
}

void memory_c::invalidate(Addr page_addr) {
}

#if 0
void memory_c::handle_coherence()
{
  if (*KNOB(KNOB_ENABLE_CACHE_COHERENCE) == false)
    return ;

  // assume that all write-back requests are in M-state (single-copy in the system)
  if (write_back == true)
    return ;

  int state = m_memory->get_td_state(req->m_addr); 
  if (m_level == MEM_LLC) {
    // LLC Read Miss
    if (!store) {
      if (state == COHE_M) {
        // TD -> M-state block : forward-data-cmd
        // M-state block -> Memory controller : write-back
        // M-state block -> Requestor : forward-data
        // Requestor -> TD : confirmation
        // Update TD
      }
      else if (state == COHE_S) {
        // TD -> any S-state block : forward-data-cmd
        // Any S-state block -> Requestor : forward-data
        // Requestor -> TD : confirmation
        // Update TD
      }
      else if (state == COHE_I) {
        // Memory controller -> L1, L2, LLC : provide-data
        // L1 -> TD : confirmation
        // Update TD
      }
    }
    // LLC Write Miss
    else {
      if (state == COHE_M) {
        // TD -> M-state block : forward data cmd
        // M-state block -> requestor : forward data / invalidate self
        // Requestor -> TD : confirmation
        // Update TD
      }
      else if (state == COHE_S) {
        // TD -> one S-state block : forward data cmd
        // TD -> all S-state blocks : invalidation
        // One S-state block -> requestor : forward data / invalidate self
        // Requestor -> TD : confirmation
        // Update TD
      }
      else if (state == COHE_I) {
        // Memory controller -> L1 : provide-data
        // L1 -> TD : confirmation
        // Update TD
      }
    }
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////

llc_coupled_network_c::llc_coupled_network_c(macsim_c* simBase)
  : memory_c(simBase) {
  ASSERT(m_num_core == m_num_llc);
  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_core; ++ii) {
    m_l1_cache[ii]->init(ii, -1, false, false, true, false, false);
    m_l2_cache[ii]->init(ii, ii, true, true, true, false, true);
    m_llc_cache[ii]->init(-1, ii, false, true, false, false, true);
  }

  NETWORK->init(m_num_cpu, m_num_gpu, m_num_l3, m_num_llc, m_num_mc);
}

llc_coupled_network_c::~llc_coupled_network_c() {
}

///////////////////////////////////////////////////////////////////////////////////////////////

llc_decoupled_network_c::llc_decoupled_network_c(macsim_c* simBase)
  : memory_c(simBase) {
  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_core; ++ii) {
    m_l1_cache[ii]->init(ii, -1, false, false, true, false, false);
    m_l2_cache[ii]->init(-1, ii, true, true, false, false, true);
  }

  for (int ii = 0; ii < m_num_l3; ++ii)
    m_l3_cache[ii]->init(-1, -1, false, false, false, true, true);

  for (int ii = 0; ii < m_num_llc; ++ii)
    m_llc_cache[ii]->init(-1, -1, false, false, false, false, true);

  NETWORK->init(m_num_cpu, m_num_gpu, m_num_l3, m_num_llc, m_num_mc);
}

llc_decoupled_network_c::~llc_decoupled_network_c() {
}

///////////////////////////////////////////////////////////////////////////////////////////////

// FIXME
l2_coupled_local_c::l2_coupled_local_c(macsim_c* simBase) : memory_c(simBase) {
  ASSERT(m_num_core == m_num_llc);
  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_core; ++ii) {
    m_l1_cache[ii]->init(ii, -1, false, false, true, false, false);
    m_l2_cache[ii]->init(ii, ii, true, true, true, false, true);
    m_llc_cache[ii]->init(-1, ii, false, true, false, true, true);
  }

  NETWORK->init(0, 0, m_num_l3, m_num_llc, m_num_mc);
}

l2_coupled_local_c::~l2_coupled_local_c() {
}

void l2_coupled_local_c::set_cache_id(mem_req_s* req) {
  req->m_cache_id[MEM_L1] = req->m_core_id;
  req->m_cache_id[MEM_L2] = req->m_core_id;
  req->m_cache_id[MEM_LLC] = req->m_core_id;
  req->m_cache_id[MEM_MC] = BANK(
    req->m_addr, m_num_mc, *m_simBase->m_knobs->KNOB_DRAM_INTERLEAVE_FACTOR);
}

///////////////////////////////////////////////////////////////////////////////////////////////

no_cache_c::no_cache_c(macsim_c* simBase) : memory_c(simBase) {
  ASSERT(m_num_core == m_num_llc);
  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_core; ++ii) {
    m_l1_cache[ii]->init(ii, -1, false, false, true, true, false);
    m_l2_cache[ii]->init(ii, ii, true, true, true, true, false);
    m_llc_cache[ii]->init(-1, ii, false, true, false, true, true);
  }

  NETWORK->init(0, 0, m_num_l3, m_num_llc, m_num_mc);
}

no_cache_c::~no_cache_c() {
}

void no_cache_c::set_cache_id(mem_req_s* req) {
  req->m_cache_id[MEM_L1] = req->m_core_id;
  req->m_cache_id[MEM_L2] = req->m_core_id;
  req->m_cache_id[MEM_LLC] = req->m_core_id;
  req->m_cache_id[MEM_MC] = BANK(
    req->m_addr, m_num_mc, *m_simBase->m_knobs->KNOB_DRAM_INTERLEAVE_FACTOR);
}

///////////////////////////////////////////////////////////////////////////////////////////////

l2_decoupled_network_c::l2_decoupled_network_c(macsim_c* simBase)
  : memory_c(simBase) {
  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_core; ++ii) {
    m_l1_cache[ii]->init(ii, -1, false, false, true, false, false);
    m_l2_cache[ii]->init(-1, ii, true, true, false, true, true);
  }

  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_l3; ++ii)
    m_l3_cache[ii]->init(-1, -1, false, false, false, true, true);

  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_llc; ++ii)
    m_llc_cache[ii]->init(-1, -1, false, false, false, false, true);

  NETWORK->init(m_num_cpu, m_num_gpu, m_num_l3, m_num_llc, m_num_mc);
}

l2_decoupled_network_c::~l2_decoupled_network_c() {
}

void l2_decoupled_network_c::set_cache_id(mem_req_s* req) {
  req->m_cache_id[MEM_L1] = req->m_core_id;
  req->m_cache_id[MEM_L2] = req->m_core_id;

  if ((m_num_l3 & (m_num_l3 - 1)) == 0)  // if m_num_l3 is a power of 2
    req->m_cache_id[MEM_L3] =
      BANK(req->m_addr, m_num_l3, m_l3_interleave_factor);
  else
    req->m_cache_id[MEM_L3] =
      (req->m_addr >> log2_int(m_l3_interleave_factor)) % m_num_l3;

  if ((m_num_llc & (m_num_llc - 1)) == 0)  // if m_num_llc is a power of 2
    req->m_cache_id[MEM_LLC] =
      BANK(req->m_addr, m_num_llc, m_llc_interleave_factor);
  else
    req->m_cache_id[MEM_LLC] =
      (req->m_addr >> log2_int(m_llc_interleave_factor)) % m_num_llc;

  if ((m_num_mc & (m_num_mc - 1)) == 0)  // if m_num_mc is a power of 2
    req->m_cache_id[MEM_MC] =
      BANK(req->m_addr, m_num_mc, m_dram_interleave_factor);
  else
    req->m_cache_id[MEM_MC] =
      (req->m_addr >> log2_int(m_dram_interleave_factor)) % m_num_mc;
}

void l2_decoupled_network_c::invalidate(Addr page_addr) {
  Addr addr = page_addr;
  Addr end_addr = page_addr + m_page_size;

  int line_size = -1;
  for (int ii = 0; ii < m_num_core; ++ii) {
    line_size = m_l1_cache[ii]->line_size();
    for (; addr < end_addr; addr += line_size) m_l1_cache[ii]->invalidate(addr);

    line_size = m_l2_cache[ii]->line_size();
    for (; addr < end_addr; addr += line_size) m_l2_cache[ii]->invalidate(addr);
  }

  for (int ii = 0; ii < m_num_l3; ++ii) {
    line_size = m_l3_cache[ii]->line_size();
    for (; addr < end_addr; addr += line_size) m_l3_cache[ii]->invalidate(addr);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////

l2_decoupled_local_c::l2_decoupled_local_c(macsim_c* simBase)
  : memory_c(simBase) {
  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_core; ++ii) {
    m_l1_cache[ii]->init(ii, -1, !HAS_DONE_FUNC, !ULINK, DLINK, ENABLE,
                         !HAS_ROUTER);
    m_l2_cache[ii]->init(-1, ii, HAS_DONE_FUNC, ULINK, !DLINK, !ENABLE,
                         !HAS_ROUTER);
  }

  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_l3; ++ii)
    m_l3_cache[ii]->init(-1, -1, !HAS_DONE_FUNC, !ULINK, !DLINK, !ENABLE,
                         HAS_ROUTER);

  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_llc; ++ii)
    m_llc_cache[ii]->init(-1, -1, !HAS_DONE_FUNC, !ULINK, !DLINK, ENABLE,
                          HAS_ROUTER);
}

l2_decoupled_local_c::~l2_decoupled_local_c() {
}

void l2_decoupled_local_c::set_cache_id(mem_req_s* req) {
  req->m_cache_id[MEM_L1] = req->m_core_id;
  req->m_cache_id[MEM_L2] = req->m_core_id;
  req->m_cache_id[MEM_L3] = BANK(req->m_addr, m_num_l3, m_l3_interleave_factor);
  req->m_cache_id[MEM_LLC] =
    BANK(req->m_addr, m_num_llc, m_llc_interleave_factor);
  req->m_cache_id[MEM_MC] = BANK(
    req->m_addr, m_num_mc, *m_simBase->m_knobs->KNOB_DRAM_INTERLEAVE_FACTOR);
}

///////////////////////////////////////////////////////////////////////////////////////////////

igpu_network_c::igpu_network_c(macsim_c* simBase) : memory_c(simBase) {
  m_igpu_sim = true;

  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_core; ++ii) {
    m_l1_cache[ii]->init(ii, -1, false, false, true, true, false);
    m_l2_cache[ii]->init(-1, ii, true, true, true, true, true);
  }

  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_l3; ++ii)
    m_l3_cache[ii]->init(-1, -1, false, false, false, false, true);

  // NEXT_ID, PREV_ID, DONE, COUPLE_UP, COUPLE_DOWN, DISABLE, HAS_ROUTER
  for (int ii = 0; ii < m_num_llc; ++ii)
    m_llc_cache[ii]->init(-1, -1, false, false, false, false, true);

  NETWORK->init(m_num_cpu, m_num_gpu, m_num_l3, m_num_llc, m_num_mc);
}

igpu_network_c::~igpu_network_c() {
}

void igpu_network_c::set_cache_id(mem_req_s* req) {
  req->m_cache_id[MEM_L1] = req->m_core_id;
  req->m_cache_id[MEM_L2] = req->m_core_id;
  req->m_cache_id[MEM_L3] = BANK(req->m_addr, m_num_l3, m_l3_interleave_factor);
  req->m_cache_id[MEM_LLC] =
    BANK(req->m_addr, m_num_llc, m_llc_interleave_factor);
  req->m_cache_id[MEM_MC] = BANK(
    req->m_addr, m_num_mc, *m_simBase->m_knobs->KNOB_DRAM_INTERLEAVE_FACTOR);
}

void igpu_network_c::invalidate(Addr page_addr) {
  Addr addr = page_addr;
  Addr end_addr = page_addr + m_page_size;

  for (int ii = 0; ii < m_num_l3; ++ii) {
    int line_size = m_l3_cache[ii]->line_size();
    for (; addr < end_addr; addr += line_size) m_l3_cache[ii]->invalidate(addr);
  }

  for (int ii = 0; ii < m_num_llc; ++ii) {
    int line_size = m_llc_cache[ii]->line_size();
    for (; addr < end_addr; addr += line_size)
      m_llc_cache[ii]->invalidate(addr);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
