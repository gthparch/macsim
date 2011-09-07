/***************************************************************************************
 * File         : pref_stride.h
 * Author       : Santhosh Srinath
 * Date         : 1/23/2005
 * CVS          : $Id: pref_stride.h,v 1.1 2008/07/30 14:18:16 kacear Exp $:
 * Description  : Stride prefetcher 
 ***************************************************************************************/

#ifndef __PREF_STRIDE_H__
#define __PREF_STRIDE_H__

#include "pref_common.h"
#include "pref.h"


#define STRIDE_REGION(x) ( x >> (*m_simBase->m_knobs->KNOB_PREF_STRIDE_REGION_BITS) )


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief stride region information table entry
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct stride_region_table_entry_struct {
  int  tid; /**< thread id */
  Addr tag; /**< address tag */
  bool valid; /**< valid */
  uns  last_access; /**< lru */

  /**
   * Constructor
   */
  stride_region_table_entry_struct() {
    valid = false;
  }
} stride_region_table_entry_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief stride prefetcher table entry
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct stride_index_table_entry_struct { 
  bool    trained; /**< trained */
  bool    train_count_mode; /**< train count mode */ 
  uns     num_states; /**< number of states */
  uns     curr_state; /**< current state */
  Addr    last_index; /**< last index */
  int     stride[2]; /**< stride information */
  int     s_cnt[2]; /**< stride count information */
  int     strans[2]; /**< stride12, stride21 */
  int     recnt; /**< recount */
  int     count; /**< count */
  int     pref_count; /**< prefetch count */
  uns     pref_curr_state; /**< prefetch current state */
  Addr    pref_last_index; /**< prefetch last index */
  Counter pref_sent; /**< number of prefetch sent */

  /**
   * Constructor
   */
  stride_index_table_entry_struct() {
    trained = false;
  }
} stride_index_table_entry_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief stride prefetcher
/// 
/// Stride prefetcher. For more detailed information, refer to pref_base_c class
/// @see pref_base_c
///////////////////////////////////////////////////////////////////////////////////////////////
class pref_stride_c : public pref_base_c
{
  friend class pref_common_c;
  public:
    /**
     * Default constructor
     */
    pref_stride_c(macsim_c* simBase);

    /**
     * Constructor
     */
    pref_stride_c(hwp_common_c *, Unit_Type, macsim_c* simBase);

    /**
     * Destructor
     */
    ~pref_stride_c();

    /**
     * Init function
     */
    void init_func(int);

    /**
     * Done function
     */
    void done_func() {}

    /**
     * L1 miss function
     */
    void l1_miss_func(int, Addr, Addr, uop_c *);

    /**
     * L1 hit function
     */
    void l1_hit_func(int, Addr, Addr, uop_c *);

    /**
     * L1 prefetch hit function
     */
    void l1_pref_hit_func(int, Addr, Addr, uop_c *) {}

    /**
     * L2 miss function
     */
    void l2_miss_func(int, Addr, Addr, uop_c *);

    /**
     * L2 hit function
     */
    void l2_hit_func(int, Addr, Addr, uop_c *);

    /**
     * L2 prefetch hit function
     */
    void l2_pref_hit_func(int, Addr, Addr, uop_c *) {}

    /**
     * Train stride table
     */
    void train(int, Addr, Addr, bool);

    /**
     * Create a new stride
     */
    void create_newentry(int idx, Addr line_addr, Addr region_tag);

  private:
    stride_region_table_entry_s *region_table; /**< address region info */
    stride_index_table_entry_s  *index_table; /**< prefetch table */


};

#endif


