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
 * File         : dram_dramsim3.cc
 * Author       : HPArch Research Group(Seonjin Na)
 * Date         : 9/21/2023
 * Description  : DRAMSim3 interface
 *********************************************************************************************/


#ifdef DRAMSIM3
// #include "DRAMSim.h"

//# DRAMSIM3 related header 
#include "configuration.h"
#include "dram_system.h"
//#########################

#include "dram_dramsim3.h"
#include "assert_macros.h"
#include "debug_macros.h"
#include "bug_detector.h"
#include "memory.h"
#include <iostream>
#include <cstring>
#include <unistd.h>


#include "all_knobs.h"
#include "statistics.h"

#undef DEBUG
#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_DRAM, ##args)

using namespace dramsim3;


void dummy_call_back(uint64_t addr) {
    // call_back_called = true;
    return;
}


void dram_dramsim3_c::read_callback2(uint64_t address){
  // find requests with this address
  auto I = m_pending_request->begin();
  auto E = m_pending_request->end();

  printf("Read Call BAck!!\n");
  while (I != E) {
    mem_req_s* req = (*I);
    ++I;

    if (req->m_addr == address) {
  int latency = *KNOB(KNOB_DRAM_ADDITIONAL_LATENCY);

      printf("address : %ld read_callback ! Latnecy: %d\n",address, latency);
      if (*KNOB(KNOB_DRAM_ADDITIONAL_LATENCY)) {
        req->m_rdy_cycle = m_cycle + *KNOB(KNOB_DRAM_ADDITIONAL_LATENCY);
        m_tmp_output_buffer->push_back(req);
      } else
        m_output_buffer->push_back(req);
      m_pending_request->remove(req);
    }
  }
  return;
}

void dram_dramsim3_c::write_callback2( uint64_t address) {
  // find requests with this address
  auto I = m_pending_request->begin();
  auto E = m_pending_request->end();
  while (I != E) {
    mem_req_s* req = (*I);
    ++I;

      // printf("address : %d read_callback ! Latnecy: %d\n",address, *KNOB(KNOB_DRAM_ADDITIONAL_LATENCY);


    if (req->m_addr == address) {
      // in case of WB, retire requests here
      MEMORY->free_req(req->m_core_id, req);
      int latency = *KNOB(KNOB_DRAM_ADDITIONAL_LATENCY);
     printf("address : %ld read_callback ! Latnecy: %d\n",address, latency);
      m_pending_request->remove(req);
    }
  }
  return ;
}


/** FIXME
 * How to handle redundant requests
 * Fix .ini file and output directories
 */

dram_dramsim3_c::dram_dramsim3_c(macsim_c* simBase) : dram_c(simBase) {
  m_output_buffer = new list<mem_req_s*>;
  m_tmp_output_buffer = new list<mem_req_s*>;
  m_pending_request = new list<mem_req_s*>;

  char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        std::cout << "Current path: " << buffer << std::endl;
    } else {
        perror("getcwd");
    }

  m_config = new Config("../src/DRAMSim3/configs/DDR4_8Gb_x8_2133_2.ini", ".");
  printf("m_config epoch_period : %d\n",m_config->epoch_period);
  // m_dramsim = new JedecDRAMSystem(m_config, ".", &dram_dramsim3_c::read_callback2,
  //                                 &dram_dramsim3_c::write_callback2);

  // function<void(uint64_t)> read_cb = &dram_dramsim3_c::read_callback2;

   read_cb = std::bind(&dram_dramsim3_c::read_callback2, this, std::placeholders::_1);
  write_cb = std::bind(&dram_dramsim3_c::write_callback2, this, std::placeholders::_1);



  // function<void(uint64_t)> write_cb =std:: 

 m_dramsim = new JedecDRAMSystem(*m_config, ".", read_cb,
                                  write_cb);

  m_dramsim->RegisterCallbacks(read_cb,write_cb);


    // std::function<void(uint64_t)> read_cb = std::bind( &dram_dramsim3_c::read_callback2,this,std::placeholder::_1);

  // std::function<void(uint64_t)> 
  // std::function<void(uint64_t)> read_cb = std::binwrite_cbd(&dram_dramsim3_c::read_callback2,this);
  // std::function<void(uint64_t)> write_cb = std::bind(&dram_dramsim3_c::write_callback2,this);


