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
 * File         : thread_schedule_rr.cc
 * Author       : Michael Goldstein
 * Date         : 12/15/2020
 * SVN          : $Id: main.cc,v 911 2009-11-20 19:08:10Z kacear $:
 * Description  : round robin thread scheduler inserted into core_c
 *********************************************************************************************/

#include "thread_schedule_rr.h"

using namespace std;

/** constructor
 * @param simBase pointer to the global macsim data
 * @param core_id int id of core this scheduler is assigned to
 */
thread_schedule_rr_c::thread_schedule_rr_c(macsim_c* simBase, int core_id) : thread_schedule_c(simBase, core_id), last(-1) {}

/** insert thread into scheduler via thread id
 * @param tid integer thread id
 */
void thread_schedule_rr_c::insert(int tid){
    this->threads.push_back(tid);
}

/** remove thread from scheduler via thread id
 * @param tid integer thread id
 */
void thread_schedule_rr_c::remove(int tid){
    this->threads.remove(tid);
}

/** print contents of scheduler
 */
void thread_schedule_rr_c::print(void){
    cout << "Round robin scheduler queue on core " << this->m_core_id << ':' << endl;
    for(auto th: this->threads){
        cout << th << ' ';
    }
    cout << endl;
}

/** cycle to the next thread in the scheduler
 */
void thread_schedule_rr_c::cycle(void){
    thread_data_s first = this->threads.front();
    this->threads.pop_front();
    this->threads.push_back(first);
}

/** get thread id of the next thread 
 * @return integer thread id on this core
 */
int thread_schedule_rr_c::fetch(void){
    this->last = this->threads.front().tid;
    return this->last;
}

/** get thread id of last fetched thread
 * @return integer thread id on this core
 */
int thread_schedule_rr_c::last_fetch(void){
    return this->last;
}