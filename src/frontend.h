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
 * File         : frontend.h
 * Author       : HPArch 
 * Date         : 12/18/2007 
 * SVN          : $Id: frontend.h 915 2009-11-20 19:13:07Z kacear $:
 * Description  : frontend 
 *                based on Jared W. Stark's code  + icache_stage.h (scarab)
 *********************************************************************************************/

#ifndef FRONTEND_H_INCLUDED 
#define FRONTEND_H_INCLUDED 


#include <list>

#include "macsim.h"
#include "global_types.h"


// FIXME
// icache tlb


#define FRONTEND_INTERFACE_PARAMS() \
    int m_core_id, \
    frontend_c* m_frontend, \
    cache_c* m_icache, \
    pqueue_c<int*>* m_q_frontend, \
    pool_c<uop_c>* m_uop_pool, \
    exec_c* m_exec, \
    bp_data_c* m_bp_data, \
    map_c* m_map, \
    int& m_running_thread_num, \
    int& m_fetching_thread_num, \
    int& m_unique_scheduled_thread_num, \
    int& m_last_terminated_tid, \
    Unit_Type m_unit_type \
// end macro

#define FRONTEND_INTERFACE_DECL() \
    int m_core_id; /**< core id */ \
    frontend_c* m_frontend; /**< frontend pointer */ \
    cache_c* m_icache; /**< icache pointer */ \
    pqueue_c<int*>* m_q_frontend; /**< frontend queue */ \
    pool_c<uop_c>* m_uop_pool; /**< uop pool */ \
    exec_c* m_exec; /**< pointer to execution stage class */ \
    bp_data_c* m_bp_data; /**< branch prediction data */ \
    map_c* m_map; /**< dependence information */ \
    int& m_running_thread_num; /**< number of running threads */ \
    int& m_fetching_thread_num; /**< number of currently fetching threads */ \
    int& m_unique_scheduled_thread_num; /**< number of unique threads */ \
    int& m_last_terminated_tid; /**< lastly terminated thread id */ \
    Unit_Type m_unit_type; /**< unit type */ \
// end macro

#define FRONTEND_INTERFACE_ARGS() \
    m_core_id, \
    m_frontend, \
    m_icache, \
    m_q_frontend, \
    m_uop_pool, \
    m_exec, \
    m_bp_data, \
    m_map, \
    m_running_thread_num, \
    m_fetching_thread_num, \
    m_unique_scheduled_thread_num, \
    m_last_terminated_tid, \
    m_unit_type \
// end macro

#define FRONTEND_INTERFACE_INIT() \
    m_core_id ( m_core_id ), \
    m_frontend ( m_frontend ), \
    m_icache ( m_icache ), \
    m_q_frontend ( m_q_frontend ), \
    m_uop_pool ( m_uop_pool ), \
    m_exec ( m_exec ), \
    m_bp_data ( m_bp_data ), \
    m_map ( m_map ), \
    m_running_thread_num ( m_running_thread_num ), \
    m_fetching_thread_num ( m_fetching_thread_num ), \
    m_unique_scheduled_thread_num ( m_unique_scheduled_thread_num ), \
    m_last_terminated_tid ( m_last_terminated_tid ), \
    m_unit_type ( m_unit_type ) \
// end macro

#define FRONTEND_INTERFACE_CAST() \
    static_cast<void>(m_core_id); \
    static_cast<void>(m_frontend); \
    static_cast<void>(m_icache); \
    static_cast<void>(m_q_frontend); \
    static_cast<void>(m_uop_pool); \
    static_cast<void>(m_exec); \
    static_cast<void>(m_bp_data); \
    static_cast<void>(m_map); \
    static_cast<void>(m_running_thread_num); \
    static_cast<void>(m_fetching_thread_num); \
    static_cast<void>(m_unique_scheduled_thread_num); \
    static_cast<void>(m_last_terminated_tid); \
    static_cast<void>(m_unit_type); \
// end macro






