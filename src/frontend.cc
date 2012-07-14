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
 * File         : frontend.cc
 * Author       : Hyesoon Kim
 * Date         : 1/8/2008
 * CVS          : $Id: frontend.cc 915 2009-11-20 19:13:07Z kacear $:
 * Description  : front-end  (based on Jared W. Stark's code)
 *********************************************************************************************/


#include "cache.h"
#include "frontend.h"
#include "trace_read.h"
#include "core.h"
#include "knob.h"
#include "exec.h"
#include "debug_macros.h"
#include "uop.h"
#include "pqueue.h"
#include "statistics.h"
#include "statsEnums.h"
#include "memreq_info.h"
#include "memory.h"
#include "utils.h"
#include "retire.h"
#include "bp.h"
#include "map.h"
#include "bug_detector.h"
#include "config.h"
#include "process_manager.h"
#include "all_knobs.h"


#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_FRONT_STAGE, ## args)
#define DEBUG_CORE(m_core_id, args...)       \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) {     \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_FRONT_STAGE, ## args); \
  }


///////////////////////////////////////////////////////////////////////////////////////////////

// wrapper function to fill an icache line 
bool icache_fill_line_wrapper(mem_req_s *req)
{
  macsim_c* m_simBase = req->m_simBase;

  bool result = true;
  DEBUG("req:%d called icache_done_func\n", req->m_id);

  // serve merged request
  list<mem_req_s*> done_list;
  for (auto I = req->m_merge.begin(), E  = req->m_merge.end(); I != E; ++I) {
    if ((*I)->m_done_func && !((*I)->m_done_func((*I)))) {
      result = false;
      continue;
    }
    done_list.push_back((*I));
  }

  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    req->m_merge.remove((*I));
    m_simBase->m_memory->free_req((*I)->m_core_id, (*I));
  }

  // serve me
  if (!m_simBase->m_core_pointers[req->m_core_id]->get_frontend()->icache_fill_line(req)) {
	  result = false;
  }
	  
  POWER_CORE_EVENT(req->m_core_id, POWER_ICACHE_MISS_BUF_W);

  return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////


// register base fetch policy
frontend_c *fetch_factory(FRONTEND_INTERFACE_PARAMS(), macsim_c* simBase) 
{
  frontend_c *new_fe = new frontend_c(FRONTEND_INTERFACE_ARGS(), simBase);

  return new_fe;
}


// frontend_s initialization
void frontend_s::init()
{
  m_fe_mode                             = FRONTEND_MODE_IFETCH;
  m_fetch_blocked                       = false;
  m_next_bid                            = false;
  m_next_rowid                          = false;
  m_sync_wait_start                     = 0;
  m_sync_wait_count                     = 0;
  m_sync_count                          = 0;
  m_extra_fetch                         = 0;
  m_prev_uop_num                        = 0;
  m_prev_uop_thread_num                 = 0;
  m_first_time                          = true;
  m_MT_load_waiting                     = 0;
  m_MT_br_waiting                       = false;
  m_reconv_data.m_work_counter          = 0;
  m_reconv_data.m_reconverge_depth      = 0;
  m_reconv_data.m_top_level_reconv_seen = 0;
  m_MT_scheduler.m_next_fetch_addr      = 0;
  m_MT_scheduler.m_fetch_addr           = 0;
  m_prev_uop                            = NULL;
}


// sync_thread_s constructor
sync_thread_s::sync_thread_s() : m_block_id(-1), m_sync_count(-1), m_num_threads_in_block(-1)
{
  /* do nothing */
}


///////////////////////////////////////////////////////////////////////////////////////////////


// frontend_c constructor
frontend_c::frontend_c(FRONTEND_INTERFACE_PARAMS(), macsim_c* simBase) 
: FRONTEND_INTERFACE_INIT() 
{
  m_simBase = simBase;

  string name;
  ToString(name, "sync_info_" << m_core_id);

  m_sync_info = new hash_c<sync_thread_s>(name);

  m_fe_running             = true;
  m_fe_stall               = false;
  m_ready_thread_available = false;
  m_fetch_arbiter          = 0;
  m_mem_access_thread_num  = 0;

  FRONTEND_CONFIG();
    
  // setting fetch policy
  string policy = m_simBase->m_knobs->KNOB_FETCH_POLICY->getValue();
  if (policy == "rr") {
    MT_fetch_scheduler = &frontend_c::fetch_rr;
  }
  else
    assert(0);
}


