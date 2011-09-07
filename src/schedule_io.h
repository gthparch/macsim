/**********************************************************************************************
 * File         : schedule_io.cc
 * Author       : Hyesoon Kim
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : inorder scheduler
 *********************************************************************************************/

#ifndef SCHEDULE_IO_H_INCLUDED
#define SCHEDULE_IO_H_INCLUDED


#include "schedule.h"
#include "uop.h"


// FIXME TOCHECK
// what is m_knob_sched_to_width? : never used


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief In-order scheduler class
///////////////////////////////////////////////////////////////////////////////////////////////
class schedule_io_c : public schedule_c 
{
  public:
    /**
     *  \brief Constructor for the Inorder scheluder class.
     *  \param m_core_id - Core identifier number
     *  \param m_alloc_q - Pointer to be updated with the allocation stage queue
     *  \param m_rob - Pointer to the Reorder buffer  
     *  \param m_exec - Pointer to m_execution unit
     *  \param m_unit_type - Parameter used to identify knob width
     *  \param m_frontend - Pointer to front end queue
     *  \return void 
     */
    schedule_io_c(int m_core_id, pqueue_c<int>** m_alloc_q, rob_c* m_rob, exec_c* m_exec, 
        Unit_Type m_unit_type, frontend_c* m_frontend, macsim_c* simBase); 

    /**
     *  \brief Destructor for the Inorder scheduler
     *  \return void 
     */
    ~schedule_io_c();

    /**
     *  \brief Function to perform the activities of a cycle for the scheduler.
     *  \return void 
     */
    void run_a_cycle();

  private:
    int m_next_inorder_to_schedule; /**< index to rob for next uop to schedule */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */


};

#endif // SCHEDULE_ORIG_H_INCLUDED

