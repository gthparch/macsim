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
 * File         : core.h
 * Author       : HPArch
 * Date         : 12/16/2007
 * SVN          : $Id: core.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : core structure
 *********************************************************************************************/

#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED


#include <string>
#include <unordered_map>

#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief per thread heartbeat data structure
///////////////////////////////////////////////////////////////////////////////////////////////
class heartbeat_s {
  public:
    Counter m_last_cycle_count; /**< last heartbeat cycle */
    Counter m_last_inst_count; /**< last heartbeat instruction count */
    Counter m_printed_inst_count; /**< last printed heartbeat inst. count */
    bool    m_check_done; /**< check heartbeat done */
    time_t  m_last_time; /**< last heartbeat time */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Core (processor) class
///
/// Each core has entire pipelines, but memory.
///////////////////////////////////////////////////////////////////////////////////////////////
class core_c
{
  public:
    /**
     *  \brief Constructor to class core_c 
     *  \param c_id - core id
     *  \param type - Type of unit
     *  \param simBase - Pointer to base simulation class for perf/stat counters
     *  \return void 
     */
    core_c(int c_id, macsim_c* simBase, Unit_Type type = UNIT_SMALL);

    /**
     *  \brief Destructor to class core_c
     *  \return void
     */
    ~core_c(void);

    /**
     *  \brief Function to add application
     *  \param tid - thread id
     *  \param process - Pointer to process to be added
     *  \return void
     */
    void add_application(int tid, process_s* process);

    /*! \fn void delete_application(int tid)
     *  \brief Function to delete application
     *  \param tid - thread id
     *  \return void 
     */
    void delete_application(int tid);

    /*! \fn int get_appl_id(int tid)
     *  \brief Function to get application id
     *  \param tid - thread id
     *  \return int - Application id 
     */
    int get_appl_id(int tid);

    /*! \fn int get_appl_id()
     *  \brief Function to get application id 
     *  \return int - Application id
     */
    int get_appl_id();

    /*! \fn string get_core_type(void)
     *  \brief Function to return core type 
     *  \return string - Core type
     */
    string get_core_type(void) { return m_core_type; }

    /*! \fn void pref_init(void)
     *  \brief Hardware prefetcher initializer
     *  \return void
     */
    void pref_init(void);

    /*! \fn void run_a_cycle()
     *  \brief Function to run a cycle of core_c
     *  \return void
     */
    void run_a_cycle();

    /*! \fn Counter get_cycle_count(void)
     *  \brief Function to return cycle count for the core
     *  \return Counter - Cycle count of the core
     */
    Counter get_cycle_count(void) {return m_core_cycle_count;}

    /*! \fn void inc_core_cycle_count(void)
     *  \brief Function to increment core cycle count
     *  \return void
     */
    void inc_core_cycle_count(void) { m_core_cycle_count++; }

    /*! \fn void advance_queues(void)
     *  \brief Function to advance queues 
     *  \return void 
     */
    void advance_queues(void);

    /*! \fn retire_c* get_retire(void)
     *  \brief Function to get pointer to retire class
     *  \return retire_c* - Pointer to retire class
     */
    retire_c* get_retire(void) { return m_retire; }

    /*! \fn frontend_c* get_frontend(void)
     *  \brief Function to get pointer to frontend class
     *  \return frontend_c* - Pointer to frontend class
     */
    frontend_c* get_frontend(void) { return m_frontend;}

    /*! \fn Counter check_heartbeat(bool final)
     *  \brief Function to check if final heartbeat for cores
     *  \param final - TRUE or FALSE
     *  \return Counter - Count of cores with final heartbeat
     */
    void check_heartbeat(bool final);

    /*! \fn Counter final_heartbeat(int thread_id)
     *  \brief Function to return final heartbeat count for a specified thread
     *  \param thread_id - thread_id
     *  \return Counter - Heart beat count
     *  @see process_manager_c::terminate_thread
     *
     *  deallocate data immediately follows
     */
    void final_heartbeat(int thread_id);