// frontend_c destructor
frontend_c::~frontend_c()
{
}


// fetch stage - fetch instruction from the icache every cycle
void frontend_c::run_a_cycle(void)
{
  // bind core id
  static bool map_core = false;
  if (!map_core) {
    m_core = m_simBase->m_core_pointers[m_core_id];
    map_core = false;
  }

  m_cur_core_cycle = m_simBase->m_core_cycle[m_core_id];

  // fetching unit is not running or currently no thread has been fetching 
  if (!m_fe_running || !m_fetching_thread_num) 
    return; 

	// Block_States table is accessed EVERY cycle, so should be before the code that fetches every FETCH_RATIO cycle
  POWER_CORE_EVENT(m_core_id, POWER_BLOCK_STATES_R);
  POWER_CORE_EVENT(m_core_id, POWER_BLOCK_STATES_W);

  // fetch every KNOB_FETCH_RATIO cycle
  // CPU : every cycle
  // NVIDIA G80 : 1/4 cycles, NVIDIA Fermi: 1/2 cycles
  m_fetch_modulo = (m_fetch_modulo + 1) % m_fetch_ratio;
  if (m_fetch_modulo) 
    return;

  // NVIDIA architecture now support dual warp schedulers
  // In every cycle, 2 instructions will be fetched from N different threads
  // However, for x86 simulations, KNOB_NUM_WARP_SCHEDULER is always set to 1
  // When *m_simBase->m_knobs->KNOB_NUM_WARP_SCHEDULER is one, work as regular fetch unit
  int prev_fetched_tid = -1;
  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_NUM_WARP_SCHEDULER; ++ii) {
    m_ready_thread_available = false;

    // get thread id to fetch
    int fetch_thread = fetch();

    DEBUG("m_core_id:%d frontend fetch thread is:%d \n", m_core_id, fetch_thread);

    // nothing to fetch
    if (fetch_thread == -1) {
      STAT_EVENT(NUM_NO_FETCH_CYCLES);
      STAT_CORE_EVENT(m_core_id, CORE_NUM_NO_FETCH_CYCLES);
      if (m_ready_thread_available == true) {
        STAT_CORE_EVENT(m_core_id, CORE_NUM_NO_FETCH_CYCLES_WITH_READY_THREADS);
      }
      break;
    }

    // in case of double fetch, stop fetching if second thread is not available
    if (prev_fetched_tid == fetch_thread) {
      DEBUG("same thread id m_core_id:%d tid:%d prev_tid:%d\n",
          m_core_id, fetch_thread, prev_fetched_tid);
      break;
    }

    prev_fetched_tid = fetch_thread;

    // main fetch routine
    if (!m_fe_stall && (m_q_frontend->space() > m_knob_fetch_width) && fetch_thread != -1) {
      // fetch data is for each threads
      frontend_s* fetch_data = m_core->get_trace_info(fetch_thread)->m_fetch_data;
      switch (fetch_data->m_fe_mode) {
        // not supported - should be removed
        case FRONTEND_MODE_PERFECT:
          fetch_data->m_fe_prev_mode = FRONTEND_MODE_PERFECT;
          fetch_data->m_fe_mode      = FRONTEND_MODE_PERFECT;
          break;
        // normal fetch mode
        case FRONTEND_MODE_IFETCH:
          fetch_data->m_fe_prev_mode = FRONTEND_MODE_IFETCH;
          fetch_data->m_fe_mode      = process_ifetch(fetch_thread, fetch_data);
          break;
        // fetch stalled
        case FRONTEND_MODE_WAIT_FOR_MISS:
          fetch_data->m_fe_prev_mode = FRONTEND_MODE_WAIT_FOR_MISS;
          if (check_fetch_ready(fetch_thread)) {
            fetch_data->m_fe_mode = FRONTEND_MODE_IFETCH;
          }
          else {
            DEBUG("fetch stalled m_core_id:%d tid:%d\n", m_core_id, fetch_thread);
          }
          break;

        default:
          ASSERTM(0, "Unknown frontend mode\n");
      }
    }
  }


  // TONAGESH
  // nagesh - comments for BAR are incomplete...
  if (m_knob_ptx_sim) {

    // handling of BAR instruction in PTX - can/should this be moved?
    // do we have any blocks for which all warps have reached (retired)
    // their next barrier?
    if (m_sync_done.size() > 0) {
      int32_t block_id;
      bool status;
      uint64_t min;

      while (m_sync_done.size() > 0) {
        block_id = m_sync_done.front();
        m_sync_done.pop_front();
        min = ~0;

        for (int tid = m_last_terminated_tid; tid < m_unique_scheduled_thread_num; ++tid) {
          /* this condition is not needed */
          if (m_core->m_fetch_ended[tid] || m_core->m_thread_reach_end[tid]) {
            continue;
          }

          if (m_core->get_trace_info(tid)->m_block_id == block_id) {
            frontend_s* fetch_data = m_core->get_trace_info(tid)->m_fetch_data;
            if (!fetch_data->m_fetch_blocked) {
              printf("[FE] fetch not blocked!!! %d %d\n", m_core_id, tid);
            }

            fetch_data->m_fetch_blocked = false;
            fetch_data->m_sync_wait_count += 
              (m_cur_core_cycle - fetch_data->m_sync_wait_start + 1);
          }
        }
        status = m_sync_info->hash_table_access_delete(block_id);

        if (!status) {
          printf("sync info not found!!!!!\n");
        }
      }
    }
  }
}


