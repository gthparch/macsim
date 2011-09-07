/**********************************************************************************************
 * File         : bug_detector.h
 * Author       : Jaekyu Lee
 * Date         : 9/8/2010
 * SVN          : $Id: dram.h 868 2009-11-05 06:28:01Z kacear $
 * Description  : Bug Detector
 *********************************************************************************************/


#include <fstream>
#include <iomanip>

#include "bug_detector.h"
#include "memreq_info.h"
#include "dram.h"
#include "assert_macros.h"

#include "uop.h"
#include "trace_read.h"

#include "all_knobs.h"

// constructor
bug_detector_c::bug_detector_c(macsim_c* simBase)
{
  m_simBase = simBase;

  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;

  m_num_core = *m_simBase->m_knobs->KNOB_NUM_SIM_CORES;

  for (int ii = 0; ii < m_num_core; ++ii) {
    map<uop_c*, uint64_t> *new_map = new map<uop_c*, uint64_t>;
    m_uop_table.push_back(new_map);
  }

  m_latency_sum   = new uint64_t[m_num_core];
  m_latency_count = new uint64_t[m_num_core];
  
  std::fill_n(m_latency_sum, m_num_core, 0);
  std::fill_n(m_latency_count, m_num_core, 0);
}


// destructor
bug_detector_c::~bug_detector_c()
{
  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;

  delete m_latency_sum;
  delete m_latency_count;

  while (!m_uop_table.empty()) {
    auto *new_map = m_uop_table.back();
    m_uop_table.pop_back();

    new_map->clear();
    delete new_map;
  }
}


// insert a new uop to the table
void bug_detector_c::allocate(uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;

  int core_id = uop->m_core_id;

  ASSERT(m_uop_table[core_id]->find(uop) == m_uop_table[core_id]->end());
  (*m_uop_table[core_id])[uop] = m_simBase->m_simulation_cycle;
}


// delete an uop from the table
void bug_detector_c::deallocate(uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;

  int core_id = uop->m_core_id;

  ASSERT(m_uop_table[core_id]->find(uop) != m_uop_table[core_id]->end());

  int latency = m_simBase->m_simulation_cycle - (*m_uop_table[core_id])[uop];
  m_latency_sum[core_id] += latency;
  ++m_latency_count[core_id];

  m_uop_table[core_id]->erase(uop);
}


// sort uop table based on uop id in ascending order
bool sort_uop(uop_c* a, uop_c* b)
{
  return a->m_uop_num < b->m_uop_num;
}


// print all bug information
void bug_detector_c::print(int core_id, int thread_id)
{
  if (!*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    return ;


  ofstream out("bug_detect_uop.out");
  for (int ii = 0; ii < m_num_core; ++ii) {
    int average_latency = 0;
    if (m_latency_count[ii] > 0)
      average_latency = m_latency_sum[ii] / m_latency_count[ii];

    out << "Current cycle:" << m_simBase->m_simulation_cycle << "\n";
    out << "Core id:" << core_id << "\n";
    out << "Last terminated thread:" << thread_id << "\n";

    out << "----------------------------------------------------------\n";
    out << "core " << ii << "\n";
    out << "average execution cycle: " << average_latency << "\n";
    out 
      << setw(10) << left << "INST_NUM"
      << setw(10) << left << "UOP_NUM"
      << setw(15) << left << "CYCLE"
      << setw(15) << left << "DELTA"
      << setw(25) << left << "STATE"
      << setw(25) << left << "OPCODE"
      << setw(20) << left << "UOP_TYPE"
      << setw(20) << left << "MEM_TYPE"
      << setw(20) << left << "CF_TYPE"
      << setw(20) << left << "DEP_TYPE" 
      << setw(6)  << left << "CHILD"
      << setw(10) << left << "PARENT"
      << "\n";

    list<uop_c*> temp_list1;
    for (auto I = m_uop_table[ii]->begin(), E = m_uop_table[ii]->end(); I != E; ++I) {
      temp_list1.push_back((*I).first);
    }
    temp_list1.sort(sort_uop);

    for (auto I = temp_list1.begin(), E = temp_list1.end(); I != E; ++I) {
      if (m_simBase->m_simulation_cycle - (*m_uop_table[ii])[(*I)] < average_latency * 5)
        continue;

      uop_c *uop = (*I);
      ASSERT(uop);
      out
        << setw(10) << left << uop->m_inst_num
        << setw(10) << left << uop->m_uop_num
        << setw(15) << left << (*m_uop_table[ii])[(*I)]
        << setw(15) << left << m_simBase->m_simulation_cycle - (*m_uop_table[ii])[(*I)]
        << setw(25) << left << uop_c::g_uop_state_name[uop->m_state] 
        << setw(25) << left << trace_read_c::g_tr_opcode_names[uop->m_opcode] 
        << setw(20) << left << uop_c::g_uop_type_name[uop->m_uop_type]
        << setw(20) << left << uop_c::g_mem_type_name[uop->m_mem_type]
        << setw(20) << left << uop_c::g_cf_type_name[uop->m_cf_type]
        << setw(20) << left << uop_c::g_dep_type_name[uop->m_bar_type]
        << setw(6)  << left << uop->m_num_child_uops
        << setw(10) << left << (uop->m_parent_uop == NULL ? 0 : uop->m_parent_uop->m_uop_num)
        << "\n";
    }
    out << "\n\n";
    temp_list1.clear();
  }
  out.close();
}

