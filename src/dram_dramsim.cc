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
 * File         : dram_dramsim.cc 
 * Author       : HPArch Research Group
 * Date         : 2/18/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : DRAMSim2 interface
 *********************************************************************************************/


#ifdef DRAMSIM
#include "DRAMSim.h"

#include "dram_dramsim.h"
#include "assert_macros.h"
#include "debug_macros.h"
#include "bug_detector.h"
#include "memory.h"

#include "all_knobs.h"


#undef DEBUG
#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_DRAM, ## args)


using namespace DRAMSim;



/** FIXME
 * How to handle redundant requests
 * Fix .ini file and output directories
 */

dram_dramsim_c::dram_dramsim_c(macsim_c* simBase)
  : dram_c(simBase)
{
  m_output_buffer = new list<mem_req_s*>;
  m_pending_request = new list<mem_req_s*>;
  m_dramsim = getMemorySystemInstance(
      "src/DRAMSim2/ini/DDR2_micron_16M_8b_x8_sg3E.ini", 
      "src/DRAMSim2/system.ini.example", 
      "..", 
      "resultsfilename", 
      16384);

	TransactionCompleteCB *read_cb = new Callback<dram_dramsim_c, void, unsigned, uint64_t, uint64_t>(this, &dram_dramsim_c::read_callback);
	TransactionCompleteCB *write_cb = new Callback<dram_dramsim_c, void, unsigned, uint64_t, uint64_t>(this, &dram_dramsim_c::write_callback);

  m_dramsim->RegisterCallbacks(read_cb, write_cb, NULL);
}


dram_dramsim_c::~dram_dramsim_c()
{
  delete m_output_buffer;
  delete m_pending_request;
  delete m_dramsim;
}


void dram_dramsim_c::print_req(void)
{
}

void dram_dramsim_c::init(int id)
{
  m_id = id;
}


void dram_dramsim_c::run_a_cycle(bool temp)
{
  send();
  m_dramsim->update();
  receive();
  ++m_cycle;
}


void dram_dramsim_c::read_callback(unsigned id, uint64_t address, uint64_t clock_cycle)
{
  // find requests with this address
  auto I = m_pending_request->begin();
  auto E = m_pending_request->end();
  while (I != E) {
    mem_req_s* req = (*I);
    ++I;

    if (req->m_addr == address) {
      m_output_buffer->push_back(req);
      m_pending_request->remove(req);
    }
  }
}


void dram_dramsim_c::write_callback(unsigned id, uint64_t address, uint64_t clock_cycle)
{
  // find requests with this address
  auto I = m_pending_request->begin();
  auto E = m_pending_request->end();
  while (I != E) {
    mem_req_s* req = (*I);
    ++I;

    if (req->m_addr == address) {
      // in case of WB, retire requests here
      MEMORY->free_req(req->m_core_id, req);
      m_pending_request->remove(req);
    }
  }
}


void dram_dramsim_c::receive(void)
{
  mem_req_s* req = NETWORK->receive(MEM_MC, m_id);
  if (!req)
    return;

  if (m_dramsim->addTransaction(req->m_type == MRT_WB, static_cast<uint64_t>(req->m_addr))) {
    m_pending_request->push_back(req);
    NETWORK->receive_pop(MEM_MC, m_id);
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE)) {
      m_simBase->m_bug_detector->deallocate_noc(req);
    }
  }
}


void dram_dramsim_c::send(void)
{
  vector<mem_req_s*> temp_list;
  for (auto I = m_output_buffer->begin(), E = m_output_buffer->end(); I != E; ++I) {
    mem_req_s* req = (*I);
    req->m_msg_type = NOC_FILL;
    bool insert_packet = NETWORK->send(req, MEM_MC, m_id, MEM_L3, req->m_cache_id[MEM_L3]);
    
    if (!insert_packet) {
      DEBUG("MC[%d] req:%d addr:0x%llx type:%s noc busy\n", 
          m_id, req->m_id, req->m_addr, mem_req_c::mem_req_type_name[req->m_type]);
      break;
    }

    temp_list.push_back(req);
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE) && *KNOB(KNOB_ENABLE_NEW_NOC)) {
      m_simBase->m_bug_detector->allocate_noc(req);
    }
  }


  for (auto I = temp_list.begin(), E = temp_list.end(); I != E; ++I) {
    m_output_buffer->remove((*I));
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
// wrapper functions to allocate dram controller object

dram_c* dramsim_controller(macsim_c* simBase)
{
  return new dram_dramsim_c(simBase);
}

#endif