    /*! \fn Counter thread_heartbeat(int tid, bool final)
     *  \brief 
     *  \param tid - thread id
     *  \param final - TRUE or FALSE
     *  \return Counter - Heart beat count
     */
    void thread_heartbeat(int tid, bool final);

    /*! \fn Counter core_heartbeat(bool final)
     *  \brief Function to return the core heartbeat
     *  \param final - TRUE or FALSE
     *  \return Counter - Core heartbeat count
     */
    void core_heartbeat(bool);

    /*! \fn Counter check_forward_progress()
     *  \brief Function to check forward progress 
     */
    void check_forward_progress();

    /*! \fn void start(void)
     *  \brief Functio to start core simulation 
     *  \return void 
     */
    void start(void);

    /*! \fn void start_fe(void)
     *  \brief Functio to start Frontend
     *  \return void 
     */
    void start_fe(void);

    /*! \fn void start_be(void)
     *  \brief Function to start backend
     *  \return void 
     */
    void start_be(void);

    /*! \fn void stop(void)
     *  \brief Function to stop core simulation
     *  \return void 
     */
    void stop(void);

    /*! \fn void stop_fe(void)
     *  \brief Function to stop front-end
     *  \return void 
     */
    void stop_fe(void);

    /*! \fn void stop_be(void)
     *  \brief Function to stop back-end
     *  \return void 
     */
    void stop_be(void);

    /*! \fn void allocate_thread_data(int)
     *  \brief Function to allocate core data
     *  \param tid - thread id
     *  \return void 
     */
    void allocate_thread_data (int tid);

    /*! \fn void deallocate_thread_data(int)
     *  \brief Function to deallocate core data
     *  \param tid - thread id
     *  \return void
     */
    void deallocate_thread_data(int tid);

    /**
     * GPU simulation : Get shared memory
     */
    sw_managed_cache_c* get_shared_memory(void) { return m_shared_memory; }

    /**
     * GPU simulation : Get constant cache
     */
    readonly_cache_c* get_const_cache(void) { return m_const_cache; }

    /**
     * GPU simulation : Get texture cache
     */
    readonly_cache_c* get_texture_cache(void) { return m_texture_cache; }
    
    /**
     * Train hardware prefetchers
     * @param level which cache level
     * @param tid thread id
     * @param addr demand address
     * @param pc pc address
     * @param uop uop that trains hardware prefetcher
     * @param hit cache hit/miss information
     */
    void train_hw_pref(int level, int tid, Addr addr, Addr pc, uop_c* uop, bool hit);

    /**
     * Get dependence map
     */
    map_c* get_map() { return m_map; }

    /**
     * Get thread trace information
     */
    thread_s* get_trace_info(int tid);

    /**
     * Create a new thread trace information
     */
    void create_trace_info(int tid, thread_s* thread);

    /**
     * Increase and return the unique uop number. Each uop will have unique uop number in a core.
     */
    Counter inc_and_get_unique_uop_num() { return ++m_unique_uop_num; }

    /**
     * Get the unique uop number
     */
    Counter get_unique_uop_num() { return m_unique_uop_num; }

    /**
     * Get maximum number of concurrently running threads
     */
    int get_max_threads_per_core() { return m_max_threads_per_core; }

    /**
     * Set maximum number of concurrently running threads
     */
    void set_max_threads_per_core(int thread_num) { m_max_threads_per_core = thread_num; }

    /**
     * Initialize core
     */
    void init(void);

  public:
    // stats to run the simulation (used for the simulation)
    int m_unique_scheduled_thread_num; /**< total number of scheduled threads */
    int m_running_block_num;    /**< number of currently running blocks */
    int m_fetching_thread_num;  /**< number of currently fetching threads */
    int m_num_thread_reach_end; /**< number of total terminated threads */
    int m_fetching_block_id;    /**< currently fetching block id */
    int m_running_thread_num;   /**< number of currently running threads */