// fetch instructions from a thread
FRONTEND_MODE frontend_c::process_ifetch(unsigned int tid, frontend_s* fetch_data)
{
  int fetched_uops = 0;
  Break_Reason break_fetch = BREAK_DONT;

  // First time : set up traces for current thread 
  if (fetch_data->m_first_time) {
    m_simBase->m_trace_reader->setup_trace(m_core_id, tid);
    fetch_data->m_first_time = false;

    ++m_core->m_inst_fetched[tid]; /*! initial increase */
    if (m_core->m_inst_fetched[tid] > m_core->m_max_inst_fetched) {
      m_core->m_max_inst_fetched = m_core->m_inst_fetched[tid];
    }

    // set up initial fetch address
    fetch_data->m_MT_scheduler.m_next_fetch_addr = 
      m_core->get_trace_info(tid)->m_prev_trace_info->m_instruction_addr;
  }


  // -------------------------------------
  // check whether previous branch misprediction has been resolved
  // -------------------------------------
  if ((m_bp_data->m_bp_recovery_cycle[tid] > m_cur_core_cycle) || 
      (m_bp_data->m_bp_redirect_cycle[tid] > m_cur_core_cycle) ) {
    STAT_CORE_EVENT(m_core_id, BP_MISPRED_STALL);

    return FRONTEND_MODE_IFETCH;
  }


  // -------------------------------------
  // Fetch main loop
  // -------------------------------------
  while (!break_fetch) {
    Addr fetch_addr;

    // get new fetch address (+ each application has own memory space)
    fetch_addr = fetch_data->m_MT_scheduler.m_next_fetch_addr;
    fetch_addr = fetch_addr + m_icache->base_cache_line((unsigned long)UINT_MAX *
        (m_core->get_trace_info(tid)->m_process->m_process_id) * 10ul);


    // -------------------------------------
    // instruction cache access
    // -------------------------------------
    bool icache_miss = access_icache(tid, fetch_addr, fetch_data);

    // instruction cache miss
    if (icache_miss) {
      DEBUG_CORE(m_core_id, "set frontend[%d] is FRONTEND_MODE_WAIT_FOR_MISS\n", tid);
      return FRONTEND_MODE_WAIT_FOR_MISS;
    } 
    // instruction cache hit
    else {
      POWER_CORE_EVENT(m_core_id, POWER_ICACHE_R);	// FIXME Jieun Apr-9-2012 should be moved to somewhere else, this is not a right place. currently ICACHE_R = #insts, but it should be 1/4

      // -------------------------------------
      // fetch instructions
      // -------------------------------------
      while ((m_q_frontend->space() > 0) && !break_fetch) {
        // allocate a new uop 
        uop_c *new_uop = m_uop_pool->acquire_entry(m_simBase);
        
        // FIXME Jieun 01-13-2012 Read counter should be between ICache and Predecoder
        POWER_CORE_EVENT(m_core_id, POWER_FETCH_QUEUE_R);	
        new_uop->allocate();
        ASSERT(new_uop); 


        // read an uop from the traces 
        if (!m_simBase->m_trace_reader->get_uops_from_traces(m_core_id, new_uop, tid)) {
          // couldn't get an uop
          DEBUG("not success\n");
          m_uop_pool->release_entry(new_uop->free());

          return  FRONTEND_MODE_IFETCH;
        }
        
        new_uop->m_state = OS_FETCHED; 
        new_uop->m_fetched_cycle = m_core->get_cycle_count();


        // FIXME (jaekyu, 10-4-2011)
        // make it member variable somehow
        // debugging purpose
				if (*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE) {
	        m_simBase->m_bug_detector->allocate(new_uop);
	        for (int ii = 0; ii < new_uop->m_num_child_uops; ++ii) {
	          m_simBase->m_bug_detector->allocate(new_uop->m_child_uops[ii]);
	        }
				}

        ++m_core->m_ops_to_be_dispatched[tid];
        m_core->m_last_fetch_cycle[tid] = m_core->get_cycle_count();

        DEBUG_CORE(m_core_id, "cycle_count:%lld m_core_id:%d tid:%d uop_num:%lld  "
            "inst_num:%lld uop.va:%s iaq:%d mem_type:%d dest:%d num_dests:%d\n",
            m_cur_core_cycle, m_core_id, new_uop->m_thread_id,
            new_uop->m_uop_num, new_uop->m_inst_num, hexstr64s(new_uop->m_vaddr),
            new_uop->m_allocq_num, new_uop->m_mem_type, new_uop->m_dest_info[0],
            new_uop->m_num_dests);


        // -------------------------------------
        // register mapping - dependence checking
        // -------------------------------------
        if (!*m_simBase->m_knobs->KNOB_IGNORE_DEP) {
          m_map->map_uop(new_uop);
          m_map->map_mem_dep(new_uop);

          POWER_CORE_EVENT(m_core_id, POWER_DEP_CHECK_LOGIC_R);
        }


        // -------------------------------------
        // access branch predictors
        // -------------------------------------
        int br_mispred = false;
        if (new_uop->m_cf_type) {
          // btb prediction 
          bool btb_miss = btb_access(new_uop); 

          // branch prediction 
          br_mispred = predict_bpu(new_uop);
          if (new_uop->m_cf_type == CF_CBR) {
            STAT_CORE_EVENT(m_core_id, BP_ON_PATH_CORRECT+br_mispred+(new_uop->m_off_path)*3);
          }

          // BTB miss is MISFETCH. In theory, processor should access the bp only if btb hits 
          // However, just to get some stats, we allow bp accesses. 
          // This might be changed in future 

          if (btb_miss & !br_mispred)
            STAT_CORE_EVENT(m_core_id, BP_ON_PATH_MISFETCH+(new_uop->m_off_path)*3);


          // set frontend misprediction */
          if (br_mispred) {
            /*! should be per core */
            m_bp_data->m_bp_recovery_cycle[new_uop->m_thread_id] = MAX_CTR;
            m_bp_data->m_bp_cause_op[new_uop->m_thread_id] = new_uop->m_uop_num;

            DEBUG_CORE(m_core_id, "m_core_id:%d tid:%d branch is mispredicted inst_num:%lld "
                "uop_num:%lld\n", m_core_id, new_uop->m_thread_id, new_uop->m_inst_num, 
                new_uop->m_uop_num);
          } 
          else if (btb_miss) { 
            m_bp_data->m_bp_redirect_cycle[new_uop->m_thread_id] = MAX_CTR; 
            m_bp_data->m_bp_cause_op[new_uop->m_thread_id] = new_uop->m_uop_num;

            DEBUG_CORE(m_core_id, "m_core_id:%d tid:%d branch is misfetched(btb_miss) "
                "inst_num:%lld uop_num:%lld\n", m_core_id, new_uop->m_thread_id, 
                new_uop->m_inst_num, new_uop->m_uop_num);
          }
          else {
            fetch_data->m_MT_scheduler.m_next_fetch_addr = new_uop->m_npc;
            DEBUG_CORE(m_core_id, "m_core_id:%d tid:%d MT_scheduler[%d]->0x%s \n",
                m_core_id, new_uop->m_thread_id, tid, hexstr64s(new_uop->m_npc));
          }
        }

        // -------------------------------------
        // push the uop into the front_end_queue */
        // -------------------------------------
        send_uop_to_qfe(new_uop);
        ++fetched_uops;


        // -------------------------------------
        // we fetch enough uops, stop fetching
        // -------------------------------------
        if (fetched_uops >= m_knob_width) {
          break_fetch = BREAK_ISSUE_WIDTH;
        }
      } // while ((m_q_frontend->space() > 0) && !break_fetch) 

      break_fetch = BREAK_LINE_END;
    } // cache hit
  } // while (!break_fetch)

  return FRONTEND_MODE_IFETCH;
}


