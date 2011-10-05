/**********************************************************************************************
 * File         : trace_read.cc
 * Author       : Hyesoon Kim 
 * Date         : 
 * SVN          : $Id: trace_read.cc 912 2009-11-20 19:09:21Z kacear $
 * Description  : modified by Sunpyo, Nagesh, and Jaekyu
 *********************************************************************************************/


#include <iostream>

#include "assert_macros.h"
#include "trace_read.h"
#include "uop.h"
#include "global_types.h"
#include "core.h"
#include "knob.h"
#include "process_manager.h"
#include "debug_macros.h"
#include "statistics.h"
#include "frontend.h"
#include "statsEnums.h"
#include "utils.h"
#include "pref_common.h"
#include "readonly_cache.h"
#include "sw_managed_cache.h"
#include "memory.h"
#include "inst_info.h"

#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ, ## args)


///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////


reg_info_s::reg_info_s()
{
  m_reg  = 0;
  m_type = INT_REG;
  m_id   = 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////


trace_uop_s::trace_uop_s()
{
  m_opcode          = 0;
  m_op_type         = UOP_INV;
  m_mem_type        = NOT_MEM;
  m_cf_type         = NOT_CF;
  m_bar_type        = NOT_BAR;
  m_num_dest_regs   = 0;
  m_num_src_regs    = 0;
  m_mem_size        = 0;
  m_inst_size       = 0;
  m_addr            = 0;
  m_va              = 0;
  m_actual_taken    = false;
  m_target          = 0;
  m_npc             = 0;
  m_pin_2nd_mem     = false;
  m_info            = NULL;
  m_rep_uop_num     = 0;
  m_eom             = false;
  m_alu_uop         = false;
  m_active_mask     = 0;
  m_taken_mask      = 0;
  m_reconverge_addr = 0;
  m_mul_mem_uops    = false;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Constructor
 */
trace_read_c::trace_read_c(macsim_c* simBase)
{
  m_simBase = simBase;

  // map opcode type to uop type 
  init_pin_convert();

  // initialization
  dprint_count = 0;
  dprint_output = new ofstream(*m_simBase->m_knobs->KNOB_STATISTICS_OUT_DIRECTORY + 
                               "/trace_debug.out");
}



/**
 * Destructor
 */
trace_read_c::~trace_read_c()
{
}


/**
 * This function is called once for each thread/warp when the thread/warp is started.
 * The simulator does read-ahead of the trace file to get next pc address
 * @param core_id - core id
 * @param sim_thread_id - thread id
 */
void trace_read_c::setup_trace(int core_id, int sim_thread_id)
{
  core_c* core = m_simBase->m_core_pointers[core_id];
  thread_s* thread_trace_info = core->get_trace_info(sim_thread_id);

  // read one instruction from the trace file to get next instruction. Always one instruction
  // will be read ahead to get next pc address
  if (core->m_running_thread_num) {
    gzread(thread_trace_info->m_trace_file, thread_trace_info->m_prev_trace_info, 
        TRACE_SIZE);

    if (*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ) {
      dprint_inst(thread_trace_info->m_prev_trace_info, core_id, sim_thread_id);
    }
  }
}


/**
 * In case of GPU simulation, from our design, each uncoalcesd accesses will be one 
 * trace instruction. To make these accesses in one instruction, we need to read trace
 * file ahead.
 * @param core_id - core id
 * @param trace_info - trace information to store an instruction
 * @param sim_thread_id - thread id
 * @param inst_read - indicate instruction read successful
 * @see get_uops_from_traces
 */
bool trace_read_c::peek_trace(int core_id, trace_info_s *trace_info, int sim_thread_id, 
    bool *inst_read)
{
  core_c *core                = m_simBase->m_core_pointers[core_id];
  int bytes_read              = -1;
  thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);
  bool ret_val                = false;

  if (!thread_trace_info->m_buffer_exhausted) {
    memcpy(trace_info, 
        &thread_trace_info->m_buffer[TRACE_SIZE * thread_trace_info->m_buffer_index], 
        TRACE_SIZE); 
    thread_trace_info->m_buffer_index = 
      (thread_trace_info->m_buffer_index + 1) % k_trace_buffer_size;

    if (thread_trace_info->m_buffer_index >= thread_trace_info->m_buffer_index_max) {
      bytes_read = 0;
    }
    else {
      bytes_read = TRACE_SIZE;
    }

    // we consume all trace buffer entries
    if (thread_trace_info->m_buffer_index == 0) {
      thread_trace_info->m_buffer_exhausted = true;
    }
  }
  // read one instruction each
  else {
    bytes_read = 
      gzread(thread_trace_info->m_trace_file, trace_info, TRACE_SIZE);
  }

  if (TRACE_SIZE == bytes_read) {
    *inst_read = true;
    ret_val    = true;
  }
  else if (0 == bytes_read) {
    *inst_read = false;
    ret_val    = true;
  }
  else {
    *inst_read = false;
    ret_val    = false;
  }

  return ret_val;
}


/**
 * After peeking trace, in case of failture, we need to rewind trace file.
 * @param core_id - core id
 * @param sim_thread_id - thread id
 * @param num_inst - number of instructions to rewind
 * @see peek_trace
 */
bool trace_read_c::ungetch_trace(int core_id, int sim_thread_id, int num_inst)
{
  core_c *core = m_simBase->m_core_pointers[core_id];
  thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);

  // if we read instructions and store it in the buffer, reduce buffer index only
  if (thread_trace_info->m_buffer_index >= num_inst) {
    thread_trace_info->m_buffer_index -= num_inst;
    return true;
  }
  // more instructions to rewind.
  else if (thread_trace_info->m_buffer_index) {
    num_inst -= thread_trace_info->m_buffer_index;
    thread_trace_info->m_buffer_index = 0;
  }


  // rewind trace file
  off_t offset = gzseek(thread_trace_info->m_trace_file, 
      -1 * num_inst * TRACE_SIZE, SEEK_CUR);

  if (offset == -1) {
    return false;
  }
  return true;
}


/**
 * @param core_id - core id
 * @param trace_info - trace information
 * @param sim_thread_id - thread id
 * @param inst_read - set true if instruction read successful
 * @see get_uops_from_traces
 */
bool trace_read_c::read_trace(int core_id, trace_info_s *trace_info, int sim_thread_id, 
    bool *inst_read)
{
  core_c* core = m_simBase->m_core_pointers[core_id];
  int bytes_read;
  thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);
  process_s* process;

  *inst_read = false;

  while (true) {
    if(core->m_running_thread_num && !thread_trace_info->m_trace_ended) {
      ///
      /// First read chunk of instructions, instead of reading one by one, then read from
      /// buffer using buffer_index. When all instruction have been read (by checking 
      /// buffer index), read another chunk, and so on.
      ///
      if (thread_trace_info->m_buffer_index == 0) {
        thread_trace_info->m_buffer_index_max  = gzread(thread_trace_info->m_trace_file, 
            thread_trace_info->m_buffer, TRACE_SIZE*k_trace_buffer_size);
        thread_trace_info->m_buffer_index_max /= TRACE_SIZE;
        thread_trace_info->m_buffer_exhausted  = false;

      }

      memcpy(trace_info, 
          &(thread_trace_info->m_buffer[TRACE_SIZE * thread_trace_info->m_buffer_index]), 
          TRACE_SIZE);
      thread_trace_info->m_buffer_index = (thread_trace_info->m_buffer_index + 1) % 
        k_trace_buffer_size; 


      if (thread_trace_info->m_buffer_index >= thread_trace_info->m_buffer_index_max) {
        bytes_read = 0;
      }
      else 
        bytes_read = TRACE_SIZE;

      if (thread_trace_info->m_buffer_index == 0) {
        thread_trace_info->m_buffer_exhausted = true;
      }


      if (*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ)
        dprint_inst(trace_info, core_id, sim_thread_id);


      ///
      /// When an application is created, only the main thread will be allocated to the
      /// trace scheduler. When the main thread is executed in trace_read, allocate
      /// all other threads. Each thread can start after certain instruction relative to
      /// the main thread (in case of spawned threads).
      ///
      if (thread_trace_info->m_main_thread) {
        ++thread_trace_info->m_inst_count;
        process = thread_trace_info->m_process;
        int no_created_thread = process->m_no_of_threads_created;

        // create other threads which are not main-thread.
        while((process->m_no_of_threads_created < process->m_no_of_threads &&
              thread_trace_info->m_inst_count == 
              process->m_thread_start_info[no_created_thread].m_inst_count) || 
            (process->m_no_of_threads_created < process->m_no_of_threads &&
             process->m_thread_start_info[no_created_thread].m_inst_count == 0)) {

          // create non-main threads here
          m_simBase->m_process_manager->create_thread_node(process, process->m_no_of_threads_created++, 
              false);
          m_simBase->m_process_manager->sim_thread_schedule();
        }
      }

      // normal trace read
      if (bytes_read == TRACE_SIZE) {
        *inst_read = true;
        break;
      } 
      // trace reading error
      else {
        if (bytes_read == 0) {
          if (!core->m_thread_finished[sim_thread_id]) { 
            thread_trace_info->m_trace_ended = true;

            DEBUG("trace ended core_id:%d thread_id:%d\n", core_id, sim_thread_id);
          } 
          else {
            return false; 
          }
        } 
        else if (bytes_read == -1) {
          report("error reading trace " << thread_trace_info->m_thread_id);
          return false;
        } 
        else {
          report("core_id:" << core_id << " error reading trace");
          return false;
        }
      }
    } // end if( thread_info )
    else {
      break;
    }
  } // end while( true )

  return 1;
}


/**
 * Dump out instruction information to the file. At most 50000 instructions will be printed
 * @param t_info - trace information
 * @param core_id - core id
 * @param thread_id - thread id
 */
