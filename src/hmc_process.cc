#include "hmc_process.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <map>
#include <set>

#include "hmc_types.h"
#include "process_manager.h"
#include "trace_read_cpu.h"
#include "trace_read.h"
#include "uop.h"
#include "global_types.h"
#include "core.h"
#include "knob.h"
#include "process_manager.h"
#include "debug_macros.h"
#include "statistics.h"
#include "frontend.h"
#include "statsEnums.h"
#include "utils.h"
#include "pref_common.h"
#include "readonly_cache.h"
#include "sw_managed_cache.h"
#include "memory.h"
#include "inst_info.h"
#include "page_mapping.h"
#include "assert_macros.h"
#include "debug_macros.h"

#include "all_knobs.h"

#define DEBUG(args...) _DEBUG(*KNOB(KNOB_DEBUG_TRACE_READ), ##args)
#define DEBUG_CORE(m_core_id, args...)                          \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) {   \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ, ##args); \
  }
#define DEBUG_HMC(args...) _DEBUG(*KNOB(KNOB_DEBUG_HMC), ##args)
#define DEBUG_HMC_CORE(m_core_id, args...)                    \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) { \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_HMC, ##args);      \
  }

// get HMC instruction information
void hmc_function_c::hmc_info_read(
  string file_name_base, map<uint64_t, hmc_inst_s> &m_hmc_info,
  map<std::pair<uint64_t, uint64_t>, hmc_inst_s> &m_hmc_info_ext) {
  ifstream hmc_info_file;
  string hmc_info = file_name_base + ".HMCinfo";
  hmc_info_file.open(hmc_info.c_str());

  unsigned linenum = 0;
  while (hmc_info_file.good()) {
    string line;
    getline(hmc_info_file, line);
    // skip empty lines
    if (line.empty()) continue;
    // skip comment lines
    if (line[0] == '#') continue;
    // skip the first line (csv header)
    if (linenum == 0) {
      linenum++;
      continue;
    }
    uint64_t caller_pc, func_pc, ret_pc, addr_pc;
    string name;
    std::stringstream ss(line);
    ss >> caller_pc >> func_pc >> ret_pc >> addr_pc >> name;

    hmc_inst_s inst;
    inst.caller_pc = caller_pc;
    inst.func_pc = func_pc;
    inst.ret_pc = ret_pc;
    inst.addr_pc = addr_pc;
    inst.name = name;
    if (m_hmc_info.find(caller_pc) == m_hmc_info.end()) {
      inst.cnt = 1;
      m_hmc_info[caller_pc] = inst;
    } else {
      m_hmc_info[caller_pc].cnt++;
    }
    m_hmc_info_ext[make_pair(caller_pc, ret_pc)] = inst;

    cout << "[HMC INFO] " << caller_pc << " " << ret_pc << " " << addr_pc << " "
         << name << endl;
    linenum++;
  }
  hmc_info_file.close();
}
void hmc_function_c::hmc_fence_info_read(string file_name_base,
                                         set<uint64_t> &m_hmc_fence_info) {
  ifstream hmc_info_file;
  string hmc_info = file_name_base + ".HMCinfo";
  hmc_info_file.open(hmc_info.c_str());

  unsigned linenum = 0;
  while (hmc_info_file.good()) {
    string line;
    getline(hmc_info_file, line);
    // skip empty lines
    if (line.empty()) continue;
    // skip comment lines
    if (line[0] == '#') continue;
    // skip the first line (csv header)
    if (linenum == 0) {
      linenum++;
      continue;
    }
    uint64_t caller_pc, func_pc, ret_pc, addr_pc;
    string name;
    std::stringstream ss(line);
    ss >> caller_pc >> func_pc >> ret_pc >> addr_pc >> name;

    m_hmc_fence_info.insert(addr_pc);
    linenum++;
  }
  hmc_info_file.close();
}

void hmc_function_c::lock_info_read(string file_name_base,
                                    map<uint64_t, hmc_inst_s> &m_lock_info) {
  ifstream lock_info_file;
  string lock_info = file_name_base + ".LOCKinfo";
  lock_info_file.open(lock_info.c_str());
  // skip the first line (csv header)
  if (lock_info_file.good()) {
    string line;
    getline(lock_info_file, line);
  }
  while (lock_info_file.good()) {
    string line;
    getline(lock_info_file, line);
    if (line.empty()) continue;

    uint64_t caller_pc, func_pc, ret_pc, addr_pc;
    string name;
    std::stringstream ss(line);
    ss >> caller_pc >> func_pc >> ret_pc >> name;

    hmc_inst_s inst;
    inst.caller_pc = caller_pc;
    inst.func_pc = func_pc;
    inst.ret_pc = ret_pc;
    inst.name = name;
    m_lock_info[caller_pc] = inst;
  }
  lock_info_file.close();
}

HMC_Type hmc_function_c::generate_hmc_inst(const hmc_inst_s &inst_info,
                                           uint64_t hmc_vaddr,
                                           trace_info_cpu_s &ret_trace_info) {
  // shared inst info
  ret_trace_info.m_opcode = XED_CATEGORY_DATAXFER;
  ret_trace_info.m_num_read_regs = 1;
  ret_trace_info.m_num_dest_regs = 0;
  ret_trace_info.m_src[0] = 14;  //   "rbp" --> this will be overwritten
  ret_trace_info.m_has_immediate = false;
  ret_trace_info.m_cf_type = NOT_CF;
  ret_trace_info.m_rep_dir = false;
  ret_trace_info.m_has_st = true;
  ret_trace_info.m_num_ld = 0;
  ret_trace_info.m_mem_read_size = 0;
  ret_trace_info.m_mem_write_size = 2;
  ret_trace_info.m_is_fp = false;
  ret_trace_info.m_ld_vaddr1 = 0;
  ret_trace_info.m_ld_vaddr2 = 0;
  ret_trace_info.m_st_vaddr = hmc_vaddr;
  ret_trace_info.m_instruction_addr = inst_info.caller_pc;
  ret_trace_info.m_actually_taken = false;
  ret_trace_info.m_branch_target = 0;
  ret_trace_info.m_write_flg = false;

  HMC_Type type = hmc_type_c::HMC_String2Type(inst_info.name);
  // if (*KNOB(KNOB_ENABLE_HMC_INST_DEP) )
  //{
  // for all CAS operations: add dest reg - rflags
  /*if (type>HMC_NONE && type<=HMC_CAS_less_16B)
  {
      ret_trace_info.m_num_dest_regs = 1;
      ret_trace_info.m_dst[0] = 34; // "rflags"
      printf("pc:0x%lx m_num_dest_regs:%d \n", ret_trace_info.m_instruction_addr, ret_trace_info.m_num_dest_regs);

  }*/
  //}
  return type;
}

