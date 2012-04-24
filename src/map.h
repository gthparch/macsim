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
 * File         : map.h
 * Author       : Hyesoon Kim 
 * Date         : 12/18/2007 
 * CVS          : $Id: frontend.h,v 1.5 2008-09-10 02:26:11 kacear Exp $:
 * Description  : map 
 *                based on scarab 
 *********************************************************************************************/

#ifndef MAP_H_INCLUDED 
#define MAP_H_INCLUDED


#include "global_types.h"
#include "global_defs.h"
#include "utils.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief dependence information
///////////////////////////////////////////////////////////////////////////////////////////////
class map_entry_c
{
  public:
    uop_c   *m_uop;             /**< uop pointer */
    Counter  m_uop_num;         /**< uop number */
    Counter  m_inst_num;        /**< uop's instruction number */
    Addr     m_pc;              /**< pc address */
    Counter  m_unique_num;      /**< uop's unique number */
    int      m_thread_id;       /**< thread id */
    int      m_mem_type;        /**< memory type */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief data structure holding memory dependence information
///////////////////////////////////////////////////////////////////////////////////////////////
class mem_map_entry_c
{
  public:
    uop_c* m_uop [BYTES_IN_QUADWORD]; /**< last op to write (invalid when committed) */
    Quad   m_data;              /**< memory dependence data */
    uns8   m_store_mask;        /**< shows position of all distinct stores 
                                  supplying a partial value to this map entry */ 
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief data structure holding dependence information
///////////////////////////////////////////////////////////////////////////////////////////////
class map_data_c
{
  public:
    map_entry_c m_reg_map[NUM_REG_IDS *2]; /**< register map per source */
    bool        m_map_flags[NUM_REG_IDS *2]; /**< map data exist for the source */
    map_entry_c m_last_store[2]; /**< last store map entry */
    bool        m_last_store_flag; /**< last store flag */

    hash_c<mem_map_entry_c>* m_oracle_mem_hash; /**< oracle memory hash */

    /**
     * Constructor
     */
    map_data_c(macsim_c* simBase);

    /**
     * Initialization
     */
    void initialize();
   
  private:
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief register dependence mapping
///////////////////////////////////////////////////////////////////////////////////////////////
class map_c
{
  public:
    /**
     * Constructor
     *  \param simBase - Pointer to base simulation class for perf/stat counters
    */
    map_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~map_c();

    /**
     * Set source N not ready
     */
    void set_not_rdy_bit (uop_c *uop, int bit); 

    /**
     * Set dependent-source information. An uop will check each source's dependence.
     * @param uop myself
     * @param src_num source id (register #) of the uop
     * @param map_entry map entry corresponding to the register #
     * @param type register dependence (always)
     */
    void add_src_from_map_entry(uop_c *uop, int src_num, map_entry_c *map_entry, 
        Dep_Type type); 

    /**
     * Set dependent-source information from a source uop
     */
    void add_src_from_uop (uop_c *uop, uop_c *src_uop, Dep_Type type); 

    /**
     * Add memory dependence information
     */
    void map_mem_dep(uop_c *uop); 

    /**
     * Update map data with destination id of an uop.
     * By adding destination register in the map, all following dependent uops will be 
     * affected.
     */
    void update_map(uop_c *uop); 

    /**
     * Wrapper function to add register/memory dependences.
     * @see frontend_c::process_ifetch
     */
    void map_uop (uop_c *uop); 

    /**
     * Read register dependence map. New register dependences will be added for each source
     * registers
     */
    void read_reg_map (uop_c *uop); 

    /**
     * Read store dependence map. When a memory operation calls this function, new store
     * dependence information will be added.
     */
    void read_store_map(uop_c *uop); 

    /**
     * Update load-store dependence
     */
    void update_store_hash (uop_c *uop); 

    /**
     * Add load-store dependence
     */
    uop_c * add_store_deps (uop_c * uop); 

    /**
     * Delete a store dependence information when store instruction has been retired.
     */
    void delete_store_hash_entry (uop_c *uop); 

    /**
     * When a thread has been terminated, delete entire dependence information
     */
    void delete_map(int);

  public:
    hash_c<map_data_c>* m_core_map_data; /**< per thread dependence table */

  private:
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

}; 

#endif // MAP_H_INCLUDED 

