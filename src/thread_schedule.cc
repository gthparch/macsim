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
 * File         : thread_schedule.cc
 * Author       : Michael Goldstein
 * Date         : 12/15/2020
 * SVN          : $Id: main.cc,v 911 2009-11-20 19:08:10Z kacear $:
 * Description  : abstract thread scheduler interface inserted into core_c
 *********************************************************************************************/

#include "thread_schedule.h"

using namespace std;

/** abstract constructor for data present in all thread schedulers
 * @param simBase pointer to the global macsim data
 * @param core_id int id of core this scheduler is assigned to
 */
thread_schedule_c::thread_schedule_c(macsim_c *simBase, int core_id) : m_simBase(simBase), m_core_id(core_id) {}

/** constructor
 * @param tid thread id 
 */
thread_schedule_c::thread_data_s::scheduled_thread_data_s(int tid = -1) : tid(tid), stalled(false) {}

/** overload insertion stream operator to print out tid and stall state in the following format:
 *  [tid,stalled]
 * @param os ostream reference
 * @param RHS thread_data_s reference containing tid
 * @return os reference
 */
ostream& operator<<(ostream& os, const thread_schedule_c::scheduled_thread_data_s& RHS){
    os << '[' << RHS.tid << ',' << RHS.stalled << ']';
    return os;
}

/** overload comparison operator
 * @param RHS thread_data_s being compared to this one
 * @return true if thread ids match
 */
bool thread_schedule_c::thread_data_s::operator==(const thread_schedule_c::scheduled_thread_data_s& RHS){
    return this->tid == RHS.tid;
}