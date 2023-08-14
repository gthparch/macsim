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
 * File         : memreq_info.cc
 * Author       : Hyesoon Kim
 * Date         : 3/20/2008
 * SVN          : $Id: memreq_info.h,v 1.5 2008-09-17 21:01:41 kacear Exp $:
 * Description  : Memory request information
 *********************************************************************************************/

#include "memreq_info.h"

// memory request type string
const char* mem_req_c::mem_req_type_name[MAX_MEM_REQ_TYPE] = {
  "IFETCH",  "DFETCH",      "DSTORE",     "IPRF",       "DPRF",       "WB",
  "SW_DPRF", "SW_DPRF_NTA", "SW_DPRF_T0", "SW_DPRF_T1", "SW_DPRF_T2",
};

// memory request state string
const char* mem_req_c::mem_state[MEM_STATE_MAX] = {
  "MEM_INV",
  "MEM_NEW",
  "MEM_MERGED",
  "MEM_OUTQUEUE_NEW",
  "MEM_IN_NOC",
  "MEM_OUT_FILL",
  "MEM_OUT_WB",
  "MEM_FILL_NEW",
  "MEM_FILL_WAIT_DONE",
  "MEM_FILL_WAIT_FILL",
  "MEM_DRAM_START",
  "MEM_DRAM_CMD",
  "MEM_DRAM_DATA",
  "MEM_DRAM_DONE",
  "MEM_NOC_START",
  "MEM_NOC_DONE",
};

mem_req_s::mem_req_s(macsim_c* simBase) {
  init();
  m_simBase = simBase;
}

void mem_req_s::init(void) {
  m_id = 0;
  m_appl_id = 0;
  m_core_id = 0;
  m_thread_id = 0;
  m_block_id = 0;
  m_state = MEM_INV;
  m_type = MRT_IFETCH;
  m_priority = 0;
  m_addr = 0;
  m_size = 0;
  m_rdy_cycle = 0;
  m_pc = 0;
  m_prefetcher_id = 0;
  m_pref_loadPC = 0;
  m_acc = false;
  m_queue = NULL;
  for (int ii = 0; ii < MEM_LAST; ++ii) m_cache_id[ii] = 0;
  m_uop = NULL;
  m_in = 0;
  m_dirty = false;
  m_done = false;
  m_merged_req = NULL;
  m_msg_type = 0;
  m_msg_src = 0;
  m_msg_dst = 0;
  m_done_func = NULL;
  m_bypass = 0;
}