// fetch allocation wrapper function
frontend_c *fetch_factory(FRONTEND_INTERFACE_PARAMS(), macsim_c* simBase);


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Instruction cache data
///////////////////////////////////////////////////////////////////////////////////////////////
class icache_data_c
{
  Addr m_addr;  /**< cache line address */
}; 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief reason for not being able to fetch
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Break_Reason_enum {
  BREAK_DONT,                   /**< don't break fetch yet */
  BREAK_ISSUE_WIDTH,            /**< break because it's reached maximum issue width */
  BREAK_CF,                     /**< break because it's reached maximum control flows */
  BREAK_BTB_MISS,               /**< break because of a btb miss */
  BREAK_ICACHE_MISS,            /**< break because of icache miss */
  BREAK_LINE_END,               /**< break because the current cache line has ended */
  BREAK_STALL,                  /**< break because the pipeline is stalled */
  BREAK_CALLSYS,                /**< break because of system call */
  BREAK_BARRIER,                /**< break because of a fetch barrier instruction */
  BREAK_OFFPATH,                /**< break because the machine is offpath */
  BREAK_ALIGNMENT,              /**< break because of misaligned fetch (offpath) */
  BREAK_TAKEN,                  /**< break because of nonsequential control flow */
  BREAK_MODEL_BEFORE,           /**< break because of model hook */
  BREAK_MODEL_AFTER,            /**< break because of model hook */
} Break_Reason;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief frontend mode
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
  FRONTEND_MODE_PERFECT,        /**< prefetch icache */
  FRONTEND_MODE_IFETCH,         /**< normal instruction fetch mode */
  FRONTEND_MODE_WAIT_FOR_MISS,  /**< wait for instruction cache serviced */
  FRONTEND_MODE_WAIT_FOR_TIMER, /**< not used */
} FRONTEND_MODE; 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief reconvergence data 
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct reconv_data_s {
  int32_t m_work_counter; /**< work counter */
  int32_t m_reconverge_depth; /**< reconvergence depth */
  int32_t m_top_level_reconv_seen; /**< top level reconvergence seen */
} reconv_data_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread synchronization data structure
///////////////////////////////////////////////////////////////////////////////////////////////
struct sync_thread_s {
  int32_t m_block_id; /**< block id */
  int32_t m_sync_count; /**< synchronization count */
  int32_t m_num_threads_in_block; /**< number of threads in a block */
  sync_thread_s();
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Multi-Threading scheduler data structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct mt_scheduler_s {
  Addr m_fetch_addr; /**< current fetch address */ 
  Addr m_next_fetch_addr; /**< next fetch address */
} mt_scheduler_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief fetch data structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct frontend_s {
  FRONTEND_MODE      m_fe_mode; /**< current frontend mode for thread */ 
  FRONTEND_MODE      m_fe_prev_mode; /**< previous frontend mode for thread */
  bool               m_fetch_blocked; /**< fetch blocked */
  int                m_next_bid; /**< next block id */
  int                m_next_rowid; /**< next dram row id */
  uint64_t           m_sync_wait_start; /**< sync wait start */
  uint64_t           m_sync_wait_count; /**< sync wait count */
  uint32_t           m_sync_count; /**< sync count */
  uint32_t           m_extra_fetch; /**< extra fetch */
  bool               m_first_time; /**< first time to be called */
  int                m_MT_load_waiting; /**< MT waiting for the load */
  bool               m_MT_br_waiting; /**< MT waiting for the branch */
  Addr               m_fetch_ready_addr; /**< fetch address waiting for begin serviced */
  reconv_data_s      m_reconv_data; /**< GPU : reconvergence data */
  mt_scheduler_s     m_MT_scheduler; /**< MT scheduler */ 
  uop_c*             m_prev_uop; /**< previous uop */
  Counter            m_prev_uop_num; /**< previous uop number */
  Counter            m_prev_uop_thread_num; /**< previous thread id of the uop */
  map<Counter, bool> m_load_waiting; /**< uop is waiting a load begin serviced */

  /**
   * Initialize
   */
  void init();
} frontend_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief frontend (fetch and decode) stage
///////////////////////////////////////////////////////////////////////////////////////////////
class frontend_c
{
  public:
    /**
     * \brief Constructor to class frontend_c
     */
    frontend_c(FRONTEND_INTERFACE_PARAMS(), macsim_c* simBase);

    /*! \fn ~frontend_c()
     *  \brief Destructor to class frontend_c
     *  \return void
     */
    virtual ~frontend_c(); 

    /*! \fn void run_a_cycle()
     *  \brief Function to run a cycle of the frontend stage
     *  \return void
     */
    void run_a_cycle();

    /*! \fn void set_core_id(int c_id)
     *  \brief Function to set core id
     *  \param c_id - core id
     *  \return void
     */
    void set_core_id(int c_id) { m_core_id = c_id; }

    /**
     *  \brief Function to provide instruction cache access
     *  \param sim_thread_id - thread id
     *  \param fetch_addr - Addr
     *  \param fetch_data - fetch data for the thread
     *  \return bool - True if successful
     */
    bool access_icache(int sim_thread_id, Addr fetch_addr, frontend_s* fetch_data);

    /*! \fn bool icache_fill_line (mem_req_s *req)
     *  \brief Function to insert in Instruction cache
     *  \param req - Pointer to memory request
     *  \return bool - True if successful
     */
    bool icache_fill_line (mem_req_s *req);

    /*! \fn bool check_fetch_ready (int sim_thread_id)
     *  \brief Function to check if fetch ready
     *  \param sim_thread_id - Thread id
     *  \return bool- True if ready
     */
    bool check_fetch_ready (int sim_thread_id);

    /*! \fn void start()
     *  \brief Function to mark frontend stage as running
     *  \return void
     */
    void start() { m_fe_running = true; }

