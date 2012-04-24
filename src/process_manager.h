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
 * File         : process_manager.h
 * Author       : Jaekyu Lee
 * Date         : 1-6-2011
 * SVN          : $Id: pref_stride.h,v 1.1 2008/07/30 14:18:16 kacear Exp $:
 * Description  : process manager (creation, termination, scheduling, ...)
 *                combine sim_process and sim_thread_schedule
 *********************************************************************************************/


#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H


#include <fstream>
#include <inttypes.h>
#include <list>
#include <zlib.h>
#include <unordered_map>

#include "global_defs.h"
#include "global_types.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Section type
///
/// Types can be Computation, memory, or barrier.
///////////////////////////////////////////////////////////////////////////////////////////////
enum Section_Type_enum
{
  SECTION_INVALID = 0,
  SECTION_COMP,                 /**< computation */
  SECTION_MEM,                  /**< memory */
  SECTION_BAR                   /**< barrier */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread trace information node
///
/// This structure corresponds to a thread. This structure is valid only in thread scheduler.
/// Once a thread is scheduled, structure will be freed.
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct thread_trace_info_node_s {
  thread_s*  m_trace_info_ptr;  /**< trace information pointer */
  process_s* m_process;         /**< pointer to the process */
  int        m_tid;             /**< thread id */
  bool       m_main;            /**< main thread */
  bool       m_ptx;             /**< GPU simulation */
  int        m_block_id;        /**< block id */
} thread_trace_info_node_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread block schedule information
///
/// Only valid for GPU simulation. Bookkeeping structure for a thread block
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct block_schedule_info_s {
  /**
   * Constructor
   */
  block_schedule_info_s();

  /**
   * Destructor
   */
  ~block_schedule_info_s();

  bool    m_start_to_fetch;     /**< start fetching */
  int     m_dispatched_core_id; /**< core id in which this block is launched */
  bool    m_retired;            /**< retired */
  int     m_dispatched_thread_num; /**< number of dispatched threads */
  int     m_retired_thread_num; /**< number of retired threads */
  int     m_total_thread_num;   /**< number of total threads */
  int     m_dispatch_done;      /**< dispatch done */
  bool    m_trace_exist;        /**< trace exist */
  Counter m_sched_cycle;        /**< scheduled cycle */
  Counter m_retire_cycle;       /**< retired cycle */
} block_schedule_info_s; 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Section information
/// 
/// A section consists of few instruction. There are several types
/// @see Section_Type_enum
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct section_info_s {
  Section_Type_enum m_section_type; /**< section type */
  int32_t           m_section_length; /**< section length in instruction counts */
} section_info_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread start information
///
/// structure which tells when each thread is ready to be started.
/// this is how the simulator works - during trace generation, 
/// for each thread, the instruction count of the main (parent) 
/// thread when the thread was created is written to a file. 
/// during simulation, a thread is ready to be started only when 
/// the main thread has executed the number of instructions 
/// specified for that thread in the file generated earlier.
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct thread_start_info_s {   
  uint32_t m_thread_id; /**< thread id */
  uint64_t m_inst_count; /**< this stores inst. count of the main thread when the thread 
                           is to be started */
} thread_start_info_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread statistics class
///
/// structure that stores the sched cycle (cycle in which the thread was assigned to a core) 
/// and end cycle
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct thread_stat_s
{
  /**
   * Constructor
   */
  thread_stat_s();

 int32 m_thread_id;             /**< thread id */
 int32 m_unique_thread_id;      /**< unique thread id */
 int32 m_block_id;              /**< block id */
 uns64 m_thread_sched_cycle;    /**< thread scheduled cycle */
 uns64 m_thread_fetch_cycle;    /**< thread fetched cycle */
 uns64 m_thread_end_cycle;      /**< thread end cycle */
} thread_stat_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread data structure
///
/// Mapping between the trace of physical thread and simulation thread. For X86 simulation,
/// a simulation thread is mapped to a physical thread. However, for GPU simulation, a thread
/// is mapped to a SIMD thread group (warp).
///
/// task struct like structure maintained for each thread
/// contains pointer to trace file for the thread + state 
/// maintained for the thread
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct thread_s {
  /**
   * Constructor
   */
  thread_s(macsim_c* simBase);

  /**
   * Destructor
   */
  ~thread_s();

  int                  m_unique_thread_id; /**< unique thread id */
  int                  m_thread_id; /**< current thread id */
  int                  m_orig_block_id; /**< original block id from a trace*/
  int                  m_orig_thread_id; /**< adjusted block id */
  int                  m_block_id; /**< block id */
  gzFile               m_trace_file; /**< gzip trace file */
  bool                 m_file_opened; /**< trace file opened? */
  bool                 m_main_thread; /**< main thread (usually thread id 0) */
  uint64_t             m_inst_count; /**< total instruction counts */
  uint64_t             m_uop_count;  /**< total uop counts */
  bool                 m_trace_ended; /**< trace ended */
  process_s           *m_process; /**< point to the application belongs to */
  bool                 m_ptx; /**< GPU thread */
  char*                m_buffer; /**< trace buffer */
  int                  m_buffer_index; /**< current trace buffer index */
  int                  m_buffer_index_max; /**< maximum buffer index */
  bool                 m_buffer_exhausted; /**< read all traces from the buffer */
  frontend_s*          m_fetch_data; /**< frontend fetch data */ 
  trace_info_s*        m_prev_trace_info; /**< prev instruction trace info */
  trace_info_s*        m_next_trace_info; /**< next instruction trace info */
  bool                 m_thread_init; /**< thread initialized */
  uint64_t             m_temp_inst_count; /**< temp instruction count */
  uint64_t             m_temp_uop_count; /**< temp uop count */
  int                  m_num_sending_uop; /**< number of sending uops */
  bool                 m_bom; /**< beginning of instruction */
  bool                 m_eom; /**< end of instruction */
  trace_uop_s*         m_trace_uop_array[MAX_PUP]; /**< trace uop array */
  trace_uop_s*         m_next_trace_uop_array[MAX_PUP]; /**< next trace uop array */

  macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

  // synchronization information
  list<section_info_s*> m_sections; /**< code sections */
  list<section_info_s*> m_bar_sections; /**< barrier sections */
  list<section_info_s*> m_mem_sections; /**< memory sections */
  list<section_info_s*> m_mem_bar_sections; /**< memory barrier sections */
  list<section_info_s*> m_mem_for_bar_sections; /**< memory for barrier sections */
} thread_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process data structure
///
/// structure holding information about an application being simulated
/// Mapping between the trace of one application and simulation process 
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct process_s {
  /**
   * Constructor
   */
  process_s();

  /**
   * Destructor
   */
  ~process_s();

  unsigned int         m_process_id; /**< current process id */
  int                  m_orig_pid; /**< original process id - in case of repetition */
  int                  m_max_block; /**< max blocks per core for the application */
  thread_start_info_s *m_thread_start_info; /**< thread start information */
  thread_s**           m_thread_trace_info; /**< thread trace information */
  unsigned int         m_no_of_threads; /**< number of total threads */
  unsigned int         m_no_of_threads_created; /**< number of threads created */
  unsigned int         m_no_of_threads_terminated; /**< number of terminated threads */
  map<int, bool>       m_core_list; /**< list of cores that this process is executed */
  queue<int>          *m_core_pool; /**< core pool pointer */
  bool                 m_ptx; /**< GPU application */
  int                  m_repeat; /**< application has been re-executed */
  vector<string>       m_applications; /**< list of sub-applications */
  vector<int>          m_kernel_block_start_count; /**< block id start count for sub-appl. */
  string               m_current_file_name_base; /**< current sub-appl.'s filename base */
  string               m_kernel_config_name; /**< kernel config file name */
  unsigned int         m_current_vector_index; /**< current index to the sub-application */
  map<int, bool>       m_block_list; /**< list of block currently running */
  uns64                m_inst_count_tot; /**< total instruction counts */
  int                  m_block_count; /**< total block counts */
} process_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Process and Thread manager class
///
/// Schedule processes and threads for the simulation.
///////////////////////////////////////////////////////////////////////////////////////////////
class process_manager_c
{
  public:
    /**
     * Constructor
     */
    process_manager_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~process_manager_c();

