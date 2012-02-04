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
 * File         : bug_detector.h
 * Author       : Jaekyu Lee
 * Date         : 9/8/2010
 * SVN          : $Id: dram.h 868 2009-11-05 06:28:01Z kacear $
 * Description  : Bug Detector
 *********************************************************************************************/

#ifndef BUG_DETECTOR_H_INCLUDED
#define BUG_DETECTOR_H_INCLUDED


#include <vector>
#include <map>
#include <unordered_map>

#include "macsim.h"
#include "global_types.h"
#include "global_defs.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief bug detection mechanism
///
/// Track each uop and memory reqeust to check where it is stuck 
///////////////////////////////////////////////////////////////////////////////////////////////
class bug_detector_c 
{
  public:
    /**
     * Bug detector class constructor
     */
    bug_detector_c(macsim_c* simBase);

    /**
     * Bug detector class destructor
     */
    ~bug_detector_c();

    /**
     * When a new uop starts exectuion, record it to the table.
     */
    void allocate(uop_c *uop);

    /**
     * When a new uop retires, take this uop out from the table.
     */
    void deallocate(uop_c *uop);

    /**
     * Print all un-retired (might be bug) uops
     */
    void print(int core_id, int thread_id);

    /**
     * Record noc packets
     */
    void allocate_noc(mem_req_s* req);

    /**
     * Deallocate serviced noc packets
     */
    void deallocate_noc(mem_req_s* req);

    /**
     * Print NoC information
     */
    void print_noc();

  private:
    int m_num_core; /**< number of simulating cores */
    vector<map<uop_c*, uint64_t> *> m_uop_table; /**< uop table */
    uint64_t *m_latency_sum; /**< sum of each uop's execution latency */
    uint64_t *m_latency_count; /**< total uop count */

    unordered_map<mem_req_s*, uint64_t>* m_packet_table; /**< memeory requests in noc */
    
    macsim_c* m_simBase; /**< pointer to the simulation base class */
};
#endif
