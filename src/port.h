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
