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
 * File         : thread_schedule_tl_rr.h
 * Author       : Michael Goldstein
 * Date         : 12/15/2020
 * SVN          : $Id: main.cc,v 911 2009-11-20 19:08:10Z kacear $:
 * Description  : two-level round robin thread scheduler inserted into core_c
 *********************************************************************************************/

#ifndef THREAD_SCHEDULE_TLRR_H_INCLUDED
#define THREAD_SCHEDULE_TLRR_H_INCLUDED

#include <list>
#include <iostream>

#include "global_types.h"
#include "global_defs.h"
#include "thread_schedule.h"

class thread_schedule_tlrr_c : public thread_schedule_c{
    public:
        /** struct containing fetch group data
         * @param threads list of threads running in this group
         * @param size number of threads in this group
         * @param stalled true if all threads in this group are stalled
         */
        typedef struct fetch_group_s {
            /** constructor
             * @param gnum group number
             */
            fetch_group_s(int gnum);

            /** insert thread into fetch group
             * @param tid thread id to be inserted
             */
            void insert(int tid);

            std::list<thread_data_s> threads;
            int size;
            int gnum;
            bool stalled;
        } fetch_group_s;

        /** constructor
         * @param simBase pointer to the global macsim data
         * @param core_id int id of core this scheduler is assigned to
         */
        thread_schedule_tlrr_c(macsim_c* simBase, int core_id);

        /** insert thread into scheduler via thread id
         * @param tid integer thread id
         */
        void insert(int tid);

        /** remove thread from scheduler via thread id
         * @param tid integer thread id
         */
        void remove(int tid);

        /** print contents of scheduler
         */
        void print(void);

        /** cycle to the next thread in the scheduler
         */
        void cycle(void);

        /** get thread id of the next thread 
         * @return integer thread id on this core
         */
        int fetch(void);

        /** get thread id of last fetched thread
         * @return integer thread id on this core
         */
        int last_fetch(void);

        /** mark a thread as stalled in the scheduler
         * @param tid thread's id
         */
        void stall(int tid);

        /** mark a thread as unstalled in the scheduler
         * @param tid thread's id
         */
        void unstall(int tid);
    private:
        std::list<thread_data_s> base_group; // list containing thread ids on this core when count < 16
        std::list<fetch_group_s> groups; // list containing fetch groups when count >= 16
        int last;
        int num_threads;
        bool grouped;
};

#endif