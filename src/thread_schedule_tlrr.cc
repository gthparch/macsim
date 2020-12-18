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
 * File         : thread_schedule_tl_rr.cc
 * Author       : Michael Goldstein
 * Date         : 12/15/2020
 * SVN          : $Id: main.cc,v 911 2009-11-20 19:08:10Z kacear $:
 * Description  : two-level round robin thread scheduler inserted into core_c
 *********************************************************************************************/

#include "thread_schedule_tlrr.h"

using namespace std;

/** fetch group constructor
 * @param gnum group number
 */
thread_schedule_tlrr_c::fetch_group_s::fetch_group_s(int gnum = 0) : gnum(gnum), stalled(false), size(0) {}

/** insert thread into fetch group
 * @param tid thread id to be inserted
 */
void thread_schedule_tlrr_c::fetch_group_s::insert(int tid){
    this->threads.push_back(tid);
    this->size = this->threads.size();
}

/** operator==
 * @param RHS fetch_group_s on the right side of the operator
 * @return true of group number is equal
 */
bool thread_schedule_tlrr_c::fetch_group_s::operator==(const fetch_group_s& RHS){
    return this->gnum == RHS.gnum;
}

/** class constructor
 * @param simBase pointer to the global macsim data
 * @param core_id int id of core this scheduler is assigned to
 */
thread_schedule_tlrr_c::thread_schedule_tlrr_c(macsim_c* simBase, int core_id) : thread_schedule_c(simBase, core_id), last(-1), num_threads(0), grouped(false), group_fetches(0) {}

/** insert thread into scheduler via thread id
 * @param tid integer thread id
 */
void thread_schedule_tlrr_c::insert(int tid){
    ++this->num_threads;
    // check if there are enough to add to the list, form a fetch group, or add to group list
    if(num_threads < 16){
        // add to list of all threads
        this->base_group.push_back(tid);
    } else if(num_threads == 16) {
        // create group list and add new thread to group 1
        fetch_group_s group0(0), group1(1);
        // divide threads into two fetch groups based on tid
        int th_num = 0;
        for(auto& th : this->base_group){
            if((th_num++) < 8){
                // insert into group 0
                group0.insert(th.tid);
            } else {
                // insert into group 1
                group1.insert(th.tid);
            }
        }
        // insert new tid at the end of group 1
        group1.insert(tid);
        // insert groups into group list
        this->groups.push_back(group0);
        this->groups.push_back(group1);
        // mark grouped
        this->grouped = true;
        // clear base group
        this->base_group.clear();
    } else {
        // add new thread onto smallest group or create new group if all are size 8
        int smallest = -1, cmp_size = 8;
        // find smallest group
        for(auto& fg : this->groups){
            if(fg.size < cmp_size){
                smallest = fg.gnum;
                cmp_size = fg.size;
            }
        }
        // if all are size 8, create a new group, otherwise add onto smallest
        if(smallest == -1){
            // create new group
            fetch_group_s group_n(this->groups.size());
            group_n.insert(tid);
            this->groups.push_back(group_n);
        } else {
            // insert onto smallest group
            for(auto& fg : groups){
                if(fg.gnum == smallest){
                    fg.insert(tid);
                }
            }
        }
    }
}

/** remove thread from scheduler via thread id
 * @param tid integer thread id
 */
void thread_schedule_tlrr_c::remove(int tid){
    --this->num_threads;
    // determine which container to look in
    if(this->grouped){
        pair<bool,int> empty_group(false,-1);
        // look in groups
        for(auto& fg : this->groups){
            fg.threads.remove(tid);
            // mark empty group for deletion
            if((empty_group.first = fg.threads.size() == 0)){
                empty_group.second = fg.gnum;
            }
        }
        // delete empty group
        if(empty_group.first){
            this->groups.remove(empty_group.second);
        }
        // convert groups back to single list if necessary
        if(num_threads < 16){
            for(auto& fg : this->groups){
                for(auto& th : fg.threads){
                    this->base_group.push_back(th);
                }
            }
            // clear groups
            this->grouped = false;
            this->groups.clear();
        }
    } else {
        // look in base_group
        this->base_group.remove(tid);
    }
}

