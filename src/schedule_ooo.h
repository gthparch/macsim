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
     *  \param simBase - simulation base class pointer
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

