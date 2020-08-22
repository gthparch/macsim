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
 * File         : dram_ramulator.h
 * Author       : HPArch Research Group
 * Date         : 11/28/2017
 * Description  : Ramulator interface
 *********************************************************************************************/

#ifndef __RAMULATOR_HH__
#define __RAMULATOR_HH__

#ifdef RAMULATOR

#include <deque>
#include <map>
#include <tuple>

#include "ramulator_wrapper.h"
#include "ramulator/src/Config.h"

#include "dram.h"
#include "memreq_info.h"
#include "network.h"

class dram_ramulator_c : public dram_c
{
private:
  unsigned int requestsInFlight;
  std::map<long, std::deque<mem_req_s *>> reads;
  std::map<long, std::deque<mem_req_s *>> writes;
  std::deque<mem_req_s *> resp_queue;

  ramulator::Config configs;
  ramulator::RamulatorWrapper *wrapper;
  std::function<void(ramulator::Request &)> read_cb_func;
  std::function<void(ramulator::Request &)> write_cb_func;

  // Default Constructor
  dram_ramulator_c();  // do not implement

  // Send a packet to NOC
  void send(void);

  // Receive a packet from NOC
  void receive(void);

  // Read callback function
  void readComplete(ramulator::Request &ramu_req);

  // Write callback function
  void writeComplete(ramulator::Request &ramu_req);

public:
  // Constructor
  dram_ramulator_c(macsim_c *simBase);

  // Destructor
  ~dram_ramulator_c();

  // Initialize MC
  void init(int);

  // Print all requests in DRB
  void print_req(void);

  // Tick a cycle
  void run_a_cycle(bool);
};

#endif  // RAMULATOR
#endif  // __RAMULATOR_HH__