// instruction cache access 
bool frontend_c::access_icache(int tid, Addr fetch_addr, frontend_s* fetch_data)
{
  Addr line_addr;
  Addr new_fetch_addr;
  icache_data_c *icache_line;
  bool cache_miss = CACHE_MISS;

  if (fetch_addr == 0) 
    return CACHE_HIT;

  new_fetch_addr = fetch_addr;

  // -------------------------------------
  // access instruction cache
  // -------------------------------------
  POWER_CORE_EVENT(m_core_id, POWER_ICACHE_R_TAG);

  // -------------------------------------
  // perfect icache
  // -------------------------------------
  if (*m_simBase->m_knobs->KNOB_PERFECT_ICACHE) {
    cache_miss = CACHE_HIT;
    DEBUG("PERFECT_ICACHE!!!\n");
  }
  else {
    int appl_id = m_core->get_appl_id(tid);
    icache_line = (icache_data_c *)m_icache->access_cache(new_fetch_addr, &line_addr, true, 
        appl_id);
    cache_miss = (icache_line) ? false: true;
  }

  // -------------------------------------
  // cache hit
  // -------------------------------------
  if (!cache_miss) {
    STAT_CORE_EVENT(m_core_id, ICACHE_HIT);
    DEBUG_CORE(m_core_id, "m_core_id:%d fetch_addr:%s new fetch_addr:%s m_icache hit\n", 
        m_core_id, hexstr64s(fetch_addr), hexstr64s(new_fetch_addr));

  } 
  // -------------------------------------
  // cache miss 
  // -------------------------------------
  else {
    DEBUG_CORE(m_core_id, "m_core_id:%d fetch_addr:0x%s m_icache miss line_addr:0x%s "
        "new_fetch_addr:0x%s\n", m_core_id, hexstr64s(fetch_addr), hexstr64s(line_addr), 
        hexstr64s(new_fetch_addr));

    // send a new icache miss memory request
    int result = m_simBase->m_memory->new_mem_req(MRT_IFETCH, line_addr, 
        m_knob_icache_line_size, 0, NULL, icache_fill_line_wrapper, 
        m_core->get_unique_uop_num(), NULL, m_core_id, tid, m_knob_ptx_sim);

    // mshr full
    if (!result) 
      return false;

    STAT_CORE_EVENT(m_core_id, ICACHE_MISS);
    STAT_EVENT(ICACHE_MISS_TOTAL);

    // by setting m_fetch_ready_addr non-zero, fetch will be blocked
    fetch_data->m_fetch_ready_addr = line_addr;  
  }
  return cache_miss;
}


