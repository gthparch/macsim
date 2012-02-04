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
 * File         : port.h
 * Author       : Hyesoon Kim 
 * Date         : 3/19/2008
 * CVS          : $Id: port.h,v 1.1 2008-04-01 04:44:47 hyesoon Exp $:
 * Description  : port 
                  origial author scarab 
**********************************************************************************************/

#ifndef PORT_H_INCLUDED 
#define PORT_H_INCLUDED 


#include <string>

#include "global_defs.h"
#include "global_types.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Port class
///
/// model execution / data cache port
///////////////////////////////////////////////////////////////////////////////////////////////
class port_c
{
  public:
    /**
     * port class constructor.
     */
    port_c ();

    /**
     * port class constructor.
     * @param name - port name 
     */
    port_c (string name);
    
    /**
     * port class constructor.
     * @param name port name
     * @param read number of read ports
     * @param write number of write ports
     * @param writes_prevent_reads write port access will block read port access
     * @param simBase the pointer to the base simulation class
     */
    port_c (string name, uns read, uns write, bool writes_prevent_reads, macsim_c* simBase);
    
    /**
     * Initialize ports
     */
    void init_port (string name, uns read, uns write, bool writes_prevent_reads); 

    /**
     * Acquire a read port. In each cycle, N (m_num_read_ports) accesses will acquire
     * read ports.
     */
    bool get_read_port (Counter cycle_count); 

    /**
     * Acquire a write port. In each cycle, N (m_num_read_ports) accesses will acquire
     * write ports.
     */
    bool get_write_port (Counter cycle_count); 

  private:
    string  m_name; /**< port name */ 
    Counter m_read_last_cycle; /**< last read port access cycle */ 
    Counter m_write_last_cycle; /**< last write port access cycle */
    uns     m_num_read_ports; /**< number of total read ports */
    uns     m_read_ports_in_use; /**< number of currently using read ports */
    uns     m_num_write_ports; /**< number of total write ports */
    uns     m_write_ports_in_use; /**< number of currently using write ports */
    bool    m_writes_prevent_reads; /**< read port blocked due to write port access */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};

#endif /* #ifndef PORT_H_INCLUDED  */ 
