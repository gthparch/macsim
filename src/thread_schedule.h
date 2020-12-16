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
 * File         : thread_schedule.h
 * Author       : Michael Goldstein
 * Date         : 12/15/2020
 * SVN          : $Id: main.cc,v 911 2009-11-20 19:08:10Z kacear $:
 * Description  : abstract thread scheduler interface inserted into core_c
 *********************************************************************************************/

#ifndef THREAD_SCHEDULE_H_INCLUDED
#define THREAD_SCHEDULE_H_INCLUDED

#include <iostream>

#include "global_types.h"
#include "global_defs.h"

class thread_schedule_c {
    public:
        /** struct to hold the data for a thread that is being scheduled
         * @param tid thread id
         * @param stalled bool indicating whether this thread is stalled
         */
        typedef struct scheduled_thread_data_s {
            /** constructor
             * @param tid thread id 
             */
            scheduled_thread_data_s(int tid);

            /** overload insertion stream operator to print out tid and stall state in the following format:
             *  [tid,stalled]
             * @param os ostream reference
             * @param RHS thread_data_s reference containing tid
             * @return os reference
             */
            friend ostream& operator<<(ostream& os, const scheduled_thread_data_s& RHS);

            /** overload comparison operator
             * @param RHS thread_data_s being compared to this one
             * @return true if thread ids match
             */
            bool operator==(const scheduled_thread_data_s& RHS);

            int tid;
            bool stalled;
        } thread_data_s;

        /** abstract constructor for data present in all thread schedulers
         * @param simBase pointer to the global macsim data
         * @param core_id int id of core this scheduler is assigned to
         */
        thread_schedule_c(macsim_c *simBase, int core_id);

        /** insert thread into scheduler via thread id
         * @param tid integer thread id
         */
        virtual void insert(int tid) = 0;

        /** remove thread from scheduler via thread id
         * @param tid integer thread id
         */
        virtual void remove(int tid) = 0;

        /** print contents of scheduler
         */
        virtual void print(void) = 0;

        /** cycle to the next thread in the scheduler
         */
        virtual void cycle(void) = 0;

        /** get thread id of the next thread 
         * @return integer thread id on this core
         */
        virtual int fetch(void) = 0;

        /** get thread id of last fetched thread
         * @return integer thread id on this core
         */
        virtual int last_fetch(void) = 0;

        /** mark a thread as stalled in the scheduler
         * @param tid thread's id
         */
        virtual void stall(int tid) = 0;

        /** mark a thread as unstalled in the scheduler
         * @param tid thread's id
         */
        virtual void unstall(int tid) = 0;
    protected:
        macsim_c* m_simBase; // pointer to global macsim data
        int m_core_id; // pointer to the core this scheduler is running on
};

#endif