// insert a new cache line in icache 
bool frontend_c::icache_fill_line(mem_req_s *req)
{
  Addr line_addr;
  Addr repl_line_addr;

  // FIXME
  // write port check?

  // -------------------------------------
  // insert a cache line
  // -------------------------------------
  if (m_icache->access_cache(req->m_addr, &line_addr, false, req->m_appl_id) == NULL) {
    m_icache->insert_cache(req->m_addr, &line_addr, &repl_line_addr, req->m_appl_id, 
        req->m_ptx);
    POWER_CORE_EVENT(req->m_core_id, POWER_ICACHE_W);
  }

  STAT_CORE_EVENT(m_core_id, ICACHE_FILL); 

  DEBUG_CORE(m_core_id, "m_icache_fill m_core_id:%d req_addr:0x%s\n", 
      m_core_id, hexstr64s(req->m_addr)); 


  // FIXME (jaekyu, 10-4-2011)
  // Too expensive to check all threads on every icache miss
  // but probably not that frequent

  // find threads that are waiting for line_addr to be filled
  for (int ii = m_last_terminated_tid; ii < m_unique_scheduled_thread_num; ++ii) {
    if (m_core->get_trace_info(ii) == NULL)
      continue;

    frontend_s* fetch_data = m_core->get_trace_info(ii)->m_fetch_data;
    if (fetch_data != NULL && fetch_data->m_fetch_ready_addr == req->m_addr) {
      // fetching from this thread is unblocked. can be fetched
      fetch_data->m_fetch_ready_addr = 0;
      DEBUG_CORE(m_core_id, "sim_thread:%d is ready now addr:0x%s \n", ii, 
          hexstr64s(req->m_addr));
    }
    else {
      DEBUG_CORE(m_core_id, "fetch_ready_addr[%d]:%s is not ready now req:0x%s \n",
          ii, hexstr64s(fetch_data->m_fetch_ready_addr), hexstr64s(req->m_addr));
    }
  }
  return true;
}


