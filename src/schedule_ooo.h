/*******************************************************************************
 * File         : schedule_ooo.h
 * Author       : Hyesoon Kim
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : out-of-order scheduler
 ******************************************************************************/

#ifndef SCHEDULE_ORIG_H_INCLUDED
#define SCHEDULE_ORIG_H_INCLUDED


#include "schedule.h"
#include "uop.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Out-of-order (OOO) scheduler
///////////////////////////////////////////////////////////////////////////////////////////////
class schedule_ooo_c : public schedule_c 
{
  public:
    /**
     *  \brief Constructor for the Out of order scheluder class.
     *  \param core_id - Core identifier number
     *  \param q_iaq - Pointer to be updated with the allocation stage queue
     *  \param rob - Pointer to the Reorder buffer  
     *  \param exec - Pointer to m_execution unit
     *  \param unit_type - Parameter used to identify knob width
     *  \param frontend - Pointer to front end queue
     *  \return void 
     */
    schedule_ooo_c(int core_id, pqueue_c<int>** q_iaq, rob_c* rob, exec_c* exec, 
        Unit_Type unit_type, frontend_c* frontend, macsim_c* simBase); 

    /*! \fn void ~schedule_ooo_c()
     *  \brief Destructor for the Out of order scheduler
     *  \return void 
     */
    ~schedule_ooo_c(void);

    /*! \fn void run_a_cycle()
     *  \brief Function to perform the activities of a cycle for the scheduler.
     *  \return void 
     */
    void run_a_cycle();

  private:
     macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
   
};

#endif // SCHEDULE_ORIG_H_INCLUDED

