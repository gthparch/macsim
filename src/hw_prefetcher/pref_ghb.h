/***************************************************************************************
 * File         : pref_ghb.h
 * Author       : Santhosh Srinath
 * Date         : 11/16/2004
 * CVS          : $Id: pref_ghb.h,v 1.1 2008/07/30 14:18:16 kacear Exp $:
 * Description  : 
 ***************************************************************************************/
#ifndef __PREF_GHB_H__

#include "../pref_common.h"
#include "../pref.h"

#define CZONE_TAG( x ) ( x >> ( *m_simBase->m_knobs->KNOB_PREF_GHB_CZONE_BITS ) )

typedef struct GHB_Index_Table_Entry_Struct {
  int tid;
  Addr czone_tag;
  bool valid;
  // CHECKME
  int  ghb_ptr;     // ptr to last entry in ghb with same czone
  int  last_access; // for lru 
} GHB_Index_Table_Entry;


typedef struct GHB_Entry_Struct { 
  Addr miss_index;
  int  ghb_ptr;    // -1 == invalid
  int  ghb_reverse_ptr; // -1 == invalid
  int  idx_reverse_ptr;
} GHB_Entry;


class pref_ghb_c : public pref_base_c{
  friend class pref_common_c;
 private:
  // Index table
  GHB_Index_Table_Entry * index_table;
  // GHB
  GHB_Entry         * ghb_buffer;

  int                 ghb_tail;
  int                 ghb_head;

  int                 deltab_size;
  int               * delta_buffer;

  uns                 pref_degree;

  uns                 pref_degree_vals[5];

  pref_ghb_c();

 public:
  pref_ghb_c(hwp_common_c *, Unit_Type, macsim_c*);
  ~pref_ghb_c();

  /*************************************************************/
  /* HWP Interface */
  void init_func(int core_id);
  void done_func() {}
  void l1_miss_func(int, Addr, Addr, uop_c *) ; 
  void l1_hit_func(int, Addr, Addr, uop_c *);
  void l1_pref_hit_func(int, Addr, Addr, uop_c *) {}
  void l2_miss_func(int tid,  Addr lineAddr, Addr loadPC, uop_c *uop);
  void l2_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *) {}
  void l2_pref_hit_func( int tid, Addr lineAddr, Addr loadPC, uop_c *uop);
  
  
  void pref_ghb_l2_train(int tid, Addr lineAddr, Addr loadPC, bool l2_hit, uop_c *uop);

  /*************************************************************/
  /* Misc functions */
  void pref_ghb_create_newentry (int idx, Addr line_addr, Addr czone_tag, int old_ptr, int tid);

  void pref_ghb_throttle(void);
  void pref_ghb_throttle_fb(void);

};


#define __PREF_GHB_H__
#endif /*  __PREF_GHB_H__*/