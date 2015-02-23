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
 * File         : dram.h 
 * Author       : HPArch Research Group
 * Date         : 2/18/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Memory controller
 *********************************************************************************************/


#ifndef DRAM_H
#define DRAM_H


#include "macsim.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Memory controller base class
///////////////////////////////////////////////////////////////////////////////////////////////
class dram_c
{
  public:
    /**
     * Constructor
     */
    dram_c(macsim_c* simBase);

    /**
     * Destructor
     */
    virtual ~dram_c() = 0;

    /**
     * Print all requests in the DRAM request buffer
     */
    virtual void print_req(void) = 0; 

    /**
     * Initialize MC
     */
    virtual void init(int id) = 0; 

    /**
     * Tick a cycle
     */
    virtual void run_a_cycle(bool) = 0;

  protected:
    /**
     * Send a packet to NOC
     */
    virtual void send(void) = 0;

    /**
     * Receive a packet from NOC
     */
    virtual void receive(void) = 0;

  private:
    dram_c(); // do not implement

  protected:
    macsim_c* m_simBase; /**< simulation base class */
    Counter m_cycle; /**< dram clock cycle */
    int m_id; /**< MC id */
};


// wrapper function to allocate a dram scheduler
dram_c* fcfs_controller(macsim_c* simBase);
dram_c* frfcfs_controller(macsim_c* simBase);
dram_c* dramsim_controller(macsim_c* simBase);
#ifdef USING_SST
dram_c* vaultsim_controller(macsim_c* simBase);
#endif


#endif