//  m_dramsim = new JedecDRAMSystem(m_config, ".",&dram_dramsim3_c::read_callback2,
//                                   &dram_dramsim3_c::write_callback2);
}

dram_dramsim3_c::~dram_dramsim3_c() {
  delete m_output_buffer;
  delete m_tmp_output_buffer;
  delete m_pending_request;
  delete m_dramsim;
}

void dram_dramsim3_c::print_req(void) {
}

void dram_dramsim3_c::init(int id) {
  m_id = id;
}

void dram_dramsim3_c::run_a_cycle(bool temp) {
  // printf("DRAMSIM3!!!!\n");
  send();
  m_dramsim->ClockTick();
  receive();
  ++m_cycle;
}


void dram_dramsim3_c::read_callback(unsigned id, uint64_t address,
                                   uint64_t clock_cycle) {
  // find requests with this address
  auto I = m_pending_request->begin();
  auto E = m_pending_request->end();
  while (I != E) {
    mem_req_s* req = (*I);
    ++I;

    if (req->m_addr == address) {
      if (*KNOB(KNOB_DRAM_ADDITIONAL_LATENCY)) {
        req->m_rdy_cycle = m_cycle + *KNOB(KNOB_DRAM_ADDITIONAL_LATENCY);
        m_tmp_output_buffer->push_back(req);
      } else
        m_output_buffer->push_back(req);
      m_pending_request->remove(req);
    }
  }
}

void dram_dramsim3_c::write_callback(unsigned id, uint64_t address,
                                    uint64_t clock_cycle) {
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

void dram_dramsim3_c::receive(void) {
  mem_req_s* req = NETWORK->receive(MEM_MC, m_id);
  if (!req) return;

  if (m_dramsim->AddTransaction(req->m_type == MRT_WB,
                                static_cast<uint64_t>(req->m_addr))) {
    STAT_EVENT(TOTAL_DRAM);
    m_pending_request->push_back(req);

    int pending_req_size = m_pending_request->size();
    printf("m_pending_request push  %d\n",pending_req_size);

    NETWORK->receive_pop(MEM_MC, m_id);
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE)) {
      m_simBase->m_bug_detector->deallocate_noc(req);
    }
  }
}

void dram_dramsim3_c::send(void) {
  vector<mem_req_s*> temp_list;

  for (auto I = m_tmp_output_buffer->begin(), E = m_tmp_output_buffer->end();
       I != E; ++I) {
    mem_req_s* req = *I;
    if (req->m_rdy_cycle <= m_cycle) {
      temp_list.push_back(req);
      m_output_buffer->push_back(req);
    } else {
      break;
    }
  }

  for (auto itr = temp_list.begin(), end = temp_list.end(); itr != end; ++itr) {
    m_tmp_output_buffer->remove((*itr));
  }

  for (auto I = m_output_buffer->begin(), E = m_output_buffer->end(); I != E;
       ++I) {
    mem_req_s* req = (*I);
    req->m_msg_type = NOC_FILL;
    bool insert_packet =
      NETWORK->send(req, MEM_MC, m_id, MEM_LLC, req->m_cache_id[MEM_LLC]);

    if (!insert_packet) {
      DEBUG("MC[%d] req:%d addr:0x%llx type:%s noc busy\n", m_id, req->m_id,
            req->m_addr, mem_req_c::mem_req_type_name[req->m_type]);
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

dram_c* dramsim3_controller(macsim_c* simBase) {
  return new dram_dramsim3_c(simBase);
}

#endif