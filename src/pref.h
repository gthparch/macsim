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


