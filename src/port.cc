/**********************************************************************************************
 * File         : port.cc
 * Author       : Hyesoon Kim 
 * Date         : 3/19/2008
 * CVS          : $Id: port.cc,v 1.2 2008-07-30 20:54:07 kacear Exp $:
 * Description  : port 
                  origial author scarab 
 *********************************************************************************************/


/*
 * Summary: Model exectuion/memory ports
 */

#include "global_defs.h"
#include "global_types.h"
#include "port.h"
#include "debug_macros.h"
#include "assert_macros.h"

#include "all_knobs.h"

#define DEBUG(args...)	 _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_PORT, ## args) 	


// port_c constructor
port_c::port_c(string name, uns read, uns write, bool writes_prevent_reads, macsim_c* simBase)
{
  m_simBase              = simBase;
  
  DEBUG("Initializing ports called '%s'.\n", name.c_str());

  m_name                 = name; 
  m_read_last_cycle      = 0;
  m_write_last_cycle     = 0;
  m_num_read_ports       = read;
  m_read_ports_in_use    = 0;
  m_num_write_ports      = write;
  m_write_ports_in_use   = 0;
  m_writes_prevent_reads = writes_prevent_reads;
}


// initialize port
void port_c::init_port(string name, uns read, uns write, bool writes_prevent_reads)
{
  DEBUG("Initializing ports called '%s'.\n", name.c_str());
  
  m_name                 = name; 
  m_read_last_cycle      = 0;
  m_write_last_cycle     = 0;
  m_num_read_ports       = read;
  m_read_ports_in_use    = 0;
  m_num_write_ports      = write;
  m_write_ports_in_use   = 0;
  m_writes_prevent_reads = writes_prevent_reads;
}


// get read port
bool port_c::get_read_port (Counter cycle_count)
{
  if (m_read_last_cycle != cycle_count) {
    ASSERT(m_num_read_ports > 0);
 
    m_read_last_cycle   = cycle_count;
    m_read_ports_in_use = 1;
    DEBUG("get_read_port successful\n");
    return true;
  }

  if (m_read_ports_in_use == m_num_read_ports) {
    DEBUG("get_read_port failed (%d ports in use)\n", m_read_ports_in_use);
    return false;
  }

  if (m_write_ports_in_use && m_writes_prevent_reads) {
    DEBUG("get_read_port failed (%d writes preventing reads)\n", 
          m_write_ports_in_use);
    return false;
  }

  DEBUG("get_read_port successful\n");
  ++m_read_ports_in_use;

  return true;
}


// get write port
bool port_c::get_write_port (Counter cycle_count)
{
  if (m_write_last_cycle != cycle_count) {
    ASSERT(m_num_write_ports > 0);

    m_write_last_cycle   = cycle_count;
    m_write_ports_in_use = 1;
    DEBUG("get_write_port successful cycle_count:%lld  ports in used:%d \n", 
          cycle_count, m_write_ports_in_use);
    return true;
  }

  if (m_write_ports_in_use == m_num_write_ports) {
    DEBUG("get_write_port failed (%d ports in use) cycle_count:%lld \n", 
          m_write_ports_in_use, cycle_count);
    return false;
  }

  if (m_writes_prevent_reads) {
    ASSERT(m_read_ports_in_use == 0); 
  }
  ++m_write_ports_in_use;

  return true;
}