void trace_read_c::dprint_inst(trace_info_s *t_info, int core_id, int thread_id) 
{
  if (dprint_count++ >= 50000 || !*m_simBase->m_knobs->KNOB_DEBUG_PRINT_TRACE)
    return ;
 
  *dprint_output << "*** begin of the data strcture *** " << endl;
  *dprint_output << "core_id:" << core_id << " thread_id:" << thread_id << endl;
  *dprint_output << "uop_opcode " <<g_tr_opcode_names[(uint32_t) t_info->m_opcode]  << endl;
  *dprint_output << "num_read_regs: " << hex <<  (uint32_t) t_info->m_num_read_regs << endl;
  *dprint_output << "num_dest_regs: " << hex << (uint32_t) t_info->m_num_dest_regs << endl;
  for (uint32_t ii = 0; ii < (uint32_t) t_info->m_num_read_regs; ++ii)
    *dprint_output << "src" << ii << ": " 
      << hex << g_tr_reg_names[static_cast<uint32_t>(t_info->m_src[ii])] << endl;

  for (uint32_t ii = 0; ii < (uint32_t) t_info->m_num_dest_regs; ++ii)
    *dprint_output << "dst" << ii << ": " 
      << hex << g_tr_reg_names[static_cast<uint32_t>(t_info->m_dst[ii])] << endl;
  *dprint_output << "cf_type: " << hex << g_tr_cf_names[(uint32_t) t_info->m_cf_type] << endl;
  *dprint_output << "has_immediate: " << hex << (uint32_t) t_info->m_has_immediate << endl;
  *dprint_output << "r_dir:" << (uint32_t) t_info->m_rep_dir << endl;
  *dprint_output << "has_st: " << hex << (uint32_t) t_info->m_has_st << endl;
  *dprint_output << "num_ld: " << hex << (uint32_t) t_info->m_num_ld << endl;
  *dprint_output << "mem_read_size: " << hex << (uint32_t) t_info->m_mem_read_size << endl;
  *dprint_output << "mem_write_size: " << hex << (uint32_t) t_info->m_mem_write_size << endl;
  *dprint_output << "is_fp: " << (uint32_t) t_info->m_is_fp << endl;
  *dprint_output << "ld_vaddr1: " << hex << (uint32_t) t_info->m_ld_vaddr1 << endl;
  *dprint_output << "ld_vaddr2: " << hex << (uint32_t) t_info->m_ld_vaddr2 << endl;
  *dprint_output << "st_vaddr: " << hex << (uint32_t) t_info->m_st_vaddr << endl;
  *dprint_output << "instruction_addr: " << hex << (uint32_t)t_info->m_instruction_addr << endl;
  *dprint_output << "branch_target: " << hex << (uint32_t)t_info->m_branch_target << endl;
  *dprint_output << "actually_taken: " << hex << (uint32_t)t_info->m_actually_taken << endl;
  *dprint_output << "write_flg: " << hex << (uint32_t)t_info->m_write_flg << endl;
  *dprint_output << "size: " << hex << (uint32_t) t_info->m_size << endl;
  *dprint_output << "*** end of the data strcture *** " << endl << endl;
}


///////////////////////////////////////////////////////////////////////////////////////////////


// FIXME
/**
 * GPU simulation : Read trace ahead to read synchronization information
 * @param trace_info - trace information
 * @see process_manager_c::sim_thread_schedule
 */
void trace_read_c::pre_read_trace(thread_s* trace_info)
{
  int bytes_read;
  trace_info_s inst_info;
  Section_Type_enum prev_type;
  Section_Type_enum cur_type;
  int32_t m_count;
  section_info_s *sec_info;

  Section_Type_enum bar_prev_type;
  Section_Type_enum bar_cur_type;
  int32_t bar_count;

  Section_Type_enum mem_prev_type;
  Section_Type_enum mem_cur_type;
  int32_t mem_count;

  Section_Type_enum mem_bar_prev_type;
  Section_Type_enum mem_bar_cur_type;
  int32_t mem_bar_count;

  int32_t mem_for_bar_count; 

  int32_t total_count;

  prev_type = bar_prev_type = mem_prev_type = mem_bar_prev_type = SECTION_INVALID;
  m_count = bar_count = mem_count = mem_bar_count = mem_for_bar_count = total_count = 0;


  while ((bytes_read = gzread(trace_info->m_trace_file, &inst_info, 
          TRACE_SIZE)) == TRACE_SIZE) {
    ++total_count;

    switch (inst_info.m_opcode) {
      case TR_MEM_LD_LM:
      case TR_MEM_LD_GM:
      case TR_MEM_ST_LM:
      case TR_MEM_ST_GM:
      case TR_DATA_XFER_LM:
      case TR_DATA_XFER_GM:
        cur_type = SECTION_MEM;
        bar_cur_type = SECTION_COMP;
        mem_cur_type = SECTION_MEM;
        mem_bar_cur_type = SECTION_MEM;

        ++mem_for_bar_count;

        break;
      case XED_CATEGORY_SEMAPHORE:
        cur_type = SECTION_BAR;
        bar_cur_type = SECTION_BAR;
        mem_cur_type = SECTION_COMP;
        mem_bar_cur_type = SECTION_BAR;

        break;
      default:
        cur_type = SECTION_COMP;
        bar_cur_type = SECTION_COMP;
        mem_cur_type = SECTION_COMP;
        mem_bar_cur_type = SECTION_COMP;

        break;
    }


    ++bar_count;
    if (bar_cur_type == SECTION_BAR) {
      sec_info = m_simBase->m_section_pool->acquire_entry();
      sec_info->m_section_type = SECTION_COMP;
      sec_info->m_section_length = bar_count;

      trace_info->m_bar_sections.push_back(sec_info);

      bar_count = 0;

      sec_info = m_simBase->m_section_pool->acquire_entry();
      sec_info->m_section_type = SECTION_MEM;
      sec_info->m_section_length = mem_for_bar_count;

      trace_info->m_mem_for_bar_sections.push_back(sec_info);

      mem_for_bar_count = 0;
    }

    ++mem_count;
    if (mem_cur_type == SECTION_MEM) {
      sec_info = m_simBase->m_section_pool->acquire_entry();
      sec_info->m_section_type = SECTION_COMP;
      sec_info->m_section_length = mem_count;

      trace_info->m_mem_sections.push_back(sec_info);

      mem_count = 0;
    }

    ++mem_bar_count;
    if (mem_bar_cur_type == SECTION_MEM || mem_bar_cur_type == SECTION_BAR) {
      sec_info = m_simBase->m_section_pool->acquire_entry();
      sec_info->m_section_type = SECTION_COMP;
      sec_info->m_section_length = mem_bar_count;

      trace_info->m_mem_bar_sections.push_back(sec_info);

      mem_bar_count = 0;
    }
  }

  if (total_count) {
    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_COMP;
    sec_info->m_section_length = total_count;

    trace_info->m_sections.push_back(sec_info);
  }


  if (bar_count) {
    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_COMP;
    sec_info->m_section_length = bar_count;

    trace_info->m_bar_sections.push_back(sec_info);

    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_MEM;
    sec_info->m_section_length = mem_for_bar_count;

    trace_info->m_mem_for_bar_sections.push_back(sec_info);
  }


  if (mem_count) {
    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_COMP;
    sec_info->m_section_length = mem_count;

    trace_info->m_mem_sections.push_back(sec_info);
  }


  if (mem_bar_count) {
    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_COMP;
    sec_info->m_section_length = mem_bar_count;

    trace_info->m_mem_bar_sections.push_back(sec_info);
  }

  // rewind the trace file
  gzrewind(trace_info->m_trace_file);
}


///////////////////////////////////////////////////////////////////////////////////////////////



/**
 * From statis instruction, add dynamic information such as load address, branch target, ...
 * @param info - instruction information from the hash table
 * @param pi - raw trace information
 * @param trace_uop - MacSim uop type
 * @param rep_offset - repetition offet
 * @param core_id - core id
 */
void trace_read_c::convert_dyn_uop(inst_info_s *info, trace_info_s *pi, trace_uop_s *trace_uop, 
    Addr rep_offset, int core_id)
{
  core_c* core    = m_simBase->m_core_pointers[core_id];
  trace_uop->m_va = 0;

  if (info->m_table_info->m_cf_type) {
    trace_uop->m_actual_taken = pi->m_actually_taken;
    trace_uop->m_target       = pi->m_branch_target;
  
    trace_uop->m_active_mask     = pi->m_ld_vaddr2;
    trace_uop->m_taken_mask      = pi->m_ld_vaddr1;
    trace_uop->m_reconverge_addr = pi->m_st_vaddr;
  } 
  else if (info->m_table_info->m_mem_type) {
    int amp_val = 1;
    if (pi->m_opcode != XED_CATEGORY_STRINGOP) {
      amp_val = *m_simBase->m_knobs->KNOB_MEM_SIZE_AMP;
    }

    if (info->m_table_info->m_mem_type == MEM_ST || 
        info->m_table_info->m_mem_type == MEM_ST_LM || 
        info->m_table_info->m_mem_type == MEM_ST_SM) {
      trace_uop->m_va = MIN2((pi->m_st_vaddr + rep_offset)*amp_val, MAX_ADDR);
      if (core->get_core_type() == "ptx")
        trace_uop->m_mem_size = pi->m_mem_write_size * amp_val;
      else   
        trace_uop->m_mem_size = MIN2((pi->m_mem_write_size)*amp_val, REP_MOV_MEM_SIZE_MAX_NEW);
    } 
    else if ((info->m_table_info->m_mem_type == MEM_LD) || 
        (info->m_table_info->m_mem_type == MEM_LD_LM) || 
        (info->m_table_info->m_mem_type == MEM_LD_SM) || 
        (info->m_table_info->m_mem_type == MEM_LD_CM) || 
        (info->m_table_info->m_mem_type == MEM_LD_TM) || 
        (info->m_table_info->m_mem_type == MEM_LD_PM) || 
        (info->m_table_info->m_mem_type == MEM_PF) || 
        (info->m_table_info->m_mem_type >= MEM_SWPREF_NTA && 
         info->m_table_info->m_mem_type <= MEM_SWPREF_T2)) {
      if (info->m_trace_info.m_second_mem)
        trace_uop->m_va = MIN2((pi->m_ld_vaddr2 + rep_offset)*amp_val, MAX_ADDR);
      else 
        trace_uop->m_va = MIN2((pi->m_ld_vaddr1 +  rep_offset)*amp_val, MAX_ADDR);

      if (core->get_core_type() == "ptx")
        trace_uop->m_mem_size = pi->m_mem_read_size * amp_val;
      else
        trace_uop->m_mem_size = MIN2((pi->m_mem_read_size)*amp_val, REP_MOV_MEM_SIZE_MAX_NEW);
    }
  }

  // next pc
  trace_uop->m_npc = trace_uop->m_addr;
}


///////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Convert instruction information (from hash table) to trace uop
 * @param info - instruction information from the hash table
 * @param trace_uop - MacSim trace format
 */
