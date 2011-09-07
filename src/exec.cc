/**********************************************************************************************
 * File         : exec.cc
 * Author       : Hyesoon Kim
 * Date         : 3/17/2008
 * CVS          : $Id: exec.cc,v 1.8 2008-09-10 02:18:41 kacear Exp $:
 * Description  : execution unit
                  origial author: Jared W.Stark  imported from
 *********************************************************************************************/


/*
 * Summary: Execution stage in the pipeline
 */


#include "exec.h"
#include "knob.h"
#include "uop.h"
#include "pqueue.h"
#include "rob.h"
#include "bp.h"
#include "frontend.h"
#include "utils.h"
#include "memory.h"
#include "statistics.h"
#include "statsEnums.h"

#include "core.h"
#include "readonly_cache.h"
#include "rob_smc.h"
#include "sw_managed_cache.h"
#include "process_manager.h"

#include "debug_macros.h"

#include "config.h"

#include "all_knobs.h"

#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_EXEC_STAGE, ## args)
#define DEBUG_CORE(m_core_id, args...) if(m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) \
					 { _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_EXEC_STAGE, ## args); }

#define CLEAR_BIT(val, pos)   (val & ~(0x1ULL << pos))


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Uop latency table 
///
/// We maintain two types of uop latency. 
/// Based on the core type, we bind different latency map 
///////////////////////////////////////////////////////////////////////////////////////////////
struct Uop_LatencyBinding_Init
{
  Uop_Type uop_type_s; /**< uop type */
  string m_name; /**< uop type name */
  int m_latency; /**< latency */
};


static Uop_LatencyBinding_Init uop_latencybinding_init_x86[] = {
#define DEFUOP(A, B) {A, # A, B},
#include "../def/uoplatency_x86.def"
};

static Uop_LatencyBinding_Init uop_latencybinding_init_ptx[] = {
#define DEFUOP(A, B) {A, # A, B},
#include "../def/uoplatency_ptx.def"
};


// exec_c constructor
exec_c::exec_c(EXEC_INTERFACE_PARAMS(), macsim_c* simBase): EXEC_INTERFACE_INIT()
{
  int int_sched_rate;
  int mem_sched_rate;
  int fp_sched_rate;

  m_simBase = simBase;

  EXEC_CONFIG();

  m_max_port[gen_ALLOCQ] = int_sched_rate;
  m_max_port[mem_ALLOCQ] = mem_sched_rate;
  m_max_port[fp_ALLOCQ]  = fp_sched_rate;

  clear_ports();


  // latency binding
  if (m_ptx_sim) {
    int factor             = *m_simBase->m_knobs->KNOB_PTX_EXEC_RATIO;
    int latency_array_size = (sizeof uop_latencybinding_init_ptx /
        sizeof (uop_latencybinding_init_ptx[0]));

    for (int ii = 0; ii < latency_array_size; ++ii) { 
      m_latency[uop_latencybinding_init_ptx[ii].uop_type_s] = 
        (factor * uop_latencybinding_init_ptx[ii].m_latency);
    }
  }
  else {
    int latency_array_size = (sizeof uop_latencybinding_init_x86 /
        sizeof (uop_latencybinding_init_x86[0]));

    for (int ii = 0; ii < latency_array_size; ++ii) {
      m_latency[uop_latencybinding_init_x86[ii].uop_type_s] =
        uop_latencybinding_init_x86[ii].m_latency;
    }
  }

  m_bank_busy = new bool[129];
  

}


// exec_c destructor
exec_c::~exec_c()
{
}


// clear execution ports
void exec_c::clear_ports()
{
  for (int ii = 0; ii < max_ALLOCQ; ++ii) {
    m_port_used[ii] = 0;
  }
}


// get uop latency
int exec_c::get_latency(Uop_Type uop_type)
{
  // when *m_simBase->m_knobs->KNOB_ONE_CYCLE_EXEC is set, all uops have 1-cyce latency
  if (*m_simBase->m_knobs->KNOB_ONE_CYCLE_EXEC) {
    return 1;
  }

  // otherwise return original latency
  return m_latency[uop_type];
}


// check available execution port for specific instruction type
bool exec_c::port_available(int uop_type)
{
  return m_port_used[uop_type] < m_max_port[uop_type];
}


// use an execution port
void exec_c::use_port(int thread_id, int entry)
{
  // get current uop
  uop_c *uop = NULL;
  if (thread_id == -1) {
    uop = (*m_rob)[entry];
  }
  else {
    rob_c *thread_rob = m_gpu_rob->get_thread_rob(thread_id);
    uop = (*thread_rob)[entry];
  }

  // use specified port
  if (!uop->m_bogus) {
    ++m_port_used[uop->m_allocq_num];
  }
}


// main execution function
// There are 3 types of instructions
// 1) memory
//   0  : memory instruction cannot be executed (port busy, ...)
//   -1 : cache miss, generate a new request
//   >0 : cache hit
// 2) branch
// 3) others (computation)
bool exec_c::exec(int thread_id, int entry, uop_c* uop)
{
  m_cur_core_cycle  = m_simBase->m_core_cycle[m_core_id];
  Uop_Type uop_type = uop->m_uop_type;
  int uop_latency   = -1;
  core_c *core      = m_simBase->m_core_pointers[m_core_id];

  DEBUG("m_core_id:%d thread_id:%d uop->iaq:%d uop_num:%lld inst_num:%lld mem_type:%d "
      "bogus:%d \n", 
      m_core_id, uop->m_thread_id, uop->m_allocq_num, uop->m_uop_num, 
      uop->m_inst_num, uop->m_mem_type, uop->m_bogus);

  DEBUG_CORE(m_core_id, "m_core_id:%d thread_id:%d uop->iaq:%d uop_num:%lld inst_num:%lld "
      " mem_type:%d bogus:%d \n", 
      m_core_id, uop->m_thread_id, uop->m_allocq_num, uop->m_uop_num, 
      uop->m_inst_num, uop->m_mem_type, uop->m_bogus);

  uop->m_state = OS_EXEC_BEGIN;

  Mem_Type type = uop->m_mem_type;


  // execute memory instructions
  if (type != NOT_MEM) { 
    // perfect dcache
    if (*m_simBase->m_knobs->KNOB_PERFECT_DCACHE) {
      uop_latency = 1;
    }
    else {
      // single uop in an instruction
      if (uop->m_num_child_uops == 0) {
        // shared memory access
        if (uop->m_mem_type == MEM_LD_SM || uop->m_mem_type == MEM_ST_SM) {
          // shared memory access
          uop_latency = core->get_shared_memory()->load(uop);
          if (uop_latency != 0) {
            uop->m_mem_start_cycle = m_cur_core_cycle;
          }
        }
        // other memory accesses
        else {
          // constant memory
          if (uop->m_mem_type == MEM_LD_CM) {
            uop_latency = core->get_const_cache()->load(uop);
          }
          else if (uop->m_mem_type == MEM_LD_TM) {
            uop_latency = core->get_texture_cache()->load(uop);
          }
          // other (global, texture, local) memory access
          else {
            // FIXME
#if PORT_FIXME
            if (m_bank_busy[uop->m_dcache_bank_id] == true) {
              STAT_EVENT(CACHE_BANK_BUSY);
              uop_latency = 0;
            }
            else {
#endif
              uop_latency = m_simBase->m_memory->access(uop);
#if PORT_FIXME
              if (uop_latency == 0) {
                if (uop->m_dcache_bank_id >= 128)
                  m_bank_busy[uop->m_dcache_bank_id] = true;
              }
            }
#endif
          }
            
          if (uop_latency != 0) {
            uop->m_mem_start_cycle = m_cur_core_cycle;
          }
        }
      }
      // memory instructions that result in multiple memory requests
      // are represented by a parent uop (to make it easy to handle 
      // these instructions in schedule and retire stages) and a list 
      // of child uops for the addresses to be accessed
      else {
        uop_latency      = 0;
        int latency      = -1;
        int max_latency  = 0;
        // m_pending_child_uops is a bitmask that tracks which of the 
        // child uops have been sent to the memory hierarchy.
        // get the next child uop for the current parent uop
        int next_set_bit = get_next_set_bit64(uop->m_pending_child_uops, 0);

        // executing children uops
        while (-1 != next_set_bit) {
          // shared memory access
          if (uop->m_mem_type == MEM_LD_SM || uop->m_mem_type == MEM_ST_SM) {
            latency = core->get_shared_memory()->load(uop->m_child_uops[next_set_bit]);
          }
          // other types of memory access
          else {
            // access can return
            // -1 - cache miss
            //  0 - could not execute / no space in the memory hierarchy, try again
            //  x > 0 - cache hit
            //  FIXME
#if PORT_FIXME
            if (0 && m_bank_busy[uop->m_child_uops[next_set_bit]->m_dcache_bank_id] == true) {
              STAT_EVENT(CACHE_BANK_BUSY);
              uop_latency = 0;
            }
            else {
#endif
              latency = m_simBase->m_memory->access(uop->m_child_uops[next_set_bit]);
#if PORT_FIXME
              if (latency == 0) {
                if (m_bank_busy[uop->m_child_uops[next_set_bit]->m_dcache_bank_id] < 128)
                  m_bank_busy[uop->m_child_uops[next_set_bit]->m_dcache_bank_id] = true;
              }
            }
#endif
          }

          if (0 != latency) { // successful execution
            if (!uop->m_mem_start_cycle) {
            	uop->m_mem_start_cycle = m_cur_core_cycle;
            }

            // mark current uop as executed
            uop->m_pending_child_uops = 
              CLEAR_BIT(uop->m_pending_child_uops, next_set_bit);

            // cache hit
            if (latency > 0) {
              ++uop->m_num_child_uops_done;
              DEBUG("m_core_id:%d thread_id:%d uop_num:%lld inst_num:%lld child_uop_num:%lld "
                  "m_dcu hit\n", 
                  m_core_id, uop->m_thread_id, uop->m_uop_num, uop->m_inst_num,
                  uop->m_child_uops[next_set_bit]->m_uop_num);

              if (latency > max_latency) {
                max_latency = latency;
              }
            }
            // cache miss
            else if (-1 == latency) {
              uop_latency = -1;
              DEBUG("m_core_id:%d thread_id:%d uop_num:%lld inst_num:%lld child_uop_num:%lld "
                  "m_dcu miss\n", 
                  m_core_id, uop->m_thread_id, uop->m_uop_num, uop->m_inst_num, 
                  uop->m_child_uops[next_set_bit]->m_uop_num);
            }
          }

          // find next uop to execute
          next_set_bit = get_next_set_bit64(uop->m_pending_child_uops, next_set_bit + 1);
        }

        // uop_latency can be
        // -1 - all child uops have been issued and some of them were cache misses
        //  0 - some child uops are yet to be issued
        //  x > 0 - all child uops have been issued and all children will complete 
        // in x cycles
        if (uop->m_pending_child_uops) {
          uop_latency = 0;
        }
        else {
          // all children uops have been executed
          if (uop->m_num_child_uops == uop->m_num_child_uops_done) {
            uop_latency = max_latency;
          }
          // some uops have in-flight memory accesses
          else {
            uop_latency = -1;
          }
        }
      }

      // memory instruction was not executed
      if (uop_latency == 0) {
        return false; 
      }

      // stats
      if (uop->m_mem_type) {
        update_memory_stats(uop);
      }
			
      // use execution port
      use_port(thread_id, entry);

      // GPU : if we use load-block policy, block current thread due to load instruction
      if (uop_latency == -1 && m_ptx_sim && *m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
        m_frontend->set_load_wait(uop->m_thread_id, uop->m_uop_num); 

        DEBUG("set_load_wait m_core_id:%d thread_id:%d uop_num:%lld inst_num:%lld\n",
            uop->m_core_id, uop->m_thread_id, uop->m_uop_num, uop->m_inst_num);
      }

      DEBUG("m_core_id:%d thread_id:%d vaddr:%s uop_num:%lld inst_num:%lld "
          "uop->uop_info.dcmiss:%d latency:%d done_cycle:%lld\n",
          m_core_id, uop->m_thread_id, hexstr64s(uop->m_vaddr), uop->m_uop_num, 
          uop->m_inst_num, uop->m_uop_info.m_dcmiss, uop_latency, uop->m_done_cycle);
    }
  }
  // non-memory (compute) instructions
  else {
    uop_latency = get_latency(uop_type);
    use_port(thread_id, entry);
  }

  // set scheduling cycle
  uop->m_sched_cycle = m_cur_core_cycle;

  ASSERT(uop_latency != 0);

  // set uop done cycle
  if (uop_latency > 0) {
    int max_latency = std::max(uop_latency, static_cast<int>(*m_simBase->m_knobs->KNOB_EXEC_RETIRE_LATENCY));
    uop->m_done_cycle = m_cur_core_cycle +  max_latency;
  }

  DEBUG("done_exec m_core_id:%d thread_id:%d core_cycle_count:%lld uop_num:%lld"
      " inst_num:%lld sched_cycle:%lld exec_cycle:%lld uop->done_cycle:%lld "
      "inst_count:%lld uop->dcmiss:%d uop_latency:%d done_cycle:%lld pc:0x%s\n",
      m_core_id, uop->m_thread_id, m_cur_core_cycle, uop->m_uop_num, uop->m_inst_num, 
      uop->m_sched_cycle, uop->m_exec_cycle, uop->m_done_cycle, uop->m_inst_num, 
      uop->m_uop_info.m_dcmiss, uop_latency, uop->m_done_cycle, hexstr64s(uop->m_pc));

  // branch execution
  if (uop->m_cf_type) {
    br_exec(uop);
  }

  return true;
}


// branch execution
void exec_c::br_exec(uop_c *uop)
{
  switch (uop->m_cf_type) {
    case CF_BR:
      break;
    case CF_CBR:
      (m_bp_data->m_bp)->update(uop);
      break;
    case CF_CALL:
      break;
    
    // indirect branches
    case CF_IBR:
    case CF_ICALL:
    case CF_ICO:
    case CF_RET:
    case CF_MITE:
      break;
  }


  // handle mispredicted branches 
  if (uop->m_mispredicted) {
    (m_bp_data->m_bp)->recover(&(uop->m_recovery_info)); 
    m_bp_data->m_bp_recovery_cycle[uop->m_thread_id] = 
      m_cur_core_cycle + 1 + *m_simBase->m_knobs->KNOB_EXTRA_RECOVERY_CYCLES;
    m_bp_data->m_bp_cause_op[uop->m_thread_id] = 0;

    STAT_CORE_EVENT(m_core_id, BP_RESOLVED); 

    DEBUG("m_core_id:%d thread_id:%d cur_core_cycle:%s branch is resolved: "
          "recovery_cycle:%lld uop_num:%lld\n", 
          m_core_id, uop->m_thread_id, unsstr64(m_cur_core_cycle), 
          m_bp_data->m_bp_recovery_cycle[uop->m_thread_id], uop->m_uop_num);
  }


  // GPU : stall on branch policy
  if (m_ptx_sim && *m_simBase->m_knobs->KNOB_MT_NO_FETCH_BR) {
    m_frontend->set_br_ready(uop->m_thread_id);
  }
}


// update memory related stats
void exec_c::update_memory_stats(uop_c* uop)
{ 
  switch (uop->m_mem_type) {
    case MEM_LD_SM:
    case MEM_ST_SM:
      STAT_EVENT(SHARED_MEM_ACCESS);
      break;

    case MEM_LD_CM:
      STAT_EVENT(CONST_CACHE_ACCESS);
      break;

    case MEM_LD_TM:
      STAT_EVENT(TEXTURE_CACHE_ACCESS);
      break;
  }
}


// run a cycle
// For now, reset bank busy mask
void exec_c::run_a_cycle(void)
{
  fill_n(m_bank_busy, 129, false);
}

