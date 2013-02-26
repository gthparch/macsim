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
 * File         : pref.h 
 * Author       : Jaekyu Lee
 * Date         : 2/14/2011
 * SVN          : $Id: dram.cc 912 2009-11-20 19:09:21Z kacear $
 * Description  : Prefetcher base class
 *********************************************************************************************/

#ifndef PREF_BASE_H
#define PREF_BASE_H


#include <string>

#include "global_defs.h"
#include "pref_common.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Hardware prefetcher base class
/// 
/// Prefetcher base class.
/// Based on the event (dcache, L2 cache / miss, hit, pref_hit),
/// corresponding function will be called
///////////////////////////////////////////////////////////////////////////////////////////////
class pref_base_c
{
  friend class pref_common_c;
  public:
    /**
     * Constructor
     */
    pref_base_c(macsim_c* simBase);

    /**
     * Virtual destructor 
     */
    virtual ~pref_base_c() {}

    /**
     * Initialize function
     */
    virtual void init_func(int) = 0;

    /**
     * Done function : When a prefetch request is serviced, done_func will be called.
     */
    virtual void done_func() = 0;

    /**
     * L1 Cache miss event handler
     */
    virtual void l1_miss_func(int tid, Addr addr, Addr pc, uop_c *uop) = 0;

    /**
     * L1 Cache hit event handler
     */
    virtual void l1_hit_func(int tid, Addr addr, Addr pc, uop_c *uop) = 0;

    /**
     * L1 Cache prefetch hit event handler
     */
    virtual void l1_pref_hit_func(int tid, Addr, Addr, uop_c *uop) =0;

    /**
     * L2 Cache miss event handler
     */
    virtual void l2_miss_func(int, Addr, Addr, uop_c *uop) = 0;

    /**
     * L2 Cache hit event handler
     */
    virtual void l2_hit_func(int, Addr, Addr, uop_c *uop)  = 0;

    /**
     * L2 Cache prefetch hit event handler
     */
    virtual void l2_pref_hit_func(int, Addr, Addr, uop_c *uop) = 0;

    bool init;                  /**< Enable init function */
    bool done;                  /**< Enable done function */
    bool l1_miss;              /**< Enable L1 miss handler */
    bool l1_hit;               /**< Enable L1 hit handler */
    bool l1_pref_hit;          /**< Enable L1 prefetch hit handler */
    bool l2_miss;              /**< Enable L2 miss handler */
    bool l2_hit;               /**< Enable L2 hit handler */
    bool l2_pref_hit;          /**< Enable L2 prefetch hit handler */

  public: 
    string        name;         /**< prefetcher name */
    HWP_Type      hwp_type;     /**< prefetcher type */
    pref_info_s*  hwp_info;     /**< prefetcher information structure */
    hwp_common_c *hwp_common;   /**< pointer to prefetcher framework */
    bool          knob_enable;  /**< enable prefetcher */
    int           core_id;      /**< core id */
    int           shift_bit;    /**< cache line address shift bits */

  protected:
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Dummy prefetcher class. 
///
/// Show how to use base prefetcher class
///////////////////////////////////////////////////////////////////////////////////////////////
class pref_dummy_c : public pref_base_c
{
  public:
    /**
     * Constructor
     */
    pref_dummy_c(macsim_c* simBase) : pref_base_c(simBase) {};

    /**
     * Destructor
     */
    ~pref_dummy_c() {}

    /**
     * Initialize function
     */
    void init_func(int a) {}
    
    /**
     * Done function : When a prefetch request is serviced, done_func will be called.
     */
    void done_func() {}
    
    /**
     * L1 Cache miss event handler
     */
    void l1_miss_func(int a, Addr b, Addr c, uop_c *d) {}

    /**
     * L1 Cache hit event handler
     */
    void l1_hit_func(int a, Addr b, Addr c, uop_c *d) {}

    /**
     * L1 Cache prefetch hit event handler
     */
    void l1_pref_hit_func(int a, Addr b, Addr c, uop_c *d) {}

    /**
     * L2 Cache miss event handler
     */
    void l2_miss_func(int a, Addr b, Addr c, uop_c *d) {}

    /**
     * L2 Cache hit event handler
     */
    void l2_hit_func(int a, Addr b, Addr c, uop_c *d) {}
    
    /**
     * L2 Cache prefetch hit event handler
     */
    void l2_pref_hit_func(int a, Addr b, Addr c, uop_c *d) {}
};

#endif


