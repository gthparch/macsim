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


/*******************************************************************************
 * File         : readonly_cache.h
 * Author       : Nagesh BL
 * Date         : 03/03/2010 
 * SVN          : $Id: 
 * Description  : read-only cache
 *               (each instance of this class encapsulates an instance of 
 *                class cache_c)
 ******************************************************************************/

#ifndef READONLY_CACHE_H
#define READONLY_CACHE_H

#include <string>

#include "cache.h"
#include "global_defs.h"
#include "global_types.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Readonly cache (Model GPU's constant/texture cache)
///
/// class for read-only cache. read-only means that the core cannot 
/// write to cache blocks in the cache, core can only perform read-only 
/// accesses. in case of a cache miss, the missing cache block will be fetched from memory
///////////////////////////////////////////////////////////////////////////////////////////////
class readonly_cache_c
{
  public:

    /**
     *  \brief constructor to create a read-only cache
     */
    readonly_cache_c(string name, int c_id, uns32 c_size, uns8 c_assoc, 
        uns8 c_line_size, uns8 c_banks, uns8 c_cycles, bool by_pass, Cache_Type c_type, 
        int c_data_size, macsim_c* simBase);

   /*! \fn ~readonly_cache_c()
    *  \brief destructor for class readonly_cache_c
    */
    ~readonly_cache_c();

   /*! \fn load(uop_c *uop)
    *  \brief access read-only cache
    *  \param uop - pointer to uop which requires access to the read-only cache
    *  \return int - 
    */
    int load (uop_c *uop); 

   /*! \fn cache_fill_line(mem_req_s *req)
    *  \brief fill read-only cache with cache block fetched from memory
    *  \param req - pointer to memory request 
    *  \return int - 
    */
    bool cache_fill_line(mem_req_s *req);

   /*! \fn base_cache_line(Addr addr)
    *  \brief get the address of the cache line containing the specified address
    *  \param addr - address whose cache line address is required
    *  \return Addr - requested cache line address
    */
    Addr base_cache_line(Addr addr);

   /*! \fn cache_line_size(void)
    *  \brief returns the cache block size for this cache
    *  \return cache block size
    */
    uns8 cache_line_size(void);

  private:
    int        m_core_id; /**< core id */
    uns32      m_cache_size; /**< cache size */
    uns8       m_cache_assoc; /**< associativity */
    uns8       m_cache_line_size; /**< cache line size */
    uns8       m_cache_banks; /**< number of cache banks */
    uns8       m_cache_cycles; /**< cache latency */
    Cache_Type m_cache_type; /**< cache type */
    cache_c   *m_cache; /**< cache data structure */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};

#endif //READONLY_CACHE_H_INCLUDED
