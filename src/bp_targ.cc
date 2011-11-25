/***************************************************************************************
 * File         : bp_targ.cc
 * Author       : Hyesoon Kim 
 * Date         : 4/5/2010 
 * SVN          : $Id: bp.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : branch target predictor 
 ***************************************************************************************/

/* 
 * Summary: Branch Target Prediction Mechanisms. The default behavior: BTB 
 */

#include "global_types.h"
#include "global_defs.h"
#include "bp_targ.h"
#include "all_knobs.h"
#include "cache.h"
#include "statistics.h"
#include "statsEnums.h"

#include "debug_macros.h"

#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_BTB, ## args)


bp_targ_c::bp_targ_c(uns core_id_arg, macsim_c* simBase) 
{

  btb = new cache_c("btb", 
		    simBase->m_knobs->KNOB_BTB_ENTRIES->getValue() * 4, 
		    simBase->m_knobs->KNOB_BTB_ASSOC->getValue(), 
		    4, 
		    sizeof(Addr), 
		    simBase->m_knobs->KNOB_BTB_BANK_NUM->getValue(), 
		    false, 
		    core_id_arg, 
		    CACHE_BTB,
		    false, 
		    simBase); 
		    
  m_core_id = core_id_arg; 
  m_simBase = simBase; 
}

Addr bp_targ_c::pred (uop_c *uop)
{
  Addr line_addr, return_addr=0;
  Addr* return_addr_ptr = NULL;
  bool perfect_pred = false; 
  
  perfect_pred = (m_simBase->m_knobs->KNOB_PERFECT_BTB); 

  if (perfect_pred) 
  {
    return_addr = uop->m_target_addr; 
    STAT_CORE_EVENT(m_core_id, PERFECT_TARGET_PRED); 
  }
  else { 
    int appl_id = m_simBase->m_core_pointers[uop->m_core_id]->get_appl_id(uop->m_thread_id);
    return_addr_ptr = (Addr *) btb->access_cache (uop->m_pc, &line_addr, true, appl_id);
    return_addr = return_addr_ptr ? *return_addr_ptr : 0; 
  }
  
  // debug 
  Addr tag; 
  int set; 
  btb->find_tag_and_set(uop->m_pc, &tag, &set);
  /*
  if (uop->m_pc == 0x401aa5) lca_count++; 
  DEBUG("Reading BTB m_pc:%s target:0x%s m_uop_num:%s core_id:%d thread_id:%d cf_type:%d btb_addr:%s set:%d tag:0x%s \n",
	//	hexstr64s(uop->uop_info.pred_addr),
	hexstr64s(uop->m_pc),
	hexstr64s(uop->m_target_addr),
	unsstr64(uop->m_uop_num),
	m_core_id, 
	uop->thread_id, 
	uop->m_cf_type,
	(return_addr) ? hexstr64s(return_addr): "-1",
	set, 
	hexstr64s(tag));
  */
  uop->m_uop_info.m_btb_set = set; 
  return return_addr; 
}



void bp_targ_c::update (uop_c *uop)
{
  Addr fetch_addr = uop->m_pc; 
  Addr *btb_line = NULL;
  Addr btb_line_addr, repl_line_addr; 
  bool insert_btb = false; 
  // bool no_btb_insert = false; 

  if (uop->m_off_path) return; 
  
  Addr tag; 
  int set; 
  btb->find_tag_and_set(uop->m_pc, &tag, &set);

  int appl_id = m_simBase->m_core_pointers[uop->m_core_id]->get_appl_id(uop->m_thread_id);
  btb_line = (Addr *)btb->access_cache(fetch_addr, &btb_line_addr, false, appl_id);




  //  ASSERTM(fetch_addr == btb_line_addr, "fetch_addr:0x%s btb_line_addr:0x%s\n", hexstr64s(fetch_addr), hexstr64s(btb_line_addr)); 


  if (btb_line == NULL) 
  {
    
    btb_line = (Addr *)btb->insert_cache(fetch_addr, &btb_line_addr, &repl_line_addr, appl_id, false); 
      insert_btb = true; 
  }

    *btb_line = uop->m_target_addr; 

      //  if ((btb_line == NULL) || (*btb_line != uop->m_target_addr) )
     DEBUG("Writing BTB pc:0x%s target:0x%s m_uop_num:%s core_id:%d thread_id:%d cf_type:%d btb_line:%s set:%d tag:0x%s insert_btb:%d\n",
	hexstr64s(uop->m_pc), 
	hexstr64s(uop->m_target_addr),
	unsstr64(uop->m_uop_num),
	m_core_id, 
	uop->m_thread_id, 
	uop->m_cf_type,
        (btb_line) ? hexstr64s(*btb_line) : "-1", 
	set, 
	hexstr64s(tag),
	insert_btb);
}
