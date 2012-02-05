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
 * File         : pref_common.h
 * Author       : HPArch
 * Date         : 11/30/2004
 * CVS          : $Id: pref_common.h,v 1.2 2008/09/12 03:42:01 kacear Exp $:
 * Description  : Common framework for working with prefetchers - less stuff to mess with
 *********************************************************************************************/


#ifndef PREF_COMMON_H
#define PREF_COMMON_H


#include <unordered_map>

#include "memreq_info.h"


///////////////////////////////////////////////////////////////////////////////////////////////

// There are many deprecated (legacy) functions with the new memory model, although these 
// functions are usable. Thereby, these functions will remain for future usage.

///////////////////////////////////////////////////////////////////////////////////////////////


#define PREF_TRACKERS_NUM 16
#define LOG2_DCACHE_LINE_SIZE log2_int(m_simBase->m_memory->line_size(core_id))
#define LOG2_PREF_REGION_SIZE log2_int(*m_simBase->m_knobs->KNOB_PREF_REGION_SIZE)


///////////////////////////////////////////////////////////////////////////////////////////////


int  pref_compare_hwp_priority(const void * const a, const void * const b);
int  pref_compare_prefloadhash(const void * const a, const void * const b);


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Prefetch request data structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct pref_mem_req_s {
  Addr line_addr;               /**< line address */
  Addr line_index;              /**< line index */
  Addr loadPC;                  /**< load pc */
  uns8 prefetcher_id;           /**< prefecher id */
  bool valid;                   /**< valid */
  int  core_id;                 /**< core_id */
  int  thread_id;               /**< thread id */

  /**
   * Constructor.
   */
  pref_mem_req_s() {
    valid = false;
  }
} pref_mem_req_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Hardware prefetcher structure (information, stats)
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct pref_info_s {
  uns8    id;                   /**< This prefetcher's id */
  bool    enabled;              /**< Is the prefetcher enabled */
  int     priority;             /**< priority this prefetcher gets in the pecking order */
  Counter useful;               /**< num of useful prefetches */
  Counter sent;                 /**< num of sent prefetches */
  Counter late;                 /**< num of late prefetches */
  Counter curr_useful;          /**< num of current period Useful prefetches */
  Counter curr_sent;            /**< num of current period sent prefetches */
  Counter curr_late;            /**< num of current period late prefetches */
  Counter hybrid_lastuseful;    /**< used for Hybrid */
  Counter hybrid_lastsent;      /**< This helps maintain a better indication of recent history */
  Addr    trackers[PREF_TRACKERS_NUM]; /**< tracking information */
  bool    trackers_used[PREF_TRACKERS_NUM]; /**< tracker used */
  int     track_num; /**< number of tracks */
  Counter track_lastsample_cycle; /**< track last sample cycle */
  Counter trackhist[10][PREF_TRACKERS_NUM]; /**< tracking history */
  Counter prefhit_count;        /**< prefetch hit count */
  uns     dyn_degree;           /**< dynamic prefetch degree */
} pref_info_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Hardware prefetcher type enumerator
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum HWP_Type_Enum {
  Mem_To_UL1,
  UL1_To_DL0,
} HWP_Type;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Dynamic prefetch aggressiveness control enumerator
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum HWP_DynAggr_Enum {
  AGGR_DEC,
  AGGR_STAY,
  AGGR_INC,
} HWP_DynAggr;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Prefetch region line status enumerator
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum HWP_Region_LineUseStatus_Enum {
  HWP_UL1_MISS,           // Data came in due to a load miss
  HWP_UL1_PREF_UNUSED,    // Data came in due to a prefetch - went unused
  HWP_UL1_PREF_USED       // Data came in due to a prefetch - was used
} HWP_Region_LineUseStatus;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Prefetch region line status information
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct pref_region_line_status_s {
  bool    l2_hit;      /**< Was this line a hit in the cache */
  bool    l2_miss;     /**< Was this line a miss in the cache */
  uns16   pref_sent;    /**< Bit vector indicating if the prefetcher sent pref before a load */
  bool    l2_evict;    /**< Was this line evicted */
  Counter cycle_evict;  /**< When was it evicted - used only for prefetched lines */
  bool    evict_onPF;   /**< Line(Demand) evicted due to a Prefetch */

  /**
   * Constructor.
   */
  pref_region_line_status_s() {
    l2_hit      = false;
    l2_miss     = false;
    pref_sent   = 0;
    l2_evict    = false;
    cycle_evict = 0;
    evict_onPF  = false;
  }
} pref_region_line_status_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Prefetch region information
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct pref_region_info_s {
  Addr    region_id; /**< id */
  Counter last_access; /**< last access cycle */
  bool    valid; /**< valid */
  bool    trained; /**< trained / for hybrid prefetching */
  uns8    pref_id; /**< id, if trained / for hybrid prefetching */
  pref_region_line_status_s* status; /**< line status */

  /**
   * Constructor.
   */
  pref_region_info_s() {
    valid = false;
  }
} pref_region_info_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Hardware prefetcher wrapper class
///////////////////////////////////////////////////////////////////////////////////////////////
class hwp_common_c 
{
  public:
    /**
     * Constructor.
     */
    hwp_common_c(int cid, Unit_Type type, macsim_c* simBase);