// insert an uop to frontend queue 
inline void frontend_c::send_uop_to_qfe(uop_c *uop)
{
  bool success = m_q_frontend->enqueue(0, (int*)uop);
  // FIXME: Jieun 01-13-2012 Write Counter should be just after ICache. 
  // Since one line of ICache has ~4 insts, W Counter is #insts/4
  POWER_CORE_EVENT(m_core_id, POWER_FETCH_QUEUE_W);	
  DEBUG("m_core_id:%d tid:%d inst_num:%s uop_num:%s opcode:%d isitEOM:%d sent to qfe \n", 
      m_core_id, uop->m_thread_id, unsstr64(uop->m_inst_num), unsstr64(uop->m_uop_num), 
      (int)uop->m_opcode, uop->m_isitEOM);

  ASSERT(success);
}


// FIXME : indirect branch handling

// branch prediction
int frontend_c::predict_bpu(uop_c *uop)
{
  uns8 pred_dir = 0; // initialized to dummy value
  bool mispredicted = false;
  switch (uop->m_cf_type) {
    case CF_BR:
      // 100 % accurate
    case CF_CBR:
      if (*KNOB(KNOB_ENABLE_BTB))
        pred_dir = (m_bp_data->m_bp_targ_pred)->pred(uop);
      else
        pred_dir = (m_bp_data->m_bp)->pred(uop);
      mispredicted = (pred_dir != uop->m_dir);
      POWER_CORE_EVENT(m_core_id, POWER_BR_PRED_R);
      break;
    case CF_CALL:
      // 100% accurate
      POWER_CORE_EVENT(m_core_id, POWER_RAS_W);
      break;
    case CF_ICALL :
      // target = bp->ibr_predict_op(uop);
      POWER_CORE_EVENT(m_core_id, POWER_RAS_W);
      break;
    case CF_IBR:
      // target = bp->ibr_predict_op(uop);
    case CF_ICO:
      // target = bp->ibr_predict_op(uop);
    case CF_RET:
      // Return address stack : TOBE implemented
      POWER_CORE_EVENT(m_core_id, POWER_RAS_R);
      break;
    case CF_MITE:
      break;

    default:
      break;
  }


  if (*m_simBase->m_knobs->KNOB_USE_BRANCH_PREDICTION) {
    // do nothing
  }
  // no branch prediction
  else {
    // GPU : stall on branch policy, stop fetching
    if (m_knob_ptx_sim && *m_simBase->m_knobs->KNOB_MT_NO_FETCH_BR) {
      set_br_wait(uop->m_thread_id);
      mispredicted = false;
    }
  }


  // perfect branch predictor
  if (*m_simBase->m_knobs->KNOB_PERFECT_BP) 
    mispredicted = false;

  uop->m_mispredicted = mispredicted;

  DEBUG("m_core_id:%d tid:%d uop_num:%s pc:0x%s cf_type:%d dir:%d pred:%d mispredicted:%d \n", 
      uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), hexstr64s(uop->m_pc), 
      uop->m_cf_type, uop->m_dir, pred_dir, mispredicted);

  return uop->m_mispredicted;
}