    // current core stats per thread
    unordered_map<int, bool> m_fetch_ended; /**< fetch ended */
    unordered_map<int, bool> m_thread_reach_end; /**< thread reaches last instruction */
    unordered_map<int, bool> m_thread_finished; /**< thread finished */
    
    // additional fetch policies
    unordered_map<int, Counter> m_inst_fetched; /**< last fetched cycle for the thread */
    unordered_map<int, Counter> m_ops_to_be_dispatched; /**< number of uops to be scheduled */
    unordered_map<int, Counter> m_last_fetch_cycle; /**< last fetched cycle */
    Counter                     m_max_inst_fetched; /**< maximum inst fetched */

  private:
    int                      m_core_id; /**< core id */
    string                   m_core_type; /**< simulation core type (x86 or ptx) */
    Unit_Type                m_unit_type; /**< core type */
    int                      m_last_terminated_tid; /**< last terminated thread id */
    unordered_map<int, bool> m_terminated_tid; /**< ids of terminated threads */
    Counter                  m_unique_uop_num; /**< unique uop number */
    time_t                   m_sim_start_time; /**< simulation start time */
    Counter                  m_core_cycle_count; /**< current core cycle */
    Counter                  m_inst_count; /**< current instruction count */

    // forward progress check
    Counter m_last_forward_progress; /**< last checked cycle */
    Counter m_last_inst_count;       /**< last checked instruction count */

    // pipeline stages
    frontend_c*                     m_frontend; /**< frontend */
    allocate_c*                     m_allocate; /**< allocation */
    smc_allocate_c*                 m_gpu_allocate; /**< GPU allocation */
    schedule_c*                     m_schedule; /**< scheduler */
    exec_c*                         m_exec; /**< execution */
    retire_c*                       m_retire; /**< retire */
    rob_c*                          m_rob; /**< reorder buffer */
    smc_rob_c*                      m_gpu_rob; /**< GPU rob */
    cache_c*                        m_icache; /**< instruction cache */
    pqueue_c<int*>*                 m_q_frontend; /**< frontend queue */
    pqueue_c<int>**                 m_q_iaq; /**< allocation queue */
    pqueue_c<gpu_allocq_entry_s>**  m_gpu_q_iaq; /**< GPU allocation queue */
    readonly_cache_c               *m_const_cache; /**< GPU : constant cache */
    readonly_cache_c               *m_texture_cache; /**< GPU : texture cache */
    sw_managed_cache_c             *m_shared_memory; /**< GPU : scatchpad memory */
    hwp_common_c                   *m_hw_pref; /**< hardware prefetcher */
    pool_c<uop_c>                  *m_uop_pool; /**< uop pool */
    map_c*                          m_map; /**< dependence information */
    bp_data_c*                      m_bp_data; /**< branch predictor */
    
    // heartbeat 
    unordered_map<int, heartbeat_s*> m_heartbeat; /**< heartbeat per thread*/
    time_t  m_heartbeat_last_time_core; /**< last heartbeat time */
    Counter m_heartbeat_last_cycle_count_core; /**< last heartbeat cycle */
    Counter m_heartbeat_last_inst_count_core; /**< last heartbeat inst. count */
    Counter m_heartbeat_printed_inst_count_core; /**< last printed heartbeat inst. count */
    bool    m_heartbeat_check_done_core; /**< check heartbeat done */

    // configuration
    bool m_knob_enable_pref;     /**< enable hardware prefetcher */
    int  m_max_threads_per_core; /**< max num (concurrently running) threads  */
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
    
    // application id mapping
    unordered_map<int, process_s *> m_tid_to_appl_map; /**< get application id with tid */
    int m_appl_id; /**< id of currently running application */
    
    unordered_map<int, thread_s*> m_thread_trace_info; /**< thread trace information */
    unordered_map<int, bp_recovery_info_c*>  m_bp_recovery_info; /**< thread bp recovery info */

    // clock cycle
    Counter m_cycle; /**< clock cycle */
};
#endif   // CORE_H_INCLUDED
