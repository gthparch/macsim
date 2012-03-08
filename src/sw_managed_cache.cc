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
 * File         : sw_managed_cache.cc
 * Author       : Nagesh BL
 * Date         : 03/03/2010 
 * SVN          : $Id: cache.h,
 * Description  : Software Managed Cache
 *********************************************************************************************/

#include "core.h"
#include "port.h"
#include "sw_managed_cache.h"
#include "uop.h"


///////////////////////////////////////////////////////////////////////////////////////////////
//  sw_managed_cache_c() - constructor
//    creates and initializes internal cache object and access ports   
///////////////////////////////////////////////////////////////////////////////////////////////
sw_managed_cache_c::sw_managed_cache_c(string name, int c_id, uns32 c_size, uns8 c_assoc, 
    uns8 c_line_size, uns8 c_banks, uns8 c_cycles, bool by_pass, Cache_Type c_type, 
    uns n_read_ports, uns n_write_ports, int c_data_size, macsim_c* simBase) :
  m_core_id(c_id), m_cache_cycles(c_cycles), m_cache_line_size(c_line_size), 
  m_num_banks(c_banks) 
{
  //Reference to simulation-scoped members
  m_simBase = simBase;

  int num_set = c_size / c_assoc / c_line_size;
  // allocate cache
  m_cache = new cache_c(name, num_set, c_assoc, c_line_size, c_data_size, 
      m_num_banks, by_pass, m_core_id, c_type, false, m_simBase);

  // allocate port
  m_ports = new port_c* [m_num_banks];
  for (int i = 0; i < m_num_banks; ++i) {
    m_ports[i] = new port_c(name, n_read_ports, n_write_ports, true, m_simBase);
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////
//  ~sw_managed_cache_c() - destructor
//    deletes internal cache object and access ports   
///////////////////////////////////////////////////////////////////////////////////////////////
sw_managed_cache_c::~sw_managed_cache_c()
{
  delete m_cache;
  for (int i = 0; i < m_num_banks; ++i) {
    delete m_ports[i];
  }
  delete [] m_ports;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//  load()
//    returns - 0 for bank conflicts
//            - x > 0, x -> no. of cycles in which the access will complete
// Since stores to this scratchpad memory have been controlled by the software,
// when we access this structure, we can assume required data is already brought
///////////////////////////////////////////////////////////////////////////////////////////////
int sw_managed_cache_c::load(uop_c *uop)
{
  Counter cur_core_cycle = m_simBase->m_core_cycle[m_core_id];
  int bank_num = m_cache->get_bank_num(uop->m_vaddr);

  // check for bank conflicts
  // in cycles when writes are being performed, for 
  // simulation purposes read ports behave as write 
  // ports
  if (m_ports[bank_num]->get_read_port(cur_core_cycle))
    return m_cache_cycles;
  else
    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//  base_cache_line()
//    returns cache line address of specified address
///////////////////////////////////////////////////////////////////////////////////////////////
Addr sw_managed_cache_c::base_cache_line(Addr addr)
{
  return m_cache->base_cache_line(addr);
}


///////////////////////////////////////////////////////////////////////////////////////////////
//  cache_line_size()
//    returns cache line size
///////////////////////////////////////////////////////////////////////////////////////////////
uns8 sw_managed_cache_c::cache_line_size(void)
{
  return m_cache_line_size;
}

