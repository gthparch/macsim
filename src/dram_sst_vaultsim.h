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
 * File         : dram_sst_vaultsim.h 
 * Author       : HPArch Research Group
 * Date         : 05/12/2014
 * Description  : Memory Controller for SST-VaultSim Component
 *********************************************************************************************/


#ifndef DRAM_SST_VAULTSIM_H
#define DRAM_SST_VAULTSIM_H

#include <list>

#include "dram.h"
#include "memreq_info.h"
#include "network.h"

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief MC - SST-VaultSim interface
///////////////////////////////////////////////////////////////////////////////////////////////
class dram_sst_vaultsim_c : public dram_c
{
  public:
    /**
     * Constructor
     */
    dram_sst_vaultsim_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~dram_sst_vaultsim_c();

    /**
     * Print all requests in DRB
     */
    void print_req(void);

    /**
     * Initialize MC
     */
    void init(int id);

    /**
     * Tick a cycle
     */
    void run_a_cycle(bool);

  private:
    dram_sst_vaultsim_c(); // do not implement

    /**
     * Send a packet to NOC
     */
    void send(void);

    /**
     * Receive a packet from NOC
     */
    void receive(void);

    void read_callback(uint64_t key);
    void write_callback(uint64_t key);
    void send_packet(mem_req_s* req);
    void receive_packet();

  private:
    list<mem_req_s*>* m_output_buffer; /**< output buffer */
    map<uint64_t, mem_req_s*> m_pending_request;
};
#endif //DRAM_SST_VAULTSIM_H