    /**
     * Create a new process.
     * @param appl application path
     */
    int create_process(string appl);
    
    /**
     * Create a new process.
     * @param appl application path
     * @param repeat whether this process has been re-executed. When we run multiple programs
              workloads, we may need to repeat terminated applications.
     * @param pid original process id
     */
    int create_process(string appl, int repeat, int pid);
    
    /**
     * Terminate a process
     */
    bool terminate_process(process_s *process);

    /**
     * Setup a process (data structure, initialization ..)
     */
    void setup_process(process_s* process);

    /**
     * Create a thread node. In scheduler, a node will be inserted or deleted. 
     * When a node is scheduled, all memory allocation and other initializations 
     * will be performed.
     * @param process process
     * @param tid thread id
     * @param main When set, indicate master thread (usually thread 0)
     */
    void create_thread_node(process_s *process, int tid, bool main);

    /**
     * Terminate a thread
     * @param core_id - core id
     * @param trace_info - thread trace information
     * @param thread_id - thread id
     * @param block_id - block id
     */
    int terminate_thread(int core_id, thread_s *trace_info, int thread_id, int block_id);

    /**
     * Schedule a new thread
     */
    void sim_thread_schedule(void);

  private:
    /**
     * GPU simulation : schedule a thread from a block
     * @param core_id - core to schedule a new thread
     */
    int sim_schedule_thread_block(int core_id);
    
    /**
     * Insert a new thread to the scheduler
     * @param incoming - a new thread
     */
    void insert_thread(thread_trace_info_node_s *incoming);
    
    /**
     * Insert a new thread to the block scheduler
     * @param incoming - a new thread
     */
    void insert_block(thread_trace_info_node_s *incoming);
    
    /**
     * Fetch a new thread which is in the front of scheduling queue
     */
    thread_trace_info_node_s *fetch_thread();
    
    /**
     * Fetch a new thread from the block which is in the front of block scheduling queue
     */
    thread_trace_info_node_s *fetch_block(int block_id);
    
    /**
     * Create a new thread. A thread has been created when create_thread_node has been called.
     * However, when a thread is actually scheduled, memory allocation and initialization
     * will be performed.
     * @param process - process
     * @param tid - thread id
     * @param main - indicate main thread (usually thread id 0)
     */
    thread_s* create_thread(process_s *process, int tid, bool main);

  private:
    list<thread_trace_info_node_s *> *m_thread_queue; /**< thread queue */
    unordered_map<int, list<thread_trace_info_node_s *> *> *m_block_queue; /**< block queue */
    pool_c<hash_c<inst_info_s> >* m_inst_hash_pool; /**< instruction hash pool */

    unordered_map<int, Counter> m_appl_cyccount_info; /**< per application cycle count info */
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
};


#endif
