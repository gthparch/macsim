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
 * File         : network_simple.h 
 * Author       : HPArch Research Group
 * Date         : 4/28/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Simple single-hop network
 *********************************************************************************************/


#ifndef NETWORK_SIMPLE_H
#define NETWORK_SIMPLE_H


#include "macsim.h"
#include "network.h"


class router_simple_c : public router_c
{
  public:
    router_simple_c(macsim_c* simBase, int type, int id);
    ~router_simple_c();

    virtual void run_a_cycle(bool);
    void process(void);
    virtual void print_link_info(void);
    virtual void stage_rc(void);
    virtual void stage_vca_pick_winner(int, int, int&, int&);
    virtual void reset(void);
    virtual int* get_num_packet_inserted(void);

  private:
    router_simple_c();

  private:
    int m_max_num_injection_from_src;
    int m_max_num_accept_in_dst;

    int m_packet_inserted;
};


class network_simple_c : public network_c
{
  public:
    network_simple_c(macsim_c* simBase);
    ~network_simple_c();
    virtual void run_a_cycle(bool);
    virtual void print(void);
    virtual void init(int num_cpu, int num_gpu, int num_l3, int num_mc);

  private:
    network_simple_c();
};


#endif
