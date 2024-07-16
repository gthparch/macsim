/***************************************************************************************
 * File         : pref_stream.h
 * Author       : Santhosh Srinath ( based on Hyesoon's code )
 * Date         : 1/20/2005
 * CVS          : $Id: pref_stream.h,v 1.1 2008/07/30 14:18:16 kacear Exp $:
 * Description  : Stream Prefetcher
 ***************************************************************************************/
#ifndef __STREAM_PREF_H__

#include "../memory.h"
#include "../pref_common.h"
#include "../pref.h"

typedef struct Stream_Buffer_Struct {
  int tid;
  Addr load_pc[4];
  Addr line_index;
  Addr sp;
  Addr ep;
  Addr start_vline;
  int dir;
  int lru;
  bool valid;
  bool buffer_full;
  bool trained;
  int train_hit;
  uns length; // Now with the pref accuracy, we can dynamically tune the length
  uns pref_issued;
  uns pref_useful;
}Stream_Buffer;

// stream HWP 
typedef struct Pref_Stream_Struct {
  pref_info_s* hwp_info;
    
  Stream_Buffer *stream;
  Stream_Buffer *l2hit_stream;

  Addr *train_filter;
  int train_filter_no;

  uns train_num; // With pref accuracy, dynamically tune the train length
  uns distance;
  uns pref_degree_vals[5];
 
  uns num_tosend;
  uns num_tosend_vals[5];
}Pref_Stream;


class pref_stream_c : public pref_base_c
{
 private:
  Pref_Stream *pref_stream;
  pref_stream_c();
  
 public:
  pref_stream_c(hwp_common_c *, Unit_Type, macsim_c* simBase);

  void init_func(int );
  void done_func(void);
  void l1_miss_func(int, Addr, Addr, uop_c *) {} 
  void l1_hit_func(int, Addr, Addr, uop_c *);
  void l1_pref_hit_func(int, Addr, Addr, uop_c *) {}
  void l2_miss_func(int tid,  Addr lineAddr, Addr loadPC, uop_c *);
  void l2_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *);
  void l2_pref_hit_func(int, Addr, Addr, uop_c*) {}


  void pref_stream_train(int tid, Addr lineAddr, Addr loadPC, bool create);

  int  pref_stream_train_create_stream_buffer(Addr line_index, bool train, bool create, int tid);
  bool pref_stream_train_stream_filter(Addr line_index);

  inline void pref_stream_addto_train_stream_filter(Addr line_index);

  bool pref_stream_req_queue_filter(Addr line_addr); 

  void pref_stream_remove_redundant_stream(int hit_index);

  //void pref_stream_runahead(Counter op_num);

  // Used when throttling using the overall accuracy numbers
  void pref_stream_throttle(void);

  void pref_stream_throttle_fb(void);

  /////////////////////////////////////////////////
  // Used when throttling for each stream separately
  // NON FUNCTIONAL CURRENTLY
  void pref_stream_throttle_streams(Addr line_index);
  void pref_stream_throttle_stream(int index);
  // Again - Use ONLY when throttling per stream
  float pref_stream_acc_getacc(int index, float pref_acc);
  void  pref_stream_acc_l2_useful(Addr line_index);
  void  pref_stream_acc_l2_issued(Addr line_index);
  ///////////////////////////////////////////////////

};


#define __STREAM_PREF_H__
#endif /*  __STREAM_PREF_H__*/