bool hmc_function_c::get_uops_from_traces_with_hmc_inst(void *ptr, int core_id,
                                                        uop_c *uop,
                                                        int sim_thread_id) {
  macsim_c *m_simBase = ((cpu_decoder_c *)ptr)->m_simBase;
  bool m_enable_physical_mapping =
    ((cpu_decoder_c *)ptr)->m_enable_physical_mapping;
  PageMapper *m_page_mapper = ((cpu_decoder_c *)ptr)->m_page_mapper;

  ASSERT(uop);

  trace_uop_s *trace_uop;
  int num_uop = 0;
  core_c *core = m_simBase->m_core_pointers[core_id];
  inst_info_s *info;

  // fetch ended : no uop to fetch
  if (core->m_fetch_ended[sim_thread_id]) return false;

  trace_info_cpu_s trace_info;
  HMC_Type cur_trace_hmc_type = HMC_NONE;

  bool read_success = true;
  struct thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);

  if (thread_trace_info->m_thread_init) {
    thread_trace_info->m_thread_init = false;
  }

  ///
  /// BOM (beginning of macro) : need to get a next instruction
  ///
  if (thread_trace_info->m_bom) {
    bool inst_read;  // indicate new instruction has been read from a trace file

    uint64_t inst_extra = 0;
    if (*KNOB(KNOB_COUNT_HMC_REMOVED_IN_MAX_INSTS))
      inst_extra = (m_simBase->m_ProcessorStats->core(
        core_id))[HMC_REMOVE_INST_COUNT - PER_CORE_STATS_ENUM_FIRST]
                     .getCount();

    if ((core->m_inst_fetched[sim_thread_id] + inst_extra) <
        *KNOB(KNOB_MAX_INSTS)) {
      if (!thread_trace_info->has_cached_inst) {
        // read next instruction
        read_success =
          ((cpu_decoder_c *)ptr)
            ->read_trace(core_id, thread_trace_info->m_next_trace_info,
                         sim_thread_id, &inst_read);
        if (!read_success) return false;
        thread_trace_info->m_next_hmc_type = HMC_NONE;

        // changed by Lifeng
        // looking for instructions from HMC functions
        // replace them with special HMC instructions
        map<uint64_t, hmc_inst_s> &hmc_info =
          thread_trace_info->m_process->m_hmc_info;
        map<std::pair<uint64_t, uint64_t>, hmc_inst_s> &hmc_info_ext =
          thread_trace_info->m_process->m_hmc_info_ext;
        uint64_t inst_addr = (static_cast<trace_info_cpu_s *>(
                                thread_trace_info->m_next_trace_info))
                               ->m_instruction_addr;
        if (hmc_info.find(inst_addr) != hmc_info.end() &&
            (!core->get_trace_info(sim_thread_id)->m_trace_ended)) {
          hmc_inst_s hmc_inst = hmc_info[inst_addr];

          trace_info_cpu_s hmc_trace_info;
          trace_info_cpu_s cur_trace_info;
          uint64_t hmc_vaddr = 0;  // target mem vaddr for HMC inst

          // cout<<"[HMC] find_inst:"<<inst_addr<<" core_id:"<<core_id<<endl;

          unsigned match_cnt = hmc_inst.cnt;
          if (inst_addr == hmc_inst.addr_pc) {
            hmc_vaddr = (static_cast<trace_info_cpu_s *>(
                           thread_trace_info->m_next_trace_info))
                          ->m_ld_vaddr1;
          }
          while (true) {
            read_success = ((cpu_decoder_c *)ptr)
                             ->read_trace(core_id, (&cur_trace_info),
                                          sim_thread_id, &inst_read);
            // break when reach trace end
            if (core->get_trace_info(sim_thread_id)->m_trace_ended) break;
            // get target mem addr of hmc inst
            if (cur_trace_info.m_instruction_addr == hmc_inst.addr_pc) {
              hmc_vaddr = cur_trace_info.m_ld_vaddr1;
              // cout<<"[HMC] set_vaddr:"<<hmc_vaddr<<" core_id:"<<core_id<<" vaddr_pc:"<<hmc_inst.addr_pc<<endl;
            }

            // break when trace read has an error
            if (!read_success) return false;

            if (match_cnt > 1) {
              map<std::pair<uint64_t, uint64_t>, hmc_inst_s>::iterator iter;
              iter = hmc_info_ext.find(
                make_pair(inst_addr, cur_trace_info.m_instruction_addr));
              if (iter != hmc_info_ext.end()) break;
            } else if (cur_trace_info.m_instruction_addr == hmc_inst.ret_pc)
              break;

            // cout<<"[HMC] skip_inst:"<<cur_trace_info.m_instruction_addr<<" core_id:"<<core_id<<endl;
            STAT_CORE_EVENT(core_id, HMC_REMOVE_INST_COUNT);
            STAT_EVENT(HMC_REMOVE_INST_COUNT_TOT);
          }
          ASSERT(read_success);  // should not reach here
          // replace hmc function with a generated hmc inst
          //  only when the full hmc function is found before trace end
          if (!core->get_trace_info(sim_thread_id)->m_trace_ended) {
            HMC_Type ret =
              generate_hmc_inst(hmc_inst, hmc_vaddr, hmc_trace_info);
            ASSERTM(ret != HMC_NONE, " hmc_inst: %s hmc_enum: %d\n",
                    hmc_inst.name.c_str(),
                    (unsigned)ret);  // fail if cannot find hmc inst info
            memcpy(thread_trace_info->m_next_trace_info, &hmc_trace_info,
                   sizeof(trace_info_cpu_s));
            thread_trace_info->m_next_hmc_type = ret;

            // cout<<"core-"<<core_id<<" thread-"<<sim_thread_id<<"  hmc-inst-pc: "<<inst_addr<<" vaddr:
            // "<<hmc_vaddr<<endl;

            // cache the inst@ret_pc for later fetch
            memcpy(&(thread_trace_info->cached_inst), &cur_trace_info,
                   sizeof(trace_info_cpu_s));
            thread_trace_info->has_cached_inst = true;

            trace_info_cpu_s *tmp_trace_info =
              (trace_info_cpu_s *)thread_trace_info->m_next_trace_info;

            DEBUG_CORE(
              core_id,
              "[HMC] core_id:%d cycle_count:%lld new trace info is created "
              "pc:%lx va:%lx"
              "return_type:%d thread_trace_info->m_next_hmc_type:%d \n",
              core_id, core->get_cycle_count(),
              tmp_trace_info->m_instruction_addr, tmp_trace_info->m_ld_vaddr1,
              ret, thread_trace_info->m_next_hmc_type);

            DEBUG_CORE(
              core_id,
              "core_id:%d thread_id:%d hmc_inst:%d m_next_hmc_type:%d\n",
              core_id, sim_thread_id, ret, thread_trace_info->m_next_hmc_type);
            STAT_CORE_EVENT(core_id, HMC_INST_COUNT);
            STAT_EVENT(HMC_INST_COUNT_TOT);
            HMC_EVENT_COUNT(core_id, ret);
          }
        }
      } else {
        // use cached instruction directly
        memcpy(thread_trace_info->m_next_trace_info,
               &(thread_trace_info->cached_inst), sizeof(trace_info_cpu_s));
        inst_read = true;
        read_success = true;
        thread_trace_info->has_cached_inst = false;
        thread_trace_info->m_next_hmc_type = HMC_NONE;
      }
    } else {
      inst_read = false;
      if (!core->get_trace_info(sim_thread_id)->m_trace_ended) {
        core->get_trace_info(sim_thread_id)->m_trace_ended = true;
      }
    }

    // Copy current instruction to data structure

    memcpy(&trace_info, thread_trace_info->m_prev_trace_info,
           sizeof(trace_info_cpu_s));
    cur_trace_hmc_type = thread_trace_info->m_prev_hmc_type;

    // Set next pc address
    trace_info_cpu_s *next_trace_info =
      static_cast<trace_info_cpu_s *>(thread_trace_info->m_next_trace_info);
    trace_info.m_instruction_next_addr = next_trace_info->m_instruction_addr;

    // Copy next instruction to current instruction field
    memcpy(thread_trace_info->m_prev_trace_info,
           thread_trace_info->m_next_trace_info, sizeof(trace_info_cpu_s));
    thread_trace_info->m_prev_hmc_type = thread_trace_info->m_next_hmc_type;

    DEBUG_CORE(core_id,
               "trace_read core_id:%d thread_id:%d pc:0x%llx opcode:%d "
               "inst_count:%llu va:0x%lx\n",
               core_id, sim_thread_id, (Addr)(trace_info.m_instruction_addr),
               static_cast<int>(trace_info.m_opcode),
               (Counter)(thread_trace_info->m_temp_inst_count),
               (trace_info.m_ld_vaddr1));

    ///
    /// Trace read failed
    ///
    if (!read_success) return false;

    // read a new instruction, so update stats
    if (inst_read) {
      ++core->m_inst_fetched[sim_thread_id];
      DEBUG_CORE(core_id, "core_id:%d thread_id:%d inst_num:%llu\n", core_id,
                 sim_thread_id,
                 (Counter)(thread_trace_info->m_temp_inst_count + 1));

      if (core->m_inst_fetched[sim_thread_id] > core->m_max_inst_fetched)
        core->m_max_inst_fetched = core->m_inst_fetched[sim_thread_id];
    }

    // So far we have raw instruction format, so we need to MacSim specific trace format
    info = ((cpu_decoder_c *)ptr)
             ->convert_pinuop_to_t_uop(&trace_info,
                                       thread_trace_info->m_trace_uop_array,
                                       core_id, sim_thread_id);

    // mark hmc uops
    for (unsigned i = 0; i < info->m_trace_info.m_num_uop; i++) {
      thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = cur_trace_hmc_type;
    }
    trace_uop = thread_trace_info->m_trace_uop_array[0];
    num_uop = info->m_trace_info.m_num_uop;
    ASSERT(info->m_trace_info.m_num_uop > 0);

    thread_trace_info->m_num_sending_uop = 1;
    thread_trace_info->m_eom = thread_trace_info->m_trace_uop_array[0]->m_eom;
    thread_trace_info->m_bom = false;

    uop->m_isitBOM = true;
    POWER_CORE_EVENT(core_id, POWER_INST_DECODER_R);
    POWER_CORE_EVENT(core_id, POWER_OPERAND_DECODER_R);
  }  // END EOM
  // read remaining uops from the same instruction
  else {
    trace_uop = thread_trace_info
                  ->m_trace_uop_array[thread_trace_info->m_num_sending_uop];
    info = trace_uop->m_info;
    thread_trace_info->m_eom = trace_uop->m_eom;
    info->m_trace_info.m_bom = 0;  // because of repeat instructions ....
    uop->m_isitBOM = false;
    ++thread_trace_info->m_num_sending_uop;
  }

  // set end of macro flag
  if (thread_trace_info->m_eom) {
    uop->m_isitEOM = true;  // mark for current uop
    thread_trace_info->m_bom = true;  // mark for next instruction
  } else {
    uop->m_isitEOM = false;
    thread_trace_info->m_bom = false;
  }

  if (core->get_trace_info(sim_thread_id)->m_trace_ended && uop->m_isitEOM) {
    --core->m_fetching_thread_num;
    core->m_fetch_ended[sim_thread_id] = true;
    uop->m_last_uop = true;
    DEBUG_CORE(core_id,
               "core_id:%d thread_id:%d inst_num:%lld uop_num:%lld "
               "fetched:%lld last uop\n",
               core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num,
               core->m_inst_fetched[sim_thread_id]);
  }

  ///
  /// Set up actual uop data structure
  ///
  uop->m_opcode = trace_uop->m_opcode;
  uop->m_uop_type = info->m_table_info->m_op_type;
  uop->m_cf_type = info->m_table_info->m_cf_type;
  uop->m_mem_type = info->m_table_info->m_mem_type;
  ASSERT(uop->m_mem_type >= 0 && uop->m_mem_type < NUM_MEM_TYPES);
  uop->m_bar_type = trace_uop->m_bar_type;
  uop->m_npc = trace_uop->m_npc;
  uop->m_active_mask = trace_uop->m_active_mask;

  // pass over hmc inst info
  uop->m_hmc_inst = trace_uop->m_hmc_inst;

  if (uop->m_hmc_inst != HMC_NONE) {
    DEBUG_CORE(
      core_id,
      "core_id:%d thread_id:%d inst_num:%lld uop_num:%lld pc:%llx va:%llx\n",
      core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num,
      trace_uop->m_addr, trace_uop->m_va);
    STAT_CORE_EVENT(core_id, HMC_UOP_COUNT);
  }
  if (uop->m_cf_type) {
    uop->m_taken_mask = trace_uop->m_taken_mask;
    uop->m_reconverge_addr = trace_uop->m_reconverge_addr;
    uop->m_target_addr = trace_uop->m_target;
  }

  if (uop->m_opcode == GPU_EN) {
    m_simBase->m_gpu_paused = false;
  }

  // address translation
  if (trace_uop->m_va == 0) {
    uop->m_vaddr = 0;
  } else {
    // since we can have 64-bit address space and each trace has 32-bit address,
    // using extra bits to differentiate address space of each application
    uop->m_vaddr =
      trace_uop->m_va +
      m_simBase->m_memory->base_addr(
        core_id,
        (unsigned long)UINT_MAX *
          (core->get_trace_info(sim_thread_id)->m_process->m_process_id) *
          10ul);

    // virtual-to-physical translation
    // physical page is allocated at this point for the time being
    if (m_enable_physical_mapping)
      uop->m_vaddr = m_page_mapper->translate(uop->m_vaddr);
  }

  uop->m_mem_size = trace_uop->m_mem_size;
  if (uop->m_mem_type != NOT_MEM) {
    int temp_num_req =
      (uop->m_mem_size + *KNOB(KNOB_MAX_TRANSACTION_SIZE) - 1) /
      *KNOB(KNOB_MAX_TRANSACTION_SIZE);

    ASSERTM(
      temp_num_req > 0,
      "pc:%llx vaddr:%llx opcode:%d size:%d max:%d num:%d type:%d num:%d\n",
      uop->m_pc, uop->m_vaddr, uop->m_opcode, uop->m_mem_size,
      (int)*KNOB(KNOB_MAX_TRANSACTION_SIZE), temp_num_req, uop->m_mem_type,
      trace_uop->m_info->m_trace_info.m_num_uop);
  }

  uop->m_dir = trace_uop->m_actual_taken;
  uop->m_pc = info->m_addr;
  uop->m_core_id = core_id;

  // we found first uop of an instruction, so add instruction count
  if (uop->m_isitBOM) ++thread_trace_info->m_temp_inst_count;

  uop->m_inst_num = thread_trace_info->m_temp_inst_count;
  uop->m_num_srcs = trace_uop->m_num_src_regs;
  uop->m_num_dests = trace_uop->m_num_dest_regs;

  ASSERTM(uop->m_num_dests < MAX_DST_NUM, "uop->num_dests=%d MAX_DST_NUM=%d\n",
          uop->m_num_dests, MAX_DST_NUM);

  // uop number is specific to the core
  uop->m_unique_num = core->inc_and_get_unique_uop_num();

  //    DEBUG_CORE(uop->m_core_id, "unique_num:%llu num_srcs:%d  trace_uop->num_src_regs:%d  num_dsts:%d
  //    num_seing_uop:%d "
  //               "pc:0x%llx dir:%d va:%llx hmc_type:%d\n", uop->m_unique_num, uop->m_num_srcs,
  //               trace_uop->m_num_src_regs, uop->m_num_dests, thread_trace_info->m_num_sending_uop, uop->m_pc,
  //               uop->m_dir, uop->m_vaddr, uop->m_hmc_inst);

  // filling the src_info, dest_info
  if (uop->m_num_srcs < MAX_SRCS) {
    for (int index = 0; index < uop->m_num_srcs; ++index) {
      uop->m_src_info[index] = trace_uop->m_srcs[index].m_id;
      // if (*KNOB(KNOB_HMC_ADD_DEP))
      // if (uop->m_hmc_inst)   uop->m_src_info[index]  = thread_trace_info->m_last_dest_reg;
      DEBUG_CORE(uop->m_core_id,
                 "thread_id:%d uop_num:%llu unique_num:%lld src_info[%d]:%u "
                 "last_dest_reg:%d dest_id[%d]:%u m_num_dests:%d\n",
                 uop->m_thread_id, uop->m_uop_num, uop->m_unique_num, index,
                 uop->m_src_info[index], thread_trace_info->m_last_dest_reg,
                 index, uop->m_dest_info[index], uop->m_num_dests);
    }
  } else {
    ASSERTM(uop->m_num_srcs < MAX_SRCS, "src_num:%d MAX_SRC:%d",
            uop->m_num_srcs, MAX_SRCS);
  }

  for (int index = 0; index < uop->m_num_dests; ++index) {
    uop->m_dest_info[index] = trace_uop->m_dests[index].m_id;
    thread_trace_info->m_last_dest_reg = uop->m_dest_info[index];
    DEBUG_CORE(uop->m_core_id,
               "unique_num:%lld dest_info[%d]:%u last_dest_reg:%u\n",
               uop->m_unique_num, index, uop->m_dest_info[index],
               thread_trace_info->m_last_dest_reg);
    ASSERT(trace_uop->m_dests[index].m_reg < NUM_REG_IDS);
  }

  uop->m_uop_num = (thread_trace_info->m_temp_uop_count++);
  uop->m_thread_id = sim_thread_id;
  uop->m_block_id = ((core)->get_trace_info(sim_thread_id))->m_block_id;
  uop->m_orig_block_id =
    ((core)->get_trace_info(sim_thread_id))->m_orig_block_id;
  uop->m_unique_thread_id =
    ((core)->get_trace_info(sim_thread_id))->m_unique_thread_id;
  uop->m_orig_thread_id =
    ((core)->get_trace_info(sim_thread_id))->m_orig_thread_id;

  if (uop->m_hmc_inst > HMC_NONE && uop->m_hmc_inst <= HMC_CAS_less_16B) {
    DEBUG_CORE(core_id,
               "-------core_id:%d thread_id:%d inst_num:%lld uop_num:%lld "
               "pc:%llx va:%llx\n",
               core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num,
               trace_uop->m_addr, trace_uop->m_va);
    // add-dep
    uop->m_num_dests = 1;
    uop->m_dest_info[0] = 34;
  }
  ///
  /// GPU simulation : handling uncoalesced accesses
  /// removed
  ///

  DEBUG_CORE(uop->m_core_id,
             "new uop: uop_num:%lld inst_num:%lld thread_id:%d unique_num:%lld "
             "src[0]:%d dst[0]:%d hmc_inst:%d m_num_dests:%d\n",
             uop->m_uop_num, uop->m_inst_num, uop->m_thread_id,
             uop->m_unique_num, uop->m_src_info[0], uop->m_dest_info[0],
             uop->m_hmc_inst, uop->m_num_dests);

  return read_success;
}
// lightweight hmc transaction support (or we call it complex hmc inst)
bool hmc_function_c::get_uops_from_traces_with_hmc_trans(void *ptr, int core_id,
                                                         uop_c *uop,
                                                         int sim_thread_id) {
  macsim_c *m_simBase = ((cpu_decoder_c *)ptr)->m_simBase;
  bool m_enable_physical_mapping =
    ((cpu_decoder_c *)ptr)->m_enable_physical_mapping;
  PageMapper *m_page_mapper = ((cpu_decoder_c *)ptr)->m_page_mapper;

  ASSERT(uop);

  trace_uop_s *trace_uop;
  int num_uop = 0;
  core_c *core = m_simBase->m_core_pointers[core_id];
  inst_info_s *info;

  // fetch ended : no uop to fetch
  if (core->m_fetch_ended[sim_thread_id]) return false;

  trace_info_cpu_s trace_info;
  HMC_Type cur_trace_hmc_type = HMC_NONE;
  uint64_t cur_trace_hmc_trans_id = 0;

  bool read_success = true;
  struct thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);

  if (thread_trace_info->m_thread_init) {
    thread_trace_info->m_thread_init = false;
  }

  ///
  /// BOM (beginning of macro) : need to get a next instruction
  ///
  if (thread_trace_info->m_bom) {
    bool inst_read;  // indicate new instruction has been read from a trace file

    if (core->m_inst_fetched[sim_thread_id] < *KNOB(KNOB_MAX_INSTS)) {
      // read next instruction
      read_success =
        ((cpu_decoder_c *)ptr)
          ->read_trace(core_id, thread_trace_info->m_next_trace_info,
                       sim_thread_id, &inst_read);
      if (!read_success) return false;
      thread_trace_info->m_next_hmc_type = HMC_NONE;

      // check lock function
      // loop over to skip the whole lock function
      if (*KNOB(KNOB_ENABLE_LOCK_SKIP)) {
        map<uint64_t, hmc_inst_s> &lock_info =
          thread_trace_info->m_process->m_lock_info;
        uint64_t inst_addr = (static_cast<trace_info_cpu_s *>(
                                thread_trace_info->m_next_trace_info))
                               ->m_instruction_addr;
        if (lock_info.find(inst_addr) != lock_info.end() &&
            (!core->get_trace_info(sim_thread_id)->m_trace_ended)) {
          uint64_t ret_pc = lock_info[inst_addr].ret_pc;
          while (true) {
            read_success =
              ((cpu_decoder_c *)ptr)
                ->read_trace(core_id, thread_trace_info->m_next_trace_info,
                             sim_thread_id, &inst_read);
            inst_addr = (static_cast<trace_info_cpu_s *>(
                           thread_trace_info->m_next_trace_info))
                          ->m_instruction_addr;

            // break when reach trace end
            if (core->get_trace_info(sim_thread_id)->m_trace_ended) break;

            // break when trace read has an error
            if (!read_success) return false;

            // break when find the return instruction
            if (inst_addr == ret_pc) break;
          }
        }
      }
      // changed by Lifeng
      // looking for instructions from HMC functions
      // replace them with special HMC instructions
      map<uint64_t, hmc_inst_s> &hmc_info =
        thread_trace_info->m_process->m_hmc_info;
      uint64_t inst_addr =
        (static_cast<trace_info_cpu_s *>(thread_trace_info->m_next_trace_info))
          ->m_instruction_addr;
      if (thread_trace_info->m_inside_hmc_func == false) {
        if (hmc_info.find(inst_addr) != hmc_info.end()) {
          uint64_t ret_pc = hmc_info[inst_addr].ret_pc;
          trace_info_cpu_s *next_trace = static_cast<trace_info_cpu_s *>(
            thread_trace_info->m_next_trace_info);
          while (next_trace->m_num_ld == 0 && next_trace->m_has_st == false) {
            read_success =
              ((cpu_decoder_c *)ptr)
                ->read_trace(core_id, thread_trace_info->m_next_trace_info,
                             sim_thread_id, &inst_read);
            if (core->get_trace_info(sim_thread_id)->m_trace_ended) break;
            if (read_success == false) return false;
            next_trace = static_cast<trace_info_cpu_s *>(
              thread_trace_info->m_next_trace_info);
          }
          if (!core->get_trace_info(sim_thread_id)->m_trace_ended) {
            thread_trace_info->m_next_hmc_type = HMC_TRANS_BEG;
            thread_trace_info->m_inside_hmc_func = true;
            thread_trace_info->m_next_hmc_func_ret = ret_pc;
            m_simBase->m_hmc_trans_id_gen++;
            thread_trace_info->m_next_hmc_trans_id =
              m_simBase->m_hmc_trans_id_gen;
            // thread_trace_info->m_cur_hmc_trans_cnt = 0;
          }
        }
      } else {
        if (inst_addr == thread_trace_info->m_next_hmc_func_ret) {
          thread_trace_info->m_inside_hmc_func = false;
          thread_trace_info->m_next_hmc_type = HMC_NONE;
        } else {
          thread_trace_info->m_next_hmc_type = HMC_TRANS_MID;
          trace_info_cpu_s *next_trace = static_cast<trace_info_cpu_s *>(
            thread_trace_info->m_next_trace_info);
          if (next_trace->m_cf_type == CF_RET) {
            thread_trace_info->m_inside_hmc_func = false;
            thread_trace_info->m_next_hmc_type = HMC_NONE;
          }
          while (next_trace->m_num_ld == 0 && next_trace->m_has_st == false) {
            read_success =
              ((cpu_decoder_c *)ptr)
                ->read_trace(core_id, thread_trace_info->m_next_trace_info,
                             sim_thread_id, &inst_read);
            inst_addr = (static_cast<trace_info_cpu_s *>(
                           thread_trace_info->m_next_trace_info))
                          ->m_instruction_addr;
            next_trace = static_cast<trace_info_cpu_s *>(
              thread_trace_info->m_next_trace_info);
            if (core->get_trace_info(sim_thread_id)->m_trace_ended ||
                inst_addr == thread_trace_info->m_next_hmc_func_ret ||
                next_trace->m_cf_type == CF_RET) {
              thread_trace_info->m_inside_hmc_func = false;
              thread_trace_info->m_next_hmc_type = HMC_NONE;
              break;
            }
          }
        }
      }
    } else {
      inst_read = false;
      if (!core->get_trace_info(sim_thread_id)->m_trace_ended) {
        core->get_trace_info(sim_thread_id)->m_trace_ended = true;
      }
    }

    // Copy current instruction to data structure
    memcpy(&trace_info, thread_trace_info->m_prev_trace_info,
           sizeof(trace_info_cpu_s));
    cur_trace_hmc_type = thread_trace_info->m_prev_hmc_type;
    cur_trace_hmc_trans_id = thread_trace_info->m_prev_hmc_trans_id;

    // Set next pc address
    trace_info_cpu_s *next_trace_info =
      static_cast<trace_info_cpu_s *>(thread_trace_info->m_next_trace_info);
    trace_info.m_instruction_next_addr = next_trace_info->m_instruction_addr;

    if (cur_trace_hmc_type != HMC_NONE &&
        thread_trace_info->m_next_hmc_type == HMC_NONE)
      cur_trace_hmc_type = HMC_TRANS_END;

    // Copy next instruction to current instruction field
    memcpy(thread_trace_info->m_prev_trace_info,
           thread_trace_info->m_next_trace_info, sizeof(trace_info_cpu_s));
    thread_trace_info->m_prev_hmc_type = thread_trace_info->m_next_hmc_type;
    thread_trace_info->m_prev_hmc_trans_id =
      thread_trace_info->m_next_hmc_trans_id;

    DEBUG_CORE(core_id,
               "trace_read core_id:%d thread_id:%d pc:0x%llx opcode:%d "
               "inst_count:%llu\n",
               core_id, sim_thread_id, (Addr)(trace_info.m_instruction_addr),
               static_cast<int>(trace_info.m_opcode),
               (Counter)(thread_trace_info->m_temp_inst_count));

    ///
    /// Trace read failed
    ///
    if (!read_success) return false;

    // read a new instruction, so update stats
    if (inst_read) {
      ++core->m_inst_fetched[sim_thread_id];
      DEBUG_CORE(core_id, "core_id:%d thread_id:%d inst_num:%llu\n", core_id,
                 sim_thread_id,
                 (Counter)(thread_trace_info->m_temp_inst_count + 1));

      if (core->m_inst_fetched[sim_thread_id] > core->m_max_inst_fetched)
        core->m_max_inst_fetched = core->m_inst_fetched[sim_thread_id];
    }

    // So far we have raw instruction format, so we need to MacSim specific trace format
    info = ((cpu_decoder_c *)ptr)
             ->convert_pinuop_to_t_uop(&trace_info,
                                       thread_trace_info->m_trace_uop_array,
                                       core_id, sim_thread_id);

    // mark hmc uops
    if (cur_trace_hmc_type == HMC_TRANS_BEG) {
      HMC_Type curr = HMC_TRANS_BEG;
      for (unsigned i = 0; i < info->m_trace_info.m_num_uop; i++) {
        if (thread_trace_info->m_trace_uop_array[i]->m_mem_type != NOT_MEM) {
          thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = curr;
          thread_trace_info->m_trace_uop_array[i]->m_hmc_trans_id =
            cur_trace_hmc_trans_id;

          DEBUG_HMC_CORE(core_id, "[HMC] %u\t id:%d\n", (unsigned)HMC_TRANS_MID,
                         (int)cur_trace_hmc_trans_id);
          // if (*KNOB(KNOB_DEBUG_HMC))
          // cout<<"[HMC] "<<curr<<"\t id: "<<cur_trace_hmc_trans_id<<endl;
          if (curr == HMC_TRANS_BEG) curr = HMC_TRANS_MID;
        } else
          thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = HMC_NONE;
      }
      for (unsigned i = 0; i < info->m_trace_info.m_num_uop; i++) {
        thread_trace_info->m_trace_uop_array[i]->m_num_src_regs = 0;
        thread_trace_info->m_trace_uop_array[i]->m_num_dest_regs = 0;
      }
    } else if (cur_trace_hmc_type == HMC_TRANS_MID) {
      for (unsigned i = 0; i < info->m_trace_info.m_num_uop; i++) {
        if (thread_trace_info->m_trace_uop_array[i]->m_mem_type != NOT_MEM) {
          thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = HMC_TRANS_MID;
          thread_trace_info->m_trace_uop_array[i]->m_hmc_trans_id =
            cur_trace_hmc_trans_id;
          DEBUG_HMC_CORE(core_id, "[HMC] %u\t id:%d\n", (unsigned)HMC_TRANS_MID,
                         (int)cur_trace_hmc_trans_id);
          // if (*KNOB(KNOB_ENABLE_HMC_DEBUG)) cout<<"[HMC] "<<(unsigned)HMC_TRANS_MID<<"\t id:
          // "<<cur_trace_hmc_trans_id<<endl;
        } else
          thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = HMC_NONE;
        thread_trace_info->m_trace_uop_array[i]->m_num_src_regs = 0;
        thread_trace_info->m_trace_uop_array[i]->m_num_dest_regs = 0;
      }
    } else if (cur_trace_hmc_type == HMC_TRANS_END) {
      HMC_Type curr = HMC_TRANS_END;
      for (unsigned s = 0; s < info->m_trace_info.m_num_uop; s++) {
        unsigned i = info->m_trace_info.m_num_uop - s - 1;
        if (thread_trace_info->m_trace_uop_array[i]->m_mem_type != NOT_MEM) {
          thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = curr;
          thread_trace_info->m_trace_uop_array[i]->m_hmc_trans_id =
            cur_trace_hmc_trans_id;
          DEBUG_HMC_CORE(core_id, "[HMC] %u\t id:%d\n", (unsigned)HMC_TRANS_MID,
                         (int)cur_trace_hmc_trans_id);
          // if (*KNOB(KNOB_ENABLE_HMC_DEBUG)) cout<<"[HMC] "<<curr<<"\t id: "<<cur_trace_hmc_trans_id<<endl;
          if (curr == HMC_TRANS_END) curr = HMC_TRANS_MID;
        } else
          thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = HMC_NONE;
      }
      for (unsigned i = 0; i < info->m_trace_info.m_num_uop; i++) {
        thread_trace_info->m_trace_uop_array[i]->m_num_src_regs = 0;
        thread_trace_info->m_trace_uop_array[i]->m_num_dest_regs = 0;
      }
    } else {
      for (unsigned i = 0; i < info->m_trace_info.m_num_uop; i++) {
        thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = HMC_NONE;
      }
    }
    trace_uop = thread_trace_info->m_trace_uop_array[0];
    num_uop = info->m_trace_info.m_num_uop;
    ASSERT(info->m_trace_info.m_num_uop > 0);

    thread_trace_info->m_num_sending_uop = 1;
    thread_trace_info->m_eom = thread_trace_info->m_trace_uop_array[0]->m_eom;
    thread_trace_info->m_bom = false;

    uop->m_isitBOM = true;
    POWER_CORE_EVENT(core_id, POWER_INST_DECODER_R);
    POWER_CORE_EVENT(core_id, POWER_OPERAND_DECODER_R);
  }  // END EOM
  // read remaining uops from the same instruction
  else {
    trace_uop = thread_trace_info
                  ->m_trace_uop_array[thread_trace_info->m_num_sending_uop];
    info = trace_uop->m_info;
    thread_trace_info->m_eom = trace_uop->m_eom;
    info->m_trace_info.m_bom = 0;  // because of repeat instructions ....
    uop->m_isitBOM = false;
    ++thread_trace_info->m_num_sending_uop;
  }

  // set end of macro flag
  if (thread_trace_info->m_eom) {
    uop->m_isitEOM = true;  // mark for current uop
    thread_trace_info->m_bom = true;  // mark for next instruction
  } else {
    uop->m_isitEOM = false;
    thread_trace_info->m_bom = false;
  }

  if (core->get_trace_info(sim_thread_id)->m_trace_ended && uop->m_isitEOM) {
    --core->m_fetching_thread_num;
    core->m_fetch_ended[sim_thread_id] = true;
    uop->m_last_uop = true;
    DEBUG_CORE(core_id,
               "core_id:%d thread_id:%d inst_num:%lld uop_num:%lld "
               "fetched:%lld last uop\n",
               core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num,
               core->m_inst_fetched[sim_thread_id]);
  }

  ///
  /// Set up actual uop data structure
  ///
  uop->m_opcode = trace_uop->m_opcode;
  uop->m_uop_type = info->m_table_info->m_op_type;
  uop->m_cf_type = info->m_table_info->m_cf_type;
  uop->m_mem_type = info->m_table_info->m_mem_type;
  ASSERT(uop->m_mem_type >= 0 && uop->m_mem_type < NUM_MEM_TYPES);
  uop->m_bar_type = trace_uop->m_bar_type;
  uop->m_npc = trace_uop->m_npc;
  uop->m_active_mask = trace_uop->m_active_mask;

  // pass over hmc inst info
  uop->m_hmc_inst = trace_uop->m_hmc_inst;
  uop->m_hmc_trans_id = trace_uop->m_hmc_trans_id;
  if (uop->m_hmc_inst != HMC_NONE) {
    DEBUG_HMC_CORE(core_id, "[HMC] m_hmc_inst:%u id:%u mem_type:%u\n",
                   (unsigned)uop->m_hmc_inst, (unsigned)uop->m_hmc_trans_id,
                   (unsigned)uop->m_mem_type);

    //  if (*KNOB(KNOB_ENABLE_HMC_DEBUG))
    // cout<<"<HMC> "<<(unsigned)uop->m_hmc_inst<<"\t id: "<<uop->m_hmc_trans_id<<" memtype:
    // "<<(unsigned)uop->m_mem_type<<endl;
    STAT_CORE_EVENT(core_id, HMC_UOP_COUNT);
  }
  if (uop->m_cf_type) {
    uop->m_taken_mask = trace_uop->m_taken_mask;
    uop->m_reconverge_addr = trace_uop->m_reconverge_addr;
    uop->m_target_addr = trace_uop->m_target;
  }

  if (uop->m_opcode == GPU_EN) {
    m_simBase->m_gpu_paused = false;
  }

  // address translation
  if (trace_uop->m_va == 0) {
    uop->m_vaddr = 0;
  } else {
    // since we can have 64-bit address space and each trace has 32-bit address,
    // using extra bits to differentiate address space of each application
    uop->m_vaddr =
      trace_uop->m_va +
      m_simBase->m_memory->base_addr(
        core_id,
        (unsigned long)UINT_MAX *
          (core->get_trace_info(sim_thread_id)->m_process->m_process_id) *
          10ul);

    // virtual-to-physical translation
    // physical page is allocated at this point for the time being
    if (m_enable_physical_mapping)
      uop->m_vaddr = m_page_mapper->translate(uop->m_vaddr);
  }

  uop->m_mem_size = trace_uop->m_mem_size;
  if (uop->m_mem_type != NOT_MEM) {
    int temp_num_req =
      (uop->m_mem_size + *KNOB(KNOB_MAX_TRANSACTION_SIZE) - 1) /
      *KNOB(KNOB_MAX_TRANSACTION_SIZE);

    ASSERTM(
      temp_num_req > 0,
      "pc:%llx vaddr:%llx opcode:%d size:%d max:%d num:%d type:%d num:%d\n",
      uop->m_pc, uop->m_vaddr, uop->m_opcode, uop->m_mem_size,
      (int)*KNOB(KNOB_MAX_TRANSACTION_SIZE), temp_num_req, uop->m_mem_type,
      trace_uop->m_info->m_trace_info.m_num_uop);
  }

  uop->m_dir = trace_uop->m_actual_taken;
  uop->m_pc = info->m_addr;
  uop->m_core_id = core_id;

  // we found first uop of an instruction, so add instruction count
  if (uop->m_isitBOM) ++thread_trace_info->m_temp_inst_count;

  uop->m_inst_num = thread_trace_info->m_temp_inst_count;
  uop->m_num_srcs = trace_uop->m_num_src_regs;
  uop->m_num_dests = trace_uop->m_num_dest_regs;

  ASSERTM(uop->m_num_dests < MAX_DST_NUM, "uop->num_dests=%d MAX_DST_NUM=%d\n",
          uop->m_num_dests, MAX_DST_NUM);

  // uop number is specific to the core
  uop->m_unique_num = core->inc_and_get_unique_uop_num();

  DEBUG_CORE(uop->m_core_id,
             "uop_num:%llu num_srcs:%d  trace_uop->num_src_regs:%d  "
             "num_dsts:%d num_seing_uop:%d "
             "pc:0x%llx dir:%d \n",
             uop->m_uop_num, uop->m_num_srcs, trace_uop->m_num_src_regs,
             uop->m_num_dests, thread_trace_info->m_num_sending_uop, uop->m_pc,
             uop->m_dir);

  // filling the src_info, dest_info
  if (uop->m_num_srcs < MAX_SRCS) {
    for (int index = 0; index < uop->m_num_srcs; ++index) {
      uop->m_src_info[index] = trace_uop->m_srcs[index].m_id;
      // DEBUG("uop_num:%lld src_info[%d]:%d\n", uop->uop_num, index, uop->src_info[index]);
    }
  } else {
    ASSERTM(uop->m_num_srcs < MAX_SRCS, "src_num:%d MAX_SRC:%d",
            uop->m_num_srcs, MAX_SRCS);
  }

  for (int index = 0; index < uop->m_num_dests; ++index) {
    uop->m_dest_info[index] = trace_uop->m_dests[index].m_id;
    ASSERT(trace_uop->m_dests[index].m_reg < NUM_REG_IDS);
  }

  uop->m_uop_num = (thread_trace_info->m_temp_uop_count++);
  uop->m_thread_id = sim_thread_id;
  uop->m_block_id = ((core)->get_trace_info(sim_thread_id))->m_block_id;
  uop->m_orig_block_id =
    ((core)->get_trace_info(sim_thread_id))->m_orig_block_id;
  uop->m_unique_thread_id =
    ((core)->get_trace_info(sim_thread_id))->m_unique_thread_id;
  uop->m_orig_thread_id =
    ((core)->get_trace_info(sim_thread_id))->m_orig_thread_id;

  ///
  /// GPU simulation : handling uncoalesced accesses
  /// removed
  ///

  DEBUG_CORE(
    uop->m_core_id,
    "new uop: uop_num:%lld inst_num:%lld thread_id:%d unique_num:%lld \n",
    uop->m_uop_num, uop->m_inst_num, uop->m_thread_id, uop->m_unique_num);

  return read_success;
}
