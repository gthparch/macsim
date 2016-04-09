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
 * File         : rob.h
 * Author       : Hyesoon Kim 
 * Date         : 1/1/2008
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : reorder buffer
 *********************************************************************************************/

#ifndef ROB_H_INCLUDED
#define ROB_H_INCLUDED


#include "global_types.h"
#include "global_defs.h"

#include "assert_macros.h"
#include "utils.h"
#include "fence.h"

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ROB (Reorder Buffer) class
///////////////////////////////////////////////////////////////////////////////////////////////
class rob_c
{
  public:
    /**
     * \brief Create a Reorder Buffer data structure.
     * \param type - Core type indicator
     * \param simBase - Pointer to base simulation class for perf/stat counters     
     * \return void. 
     */
    rob_c(Unit_Type type, macsim_c* simBase);

    /*! \fn void ~rob_c()
     *  \brief Reorder Buffer data structure destructor.
     *  \return void. 
     */
    ~rob_c(); 

    /*! \fn void push(uop_c* s)
     *  \brief Function to push an arguement into the ROB.
     *  \param s - Uop to be pushed
     *  \return void. 
     */
    void push(uop_c* s);

    /*! \fn void pop()
     *  \brief Function to pop an arguement off the ROB.
     *  \return void. 
     */
    void pop();

    /*! \fn void reinit()
     *  \brief Function to re-nitialize the ROB.
     *  \return void. 
     */
    void reinit();

    /*! \fn void entries()
     *  \brief Function to return current entry count of the ROB.
     *  \return int Entry count for the ROB. 
     */
    inline int entries() 
    {
      return (m_usable_cnt - m_free_cnt);
    }

    /*! \fn void space()
     *  \brief Function to return the free space count of the ROB.
     *  \return int free space in the ROB. 
     */
    int space() 
    {
      return m_free_cnt;
    }

    /*! \fn uop_c*& operator[](int index)
     *  \brief Function to overload "[]" for indexing into the ROB.
     *  \return uop_c* Entry at the index in ROB. 
     */
    uop_c*& operator[](int index) 
    {
      // no bounds checking
      return m_rob[index];
    }

    /*! \fn uop_c* back()
     *  \brief Function to return the last entry in the ROB.
     *  \return uop_c*  Last Uop in ROB. 
     */
    uop_c* back() 
    {
      return m_rob[(m_last_entry + m_max_cnt - 1) % m_max_cnt];
    }

    /*! \fn uop_c* front()
     *  \brief Function to return the first entry in the ROB. 
     *  \return First Uop in ROB. 
     */
    uop_c* front() 
    {
      return m_rob[m_first_entry];
    }

    /*! \fn int front_rob() 
     *  \brief Function to return the index of first ROB entry
     *  \return int Index of first entry. 
     */
    int front_rob() 
    {
      return this->m_first_entry;
    }

    /*! \fn int last_rob()
     *  \brief Function to return the index of last ROB entry 
     *  \return int Index of last entry. 
     */
    int last_rob() 
    {
      return this->m_last_entry;
    }

    /*! \fn int inc_index(int index) 
     *  \brief Function to increment index based on ROB size.
     *  \return int Incremented index. 
     */
    int inc_index(int index) 
    {
      return (index + 1) % m_max_cnt;
    }

    /*! \fn int dec_index(int index)
     *  \brief Function to decrement index based on ROB size. 
     *  \return int Decremented index.
     */
    int dec_index(int index) 
    {
      return (index + m_max_cnt - 1) % m_max_cnt;
    }

    /**
     * Check if there are any non-retired memory ops
     */
    bool pending_mem_ops(int entry);

    /**
     * Increment the number of fences in ROB
     */
    void ins_fence_entry(int entry, enum fence_type ft = FENCE_FULL);

    /**
     * Remove the number of fence in ROB
     */
    void del_fence_entry(enum fence_type ft = FENCE_FULL);

    /**
     * Check if a fence is active in the ROB
     */
    bool is_fence_active();

    /**
     * Check if memory ordering is to be ensured
     */
    bool ensure_mem_ordering(int entry);

    /**
     * Set the write buffer empty flag
     */
    void set_wb_empty(bool state);

    /**
     * Update and populate version information
     */
    void process_version(uop_c* uop);

    /**
     * Get the lowest version in the ordering root queue
     */
    uint16_t get_orq_version(void)
    {
      if (m_orq.empty())
        return -1;

      return m_orq.front().version;
    }

    /**
     * Check if fences in orq can be removed
     */
    void update_orq(Counter lowest_age);
    void update_root(uop_c* uop);

    /** 
     * Debug: Print version info
     */
    void print_version_info(void);

    /**
     * Check if ordering satisfied according to version
     */
    bool version_ordering_check(uop_c* uop);

    /**
     * Check if version bits are available
     */
    bool version_bits_avail(void) { return m_last_fence_version < 0xFFFF;  }

  private:
    bool is_later_entry(int first_fence_entry, int entry);
    void print_fence_entries(enum fence_type ft = FENCE_FULL);
    int  get_relative_index(int entry);

  private:
    int       m_max_cnt; /**< max rob entries */
    int       m_usable_cnt; /**< usable rob entries */
    int       m_free_cnt; /**< free rob entries */
    uop_c**   m_rob; /**< rob */
    int       m_first_entry; /**< to maintain circular rob */ 
    int       m_last_entry; /**< to maintain circular rob */
    Unit_Type m_unit_type; /**< core type */ 
    uns16     m_knob_rob_size; /**< reorder buffer size */

    /* All fences in the ROB */
    fence_c         m_fence;
    bool            m_wb_empty;
    bool            m_wb_perm;

    uint16_t        m_version; /*< Current version of loads/stores */
    Counter         m_reset_uop_num;
    uint16_t        m_last_fence_version; /*< version of the last scheduled fence */

    struct orq_entry {
      Counter  uop_num;
      uint16_t  version;
    };

    list<orq_entry>  m_orq; /*< FIFO queue of root fence versions */
    unordered_map<Counter, bool> m_root_fences; /*< parent uop of fence inst */ 
    
    macsim_c*       m_simBase; /**< macsim_c base class for simulation globals */
};

#endif // ROB_H_INCLUDED