void trace_read_c::convert_info_uop(inst_info_s *info, trace_uop_s *trace_uop)
{
  trace_uop->m_op_type       = info->m_table_info->m_op_type;
  trace_uop->m_mem_type      = info->m_table_info->m_mem_type;
  trace_uop->m_cf_type       = info->m_table_info->m_cf_type;
  trace_uop->m_bar_type      = info->m_table_info->m_bar_type;
  trace_uop->m_num_dest_regs = info->m_table_info->m_num_dest_regs;
  trace_uop->m_num_src_regs  = info->m_table_info->m_num_src_regs;
  trace_uop->m_mem_size      = info->m_table_info->m_mem_size;
  trace_uop->m_addr          = info->m_addr;
  trace_uop->m_inst_size     = info->m_trace_info.m_inst_size;
  trace_uop->m_num_src_regs  = info->m_table_info->m_num_src_regs;
  trace_uop->m_num_dest_regs = info->m_table_info->m_num_dest_regs;

  for (int ii = 0; ii < info->m_table_info->m_num_src_regs; ++ii) {
    trace_uop->m_srcs[ii].m_type = INT_REG;
    trace_uop->m_srcs[ii].m_id   = info->m_srcs[ii].m_id;
    trace_uop->m_srcs[ii].m_reg  = info->m_srcs[ii].m_reg;
  }

  for (int ii = 0; ii < info->m_table_info->m_num_dest_regs; ++ii) {
    trace_uop->m_dests[ii].m_id   = info->m_dests[ii].m_id ;
    trace_uop->m_dests[ii].m_reg  = info->m_dests[ii].m_reg;
    ASSERT(trace_uop->m_dests[ii].m_reg < NUM_REG_IDS);
    trace_uop->m_dests[ii].m_type = INT_REG;
  }

  trace_uop->m_pin_2nd_mem = info->m_trace_info.m_second_mem;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Convert MacSim trace to instruction information (hash table)
 * @param t_uop - MacSim trace
 * @param info - instruction information in hash table
 */
void trace_read_c::convert_t_uop_to_info(trace_uop_s *t_uop, inst_info_s *info)
{
  info->m_table_info->m_op_type       = t_uop->m_op_type;
  info->m_table_info->m_mem_type      = t_uop->m_mem_type;
  info->m_table_info->m_cf_type       = t_uop->m_cf_type;
  info->m_table_info->m_bar_type      = t_uop->m_bar_type;
  info->m_table_info->m_num_dest_regs = t_uop->m_num_dest_regs;
  info->m_table_info->m_num_src_regs  = t_uop->m_num_src_regs;
  info->m_table_info->m_mem_size      = t_uop->m_mem_size;                              
  info->m_table_info->m_type          = 0;                              
  info->m_table_info->m_mask          = 0;                             
  info->m_addr                        = t_uop->m_addr;
  info->m_trace_info.m_inst_size      = (t_uop->m_inst_size); 
  info->m_table_info->m_num_src_regs  = t_uop->m_num_src_regs;
  info->m_table_info->m_num_dest_regs = t_uop->m_num_dest_regs;

  ASSERTM(t_uop->m_num_dest_regs <= MAX_DESTS, "num_dest_regs:%d ", t_uop->m_num_dest_regs);
  ASSERTM(t_uop->m_num_src_regs <= MAX_SRCS, "num_src_regs:%d  ", t_uop->m_num_src_regs);

  for (int ii = 0; ii < info->m_table_info->m_num_src_regs; ++ii) {
    info->m_srcs[ii].m_type = INT_REG;
    info->m_srcs[ii].m_id   = t_uop->m_srcs[ii].m_id;
    info->m_srcs[ii].m_reg  = t_uop->m_srcs[ii].m_reg;
  }

  // only one destination - temporary that is going to be read by the second t_uop
  for (int ii = 0; ii < info->m_table_info->m_num_dest_regs; ++ii) {
    info->m_table_info->m_num_dest_regs = 1;
    info->m_dests[ii].m_type = INT_REG;
    info->m_dests[ii].m_id   = t_uop->m_dests[ii].m_id;
    info->m_dests[ii].m_reg  = t_uop->m_dests[ii].m_reg;
  }

  info->m_trace_info.m_second_mem = t_uop->m_pin_2nd_mem;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Function to decode an instruction from the trace file into a sequence of uops
 * @param pi - raw trace format
 * @param trace_uop - micro uops storage for this instruction
 * @param core_id - core id
 * @param sim_thread_id - thread id  
 */
inst_info_s* trace_read_c::convert_pinuop_to_t_uop(trace_info_s *pi, trace_uop_s **trace_uop, 
    int core_id, int sim_thread_id)
{
  core_c* core = m_simBase->m_core_pointers[core_id];

  // simulator maintains a cache of decoded instructions (uop) for each process, 
  // this avoids decoding of instructions everytime an instruction is executed
  int process_id = core->get_trace_info(sim_thread_id)->m_process->m_process_id;
  hash_c<inst_info_s>* htable = m_simBase->m_inst_info_hash[process_id];


  // since each instruction can be decoded into multiple uops, the key to the 
  // hashtable has to be (instruction addr + something else)
  // the instruction addr is shifted left by 3-bits and the number of the uop 
  // in the decoded sequence is added to the shifted value to obtain the key
  bool new_entry = false;
  Addr key_addr = (pi->m_instruction_addr << 3);


  // Get instruction information from the hash table if exists. 
  // Else create a new entry
  inst_info_s *info = htable->hash_table_access_create(key_addr, &new_entry);

  inst_info_s *first_info = info;
  int  num_uop = 0;
  int  dyn_uop_counter = 0;
  bool tmp_reg_needed = false;
  bool inst_has_ALU_uop = false;
  bool inst_has_ld_uop = false;
  int  ii, jj, kk;

  if (new_entry) {
    // Since we found a new instruction, we need to decode this instruction and store all
    // uops to the hash table
    int write_dest_reg = 0;

    trace_uop[0]->m_rep_uop_num = 0;
    trace_uop[0]->m_opcode = pi->m_opcode;

    // temporal register rules:
    // load->dest_reg (through tmp), load->store (through tmp), dest_reg->store (real reg)
    // load->cf (through tmp), dest_reg->cf (thought dest), st->cf (no dependency)
    

    ///
    /// 1. This instruction has a memory load operation
    ///
    if (pi->m_num_ld) {
      num_uop = 1;

      // set memory type
      switch (pi->m_opcode) {
        case XED_CATEGORY_PREFETCH:
          trace_uop[0]->m_mem_type = MEM_PF;
          break;
        case TR_MEM_LD_GM:
        case TR_DATA_XFER_GM:
          trace_uop[0]->m_mem_type = MEM_LD;
          break;
        // XFER_LM is not valid for current PTX
        // check ptx manual, Atom and Red instrutions can be in only global/shared
        case TR_MEM_LD_LM:
        case TR_DATA_XFER_LM:
          trace_uop[0]->m_mem_type = MEM_LD_LM;
          break;
        case TR_MEM_LD_SM:
        case TR_DATA_XFER_SM:
          trace_uop[0]->m_mem_type = MEM_LD_SM;
          break;
        case TR_MEM_LD_CM:
          trace_uop[0]->m_mem_type = MEM_LD_CM;
          break;
        case TR_MEM_LD_TM:
          trace_uop[0]->m_mem_type = MEM_LD_TM;
          break;
        // parameter access same as global access
        case TR_MEM_LD_PM:
          trace_uop[0]->m_mem_type = MEM_LD; 
          break;
        default:
          trace_uop[0]->m_mem_type = MEM_LD;
          break;
      }
      
      // prefetch instruction
      if (pi->m_opcode == XED_CATEGORY_PREFETCH 
          || (pi->m_opcode >= PREFETCH_NTA && pi->m_opcode <= PREFETCH_T2)) {
        switch (pi->m_opcode) {
          case PREFETCH_NTA:
            trace_uop[0]->m_mem_type = MEM_SWPREF_NTA;
            break;

          case PREFETCH_T0:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T0;
            break;

          case PREFETCH_T1:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T1;
            break;

          case PREFETCH_T2:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T2;
            break;
        }
      }
   

      trace_uop[0]->m_cf_type  = NOT_CF;
      trace_uop[0]->m_op_type  = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      trace_uop[0]->m_bar_type = NOT_BAR;

      if (pi->m_opcode == XED_CATEGORY_DATAXFER || core->get_core_type() == "ptx") {
        trace_uop[0]->m_num_dest_regs = pi->m_num_dest_regs;
      }
      else {
        trace_uop[0]->m_num_dest_regs = 0;
      }

      trace_uop[0]->m_num_src_regs = pi->m_num_read_regs;
      trace_uop[0]->m_pin_2nd_mem  = 0;
      trace_uop[0]->m_eom      = 0;
      trace_uop[0]->m_alu_uop  = false;
      trace_uop[0]->m_inst_size = pi->m_size;

      // m_has_immediate is overloaded - in case of ptx simulations, for uncoalesced 
      // accesses, multiple memory access are generated and for each access there is
      // an instruction in the trace. the m_has_immediate flag is used to mark the
      // first and last accesses of an uncoalesced memory instruction
      trace_uop[0]->m_mul_mem_uops = pi->m_has_immediate; 


      ///
      /// There are two load operations in an instruction. Note that now array index becomes 1
      ///
      if (pi->m_num_ld == 2) {
        trace_uop[1]->m_opcode = pi->m_opcode;

        switch (pi->m_opcode) {
          case XED_CATEGORY_PREFETCH:
            trace_uop[1]->m_mem_type = MEM_PF;
            break;
          // for ptx traces, aon, num_ld will always be 1
          case TR_MEM_LD_GM:
          case TR_DATA_XFER_GM:
          case TR_MEM_LD_LM:
          case TR_DATA_XFER_LM:
          case TR_MEM_LD_SM:
          case TR_DATA_XFER_SM:
          case TR_MEM_LD_CM:
          case TR_MEM_LD_TM:
          case TR_MEM_LD_PM:
            ASSERT(0);
            break;
          default:
            trace_uop[1]->m_mem_type = MEM_LD;
            break;
        }
        
        // prefetch instruction
        if (pi->m_opcode == XED_CATEGORY_PREFETCH || 
            (pi->m_opcode >= PREFETCH_NTA && pi->m_opcode <= PREFETCH_T2)) {
          switch (pi->m_opcode) {
            case PREFETCH_NTA:
              trace_uop[1]->m_mem_type = MEM_SWPREF_NTA;
              break;
            case PREFETCH_T0:
              trace_uop[1]->m_mem_type = MEM_SWPREF_T0;
              break;
            case PREFETCH_T1:
              trace_uop[1]->m_mem_type = MEM_SWPREF_T1;
              break;
            case PREFETCH_T2:
              trace_uop[1]->m_mem_type = MEM_SWPREF_T2;
              break;
          }
        }

        trace_uop[1]->m_cf_type    = NOT_CF;
        trace_uop[1]->m_op_type    = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
        trace_uop[1]->m_bar_type   = NOT_BAR;
        trace_uop[1]->m_num_dest_regs = 0;
        trace_uop[1]->m_num_src_regs = pi->m_num_read_regs;

        if (pi->m_opcode == XED_CATEGORY_DATAXFER || core->get_core_type() == "ptx") {
          trace_uop[1]->m_num_dest_regs = pi->m_num_dest_regs;
        }
        else {
          trace_uop[1]->m_num_dest_regs = 0;
        }

        trace_uop[1]->m_pin_2nd_mem  = 1;
        trace_uop[1]->m_eom       = 0;
        trace_uop[1]->m_alu_uop      = false;
        trace_uop[1]->m_inst_size    = pi->m_size;
        trace_uop[1]->m_mul_mem_uops = pi->m_has_immediate; // uncoalesced memory accesses

        num_uop = 2;
      } // NUM_LOAD == 2


      if (pi->m_opcode == XED_CATEGORY_DATAXFER || core->get_core_type() == "ptx") {
        write_dest_reg = 1;
      }

      if (trace_uop[0]->m_mem_type == MEM_LD || 
          trace_uop[0]->m_mem_type == MEM_LD_LM || 
          trace_uop[0]->m_mem_type == MEM_LD_SM || 
          trace_uop[0]->m_mem_type == MEM_LD_CM || 
          trace_uop[0]->m_mem_type == MEM_LD_TM || 
          trace_uop[0]->m_mem_type == MEM_LD_PM) {
        inst_has_ld_uop = true;
      }
    } // HAS_LOAD


    // Add one more uop when temporary register is required
    if (pi->m_num_dest_regs && !write_dest_reg) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop) {
       tmp_reg_needed = true;
      }
   
      cur_trace_uop->m_opcode        = pi->m_opcode;
      cur_trace_uop->m_mem_type    = NOT_MEM;
      cur_trace_uop->m_cf_type     = NOT_CF;
      cur_trace_uop->m_op_type     = (Uop_Type)((pi->m_is_fp) ? 
          m_fp_uop_table[pi->m_opcode] : 
          m_int_uop_table[pi->m_opcode]);
      cur_trace_uop->m_bar_type    = NOT_BAR;
      cur_trace_uop->m_num_src_regs = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = pi->m_num_dest_regs;
      cur_trace_uop->m_pin_2nd_mem  = 0;
      cur_trace_uop->m_eom        = 0;
      cur_trace_uop->m_alu_uop     = true;

      inst_has_ALU_uop = true;
    }


    ///
    /// 2. Instruction has a memory store operation
    ///
    if (pi->m_has_st) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop) 
        tmp_reg_needed = true;
   
      // set memory type
      switch (pi->m_opcode) {
        case TR_MEM_ST_GM:
        case TR_DATA_XFER_GM:
          cur_trace_uop->m_mem_type = MEM_ST;
          break;

        case TR_MEM_ST_LM:
        case TR_DATA_XFER_LM: // XFER_LM is not valid for current PTX
          cur_trace_uop->m_mem_type = MEM_ST_LM;
          break;
        case TR_MEM_ST_SM:
        case TR_DATA_XFER_SM:
          cur_trace_uop->m_mem_type = MEM_ST_SM;
          break;
        default:
          cur_trace_uop->m_mem_type = MEM_ST;
          break;
      }

      cur_trace_uop->m_opcode        = pi->m_opcode;
      cur_trace_uop->m_cf_type       = NOT_CF;
      cur_trace_uop->m_op_type       = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      cur_trace_uop->m_bar_type      = NOT_BAR;
      cur_trace_uop->m_num_src_regs  = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = 0;
      cur_trace_uop->m_pin_2nd_mem   = 0;
      cur_trace_uop->m_eom           = 0;
      cur_trace_uop->m_alu_uop       = false;
      cur_trace_uop->m_inst_size     = pi->m_size;
      cur_trace_uop->m_mul_mem_uops  = pi->m_has_immediate; // uncoalesced memory accesses
    }


    ///
    /// 3. Instruction has a branch operation
    ///
    if (pi->m_cf_type) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];

      if (inst_has_ld_uop)
        tmp_reg_needed = true;
   
      cur_trace_uop->m_mem_type      = NOT_MEM;
      cur_trace_uop->m_cf_type       = (Cf_Type)((pi->m_cf_type >= PIN_CF_SYS) ? 
          CF_ICO : pi->m_cf_type);
      cur_trace_uop->m_op_type       = UOP_CF;
      cur_trace_uop->m_bar_type      = NOT_BAR;
      cur_trace_uop->m_num_src_regs  = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = 0;
      cur_trace_uop->m_pin_2nd_mem   = 0;
      cur_trace_uop->m_eom           = 0;
      cur_trace_uop->m_alu_uop       = false;
      cur_trace_uop->m_inst_size     = pi->m_size;
    }


    ///
    /// Non-memory, non-branch instruction
    ///
    if (num_uop == 0) {
      trace_uop[0]->m_opcode        = pi->m_opcode;
      trace_uop[0]->m_mem_type      = NOT_MEM;
      trace_uop[0]->m_cf_type       = NOT_CF;
      trace_uop[0]->m_op_type       = UOP_NOP;
      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs  = 0;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 1;
      trace_uop[0]->m_inst_size     = pi->m_size;
      ++num_uop;
    }


    info->m_trace_info.m_bom     = true;
    info->m_trace_info.m_eom     = false;
    info->m_trace_info.m_num_uop = num_uop;


    ///
    /// Process each static uop to dynamic uop
    ///
    for (ii = 0; ii < num_uop; ++ii) {
      // For the first uop, we have already created hash entry. However, for following uops
      // we need to create hash entries
      if (ii > 0) {
        key_addr                 = ((pi->m_instruction_addr << 3) + ii);
        info                     = htable->hash_table_access_create(key_addr, &new_entry);
        info->m_trace_info.m_bom = false;
        info->m_trace_info.m_eom = false;
      }
      ASSERTM(new_entry, "Add new uops to hash_table for core id::%d\n", core_id);

      trace_uop[ii]->m_addr = pi->m_instruction_addr;

      DEBUG("pi->instruction_addr:0x%s trace_uop[%d]->addr:0x%s num_src_regs:%d "
          "num_read_regs:%d pi:num_dst_regs:%d uop:num_dst_regs:%d \n",
          hexstr64s(pi->m_instruction_addr), ii, hexstr64s(trace_uop[ii]->m_addr), 
          trace_uop[ii]->m_num_src_regs, pi->m_num_read_regs, pi->m_num_dest_regs, 
          trace_uop[ii]->m_num_dest_regs);

      // set source register
      for (jj = 0; jj < trace_uop[ii]->m_num_src_regs; ++jj) {
        (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_srcs[jj].m_id   = pi->m_src[jj];
        (trace_uop[ii])->m_srcs[jj].m_reg  = pi->m_src[jj];
      }

      // store or control flow has a dependency whoever the last one
      if ((trace_uop[ii]->m_mem_type == MEM_ST) || 
          (trace_uop[ii]->m_mem_type == MEM_ST_LM) || 
          (trace_uop[ii]->m_mem_type == MEM_ST_SM) || 
          (trace_uop[ii]->m_cf_type != NOT_CF)) {

        if (tmp_reg_needed && !inst_has_ALU_uop) {
          (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
          (trace_uop[ii])->m_srcs[jj].m_id  = TR_REG_TMP0;
          (trace_uop[ii])->m_srcs[jj].m_reg  = TR_REG_TMP0;
          trace_uop[ii]->m_num_src_regs += 1;
        } 
        else if (inst_has_ALU_uop) {
          for (kk = 0; kk < pi->m_num_dest_regs; ++kk) {
            (trace_uop[ii])->m_srcs[jj+kk].m_type = (Reg_Type)0;
            (trace_uop[ii])->m_srcs[jj+kk].m_id   = pi->m_dst[kk];
            (trace_uop[ii])->m_srcs[jj+kk].m_reg  = pi->m_dst[kk];
          }

          trace_uop[ii]->m_num_src_regs += pi->m_num_dest_regs;
        }
      }

      // alu uop only has a dependency with a temp register
      if (trace_uop[ii]->m_alu_uop) {
        if (tmp_reg_needed) {
          (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
          (trace_uop[ii])->m_srcs[jj].m_id  = TR_REG_TMP0;
          (trace_uop[ii])->m_srcs[jj].m_reg  = TR_REG_TMP0;
          trace_uop[ii]->m_num_src_regs     += 1;
        }
      }

      for (jj = 0; jj < trace_uop[ii]->m_num_dest_regs; ++jj) {
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id   = pi->m_dst[jj];
        (trace_uop[ii])->m_dests[jj].m_reg = pi->m_dst[jj];
      }

      // add tmp register as a destination register
      if (tmp_reg_needed && 
          ((trace_uop[ii]->m_mem_type == MEM_LD) || 
           (trace_uop[ii]->m_mem_type == MEM_LD_LM) ||
           (trace_uop[ii]->m_mem_type == MEM_LD_SM) || 
           (trace_uop[ii]->m_mem_type == MEM_LD_CM) ||
           (trace_uop[ii]->m_mem_type == MEM_LD_TM) || 
           (trace_uop[ii]->m_mem_type == MEM_LD_PM))) {
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id   = TR_REG_TMP0;
        (trace_uop[ii])->m_dests[jj].m_reg = TR_REG_TMP0;
        trace_uop[ii]->m_num_dest_regs     += 1;
      }


      // the last uop
      if (ii == (num_uop - 1) && trace_uop[num_uop-1]->m_mem_type == NOT_MEM) {
        /* last uop's info */
        if ((pi->m_opcode == XED_CATEGORY_SEMAPHORE) ||
            (pi->m_opcode == XED_CATEGORY_IO) ||
            (pi->m_opcode == XED_CATEGORY_INTERRUPT) ||
            (pi->m_opcode == XED_CATEGORY_SYSTEM) ||
            (pi->m_opcode == XED_CATEGORY_SYSCALL) ||
            (pi->m_opcode == XED_CATEGORY_SYSRET)) {
          // only the last instruction will have bar type
          trace_uop[(num_uop-1)]->m_bar_type = BAR_FETCH;
          trace_uop[(num_uop-1)]->m_cf_type  = CF_ICO;
        }
      }

      // update instruction information with MacSim trace
      convert_t_uop_to_info(trace_uop[ii], info);

      DEBUG("tuop: pc %s num_src_reg:%d num_dest_reg:%d \n",
          hexstr64s(trace_uop[ii]->m_addr), trace_uop[ii]->m_num_src_regs, 
          trace_uop[ii]->m_num_dest_regs);
      
      trace_uop[ii]->m_info = info;

      // Add dynamic information to the uop
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);
    }
  
    // set end of macro flag to the last uop
    trace_uop[num_uop - 1]->m_eom = 1;

    ASSERT(num_uop > 0);
  } // NEW_ENTRY
  ///
  /// Hash table already has matching instruction, we can skip above decoding process
  ///
  else {
    ASSERT(info);

    num_uop = info->m_trace_info.m_num_uop;
    for (ii = 0; ii < num_uop; ++ii) {
      if (ii > 0) {
        key_addr  = ((pi->m_instruction_addr << 3) + ii);
        info      = htable->hash_table_access_create(key_addr, &new_entry);
      }
      ASSERTM(!new_entry, "Core id %d index %d\n", core_id, ii);

      // convert raw instruction trace to MacSim trace format
      convert_info_uop(info, trace_uop[ii]);

      // add dynamic information
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);

      trace_uop[ii]->m_info   = info;
      trace_uop[ii]->m_eom    = 0;
      trace_uop[ii]->m_addr   = pi->m_instruction_addr;
      trace_uop[ii]->m_opcode = pi->m_opcode;
      if (core->get_core_type() == "ptx" && trace_uop[ii]->m_mem_type) {
        //nagesh mar-10-2010 - to form single uop for uncoalesced memory accesses
        //this checking should be done for every instance of the instruction,
        //not for only the first instance, because depending on the address
        //calculation, some accesses may be coalesced, some may be uncoalesced
        trace_uop[ii]->m_mul_mem_uops = pi->m_has_immediate; 
      }
    }

    // set end of macro flag to the last uop
    trace_uop[num_uop-1]->m_eom = 1;

    ASSERT(num_uop > 0);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  // end of instruction decoding
  /////////////////////////////////////////////////////////////////////////////////////////////

  dyn_uop_counter = num_uop;

  ///
  /// Repeat move
  ///
  if (pi->m_opcode == XED_CATEGORY_STRINGOP) {
    int rep_counter = 1;
    int rep_dir     = 0;

    // generate multiple uops with different memory addresses
    key_addr  = ((pi->m_instruction_addr << 3));
    info      = htable->hash_table_access_create(key_addr, &new_entry);

    int rep_mem_size = (int) pi->m_mem_read_size;

    if (rep_mem_size == 0) {
      trace_uop[0]->m_mem_type      = NOT_MEM;
      trace_uop[0]->m_cf_type       = NOT_CF;
      trace_uop[0]->m_op_type       = UOP_NOP;
      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs  = 0;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 1;
      trace_uop[0]->m_inst_size     = pi->m_size;
      
      num_uop = 1;

      // update instruction information with MacSim trace
      convert_t_uop_to_info(trace_uop[0], info);

      trace_uop[0]->m_info = info;

      // add dynamic information
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);

      dyn_uop_counter      = 1;
      trace_uop[0]->m_eom  = 1;
      trace_uop[0]->m_addr = pi->m_instruction_addr;
    } 
    else {
      rep_mem_size -= REP_MOV_MEM_SIZE_MAX_NEW;
      rep_dir       = pi->m_rep_dir;

      while ((rep_mem_size > 0 ) && (dyn_uop_counter < MAX_PUP)) {
        int rep_offset = (rep_dir == 1) ? 
          (REP_MOV_MEM_SIZE_MAX_NEW * (rep_counter) * (-1)) : 
          (REP_MOV_MEM_SIZE_MAX_NEW * (rep_counter) * (1));

        trace_uop[dyn_uop_counter-1]->m_npc = pi->m_instruction_addr;

        ASSERT(num_uop < 8);
        for (ii = 0; ii < num_uop; ++ii) {
          // can't skip when ii = 0; because this routine is repeating ...
          key_addr  = ((pi->m_instruction_addr << 3) + ii);
          info      = htable->hash_table_access_create(key_addr, &new_entry);

          info->m_trace_info.m_bom = false;
          info->m_trace_info.m_eom = false;

          ASSERT(!new_entry);

          // add dynamic information
          convert_dyn_uop(info, pi, trace_uop[dyn_uop_counter], rep_offset, core_id);

          trace_uop[dyn_uop_counter]->m_mem_size = MIN2(rep_mem_size, REP_MOV_MEM_SIZE_MAX_NEW);
          trace_uop[dyn_uop_counter]->m_info     = info;
          trace_uop[dyn_uop_counter]->m_eom      = 0;
          trace_uop[dyn_uop_counter]->m_addr     = pi->m_instruction_addr;
          ++dyn_uop_counter;
        }

        rep_mem_size -= REP_MOV_MEM_SIZE_MAX_NEW;
        ++rep_counter;
      }

      ASSERT(dyn_uop_counter > 0);
    }

    trace_uop[0]->m_rep_uop_num = dyn_uop_counter;
  } // XED_CATEGORY_STRINGOP

  ASSERT(dyn_uop_counter);


  // set eom flag and next pc address for the last uop of this instruction
  trace_uop[dyn_uop_counter-1]->m_eom = 1;
  trace_uop[dyn_uop_counter-1]->m_npc = pi->m_instruction_next_addr;

  STAT_CORE_EVENT(core_id, OP_CAT_INVALID + (pi->m_opcode)); 

  switch (pi->m_opcode) {
    case XED_CATEGORY_INTERRUPT:
    case XED_CATEGORY_IO:
    case XED_CATEGORY_IOSTRINGOP:
    case XED_CATEGORY_MISC:
    case XED_CATEGORY_RET:
    case XED_CATEGORY_ROTATE:
    case XED_CATEGORY_SEMAPHORE:
    case XED_CATEGORY_SYSCALL:
    case XED_CATEGORY_SYSRET:
    case XED_CATEGORY_SYSTEM:
    case XED_CATEGORY_VTX:
    case XED_CATEGORY_XSAVE:
      STAT_CORE_EVENT(core_id, POWER_CONTROL_REGISTER_W);
      break;

  }

  if (pi->m_num_ld || pi->m_has_st) {
    STAT_CORE_EVENT(core_id, POWER_SEGMENT_REGISTER_W);
  }

  ASSERT(num_uop > 0);
  first_info->m_trace_info.m_num_uop = num_uop;

  return first_info;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Get an uop from trace
 * Called by frontend.cc
 * @param core_id - core id
 * @param uop - uop object to hold instruction information
 * @param sim_thread_id thread id
 */
