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
 * File         : allocate.h
 * Author       : HPArch
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : allocate an instruction (uop) into reorder buffer
 *********************************************************************************************/

#ifndef ALLOCATE_H_INCLUDED
#define ALLOCATE_H_INCLUDED 

#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"

#define ALLOCATE_INTERFACE_PARAMS() \
    int core_id, \
    pqueue_c<int*>* q_frontend, \
    pqueue_c<int>** q_iaq, \
    pqueue_c<gpu_allocq_entry_s>** gpu_q_iaq, \
    pool_c<uop_c>* uop_pool, \
    rob_c* rob, \
    smc_rob_c* gpu_rob, \
    memory_c* mem_system, \
    bp_data_c* m_bp_data, \
    Unit_Type unit_type \
// end macro


#define ALLOCATE_INTERFACE_DECL() \
    int core_id; \
    pqueue_c<int*>* q_frontend; \
    pqueue_c<int>** q_iaq; \
    pqueue_c<gpu_allocq_entry_s>** gpu_q_iaq; \
    pool_c<uop_c>* uop_pool; \
    rob_c* rob; \
    smc_rob_c* gpu_rob; \
    memory_c* mem_system; \
    bp_data_c* m_bp_data; \
    Unit_Type unit_type; \
// end macro


#define ALLOCATE_INTERFACE_ARGS() \
    core_id, \
    q_frontend, \
    q_iaq, \
    gpu_q_iaq, \
    uop_pool, \
    rob, \
    gpu_rob, \
    mem_system, \
    m_bp_data, \
    unit_type \
// end macro


#define ALLOCATE_INTERFACE_INIT() \
    core_id ( core_id ), \
    q_frontend ( q_frontend ), \
    q_iaq ( q_iaq ), \
    gpu_q_iaq ( gpu_q_iaq ), \
    uop_pool ( uop_pool ), \
    rob ( rob ), \
    gpu_rob ( gpu_rob ), \
    mem_system ( mem_system ), \
    m_bp_data ( m_bp_data ), \
    unit_type ( unit_type ) \
// end macro


#define ALLOCATE_INTERFACE_CAST() \
    static_cast<void>(core_id); \
    static_cast<void>(q_frontend); \
    static_cast<void>(q_iaq); \
    static_cast<void>(gpu_q_iaq); \
    static_cast<void>(uop_pool); \
    static_cast<void>(rob); \
    static_cast<void>(gpu_rob); \
    static_cast<void>(mem_system); \
    static_cast<void>(m_bp_data); \
    static_cast<void>(unit_type); \
// end macro


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Allocation stage 
///
/// Allocate an instruction (uop) in ROB from frontend queue.
///////////////////////////////////////////////////////////////////////////////////////////////
class allocate_c
{
  public:
    /**
     *  \brief Create a data structures needed for the allocation stage in the pipeline.
     *  \param core_id - Core identifier number
     *  \param q_frontend - Pointer to front end queue
     *  \param alloc_q - Pointer to be updated with the allocation stage queue
     *  \param uop_pool - Pointer to uop pool
     *  \param rob - Pointer to the Reorder buffer
     *  \param unit_type - Parameter used to identify knob width 
     *  \param num_queues - Number of alloc queues
     *  \param simBase - Pointer to base simulation class for perf/stat counters
     *  \return void 
     */
    allocate_c(int core_id, pqueue_c<int*>* q_frontend, pqueue_c<int>** alloc_q, 
        pool_c<uop_c>* uop_pool, rob_c* rob, Unit_Type unit_type, int num_queues,
        macsim_c* simBase);

    /*! \fn ~allocate_c()
     *  \brief Destructor
     *  \return void 
     */
    ~allocate_c()  {}

    /** \fn run_a_cycle()
     *  \brief Function to be called every cycle by the simulator to perform 
     *   the duties of the allocation stage. 
     */
    void run_a_cycle();

    /*! \fn start()
     *  \brief Mark allocation as running
     *  \return void 
     */
    void start() 
    { 
      m_allocate_running = true;  
    }

    /*! \fn stop()
     *  \brief Mark allocation as halted
     *  \return void 
     */
    void stop()  
    { 
      m_allocate_running = false; 
    }

    /*! \fn is_running()
     *  \brief Check if allocation stage is running i.e., not halted
     *  \return void 
     */
    bool is_running() 
    { 
      return m_allocate_running; 
    }

  private:
    int             m_core_id; /**< core id */
    pqueue_c<int*>* m_frontend_q; /**< frontend queue */
    pqueue_c<int>** m_alloc_q;  /**< allocation queue */
    pool_c<uop_c>*  m_uop_pool; /**< uop pool */
    rob_c*          m_rob; /**< reorder buffer */
    Unit_Type       m_unit_type; /**< core type */
    uns16           m_knob_width; /**< width */
    bool            m_allocate_running; /**< Enable allocation stage */
    Counter         m_cur_core_cycle; /**< current core cycle */
    int             m_num_queues; /**< number of allocation queue types */
    bp_data_c*      m_bp_data;  /**< branch predictor data structure */ 
    macsim_c*       m_simBase; /**< macsim_c base class for simulation globals */

};

#endif // ALLOCATE_H_INCLUDED 

