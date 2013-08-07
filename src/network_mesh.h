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
 * File         : network_mesh.h 
 * Author       : HPArch Research Group
 * Date         : 2/18/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Ring network
 *********************************************************************************************/


#ifndef NETWORK_MESH_H
#define NETWORK_MESH_H

#include <vector>
#include <unordered_map>
#include <queue>
#include <list>

#include "macsim.h"
#include "network.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Router class
///////////////////////////////////////////////////////////////////////////////////////////////
class router_mesh_c : public router_c
{
  public:
    /**
     * Constructor
     */
    router_mesh_c(macsim_c* simBase, int type, int id);

    /**
     * Destructor
     */
    ~router_mesh_c();
    
    /**
     * Print link information
     */
    void print_link_info();

    /**
     * RC (Route Calculation) stage
     */
    void stage_rc(void);

    /**
     * VC arbitration
     */
    void stage_vca_pick_winner(int, int, int&, int&);

  private:
    router_mesh_c(); // do not implement
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Router Wrapper Class
///////////////////////////////////////////////////////////////////////////////////////////////
class network_mesh_c : public network_c
{
  public:
    /**
     * Constructor
     */
    network_mesh_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~network_mesh_c();

    /**
     * Run a cycle
     */
    void run_a_cycle(bool);

    /**
     * Initialize interconnection network
     */
    
    void init(int num_cpu, int num_gpu, int num_l3, int num_mc);

//    bool send(mem_req_s* req, int src_level, int src_id, int dst_level, int dst_id);
    
//    mem_req_s* receive(int level, int id);

//    void receive_pop(int level, int id);

    /**
     * Print all router information
     */
    void print(void);

  private:
    network_mesh_c();
};

#endif