bool trace_read_c::get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id)
{
  ASSERT(uop);

  trace_uop_s *trace_uop;
  trace_uop_s *next_trace_uop;
  int num_uop  = 0;
  core_c* core = m_simBase->m_core_pointers[core_id];
  inst_info_s *info;

  // fetch ended : no uop to fetch
  if (core->m_fetch_ended[sim_thread_id]) 
    return false;

  trace_info_s trace_info;
  trace_info_s next_inst_trace_info;
  bool read_success = true;
  thread_s* thread_trace_info = core->get_trace_info(sim_thread_id);

  if (thread_trace_info->m_thread_init) {
    thread_trace_info->m_thread_init = false;
  }


  ///
  /// BOM (beginning of macro) : need to get a next instruction
  ///
  if (thread_trace_info->m_bom) {
    bool inst_read; // indicate new instruction has been read from a trace file
    int stride;
    
    if (core->m_inst_fetched[sim_thread_id] < *m_simBase->m_knobs->KNOB_MAX_INSTS)  {
      // read next instruction
      read_success = read_trace(core_id, thread_trace_info->m_next_trace_info, 
          sim_thread_id, &inst_read);
    }
    else {
      inst_read = false;
      if (!core->get_trace_info(sim_thread_id)->m_trace_ended) { 
        core->get_trace_info(sim_thread_id)->m_trace_ended = true;
      }
    }


    // Copy current instruction to data structure
    memcpy(&trace_info, thread_trace_info->m_prev_trace_info, sizeof(trace_info_s));

    // Set next pc address
    trace_info.m_instruction_next_addr = 
      thread_trace_info->m_next_trace_info->m_instruction_addr;

    // Copy next instruction to current instruction field
    memcpy(thread_trace_info->m_prev_trace_info, thread_trace_info->m_next_trace_info, 
        sizeof(trace_info_s));

    DEBUG("trace_read nm core_id:%d thread_id:%d pc:%s opcode:%d inst_count:%lu\n",
        core_id, sim_thread_id, hexstr64s(trace_info.m_instruction_addr), 
        static_cast<int>(trace_info.m_opcode), thread_trace_info->m_temp_inst_count);


    ///
    /// Trace read failed
    ///
    if (!read_success) 
      return false;


    // read a new instruction, so update stats
    if (inst_read) { 
      ++core->m_inst_fetched[sim_thread_id];
      DEBUG("core_id:%d thread_id:%d inst_num:%lu\n",
          core_id, sim_thread_id, thread_trace_info->m_temp_inst_count + 1);

      if (core->m_inst_fetched[sim_thread_id] > core->m_max_inst_fetched) 
        core->m_max_inst_fetched = core->m_inst_fetched[sim_thread_id];
    }


    // GPU simulation : if we use common cache for the shared memory
    // Set appropiriate opcode type (not using shared memory)
    if (core->get_core_type() == "ptx" && *m_simBase->m_knobs->KNOB_PTX_COMMON_CACHE) {
      switch (trace_info.m_opcode) {
        case TR_MEM_LD_SM:
          trace_info.m_opcode = TR_MEM_LD_LM;
          break;
        case TR_MEM_ST_SM:
          trace_info.m_opcode = TR_MEM_ST_LM;
          break;
        case TR_DATA_XFER_SM:
          trace_info.m_opcode = TR_DATA_XFER_LM;
          break;
      }
    }


    // So far we have raw instruction format, so we need to MacSim specific trace format
    info = convert_pinuop_to_t_uop(&trace_info, thread_trace_info->m_trace_uop_array, 
        core_id, sim_thread_id);


    trace_uop = thread_trace_info->m_trace_uop_array[0];
    num_uop   = info->m_trace_info.m_num_uop;
    ASSERT(info->m_trace_info.m_num_uop > 0);

    thread_trace_info->m_num_sending_uop = 1;
    thread_trace_info->m_eom             = thread_trace_info->m_trace_uop_array[0]->m_eom;
    thread_trace_info->m_bom             = false;

    uop->m_isitBOM = true;
    STAT_CORE_EVENT(core_id, POWER_INST_DECODER_R);
    STAT_CORE_EVENT(core_id, POWER_MICRO_OP_SEQ_R);
  } // END EOM
  // read remaining uops from the same instruction
  else { 
    trace_uop                = 
      thread_trace_info->m_trace_uop_array[thread_trace_info->m_num_sending_uop];
    info                     = trace_uop->m_info;
    thread_trace_info->m_eom = trace_uop->m_eom;
    info->m_trace_info.m_bom = 0; // because of repeat instructions ....
    uop->m_isitBOM           = false;
    ++thread_trace_info->m_num_sending_uop;
  }


  // set end of macro flag
  if (thread_trace_info->m_eom) {
    uop->m_isitEOM           = true; // mark for current uop
    thread_trace_info->m_bom = true; // mark for next instruction
  }
  else {
    uop->m_isitEOM           = false;
    thread_trace_info->m_bom = false;
  }


  if (core->get_trace_info(sim_thread_id)->m_trace_ended && uop->m_isitEOM) {
    --core->m_fetching_thread_num;
    core->m_fetch_ended[sim_thread_id] = true;
    uop->m_last_uop                    = true;
    DEBUG("core_id:%d thread_id:%d inst_num:%lld uop_num:%lld fetched:%lld last uop\n",
        core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num, 
        core->m_inst_fetched[sim_thread_id]);
  }


  /* BAR_FETCH */
  if (core->get_core_type() == "ptx") {
    if (trace_uop->m_bar_type == BAR_FETCH) { //only last uop with have BAR_FETCH set
      frontend_c *frontend   = core->get_frontend();
      frontend_s *fetch_data = core->get_trace_info(sim_thread_id)->m_fetch_data;

      fetch_data->m_fetch_blocked = true;

      bool new_entry = false;
      sync_thread_s* sync_info = frontend->m_sync_info->hash_table_access_create(
          core->get_trace_info(sim_thread_id)->m_block_id, &new_entry);

      // new synchronization information
      if (new_entry) {
        sync_info->m_block_id = core->get_trace_info(sim_thread_id)->m_block_id;
        sync_info->m_sync_count = 0;
        sync_info->m_num_threads_in_block = 
          m_simBase->m_block_schedule_info[sync_info->m_block_id]->m_total_thread_num;
      }

      ++fetch_data->m_sync_count;
      fetch_data->m_sync_wait_start = core->get_cycle_count();
   
    }
  }


  ///
  /// Set up actual uop data structure
  ///
  uop->m_opcode      = trace_uop->m_opcode;
  uop->m_uop_type    = info->m_table_info->m_op_type;
  uop->m_cf_type     = info->m_table_info->m_cf_type;
  uop->m_mem_type    = info->m_table_info->m_mem_type;
  ASSERT(uop->m_mem_type >= 0 && uop->m_mem_type < NUM_MEM_TYPES);
  uop->m_bar_type    = trace_uop->m_bar_type;
  uop->m_npc         = trace_uop->m_npc;
  uop->m_active_mask = trace_uop->m_active_mask;

  if (uop->m_cf_type) { 
    uop->m_taken_mask      = trace_uop->m_taken_mask;
    uop->m_reconverge_addr = trace_uop->m_reconverge_addr;
    uop->m_target_addr     = trace_uop->m_target;
  }


  // address translation
  if (trace_uop->m_va == 0) {
    uop->m_vaddr = 0;
  } 
  else {
    // since we can have 64-bit address space and each trace has 32-bit address,
    // using extra bits to differentiate address space of each application
    uop->m_vaddr = trace_uop->m_va + m_simBase->m_memory->base_addr(core_id,
        (unsigned long)UINT_MAX * 
        (core->get_trace_info(sim_thread_id)->m_process->m_process_id) * 10ul);
  }


  uop->m_mem_size = trace_uop->m_mem_size;
  if (uop->m_mem_type != NOT_MEM && 
      uop->m_mem_type != MEM_LD_SM && 
      uop->m_mem_type != MEM_ST_SM && 
      uop->m_mem_type != MEM_LD_CM && 
      uop->m_mem_type != MEM_LD_TM) {
    int temp_num_req = (uop->m_mem_size + *m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE - 1) / 
      *m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE;

    ASSERTM(temp_num_req > 0, "size:%d max:%d num:%d type:%d num:%d\n", uop->m_mem_size, 
        (int)*m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE, temp_num_req, uop->m_mem_type, 
        trace_uop->m_info->m_trace_info.m_num_uop);
  }

  uop->m_dir     = trace_uop->m_actual_taken;
  uop->m_pc      = info->m_addr;
  uop->m_core_id = core_id;


  // we found first uop of an instruction, so add instruction count
  if (uop->m_isitBOM) 
    ++thread_trace_info->m_temp_inst_count;

  uop->m_inst_num  = thread_trace_info->m_temp_inst_count;
  uop->m_num_srcs  = trace_uop->m_num_src_regs;
  uop->m_num_dests = trace_uop->m_num_dest_regs;

  ASSERTM(uop->m_num_dests < MAX_DST_NUM, "uop->num_dests=%d MAX_DST_NUM=%d\n", 
      uop->m_num_dests, MAX_DST_NUM);


  // uop number is specific to the core
  uop->m_unique_num = core->inc_and_get_unique_uop_num();

  DEBUG("uop_num:%lld num_srcs:%d  trace_uop->num_src_regs:%d  num_dsts:%d num_seing_uop:%d "
      "pc:0x%s dir:%d \n",
      uop->m_uop_num, uop->m_num_srcs, trace_uop->m_num_src_regs, uop->m_num_dests, 
      thread_trace_info->m_num_sending_uop, hexstr64s(uop->m_pc), uop->m_dir);

  // filling the src_info, dest_info
  if (uop->m_num_srcs < MAX_SRCS) {
    for (int index=0; index < uop->m_num_srcs; ++index) {
      uop->m_src_info[index] = trace_uop->m_srcs[index].m_id;
      //DEBUG("uop_num:%lld src_info[%d]:%d\n", uop->uop_num, index, uop->src_info[index]);
    }
  } 
  else {
    ASSERTM(uop->m_num_srcs < MAX_SRCS, "src_num:%d MAX_SRC:%d", uop->m_num_srcs, MAX_SRCS);
  }



  for (int index = 0; index < uop->m_num_dests; ++index) {
    uop->m_dest_info[index] = trace_uop->m_dests[index].m_id;
    ASSERT(trace_uop->m_dests[index].m_reg < NUM_REG_IDS);
  }

  uop->m_uop_num          = (thread_trace_info->m_temp_uop_count++);
  uop->m_thread_id        = sim_thread_id;
  uop->m_block_id         = ((core)->get_trace_info(sim_thread_id))->m_block_id; 
  uop->m_orig_block_id    = ((core)->get_trace_info(sim_thread_id))->m_orig_block_id;
  uop->m_unique_thread_id = ((core)->get_trace_info(sim_thread_id))->m_unique_thread_id;
  uop->m_orig_thread_id   = ((core)->get_trace_info(sim_thread_id))->m_orig_thread_id;

  
  ///
  /// GPU simulation : handling uncoalesced accesses
  ///
  if ("ptx" == core->get_core_type() && (uop->m_mem_type != NOT_MEM)) {
    int index                = thread_trace_info->m_num_sending_uop - 1;
    bool multiple_mem_access = thread_trace_info->m_trace_uop_array[index]->m_mul_mem_uops;
    bool coalesced;


    if (multiple_mem_access) {
      coalesced = false;
      STAT_EVENT(UNCOAL_INST);
    }
    else {
      coalesced = true;
      STAT_EVENT(COAL_INST);
    }

    // update stats
    switch (uop->m_mem_type) {
      case MEM_ST_SM:
      case MEM_LD_SM:
        if (coalesced) STAT_EVENT(SM_COAL_INST); else STAT_EVENT(SM_UNCOAL_INST);
        break;
      case MEM_LD_CM:
        if (coalesced) STAT_EVENT(CM_COAL_INST); else STAT_EVENT(CM_UNCOAL_INST);
        break;
      case MEM_LD_TM:
        if (coalesced) STAT_EVENT(TM_COAL_INST); else STAT_EVENT(TM_UNCOAL_INST);
        break;
      default:
        if (coalesced) STAT_EVENT(DM_COAL_INST); else STAT_EVENT(DM_UNCOAL_INST);
        break;
    }

    Addr cache_line_addr;
    int cache_line_size;
    switch (uop->m_mem_type) {
      // shared memory
      case MEM_LD_SM:
      case MEM_ST_SM:
        cache_line_addr = core->get_shared_memory()->base_cache_line(uop->m_vaddr);
        cache_line_size = core->get_shared_memory()->cache_line_size();
        break;
      // constant memory
      case MEM_LD_CM:
        cache_line_addr = core->get_const_cache()->base_cache_line(uop->m_vaddr);
        cache_line_size = core->get_const_cache()->cache_line_size();
        break;
      // texture memory
      case MEM_LD_TM:
        cache_line_addr = core->get_texture_cache()->base_cache_line(uop->m_vaddr);
        cache_line_size = core->get_texture_cache()->cache_line_size();
        break;
      // global memory, parameter memory
      default:
        if (*m_simBase->m_knobs->KNOB_BYTE_LEVEL_ACCESS) {
          cache_line_addr = uop->m_vaddr;
          cache_line_size = *m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE;
        }
        else {
          cache_line_addr = m_simBase->m_memory->base_addr(core_id, uop->m_vaddr);
          cache_line_size = m_simBase->m_memory->line_size(core_id);
        }
        break;
    }


    // Even though all accesses are coalesced, based on the maximum transaction size,
    // we may have multiple uops. This only happens in GPU simulation
    int num_mem_req = ((uop->m_vaddr + uop->m_mem_size - cache_line_addr) + 
        (cache_line_size - 1)) / cache_line_size;


    // FIXME
    // multiple transactions due to 1) uncoalesced accesses or 2) too large memory request
    if (multiple_mem_access || num_mem_req > 1) {
      // update stats
      if (coalesced) {
        STAT_EVENT(COAL_INST_MUL_TRANS);

        switch (uop->m_mem_type) {
          case MEM_ST_SM:
          case MEM_LD_SM:
            STAT_EVENT(SM_COAL_INST_MUL_TRANS);
            break;
          case MEM_LD_CM:
            STAT_EVENT(CM_COAL_INST_MUL_TRANS);
            break;
          case MEM_LD_TM:
            STAT_EVENT(TM_COAL_INST_MUL_TRANS);
            break;
          default:
            STAT_EVENT(DM_COAL_INST_MUL_TRANS);
            break;
        }
      }
      else {
        STAT_EVENT(UNCOAL_INST_MUL_TRANS);
      }


      // FIXME
      bool subsequent_mem = false;
      if (multiple_mem_access) {
        if (!thread_trace_info->m_trace_uop_array[index]->m_eom) {
          for (int i = 0; ; ++i) {
            ASSERT((index + 1) < MAX_PUP);
            if (thread_trace_info->m_trace_uop_array[++index]->m_mem_type) {
              subsequent_mem = true;
              break;
            }
            ASSERT((index + 1) < MAX_PUP);
            if (thread_trace_info->m_trace_uop_array[++index]->m_eom) {
              break;
            }
          }
        }
      }

      if (multiple_mem_access && !thread_trace_info->m_trace_ended)
        ASSERT(ungetch_trace(core_id, sim_thread_id, 1));

      uop_c *child_mem_uop = NULL;
      int mem_req_count    = 0;
      uop_c *child_mem_reqs[128];

      bool inst_read         = false;
      bool last_inst         = false;
      int num_inst_to_unread = 0;

      do {
        for (int i = 0; i < num_mem_req; ++i) {
          child_mem_uop = core->get_frontend()->get_uop_pool()->acquire_entry(m_simBase);
          child_mem_uop->allocate();
          ASSERT(child_mem_uop); 

          memcpy(child_mem_uop, uop, sizeof(uop_c));

          child_mem_reqs[mem_req_count++] = child_mem_uop;
          child_mem_uop->m_parent_uop     = uop;

          child_mem_uop->m_vaddr    = cache_line_addr + i * cache_line_size;
          Addr vaddr = uop->m_vaddr + uop->m_mem_size;
          child_mem_uop->m_mem_size = (i != (num_mem_req - 1)) ? 
            cache_line_size : (vaddr - cache_line_addr) - (num_mem_req - 1) * cache_line_size;
          child_mem_uop->m_uop_num    = thread_trace_info->m_temp_uop_count++;
          child_mem_uop->m_unique_num = core->inc_and_get_unique_uop_num();
          child_mem_uop->m_uncoalesced_flag = uop->m_uncoalesced_flag;
        }

        if (!multiple_mem_access || last_inst) {
          if (subsequent_mem) {
            //rewind trace
            ASSERT(ungetch_trace(core_id, sim_thread_id, num_inst_to_unread));
          }
          else {
            if (multiple_mem_access) {
              if (!thread_trace_info->m_trace_ended) {
                read_success = peek_trace(core_id, thread_trace_info->m_prev_trace_info, 
                    sim_thread_id, &inst_read);
                if (read_success) {
                  if (inst_read) {
                    uop->m_npc = thread_trace_info->m_prev_trace_info->m_instruction_addr;
                  }
                  else {
                    thread_trace_info->m_trace_ended = true;
                    DEBUG("trace ended core_id:%d thread_id:%d\n", core_id, sim_thread_id);
                  }
                }
                else {
                  ASSERT(0);
                }
              }
            }
            else {
              uop->m_npc = thread_trace_info->m_prev_trace_info->m_instruction_addr;
            }
          }


          // create children uops for uncoalcesed accesses
          uop->m_child_uops = new uop_c *[mem_req_count];

          uop->m_num_child_uops      = mem_req_count;
          uop->m_num_child_uops_done = 0;
          uop->m_pending_child_uops  = N_BIT_MASK(uop->m_num_child_uops);
          uop->m_vaddr               = 0;
          uop->m_mem_size            = 0;

          for (int ii = 0; ii < mem_req_count; ++ii) {
            uop->m_child_uops[ii] = child_mem_reqs[ii];
            child_mem_reqs[ii]    = NULL;
          }

          break;
        } // end if (!multiple_mem_access || last_inst)


        // we dont want the side-effects of read_trace()
        read_success = peek_trace(core_id, &trace_info, sim_thread_id, &inst_read);
        if (!read_success || (read_success && !inst_read)) {
          ASSERT(0);
        }
        ++num_inst_to_unread;

        // has_immediate indicates whether the trace instruction is part of 
        // an uncoalesced memory access
        last_inst = trace_info.m_has_immediate; 

        //TODO: not using active mask value - here and in other places as well
        if (uop->m_mem_type == MEM_LD || 
            uop->m_mem_type == MEM_LD_LM ||
            uop->m_mem_type == MEM_LD_SM || 
            uop->m_mem_type == MEM_LD_CM ||
            uop->m_mem_type == MEM_LD_TM || 
            uop->m_mem_type == MEM_LD_PM) {
          uop->m_vaddr    = trace_info.m_ld_vaddr1;
          // BYTE
          uop->m_mem_size = trace_info.m_mem_read_size;
          ASSERTM(trace_info.m_mem_read_size > 0, "mem_type:%d\n", uop->m_mem_type);
          
          if (uop->m_mem_type != NOT_MEM && uop->m_mem_type != MEM_LD_SM && 
              uop->m_mem_type != MEM_ST_SM && uop->m_mem_type != MEM_LD_CM && 
              uop->m_mem_type != MEM_LD_TM) {
            int temp_num_req = (uop->m_mem_size + *m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE - 1) / 
              *m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE;
            ASSERTM(temp_num_req > 0, "size:%d max:%d num:%d type:%s\n", 
                uop->m_mem_size, (int)*m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE, temp_num_req, 
                uop_c::g_mem_type_name[uop->m_mem_type]);

          }
        }
        else if (uop->m_mem_type == MEM_ST || 
            uop->m_mem_type == MEM_ST_LM || 
            uop->m_mem_type == MEM_ST_SM) {
          uop->m_vaddr    = trace_info.m_st_vaddr;
          // BYTE
          uop->m_mem_size = trace_info.m_mem_write_size;

          if (uop->m_mem_type != NOT_MEM && uop->m_mem_type != MEM_LD_SM && 
              uop->m_mem_type != MEM_ST_SM && uop->m_mem_type != MEM_LD_CM && 
              uop->m_mem_type != MEM_LD_TM) {
            int temp_num_req = (uop->m_mem_size + *m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE - 1) / 
              *m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE;
            ASSERTM(temp_num_req > 0, "size:%d max:%d num:%d type:%s\n", 
                uop->m_mem_size, (int)*m_simBase->m_knobs->KNOB_MAX_TRANSACTION_SIZE, temp_num_req, 
                uop_c::g_mem_type_name[uop->m_mem_type]);
          }
        }
        else {
          ASSERT(0);
        }

        // address translation
        int process_id = thread_trace_info->m_process->m_process_id;
        unsigned long offset = UINT_MAX * process_id * 10;
        uop->m_vaddr += m_simBase->m_memory->base_addr(core_id, offset); 

        switch (uop->m_mem_type) {
          // shared memory
          case MEM_LD_SM:
          case MEM_ST_SM:
            cache_line_addr = core->get_shared_memory()->base_cache_line(uop->m_vaddr);
            break;
          // constant memory
          case MEM_LD_CM:
            cache_line_addr = core->get_const_cache()->base_cache_line(uop->m_vaddr);
            break;
          // texture cache
          case MEM_LD_TM:
            cache_line_addr = core->get_texture_cache()->base_cache_line(uop->m_vaddr);
            break;
          // global memory, parameter memory
          default:
            if (*m_simBase->m_knobs->KNOB_BYTE_LEVEL_ACCESS) {
              cache_line_addr = uop->m_vaddr;
            }
            else {
              cache_line_addr = m_simBase->m_memory->base_addr(core_id, uop->m_vaddr);
            }
            break;
        }
        Addr vaddr = uop->m_vaddr + uop->m_mem_size;
        num_mem_req = ((vaddr - cache_line_addr) + (cache_line_size - 1)) / cache_line_size;
      } while (1);
    } // MULTIPLE_TRANSACTION
    else {
      ASSERT(coalesced);

      // update stats
      STAT_EVENT(COAL_INST_SINGLE_TRANS);

      switch (uop->m_mem_type) {
        case MEM_LD_SM:
        case MEM_ST_SM:
          STAT_EVENT(SM_COAL_INST_SINGLE_TRANS);
          break;
        case MEM_LD_CM:
          STAT_EVENT(CM_COAL_INST_SINGLE_TRANS);
          break;
        case MEM_LD_TM:
          STAT_EVENT(TM_COAL_INST_SINGLE_TRANS);
          break;
        default:
          STAT_EVENT(DM_COAL_INST_SINGLE_TRANS);
          break;
      }
    }
  }

  DEBUG("new uop: uop_num:%lld inst_num:%lld thread_id:%d unique_num:%lld \n",
      uop->m_uop_num, uop->m_inst_num, uop->m_thread_id, uop->m_unique_num);

  return read_success;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Initialize the mapping between trace opcode and uop type
 */
void trace_read_c::init_pin_convert(void)
{
  m_int_uop_table[XED_CATEGORY_INVALID]    = UOP_INV;
  m_int_uop_table[XED_CATEGORY_3DNOW]      = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_AES]        = UOP_IMUL;
  m_int_uop_table[XED_CATEGORY_BASE]       = UOP_IMUL;
  m_int_uop_table[XED_CATEGORY_BINARY]     = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_BITBYTE]    = UOP_BYTE;
  m_int_uop_table[XED_CATEGORY_CALL]       = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_CMOV]       = UOP_CMOV;
  m_int_uop_table[XED_CATEGORY_COND_BR]    = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_DATAXFER]   = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_DECIMAL]    = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_FCMOV]      = UOP_FCMOV;
  m_int_uop_table[XED_CATEGORY_FLAGOP]     = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_INTERRUPT]  = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_IO]         = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_IOSTRINGOP] = UOP_IMUL;
  m_int_uop_table[XED_CATEGORY_LOGICAL]    = UOP_LOGIC;
  m_int_uop_table[XED_CATEGORY_MISC]       = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_MMX]        = UOP_MM;
  m_int_uop_table[XED_CATEGORY_NOP]        = UOP_NOP;
  m_int_uop_table[XED_CATEGORY_PCLMULQDQ]  = UOP_IMUL;
  m_int_uop_table[XED_CATEGORY_POP]        = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_PREFETCH]   = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_PUSH]       = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_RET]        = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_ROTATE]     = UOP_SHIFT;
  m_int_uop_table[XED_CATEGORY_SEGOP]      = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SEMAPHORE]  = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SHIFT]      = UOP_SHIFT;
  m_int_uop_table[XED_CATEGORY_SSE]        = UOP_FADD;
  m_int_uop_table[XED_CATEGORY_SSE5]       = UOP_FADD;
  m_int_uop_table[XED_CATEGORY_STRINGOP]   = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SYSCALL]    = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SYSRET]     = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SYSTEM]     = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_UNCOND_BR]  = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_VTX]        = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_WIDENOP]    = UOP_NOP;
  m_int_uop_table[XED_CATEGORY_X87_ALU]    = UOP_FADD;
  m_int_uop_table[XED_CATEGORY_XSAVE]      = UOP_IMUL;
  m_int_uop_table[TR_MUL]                  = UOP_IMUL;
  m_int_uop_table[TR_DIV]                  = UOP_IMUL;
  m_int_uop_table[TR_FMUL]                 = UOP_FMUL;
  m_int_uop_table[TR_FDIV]                 = UOP_FDIV;
  m_int_uop_table[TR_NOP]                  = UOP_NOP;
  m_int_uop_table[PREFETCH_NTA]            = UOP_IADD;
  m_int_uop_table[PREFETCH_T0]             = UOP_IADD;
  m_int_uop_table[PREFETCH_T1]             = UOP_IADD;
  m_int_uop_table[PREFETCH_T2]             = UOP_IADD;
  m_int_uop_table[TR_MEM_LD_LM]            = UOP_IADD;
  m_int_uop_table[TR_MEM_LD_SM]            = UOP_IADD;
  m_int_uop_table[TR_MEM_LD_GM]            = UOP_IADD;
  m_int_uop_table[TR_MEM_ST_LM]            = UOP_IADD;
  m_int_uop_table[TR_MEM_ST_SM]            = UOP_IADD;
  m_int_uop_table[TR_MEM_ST_GM]            = UOP_IADD;
  m_int_uop_table[TR_DATA_XFER_LM]         = UOP_IADD;
  m_int_uop_table[TR_DATA_XFER_SM]         = UOP_IADD;
  m_int_uop_table[TR_DATA_XFER_GM]         = UOP_IADD;
  m_int_uop_table[TR_MEM_LD_CM]            = UOP_IADD;
  m_int_uop_table[TR_MEM_LD_TM]            = UOP_IADD;
  m_int_uop_table[TR_MEM_LD_PM]            = UOP_IADD;

  m_fp_uop_table[XED_CATEGORY_INVALID]    = UOP_INV;
  m_fp_uop_table[XED_CATEGORY_3DNOW]      = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_AES]        = UOP_FMUL;
  m_fp_uop_table[XED_CATEGORY_BASE]       = UOP_FMUL;
  m_fp_uop_table[XED_CATEGORY_BINARY]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_BITBYTE]    = UOP_BYTE;
  m_fp_uop_table[XED_CATEGORY_CALL]       = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_CMOV]       = UOP_CMOV;
  m_fp_uop_table[XED_CATEGORY_COND_BR]    = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_DATAXFER]   = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_DECIMAL]    = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_FCMOV]      = UOP_FCMOV;
  m_fp_uop_table[XED_CATEGORY_FLAGOP]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_INTERRUPT]  = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_IO]         = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_IOSTRINGOP] = UOP_FMUL;
  m_fp_uop_table[XED_CATEGORY_LOGICAL]    = UOP_LOGIC;
  m_fp_uop_table[XED_CATEGORY_MISC]       = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_MMX]        = UOP_MM;
  m_fp_uop_table[XED_CATEGORY_NOP]        = UOP_NOP;
  m_fp_uop_table[XED_CATEGORY_PCLMULQDQ]  = UOP_FMUL;
  m_fp_uop_table[XED_CATEGORY_POP]        = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_PREFETCH]   = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_PUSH]       = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_RET]        = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_ROTATE]     = UOP_SHIFT;
  m_fp_uop_table[XED_CATEGORY_SEGOP]      = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SEMAPHORE]  = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SHIFT]      = UOP_SHIFT;
  m_fp_uop_table[XED_CATEGORY_SSE]        = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SSE5]       = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_STRINGOP]   = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SYSCALL]    = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SYSRET]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SYSTEM]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_UNCOND_BR]  = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_VTX]        = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_WIDENOP]    = UOP_NOP;
  m_fp_uop_table[XED_CATEGORY_X87_ALU]    = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_XSAVE]      = UOP_FMUL;
  m_fp_uop_table[TR_MUL]                  = UOP_IMUL;
  m_fp_uop_table[TR_DIV]                  = UOP_IMUL;
  m_fp_uop_table[TR_FMUL]                 = UOP_FMUL;
  m_fp_uop_table[TR_FDIV]                 = UOP_FDIV;
  m_fp_uop_table[TR_NOP]                  = UOP_NOP;
  m_fp_uop_table[PREFETCH_NTA]            = UOP_FADD;
  m_fp_uop_table[PREFETCH_T0]             = UOP_FADD;
  m_fp_uop_table[PREFETCH_T1]             = UOP_FADD;
  m_fp_uop_table[PREFETCH_T2]             = UOP_FADD;
  m_fp_uop_table[TR_MEM_LD_LM]            = UOP_FADD;
  m_fp_uop_table[TR_MEM_LD_SM]            = UOP_FADD;
  m_fp_uop_table[TR_MEM_LD_GM]            = UOP_FADD;
  m_fp_uop_table[TR_MEM_ST_LM]            = UOP_FADD;
  m_fp_uop_table[TR_MEM_ST_SM]            = UOP_FADD;
  m_fp_uop_table[TR_MEM_ST_GM]            = UOP_FADD;
  m_fp_uop_table[TR_DATA_XFER_LM]         = UOP_FADD;
  m_fp_uop_table[TR_DATA_XFER_SM]         = UOP_FADD;
  m_fp_uop_table[TR_DATA_XFER_GM]         = UOP_FADD;
  m_fp_uop_table[TR_MEM_LD_CM]            = UOP_FADD;
  m_fp_uop_table[TR_MEM_LD_TM]            = UOP_FADD;
  m_fp_uop_table[TR_MEM_LD_PM]            = UOP_FADD;
}


