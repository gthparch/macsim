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
 * File         : retire.h
 * Author       : Hyesoon Kim 
 * Date         : 3/17/2008
 * CVS          : $Id: retire.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : retirement logic 
                  origial author: Jared W.Stark  imported from 
 *********************************************************************************************/

#ifndef RETIRE_H_INCLUDED
#define RETIRE_H_INCLUDED


#include <inttypes.h>
#include <queue>
#include <unordered_map>

#include "global_types.h"
#include "debug_macros.h"


#define RETIRE_INTERFACE_PARAMS() \
    int m_core_id, \
    pqueue_c<int*>* m_q_frontend, \
    pqueue_c<int>** m_q_iaq, \
    pqueue_c<gpu_allocq_entry_s>** m_gpu_q_iaq, \
    pool_c<uop_c>* m_uop_pool, \
    rob_c* m_rob, \
    smc_rob_c* m_gpu_rob, \
    Unit_Type m_unit_type \
// end macro


#define RETIRE_INTERFACE_DECL() \
    int m_core_id; /**< core id */ \
    pqueue_c<int*>* m_q_frontend; /**< frontend queue */ \
    pqueue_c<int>** m_q_iaq; /**< ia queue */ \
    pqueue_c<gpu_allocq_entry_s>** m_gpu_q_iaq; /**< gpu ia queue */ \
    pool_c<uop_c>* m_uop_pool; /**< uop pool */ \
    rob_c* m_rob; /**< reorder buffer */ \
    smc_rob_c* m_gpu_rob; /**< gpu reorder buffer */ \
    Unit_Type m_unit_type; /**< unit type */ \
// end macro


#define RETIRE_INTERFACE_ARGS() \
    m_core_id, \
    m_q_frontend, \
    m_q_iaq, \
    m_gpu_q_iaq, \
    m_uop_pool, \
    m_rob, \
    m_gpu_rob, \
    m_unit_type \
// end macro


#define RETIRE_INTERFACE_INIT() \
    m_core_id ( m_core_id ), \
    m_q_frontend ( m_q_frontend ), \
    m_q_iaq ( m_q_iaq ), \
    m_gpu_q_iaq ( m_gpu_q_iaq ), \
    m_uop_pool ( m_uop_pool ), \
    m_rob ( m_rob ), \
    m_gpu_rob ( m_gpu_rob ), \
    m_unit_type ( m_unit_type ) \
// end macro


#define RETIRE_INTERFACE_CAST() \
    static_cast<void>(m_core_id); \
    static_cast<void>(m_q_frontend); \
    static_cast<void>(m_q_iaq); \
    static_cast<void>(m_gpu_q_iaq); \
    static_cast<void>(m_uop_pool); \
    static_cast<void>(m_rob); \
    static_cast<void>(m_gpu_rob); \
    static_cast<void>(m_unit_type); \
// end macro


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief retirement stage
///
/// Retire (Commit) stage in the pipeline
///////////////////////////////////////////////////////////////////////////////////////////////
class retire_c
{
  public:
    /**
     * Constructor
     */
    retire_c(RETIRE_INTERFACE_PARAMS(), macsim_c* simBase);

    /**
     * Destructor
     */
    ~retire_c();

    /**
     * Tick a cycle
     */
    void run_a_cycle();

    /**
     * Enable retirement stage
     */
    void start();

    /**
     * Disable retirement stage
     */
    void stop();

    /**
     * Check retirement stage is running
     */
    bool is_running();

    /**
     * Get number of retired instruction with thread_id
     */
    inline Counter get_instrs_retired(int thread_id) {
      return m_insts_retired[thread_id]; 
    }

    /**
     * Get number of retired uops with thread_id
     */
    Counter get_uops_retired(int thread_id);

    /**
     * Get number of total retired instructions
     */
    Counter get_total_insts_retired(); 

    /**
     * When a new thread has been scheduled, reset data
     */
    void allocate_retire_data(int);

    /**
     * Deallocate per thread data 
     */
    void deallocate_retire_data(int);

    /**
     * Get the number of retired instruction periodically
     */
    int get_periodic_inst_count(void)
    {
      int result = m_period_inst_count;
      m_period_inst_count = 0;

      return result;
    }

  private:
    /**
     * Private constructor. Do not implement
     */
    retire_c (const retire_c& rhs);

    /**
     * Overridden operator =
     */
    retire_c& operator=(const retire_c& rhs);

    /**
     * Update stats when an application is done
     */
    void update_stats(process_s*);

    /**
     * Repeat (terminated) application
     */
    void repeat_traces(process_s*);

  private:
    RETIRE_INTERFACE_DECL(); /**< declaration macro */

    bool                        m_retire_running; /**< retire enabled */
    Counter                     m_total_insts_retired; /**< total retired instructions */ 
    Counter                     m_cur_core_cycle; /**< current core cycle */ 
    uns16                       m_knob_width; /**< pipeline width */
    bool                        m_knob_ptx_sim; /**< gpu simulation */
    unordered_map<int, Counter> m_insts_retired; /**< number of retired inst. per thread */
    unordered_map<int, Counter> m_uops_retired; /**< number of retired uop per thread */

    int m_period_inst_count; /**< counter for periodic logging number of retired inst. */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};

#endif // RETIRE_H_INCLUDED