    /**
     * Destructor.
     */
    ~hwp_common_c();

    /**
     * Send a prefetch request to L1 request queue.
     */
    bool pref_addto_l1req_queue(Addr line_index, uns8 prefetcher_id );

    /**
     * Send a prefetch request to L2 request queue.
     */
    bool pref_addto_l2req_queue(Addr line_index, uns8 prefetcher_id );

    /**
     * Send a prefetch request to L2 request queue.
     */
    bool pref_addto_l2req_queue(Addr line_index, uns8 prefetcher_id, Addr loadAddr );

    /**
     * Send a prefetch request to L2 request queue.
     */
    bool pref_addto_l2req_queue(Addr line_index, uns8 prefetcher_id, Addr loadAddr, int tid);

    /**
     * Send a prefetch request to L2 request queue.
     */
    bool pref_addto_l2req_queue_set(Addr line_index, uns8 prefetcher_id, bool Begin, \
        bool End, Addr loadAddr);

    /**
     * Send a prefetch request to L2 request queue.
     */
    bool pref_addto_l2req_queue_set(Addr line_index, uns8 prefetcher_id, bool Begin, \
        bool End, Addr loadAddr, int tid);

    /**
     * Train hardware prefetchers.
     */
    void train(int level, int tid, Addr line_addr, Addr load_PC, uop_c* uop, bool hit);

    /**
     * Initialize prefetchers.
     */
    void pref_init(bool ptx);

    /**
     * Done wrapper function when a prefetch request is serviced.
     */
    void pref_done(void);

    /**
     * L1 miss handler
     */
    void pref_l1_miss(int tid, Addr line_addr, Addr load_PC, uop_c *uop);

    /**
     * L1 hit handler
     */
    void pref_l1_hit(int tid, Addr line_addr, Addr load_PC, uop_c *uop);

    /**
     * L1 prefetch hit handler
     */
    void pref_l1_pref_hit(int tid, Addr line_addr, Addr load_PC, uns8 prefetcher_id, \
        uop_c *uop);

    /**
     * L2 miss handler
     */
    void pref_l2_miss(mem_req_s *req);

    /**
     * L2 miss handler
     */
    void pref_l2_miss(int tid, Addr line_addr, uop_c* uop);

    /**
     * L2 hit handler
     */
    void pref_l2_hit (mem_req_s *req);

    /**
     * L2 hit handler
     */
    void pref_l2_hit(int tid, Addr line_addr, Addr load_PC, uop_c *uop);

    /**
     * L2 prefetch hit handler
     */
    void pref_l2_pref_hit(int tid, Addr line_addr, Addr load_PC, uns8 prefetcher_id, 
        uop_c *uop);

    /**
     * L2 late prefetch hit handler. Late prefetch indicates that prefetch request is
     * begin serviced when a demand arrives.
     */
    void pref_l2_pref_hit_late(int tid, Addr line_addr, Addr load_PC, uns8 prefetcher_id, \
        uop_c *uop);

    /**
     * Update prefetch queue
     */
    void pref_update_queues(void);

    /**
     * Get region based accuracy
     */
    float pref_get_regionbased_acc(void);

    /**
     * Update prefetch region information
     */
    void  pref_update_regioninfo(Addr line_addr, bool l2_hit, bool l2_miss, bool evict_onPF, 
        Counter cycle_evict, uns8 prefetcher_id);
    
    /**
     * Find matching region. If no matching region found, allocate a new region in
     * LRU position.
     */
    int pref_matchregion(Addr reg_id, bool);
    
    /**
     * Get L2 cache pollution rate
     */
    float pref_get_l2pollution(void);
    
    /**
     * Get overall prefetch accuracy
     */
    float pref_get_overallaccuracy(HWP_Type);