const char *trace_read_c::g_tr_reg_names[MAX_TR_REG] = {
  "*invalid*", // 0
  "*none*",
  "*imm8*",
  "*imm*",
  "*imm32*",
  "*mem*",
  "*mem*",
  "*mem*",
  "*off*",
  "*off*",
  "*off*", // 10
  "*modx*",
  "rdi",
  "rsi",
  "rbp",
  "rsp",
  "rbx",
  "rdx",
  "rcx",
  "rax",
  "r8", // 20
  "r9",
  "r10",
  "r11",
  "r12",
  "r13",
  "r14",
  "r15",
  "cs",
  "ss",
  "ds", // 30
  "es",
  "fs",
  "gs",
  "rflags",
  "rip",
  "al",
  "ah",
  "ax",
  "cl",
  "ch", // 40
  "cx",
  "dl",
  "dh",
  "dx",
  "bl",
  "bh",
  "bx",
  "bp",
  "si",
  "di", // 50
  "sp",
  "flags",
  "ip",
  "edi",
  "dil",
  "esi",
  "sil",
  "ebp",
  "bpl",
  "esp", // 60
  "spl",
  "ebx",
  "edx",
  "ecx",
  "eax",
  "eflags",
  "eip",
  "r8b",
  "r8w",
  "r8d", // 70
  "r9b",
  "r9w",
  "r9d",
  "r10b",
  "r10w",
  "r10d",
  "r11b",
  "r11w",
  "r11d",
  "r12b", // 80
  "r12w",
  "r12d",
  "r13b",
  "r13w",
  "r13d",
  "r14b",
  "r14w",
  "r14d",
  "r15b",
  "r15w", // 90
  "r15d",
  "mm0",
  "mm1",
  "mm2",
  "mm3",
  "mm4",
  "mm5",
  "mm6",
  "mm7",
  "emm0", // 100
  "emm1",
  "emm2",
  "emm3",
  "emm4",
  "emm5",
  "emm6",
  "emm7",
  "mxt",
  "xmm0",
  "xmm1", // 110
  "xmm2",
  "xmm3",
  "xmm4",
  "xmm5",
  "xmm6",
  "xmm7",
  "xmm8",
  "xmm9",
  "xmm10",
  "xmm11", // 120
  "xmm12",
  "xmm13",
  "xmm14",
  "xmm15",
  "mxcsr",
  "dr0",
  "dr1",
  "dr2",
  "dr3",
  "dr4", // 130
  "dr5",
  "dr6",
  "dr7",
  "cr0",
  "cr1",
  "cr2",
  "cr3",
  "cr4",
  "tssr",
  "ldtr", // 140
  "tr0",
  "tr1",
  "tr2",
  "tr3",
  "tr4",
  "tr5",
  "fpcw",
  "fpsw",
  "fptag",
  "fpip_off", // 150
  "fpip_sel",
  "fpopcode",
  "fpdp_off",
  "fpdp_sel",
  "st0",
  "st1",
  "st2",
  "st3",
  "st4",
  "st5", // 160
  "st6",
  "st7",
  "r_status_flags",
  "rdf",
  "fpst_all",
  "pin_reg", // 166
  "temp0"
};