// btb accesses 
bool frontend_c::btb_access(uop_c *uop)
{
  if (!*KNOB(KNOB_ENABLE_BTB))
    return false;

  bool btb_miss = false; 
  Addr pred_targ_addr = m_bp_data->m_bp_targ_pred->pred(uop);
  if (pred_targ_addr != uop->m_npc) 
    btb_miss = true; 

  uop->m_uop_info.m_btb_miss = btb_miss; 

  DEBUG("m_core_id:%d tid:%d uop_num:%s pc:0x%s cf_type:%d dir:%d oracle_npc:%s pred_targ:%s "
      "btb_miss:%d \n", 
      uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), hexstr64s(uop->m_pc), 
      uop->m_cf_type, uop->m_dir, hexstr64s(uop->m_npc), hexstr64s(pred_targ_addr), btb_miss);

  // need to set up redirect 

  return uop->m_uop_info.m_btb_miss; 

}

int frontend_c::fetch(void)
{
  return (*this.*MT_fetch_scheduler)();
}


// FIXME
// every 4 cycles, controll will go to the next thread always

// round-robin instruction fetch
int frontend_c::fetch_rr(void)
{
  int try_again = 1;
  int fetch_id = -1;

  DEBUG("m_core_id:%d m_running_thread_num:%d m_fetching_thread_num:%d "
      "m_unique_scheduled_thread_num:%d \n",m_core_id, m_running_thread_num,
      m_fetching_thread_num, m_unique_scheduled_thread_num);

  int max_try = m_unique_scheduled_thread_num - m_last_terminated_tid;

  while (m_fetching_thread_num && try_again && try_again <= max_try) {
    // find next thread id to fetch
    fetch_id = m_fetch_arbiter % m_unique_scheduled_thread_num;

    // update arbiter for next fetching
    m_fetch_arbiter = (m_fetch_arbiter + 1) % m_unique_scheduled_thread_num;

    // when fetch id goes back to 0, fast-forward until last terminated thread
    if (m_fetch_arbiter < m_last_terminated_tid) 
      m_fetch_arbiter = m_last_terminated_tid;

    // already terminated or fetch not ready
    if (m_core->m_fetch_ended[fetch_id] || m_core->m_thread_reach_end[fetch_id] || 
        (*m_simBase->m_knobs->KNOB_NO_FETCH_ON_ICACHE_MISS && !check_fetch_ready(fetch_id))) {
      ++try_again;
      continue;
    }

    // fetch blocked, try next thread
    frontend_s* fetch_data = m_core->get_trace_info(fetch_id)->m_fetch_data;
    if (fetch_data!= NULL && fetch_data->m_fetch_blocked) {
      DEBUG("m_core_id:%d tid:%d fetch_blocked\n", m_core_id, fetch_id);
      ++try_again;
      continue;
    }

    // check the thread is ready to fetch
    if (m_knob_ptx_sim) {
      // GPU : stall on branch policy, check whether previous branch has been resolved
      if (*m_simBase->m_knobs->KNOB_MT_NO_FETCH_BR && !check_br_ready(fetch_id)) {
        DEBUG("m_core_id:%d tid:%d br not ready\n", m_core_id, fetch_id);
        if (try_again == 1) 
          STAT_EVENT(FETCH_THREAD_SKIP_BR_WAIT);
        ++try_again;
        continue;
      }
      // GPU : stall on memory policy, check whether previous memory has been serviced
      if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY && !check_load_ready(fetch_id)) {
        DEBUG("m_core_id:%d tid:%d load not ready\n", m_core_id, fetch_id);
        if (try_again == 1) 
          STAT_EVENT(FETCH_THREAD_SKIP_LD_WAIT);
        ++try_again;
        continue;
      }
    }

    // find a thread to fetch
    if (fetch_id != -1) {
      break;
    }
    // no thread found
    else {
      ++try_again;
    }
  } // while ()


  // no thread to fetch
  if (try_again > max_try) 
    fetch_id = -1;

  DEBUG_CORE(m_core_id, "m_core_id:%d try_agin:%d tid:%d\n", m_core_id, try_again, fetch_id);

  return fetch_id;
}


