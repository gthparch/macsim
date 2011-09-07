/**********************************************************************************************
 * File         : allocate.h
 * Author       : Hyesoon Kim
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : allocate an instruction (uop) into reorder buffer
 *********************************************************************************************/

#ifndef ALLOCATE_H_INCLUDED
#define ALLOCATE_H_INCLUDED 

#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"

#include "allocate_interface.h"


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
    
    macsim_c*       m_simBase; /**< macsim_c base class for simulation globals */

};

#endif // ALLOCATE_H_INCLUDED 