const char* trace_read_c::g_tr_opcode_names[MAX_TR_OPCODE_NAME] = {
  "XED_CATEGORY_INVALID", // 0
  "XED_CATEGORY_3DNOW",
  "XED_CATEGORY_AES",
  "XED_CATEGORY_BASE",
  "XED_CATEGORY_BINARY",
  "XED_CATEGORY_BITBYTE",
  "XED_CATEGORY_CALL",
  "XED_CATEGORY_CMOV",
  "XED_CATEGORY_COND_BR",
  "XED_CATEGORY_DATAXFER",
  "XED_CATEGORY_DECIMAL", // 10
  "XED_CATEGORY_FCMOV",
  "XED_CATEGORY_FLAGOP",
  "XED_CATEGORY_INTERRUPT",
  "XED_CATEGORY_IO",
  "XED_CATEGORY_IOSTRINGOP",
  "XED_CATEGORY_LOGICAL",
  "XED_CATEGORY_MISC",
  "XED_CATEGORY_MMX",
  "XED_CATEGORY_NOP",
  "XED_CATEGORY_PCLMULQDQ", // 20
  "XED_CATEGORY_POP",
  "XED_CATEGORY_PREFETCH",
  "XED_CATEGORY_PUSH",
  "XED_CATEGORY_RET",
  "XED_CATEGORY_ROTATE",
  "XED_CATEGORY_SEGOP",
  "XED_CATEGORY_SEMAPHORE",
  "XED_CATEGORY_SHIFT",
  "XED_CATEGORY_SSE",
  "XED_CATEGORY_SSE5", // 30
  "XED_CATEGORY_STRINGOP",
  "XED_CATEGORY_SYSCALL",
  "XED_CATEGORY_SYSRET",
  "XED_CATEGORY_SYSTEM",
  "XED_CATEGORY_UNCOND_BR",
  "XED_CATEGORY_VTX",
  "XED_CATEGORY_WIDENOP",
  "XED_CATEGORY_X87_ALU",
  "XED_CATEGORY_XSAVE",
  "TR_MUL", // 40
  "TR_DIV",
  "TR_FMUL",
  "TR_FDIV",
  "TR_NOP",
  "PREFETCH_NTA",
  "PREFETCH_T0",
  "PREFETCH_T1",
  "PREFETCH_T2",
  "TR_MEM_LD_LM",
  "TR_MEM_LD_SM", // 50
  "TR_MEM_LD_GM",
  "TR_MEM_ST_LM",
  "TR_MEM_ST_SM",
  "TR_MEM_ST_GM",
  "TR_DATA_XFER_LM",
  "TR_DATA_XFER_SM",
  "TR_DATA_XFER_GM",
  "TR_MEM_LD_CM",
  "TR_MEM_LD_TM",
  "TR_MEM_LD_PM" // 60
};