// check thread is ready to be fetched
bool frontend_c::check_fetch_ready(int tid)
{
  frontend_s* fetch_data = m_core->get_trace_info(tid)->m_fetch_data;
  if (fetch_data->m_fetch_ready_addr == 0) {
    return true;
  }
  else {
    return false;
  }
}


// set current branch wait - this thread cannot be fetched until serviced
void frontend_c::set_br_wait(int fetch_id) 
{
  frontend_s* fetch_data = m_core->get_trace_info(fetch_id)->m_fetch_data;
  fetch_data->m_MT_br_waiting = true;
}


// set previous load ready
void frontend_c::set_load_ready(int fetch_id, Counter uop_num) 
{
  frontend_s* fetch_data = m_core->get_trace_info(fetch_id)->m_fetch_data;

  if (fetch_data->m_load_waiting.find(uop_num) != fetch_data->m_load_waiting.end()) {
    --fetch_data->m_MT_load_waiting; 
    fetch_data->m_load_waiting.erase(uop_num);
  }

  if (!fetch_data->m_MT_load_waiting) {
    --m_mem_access_thread_num;
  }
}


// set previous branch ready 
void frontend_c::set_br_ready(int fetch_id) 
{
  frontend_s* fetch_data = m_core->get_trace_info(fetch_id)->m_fetch_data;
  fetch_data->m_MT_br_waiting = false;
}


// TONAGESH
void frontend_c::synch_thread(int block_id, int tid)
{
  sync_thread_s *sync;

  sync = m_sync_info->hash_table_access(block_id);
  ++sync->m_sync_count;

  if (sync->m_sync_count == sync->m_num_threads_in_block) {
    m_sync_done.push_back(block_id);
  }
}


// check previous load from the thread has been serviced 
bool frontend_c::check_load_ready(int fetch_id) 
{
  frontend_s* fetch_data = m_core->get_trace_info(fetch_id)->m_fetch_data;
  return !fetch_data->m_MT_load_waiting;
}


// check previous branch from the thread has been serviced
bool frontend_c::check_br_ready(int fetch_id) 
{
  frontend_s* fetch_data = m_core->get_trace_info(fetch_id)->m_fetch_data;
  return !fetch_data->m_MT_br_waiting;
}


// set current load wait - this thread cannot be fetched until serviced
void frontend_c::set_load_wait(int fetch_id, Counter uop_num) 
{
  frontend_s* fetch_data = m_core->get_trace_info(fetch_id)->m_fetch_data;
  if (!fetch_data->m_MT_load_waiting) {
    ++m_mem_access_thread_num;
    if (m_mem_access_thread_num == m_core->m_running_thread_num) {
    }
  }
  ++fetch_data->m_MT_load_waiting;
  fetch_data->m_load_waiting[uop_num] = true;
}

