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

  // allocate cache
  m_cache = new cache_c(name, c_size, c_assoc, c_line_size, c_data_size, 
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