/** print contents of scheduler
 */
void thread_schedule_tlrr_c::print(void){
    cout << "Two-level round robin scheduler on core " << this->m_core_id << ':' << endl;
    // determine which container to look in
    if(this->grouped){
        // look in groups
        for(auto& fg : this->groups){
            cout << "Group " << fg.gnum << ": " << "[ ";
            for(auto& th : fg.threads){
                cout << th << ' ';
            }
            cout << ']' << endl;
        }
    } else {
        // look in base_group
        if(this->base_group.size() == 0){
            cout << "Scheduler is empty." << endl;
        } else {
            cout << "Base group: ";
            for(auto& th : this->base_group){
                cout << th << ' ';
            }
            cout << endl;
        }
    }
}

/** cycle to the next thread in the scheduler
 */
void thread_schedule_tlrr_c::cycle(void){
    // determine which container to look in
    if(this->grouped){
        // look in groups
        fetch_group_s first_group = this->groups.front();
        if(first_group.stalled){
            this->groups.pop_front();
            this->groups.push_back(first_group);
        } else {
            thread_data_s first_thread = first_group.threads.front();
            first_group.threads.pop_front();
            first_group.threads.push_back(first_thread);
        }
    } else {
        // look in base_group
        thread_data_s first_thread = this->base_group.front();
        this->base_group.pop_front();
        this->base_group.push_back(first_thread);
    }
}

/** cycle to the next fetch group in the scheduler
 */
inline void thread_schedule_tlrr_c::cycle_internal(void){
    // look in groups
    fetch_group_s first_group = this->groups.front();
    this->groups.pop_front();
    this->groups.push_back(first_group);
}

/** get thread id of the next thread 
 * @return integer thread id on this core
 */
int thread_schedule_tlrr_c::fetch(void){
    // determine which container to look in
    if(this->grouped){
        // look in groups
        this->last = this->groups.front().threads.front().tid;
        // increment group fetches and check if it has been long enough without a pause to cycle
        if(++this->group_fetches == 1024){
            this->cycle_internal();
            this->group_fetches = 0;
        }
    } else {
        // look in base_group
        this->last = this->base_group.front().tid;
    }
    return this->last;
}

/** get thread id of last fetched thread
 * @return integer thread id on this core
 */
int thread_schedule_tlrr_c::last_fetch(void){
    return this->last;
}

/** mark a thread as stalled in the scheduler
 * @param tid thread's id
 */
void thread_schedule_tlrr_c::stall(int tid){
    // determine which container to look in
    if(this->grouped){
        // look in groups
        bool found = false;
        for(auto& fg : this->groups){
            for(auto& th : fg.threads){
                if(th.tid == tid){
                    found = true;
                    th.stalled = true;
                    break;
                }
            }
            // check if all are stalled 
            if(found){
                bool all_stalled = true;
                for(auto& th : fg.threads){
                    if(!th.stalled){
                        all_stalled = false;
                        break;
                    }
                }
                if(all_stalled){
                    fg.stalled = true;
                }
                break;
            }
        }
    } else {
        // look in base_group
        for(auto& th : this->base_group){
            if(th.tid == tid){
                th.stalled = true;
                break;
            }
        }
    }
}

/** mark a thread as unstalled in the scheduler
 * @param tid thread's id
 */
void thread_schedule_tlrr_c::unstall(int tid){
    // determine which container to look in
    if(this->grouped){
        // look in groups
        bool found = false;
        for(auto& fg : this->groups){
            for(auto& th : fg.threads){
                if(th.tid == tid){
                    found = true;
                    th.stalled = false;
                    break;
                }
            }
            // check if found 
            if(found){
                fg.stalled = false;
                break;
            }
        }
    } else {
        // look in base_group
        for(auto& th : this->base_group){
            if(th.tid == tid){
                th.stalled = false;
                break;
            }
        }
    }
}