    /*! \fn void stop()
     *  \brief Function to mark frontend stage as halted
     *  \return void
     */
    void stop() { m_fe_running = false; }

    /*! \fn bool is_running()
     *  \brief Function to check if frontend stage is running
     *  \return bool - True if running
     */
    bool is_running() { return m_fe_running; }

    /*! \fn int predict_bpu(uop_c *uop)
     *  \brief Function to call branch predictor
     *  \param uop - Pointter to an branch uop
     *  \return int - True if predicted correctly
     */
    int predict_bpu(uop_c *uop);

    /*! \fn bool check_load_ready(int fetch_id)
     *  \brief Function to check if load ready
     *  \param fetch_id - Fetch id
     *  \return bool - True if ready
     */
    bool check_load_ready(int fetch_id);

    /*! \fn bool check_br_ready(int fetch_id)
     *  \brief Function to check if branch ready
     *  \param fetch_id - Fetch id
     *  \return bool - True if ready
     */
    bool check_br_ready(int fetch_id);

    /*! \fn void set_load_wait(int fetch_id, Counter uop_num)
     *  \brief Function to set load waiting
     *  \param fetch_id - Fetch id
     *  \param uop_num - uop number
     *  \return void
     */
    void set_load_wait(int fetch_id, Counter uop_num);

    /*! \fn void set_br_wait(int fetch_id)
     *  \brief Function to set branch waiting
     *  \param fetch_id - Fetch id
     *  \return void
     */
    void set_br_wait(int fetch_id);

    /*! \fn void set_load_ready(int fetch_id, Counter uop_num)
     *  \brief Function to set load ready
     *  \param fetch_id - Fetch id
     *  \param uop_num - Uop number
     *  \return void
     */
    void set_load_ready(int fetch_id, Counter uop_num);

    /*! \fn  void set_br_ready(int fetch_id)
     *  \brief Function to set branch ready
     *  \param fetch_id - Fetch id
     *  \return void
     */
    void set_br_ready(int fetch_id);

    /*! \fn void synch_thread(int block_id, int thread_id)
     *  \brief Function to syncronise threads
     *  \param block_id - Block id
     *  \param thread_id - Thread id
     *  \return void
     */
    void synch_thread(int block_id, int thread_id);

    /**
     * Choose a thread id to fetch (SMT, GPU)
     */
    virtual int fetch();
    
    /**
     * Round-robin fetch policy
     */
    int fetch_rr();
    
    
    /**
     * Access BTB (branch target buffer)
     */
    bool btb_access(uop_c *uop); 

    /**
     * Get the uop pool
     */
    pool_c<uop_c>* get_uop_pool() { return m_uop_pool; }

  public:
    hash_c<sync_thread_s>* m_sync_info; /**< synchronization information */

  protected:
    /**
     * Copy constructor
     */
    frontend_c (const frontend_c& rhs);

    /**
     * Overridden operator =
     */
    frontend_c& operator = (const frontend_c& rhs);

    /**
     * Default constructor
     */
    frontend_c();

    /**
     *  \brief Functio to fetch an instruction from a thread
     *  \param sim_thread_id - Thread id
     *  \param fetch_data - fetch information for the thread
     *  \return FRONTEND_MODE - Frontend fetch mode
     *  @see run_a_cycle
     */
    FRONTEND_MODE process_ifetch(unsigned int sim_thread_id, frontend_s* fetch_data);

    /**
     *  \brief Function to enqueue uop to fetch queue
     *  \param uop - Pointer to Uop to be enqueued
     *  \return void
     */
    void send_uop_to_qfe(uop_c *);

  protected:
    FRONTEND_INTERFACE_DECL(); /**< declaration macro */

    bool          m_fe_stall; /**< frontend stalled */
    bool          m_fe_running; /**< enabled frontend */
    Counter       m_cur_core_cycle; /**< current core cycle */
    int           m_fetch_modulo; /**< fetch modulo */
    list<int32_t> m_sync_done; /**< synchronization information */
    int           m_fetch_arbiter; /**< fetch arbiter */
    uns16         m_knob_width; /**< width */
    uns16         m_knob_fetch_width; /**< fetch width */
    uns           m_knob_icache_line_size; /**< icache line size */
    bool          m_knob_ptx_sim; /**< GPU simulation */
    core_c*       m_core; /**< core pointer */
    bool          m_ready_thread_available; /**< ready thread available */
    int           m_mem_access_thread_num; /**< number of threads that access memory */
    int           m_fetch_ratio; /**< how often fetch an instruction (GPU only) */

    bool          m_dcache_bank_busy[129]; /**< dcache bank busy status */

    macsim_c*     m_simBase; /**< macsim base class for simulation globals */
    
    int (frontend_c::*MT_fetch_scheduler)(void); /**< current fetch scheduler */
    
    // FIXME : implement itlb
    // tlb_c            *m_itlb;
};


#endif 
