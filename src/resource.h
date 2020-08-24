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
 * File         : resource.h
 * Author       : Hyesoon Kim
 * Date         : 1/1/2016
 * Description  : resource structre
 *********************************************************************************************/

#ifndef RESOURCE_H_INCLUDED
#define RESOURCE_H_INCLUDED

#include "global_types.h"
#include "global_defs.h"

#include "assert_macros.h"
#include "utils.h"

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Resource Structure class
///////////////////////////////////////////////////////////////////////////////////////////////
class resource_c
{
public:
  /**
   * \breif Create a resource data structure.
   * \param - Core type indicator, simulator pointer
   * \return void.
   */

  resource_c(Unit_Type type, macsim_c* simBase);

  /*! \fn void ~resource_c()
   *  \brief resource structure destructor.
   *  \return void.
   */
  ~resource_c();

  /*! \fn void alloc_sb()
   *  \brief Function to allocate store buffers.
   *  \return void.
   */

  void alloc_sb() {
    ASSERT(m_num_sb > 0);
    m_num_sb--;
  }

  /*! \fn dealloc_sb()
   *  \brief Function to deallocate store buffers.
   *  \return void.
   */

  void dealloc_sb() {
    ASSERT(m_num_sb < m_max_sb_cnt);
    m_num_sb++;
  }

  /*! \fn int get_num_sb()
   *  \brief Function to return count of store buffers.
   *  \return int Storage buffer count.
   */

  int get_num_sb() {
    return m_num_sb;
  }

  /*! \fn void alloc_lb()
   *  \brief Function allocate load buffer.
   *  \return void.
   */

  void alloc_lb() {
    ASSERT(m_num_lb > 0);
    m_num_lb--;
  }

  /*! \fn dealloc_lb()
   *  \brief Function deallocate load buffer.
   *  \return void.
   */

  void dealloc_lb() {
    ASSERT(m_num_lb < m_max_lb_cnt);
    m_num_lb++;
  }

  /*! \fn int get_num_lb()
   *  \brief Function to return load buffer count.
   *  \return int Load buffer count.
   */

  int get_num_lb() {
    return m_num_lb;
  }

  /*! \fn get_num_int_regs()
   *  \brief Function to return count of integer registers.
   *  \return int Integer register count.
   */

  int get_num_int_regs() {
    return m_num_int_regs;
  }

  /*! \fn get_num_fp_regs()
   *  \brief Function to return count of floating point registers.
   *  \return int Floating point register count.
   */

  int get_num_fp_regs() {
    return m_num_fp_regs;
  }

  /**
   * Allocate a integer register
   */

  void alloc_int_reg() {
    ASSERT(m_num_int_regs > 0);
    --m_num_int_regs;
  }

  /**
   * Allocate a fp register
   */

  void alloc_fp_reg() {
    ASSERT(m_num_fp_regs > 0);
    --m_num_fp_regs;
  }

  /**
   * Deallocate integer register
   */

  void dealloc_int_reg() {
    ++m_num_int_regs;
  }

  /**
   * Deallocate fp register
   */

  void dealloc_fp_reg() {
    ++m_num_fp_regs;
  }

  /**
   * reset the hardware resource counter values
   */

  void reset();

private:
  int m_max_sb_cnt; /**< max store buffer size */
  int m_num_sb; /**< number of available store buffer */
  int m_max_lb_cnt; /**< max load buffer size */
  int m_num_lb; /**< number of available load buffer */
  int m_max_int_regs; /**< max integer register */
  int m_num_int_regs; /**< number of available int register */
  int m_max_fp_regs; /**< max fp register */
  int m_num_fp_regs; /**< number of available fp register */

  Unit_Type m_unit_type; /**< core type */
  macsim_c* m_simBase; /**< macsim_c base class for simulation globals */
};

#endif  // RESOURCE_H INCLUDED