const char* trace_read_c::g_tr_cf_names[10] = {
  "NOT_CF",       // not a control flow instruction
  "CF_BR",       // an unconditional branch
  "CF_CBR",       // a conditional branch
  "CF_CALL",      // a call
  "CF_IBR",       // an indirect branch
  "CF_ICALL",      // an indirect call
  "CF_ICO",       // an indirect jump to co-routine
  "CF_RET",       // a return
  "CF_SYS",
  "CF_ICBR"
};


const char *trace_read_c::g_optype_names[37] = {
  "OP_INV",       // invalid opcode
  "OP_SPEC",      // something weird (rpcc)
  "OP_NOP",       // is a decoded nop
  "OP_CF",       // change of flow
  "OP_CMOV",      // conditional move
  "OP_LDA",         // load address
  "OP_IMEM",      // int memory instruction
  "OP_IADD",      // integer add
  "OP_IMUL",      // integer multiply
  "OP_ICMP",      // integer compare
  "OP_LOGIC",      // logical
  "OP_SHIFT",      // shift
  "OP_BYTE",      // byte manipulation
  "OP_MM",       // multimedia instructions
  "OP_FMEM",      // fp memory instruction
  "OP_FCF",
  "OP_FCVT",      // floating point convert
  "OP_FADD",      // floating point add
  "OP_FMUL",      // floating point multiply
  "OP_FDIV",      // floating point divide
  "OP_FCMP",      // floating point compare
  "OP_FBIT",      // floating point bit
  "OP_FCMOV"        // floating point cond move
};


const char *trace_read_c::g_mem_type_names[20] = {
  "NOT_MEM",     // not a memory instruction
  "MEM_LD",       // a load instruction
  "MEM_ST",       // a store instruction
  "MEM_PF",       // a prefetch
  "MEM_WH",       // a write hint
  "MEM_EVICT",     // a cache block eviction hint
  "MEM_SWPREF_NTA", 
  "MEM_SWPREF_T0",
  "MEM_SWPREF_T1",
  "MEM_SWPREF_T2",
  "MEM_LD_LM",
  "MEM_LD_SM",
  "MEM_LD_GM",
  "MEM_ST_LM",
  "MEM_ST_SM",
  "MEM_ST_GM",
  "NUM_MEM_TYPES"
};


