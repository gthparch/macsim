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

    /*! \fn void alloc_sb()
     *  \brief Function to allocate store buffers.
     *  \return void. 
     */
    void alloc_sb() 
    {
      ASSERT(m_num_sb > 0);
      m_num_sb--;
    }

    /*! \fn dealloc_sb()
     *  \brief Function to deallocate store buffers.
     *  \return void. 
     */
    void dealloc_sb() 
    {
      ASSERT(m_num_sb < m_max_sb_cnt);
      m_num_sb++;
    }

    /*! \fn int get_num_sb()
     *  \brief Function to return count of store buffers.
     *  \return int Storage buffer count. 
     */
    int get_num_sb() 
    {
      return m_num_sb;
    }

    /*! \fn void alloc_lb()
     *  \brief Function allocate load buffer.
     *  \return void. 
     */
    void alloc_lb() 
    {
      ASSERT(m_num_lb > 0);
      m_num_lb--;
    }

    /*! \fn dealloc_lb()
     *  \brief Function deallocate load buffer.
     *  \return void. 
     */
    void dealloc_lb() 
    {
      ASSERT(m_num_lb < m_max_lb_cnt);
      m_num_lb++;
    }

    /*! \fn int get_num_lb()
     *  \brief Function to return load buffer count.
     *  \return int Load buffer count. 
     */
    int get_num_lb() 
    {
      return m_num_lb;
    }

    /*! \fn get_num_int_regs()
     *  \brief Function to return count of integer registers.
     *  \return int Integer register count. 
     */
    int get_num_int_regs() 
    {
      return m_num_int_regs;
    }

    /*! \fn get_num_fp_regs()
     *  \brief Function to return count of floating point registers. 
     *  \return int Floating point register count.
     */
    int get_num_fp_regs() 
    {
      return m_num_fp_regs;
    }

    /**
     * Allocate a integer register
     */
    void alloc_int_reg();

    /**
     * Allocate a fp register
     */
    void alloc_fp_reg();

    /**
     * Deallocate integer register
     */
    void dealloc_int_reg();

    /**
     * Deallocate fp register
     */
    void dealloc_fp_reg();

  private:
    int       m_max_cnt; /**< max rob entries */
    int       m_usable_cnt; /**< usable rob entries */
    int       m_free_cnt; /**< free rob entries */
    uop_c**   m_rob; /**< rob */
    int       m_first_entry; /**< to maintain circular rob */ 
    int       m_last_entry; /**< to maintain circular rob */
    int       m_max_sb_cnt; /**< max store buffer size */
    int       m_num_sb; /**< number of available store buffer */
    int       m_max_lb_cnt; /**< max load buffer size */
    int       m_num_lb; /**< number of available load buffer */
    int       m_max_int_regs; /**< max integer register */
    int       m_num_int_regs; /**< number of available int register */ 
    int       m_max_fp_regs; /**< max fp register */
    int       m_num_fp_regs; /**< number of available fp register */
    Unit_Type m_unit_type; /**< core type */ 
    uns16     m_knob_rob_size; /**< reorder buffer size */
    
    macsim_c*       m_simBase; /**< macsim_c base class for simulation globals */
};
#endif // ROB_H_INCLUDED 