    /**
     * Get prefetch accuracy.
     * @param prefetcher_id prefetcher id
     */
    float pref_get_accuracy(uns8 prefetcher_id);

    /**
     * Get the new dynamic degree of the prefetcher. Used along with feedback driven 
     * hardware prefetcher.
     */
    HWP_DynAggr pref_get_degfb(uns8 prefetcher_id);

    /**
     * Get the prefetch timeliness.
     */
    float pref_get_timeliness(uns8 prefetcher_id);

    /**
     * For hybrid prefetching, try to make a prefetcher selection based on the policy.
     */
    void pref_hybrid_makeselection(int reg_idx);

    /**
     * Update trackers_used information.
     */
    void pref_acc_useupdate(Addr line_addr);

    /**
     * Access prefetch pollution bit vector.
     */
    char* pref_polbv_access(Addr lineIndex);

    /**
     * Update useless prefetch counter for unused prefetch eviction (Deprecated)
     */
    void pref_evictline_notused(Addr);

    /**
     * Update L2 prefetch eviction counter. (Deprecated)
     */
    void pref_l2evict(Addr addr);

    /**
     * L2 eviction by prefetch. Update pollution related data. (Deprecated)
     */
    void pref_l2evictOnPF(Addr addr);
    
    /**
     * Remove entries in l1 request queue with the given address (Deprecated)
     */
    bool pref_l1req_queue_filter(Addr line_addr);

    /**
     * Remove entries in l2 request queue with the given address (Deprecated)
     */
    bool pref_l2req_queue_filter(Addr line_addr);
    
    /**
     * Event handler : prefetch missed in the l2 and went out on the bus. (Deprecated)
     */
    void pref_l2sent(uns8 prefetcher_id);
    
    /**
     * Get the prefetch accuracy. (Deprecated)
     */
    float pref_get_replaccuracy(uns8 prefetcher_id);

  public:
    vector<pref_base_c *> pref_table; /**< prefetchers table */

  private:
    /**
     * Default constructor. Do not implement.
     */
    hwp_common_c();

  private:
    int core_id; /**< core id */
    bool knob_ptx_sim; /**< GPU simulation */
    int m_shift_bit; /**< shift bit to get cache line index */
    bool m_update_acc; /**< set to recalculate the accuracy */

    pref_mem_req_s* m_l1req_queue; /**< L1 prefetch req queue */
    pref_mem_req_s* m_l2req_queue; /**< L2 prefetch req queue */

    int m_l1req_queue_req_pos; /**< L1 request queue insert index */
    int m_l1req_queue_send_pos; /**< L1 request queue send index */
    int m_l2req_queue_req_pos; /**< L2 request queue insert index */ 
    int m_l2req_queue_send_pos; /**< L2 request queue send index */

    // Accuracy Throttling Studies
    Counter m_overall_l1useful; /**< Total number of useful prefetches */
    Counter m_overall_l1sent; /**< Total number of prefetches sent */
    Counter m_overall_l2useful; /**< Total number of useful prefetches */
    Counter m_overall_l2sent; /**< Total number of prefetches sent */
    Counter m_num_uselesspref_evict; /**< Total number of useless prefetches */ 
    Counter m_num_l2_evicts; /**< Total prefetch l2 evictions */

    // stats
    pref_region_info_s *region_info; /**< prefetch region information */
    Counter m_l2_misses; /**< number of total l2 misses */ 
    Counter m_curr_l2_misses; /**< number of total l2 misses in current period */
    Counter m_pfpol; /**< number of prefetch pollution */
    Counter m_curr_pfpol; /**< number of prefetch pollution in current period */
    Counter m_useful;  /**< number of useful prefetches */
    Counter m_region_sent; /**< number of region sent */
    Counter m_region_useful; /**< number of useful region */
    Counter m_curr_region_sent; /**< number of region sent in current period */
    Counter m_curr_region_useful; /**< number of useful region in current period */
    uns m_phase; /**< number of total phase counter */
    char* m_polbv_info; /**< pollution bit vector */

    // for hybrid prefetching
    uns8 m_default_prefetcher; /**< default (best of hybrid) prefetcher */
    Counter m_last_update_time; /**< when did we update the default prefetcher last? */

    unordered_map<int, Counter> m_last_inst_num; /**< train hardware pref once per inst. */

    FILE * PREF_TRACE_OUT; /**< prefetch trace output stream */
    FILE * PREF_ACC_OUT; /**< prefetch accuracy information output stream */
    FILE * PREF_DEGFB_FILE; /**< prefetch degree (with feedback) output stream */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
};

#endif /*  __PREF_COMMON_H__*/


