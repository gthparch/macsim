#include "hmc_process.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <map>

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

#define DEBUG(args...)   _DEBUG(*KNOB(KNOB_DEBUG_TRACE_READ), ## args)
#define DEBUG_CORE(m_core_id, args...)       \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) {     \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ, ## args); \
  }





// get HMC instruction information
void hmc_function_c::hmc_info_read(string file_name_base,
                                   map<uint64_t, hmc_inst_s> & m_hmc_info)
{
    ifstream hmc_info_file;
    string hmc_info = file_name_base + ".HMCinfo";
    hmc_info_file.open(hmc_info.c_str());
    // skip the first line (csv header)
    if (hmc_info_file.good())
    {
        string line;
        getline(hmc_info_file, line);
    }
    while (hmc_info_file.good())
    {
        string line;
        getline(hmc_info_file, line);
        if (line.empty()) continue;

        uint64_t caller_pc,func_pc,ret_pc,addr_pc;
        string name;
        std::stringstream ss(line);
        ss >> caller_pc >> func_pc >> ret_pc >> addr_pc >> name;

        hmc_inst_s inst;
        inst.caller_pc = caller_pc;
        inst.func_pc = func_pc;
        inst.ret_pc = ret_pc;
        inst.addr_pc = addr_pc;
        inst.name = name;
        m_hmc_info[caller_pc] = inst;
    }
    hmc_info_file.close();
}

HMC_Type hmc_function_c::generate_hmc_inst(const hmc_inst_s & inst_info,
        uint64_t hmc_vaddr,
        trace_info_cpu_s & ret_trace_info)
{
    // shared inst info
    ret_trace_info.m_opcode = XED_CATEGORY_DATAXFER;
    ret_trace_info.m_num_read_regs = 1;
    ret_trace_info.m_num_dest_regs = 0;
    ret_trace_info.m_src[0] = 14; // "rbp"
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

    if (inst_info.name == "HMC_CAS_equal_16B")
    {
        return HMC_CAS_equal_16B;
    }
    else if (inst_info.name == "HMC_CAS_zero_16B")
    {
        return HMC_CAS_zero_16B;
    }
    else if (inst_info.name == "HMC_CAS_greater_16B")
    {
        return HMC_CAS_greater_16B;
    }
    else if (inst_info.name == "HMC_CAS_less_16B")
    {
        return HMC_CAS_less_16B;
    }
    else if (inst_info.name == "HMC_ADD_16B")
    {
        return HMC_ADD_16B;
    }
    else
    {
        return HMC_NONE;
    }
    return HMC_NONE;
}

bool hmc_function_c::get_uops_from_traces_with_hmc_inst(
    void * ptr,
    int core_id,
    uop_c *uop,
    int sim_thread_id)
{
    macsim_c * m_simBase = ((cpu_decoder_c*)ptr)->m_simBase;
    bool m_enable_physical_mapping = ((cpu_decoder_c*)ptr)->m_enable_physical_mapping;
    PageMapper * m_page_mapper = ((cpu_decoder_c*)ptr)->m_page_mapper;

    ASSERT(uop);

    trace_uop_s *trace_uop;
    int num_uop  = 0;
    core_c* core = m_simBase->m_core_pointers[core_id];
    inst_info_s *info;

    // fetch ended : no uop to fetch
    if (core->m_fetch_ended[sim_thread_id])
        return false;

    trace_info_cpu_s trace_info;
    HMC_Type cur_trace_hmc_type = HMC_NONE;

    bool read_success = true;
    struct thread_s* thread_trace_info = core->get_trace_info(sim_thread_id);

    if (thread_trace_info->m_thread_init)
    {
        thread_trace_info->m_thread_init = false;
    }


    ///
    /// BOM (beginning of macro) : need to get a next instruction
    ///
    if (thread_trace_info->m_bom)
    {
        bool inst_read; // indicate new instruction has been read from a trace file

        if (core->m_inst_fetched[sim_thread_id] < *KNOB(KNOB_MAX_INSTS))
        {
            if (!thread_trace_info->has_cached_inst)
            {
                // read next instruction
                read_success = ((cpu_decoder_c*)ptr)->read_trace(core_id, thread_trace_info->m_next_trace_info,
                               sim_thread_id, &inst_read);
                thread_trace_info->m_next_hmc_type = HMC_NONE;

                // changed by Lifeng
                // looking for instructions from HMC functions
                // replace them with special HMC instructions
                map<uint64_t, hmc_inst_s> & hmc_info = thread_trace_info->m_process->m_hmc_info;
                uint64_t inst_addr = (static_cast<trace_info_cpu_s*>
                                      (thread_trace_info->m_next_trace_info))->m_instruction_addr;
                if (hmc_info.find(inst_addr) != hmc_info.end())
                {

                    hmc_inst_s hmc_inst = hmc_info[inst_addr];

                    trace_info_cpu_s hmc_trace_info;
                    trace_info_cpu_s cur_trace_info;
                    uint64_t hmc_vaddr = 0; // target mem vaddr for HMC inst
                    while (true)
                    {
                        read_success = ((cpu_decoder_c*)ptr)->read_trace(core_id, (&cur_trace_info),
                                       sim_thread_id, &inst_read);
                        // get target mem addr of hmc inst
                        if (cur_trace_info.m_instruction_addr == hmc_inst.addr_pc)
                            hmc_vaddr = cur_trace_info.m_ld_vaddr1;

                        if (cur_trace_info.m_instruction_addr == hmc_inst.ret_pc
                                || read_success == false)
                            break;
                    }
                    ASSERT(read_success); // should not reach trace end before hmc func ret

                    HMC_Type ret = generate_hmc_inst(hmc_inst,hmc_vaddr,hmc_trace_info);
                    ASSERT(ret != HMC_NONE); // fail if cannot find hmc inst info
                    memcpy(thread_trace_info->m_next_trace_info, &hmc_trace_info, sizeof(trace_info_cpu_s));
                    thread_trace_info->m_next_hmc_type = ret;

                    // cache the inst@ret_pc for later fetch
                    memcpy(&(thread_trace_info->cached_inst), &cur_trace_info, sizeof(trace_info_cpu_s));
                    thread_trace_info->has_cached_inst = true;

                    STAT_CORE_EVENT(core_id, HMC_INST_COUNT);
                }
            }
            else
            {
                // use cached instruction directly
                memcpy(thread_trace_info->m_next_trace_info,
                       &(thread_trace_info->cached_inst),
                       sizeof(trace_info_cpu_s));
                inst_read = true;
                read_success = true;
                thread_trace_info->has_cached_inst = false;
                thread_trace_info->m_next_hmc_type = HMC_NONE;
            }
        }
        else
        {
            inst_read = false;
            if (!core->get_trace_info(sim_thread_id)->m_trace_ended)
            {
                core->get_trace_info(sim_thread_id)->m_trace_ended = true;
            }
        }


        // Copy current instruction to data structure
        memcpy(&trace_info, thread_trace_info->m_prev_trace_info, sizeof(trace_info_cpu_s));
        cur_trace_hmc_type = thread_trace_info->m_prev_hmc_type;

        // Set next pc address
        trace_info_cpu_s *next_trace_info = static_cast<trace_info_cpu_s *>(thread_trace_info->m_next_trace_info);
        trace_info.m_instruction_next_addr = next_trace_info->m_instruction_addr;

        // Copy next instruction to current instruction field
        memcpy(thread_trace_info->m_prev_trace_info, thread_trace_info->m_next_trace_info,
               sizeof(trace_info_cpu_s));
        thread_trace_info->m_prev_hmc_type = thread_trace_info->m_next_hmc_type;

        DEBUG_CORE(core_id, "trace_read core_id:%d thread_id:%d pc:0x%llx opcode:%d inst_count:%llu\n", core_id, sim_thread_id,
                   (Addr)(trace_info.m_instruction_addr), static_cast<int>(trace_info.m_opcode), (Counter)(thread_trace_info->m_temp_inst_count));

        ///
        /// Trace read failed
        ///
        if (!read_success)
            return false;


        // read a new instruction, so update stats
        if (inst_read)
        {
            ++core->m_inst_fetched[sim_thread_id];
            DEBUG_CORE(core_id, "core_id:%d thread_id:%d inst_num:%llu\n", core_id, sim_thread_id,
                       (Counter)(thread_trace_info->m_temp_inst_count + 1));

            if (core->m_inst_fetched[sim_thread_id] > core->m_max_inst_fetched)
                core->m_max_inst_fetched = core->m_inst_fetched[sim_thread_id];
        }


        // So far we have raw instruction format, so we need to MacSim specific trace format
        info = ((cpu_decoder_c*)ptr)->convert_pinuop_to_t_uop(&trace_info, thread_trace_info->m_trace_uop_array,
                core_id, sim_thread_id);

        // mark hmc uops
        for (unsigned i = 0; i < info->m_trace_info.m_num_uop; i++)
        {
            thread_trace_info->m_trace_uop_array[i]->m_hmc_inst = cur_trace_hmc_type;
        }

        trace_uop = thread_trace_info->m_trace_uop_array[0];
        num_uop   = info->m_trace_info.m_num_uop;
        ASSERT(info->m_trace_info.m_num_uop > 0);

        thread_trace_info->m_num_sending_uop = 1;
        thread_trace_info->m_eom             = thread_trace_info->m_trace_uop_array[0]->m_eom;
        thread_trace_info->m_bom             = false;

        uop->m_isitBOM = true;
        POWER_CORE_EVENT(core_id, POWER_INST_DECODER_R);
        POWER_CORE_EVENT(core_id, POWER_OPERAND_DECODER_R);
    } // END EOM
    // read remaining uops from the same instruction
    else
    {
        trace_uop                =
            thread_trace_info->m_trace_uop_array[thread_trace_info->m_num_sending_uop];
        info                     = trace_uop->m_info;
        thread_trace_info->m_eom = trace_uop->m_eom;
        info->m_trace_info.m_bom = 0; // because of repeat instructions ....
        uop->m_isitBOM           = false;
        ++thread_trace_info->m_num_sending_uop;
    }


    // set end of macro flag
    if (thread_trace_info->m_eom)
    {
        uop->m_isitEOM           = true; // mark for current uop
        thread_trace_info->m_bom = true; // mark for next instruction
    }
    else
    {
        uop->m_isitEOM           = false;
        thread_trace_info->m_bom = false;
    }


    if (core->get_trace_info(sim_thread_id)->m_trace_ended && uop->m_isitEOM)
    {
        --core->m_fetching_thread_num;
        core->m_fetch_ended[sim_thread_id] = true;
        uop->m_last_uop                    = true;
        DEBUG_CORE(core_id, "core_id:%d thread_id:%d inst_num:%lld uop_num:%lld fetched:%lld last uop\n",
                   core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num, core->m_inst_fetched[sim_thread_id]);
    }


    ///
    /// Set up actual uop data structure
    ///
    uop->m_opcode      = trace_uop->m_opcode;
    uop->m_uop_type    = info->m_table_info->m_op_type;
    uop->m_cf_type     = info->m_table_info->m_cf_type;
    uop->m_mem_type    = info->m_table_info->m_mem_type;
    ASSERT(uop->m_mem_type >= 0 && uop->m_mem_type < NUM_MEM_TYPES);
    uop->m_bar_type    = trace_uop->m_bar_type;
    uop->m_npc         = trace_uop->m_npc;
    uop->m_active_mask = trace_uop->m_active_mask;

    // pass over hmc inst info
    uop->m_hmc_inst    = trace_uop->m_hmc_inst;
    if (uop->m_hmc_inst != HMC_NONE)
    {
        STAT_CORE_EVENT(core_id, HMC_UOP_COUNT);
    }
    if (uop->m_cf_type)
    {
        uop->m_taken_mask      = trace_uop->m_taken_mask;
        uop->m_reconverge_addr = trace_uop->m_reconverge_addr;
        uop->m_target_addr     = trace_uop->m_target;
    }

    if (uop->m_opcode == GPU_EN)
    {
        m_simBase->m_gpu_paused = false;
    }

    // address translation
    if (trace_uop->m_va == 0)
    {
        uop->m_vaddr = 0;
    }
    else
    {
        // since we can have 64-bit address space and each trace has 32-bit address,
        // using extra bits to differentiate address space of each application
        uop->m_vaddr = trace_uop->m_va + m_simBase->m_memory->base_addr(core_id,
                       (unsigned long)UINT_MAX *
                       (core->get_trace_info(sim_thread_id)->m_process->m_process_id) * 10ul);

        // virtual-to-physical translation
        // physical page is allocated at this point for the time being
        if (m_enable_physical_mapping)
            uop->m_vaddr = m_page_mapper->translate(uop->m_vaddr);
    }


    uop->m_mem_size = trace_uop->m_mem_size;
    if (uop->m_mem_type != NOT_MEM)
    {
        int temp_num_req = (uop->m_mem_size + *KNOB(KNOB_MAX_TRANSACTION_SIZE) - 1) /
                           *KNOB(KNOB_MAX_TRANSACTION_SIZE);

        ASSERTM(temp_num_req > 0, "pc:%llx vaddr:%llx opcode:%d size:%d max:%d num:%d type:%d num:%d\n",
                uop->m_pc, uop->m_vaddr, uop->m_opcode, uop->m_mem_size,
                (int)*KNOB(KNOB_MAX_TRANSACTION_SIZE), temp_num_req, uop->m_mem_type,
                trace_uop->m_info->m_trace_info.m_num_uop);
    }

    uop->m_dir     = trace_uop->m_actual_taken;
    uop->m_pc      = info->m_addr;
    uop->m_core_id = core_id;


    // we found first uop of an instruction, so add instruction count
    if (uop->m_isitBOM)
        ++thread_trace_info->m_temp_inst_count;

    uop->m_inst_num  = thread_trace_info->m_temp_inst_count;
    uop->m_num_srcs  = trace_uop->m_num_src_regs;
    uop->m_num_dests = trace_uop->m_num_dest_regs;

    ASSERTM(uop->m_num_dests < MAX_DST_NUM, "uop->num_dests=%d MAX_DST_NUM=%d\n",
            uop->m_num_dests, MAX_DST_NUM);


    // uop number is specific to the core
    uop->m_unique_num = core->inc_and_get_unique_uop_num();

    DEBUG_CORE(uop->m_core_id, "uop_num:%llu num_srcs:%d  trace_uop->num_src_regs:%d  num_dsts:%d num_seing_uop:%d "
               "pc:0x%llx dir:%d \n", uop->m_uop_num, uop->m_num_srcs, trace_uop->m_num_src_regs, uop->m_num_dests,
               thread_trace_info->m_num_sending_uop, uop->m_pc, uop->m_dir);

    // filling the src_info, dest_info
    if (uop->m_num_srcs < MAX_SRCS)
    {
        for (int index=0; index < uop->m_num_srcs; ++index)
        {
            uop->m_src_info[index] = trace_uop->m_srcs[index].m_id;
            //DEBUG("uop_num:%lld src_info[%d]:%d\n", uop->uop_num, index, uop->src_info[index]);
        }
    }
    else
    {
        ASSERTM(uop->m_num_srcs < MAX_SRCS, "src_num:%d MAX_SRC:%d", uop->m_num_srcs, MAX_SRCS);
    }



    for (int index = 0; index < uop->m_num_dests; ++index)
    {
        uop->m_dest_info[index] = trace_uop->m_dests[index].m_id;
        ASSERT(trace_uop->m_dests[index].m_reg < NUM_REG_IDS);
    }

    uop->m_uop_num          = (thread_trace_info->m_temp_uop_count++);
    uop->m_thread_id        = sim_thread_id;
    uop->m_block_id         = ((core)->get_trace_info(sim_thread_id))->m_block_id;
    uop->m_orig_block_id    = ((core)->get_trace_info(sim_thread_id))->m_orig_block_id;
    uop->m_unique_thread_id = ((core)->get_trace_info(sim_thread_id))->m_unique_thread_id;
    uop->m_orig_thread_id   = ((core)->get_trace_info(sim_thread_id))->m_orig_thread_id;


    ///
    /// GPU simulation : handling uncoalesced accesses
    /// removed
    ///

    DEBUG_CORE(uop->m_core_id, "new uop: uop_num:%lld inst_num:%lld thread_id:%d unique_num:%lld \n",
               uop->m_uop_num, uop->m_inst_num, uop->m_thread_id, uop->m_unique_num);

    return read_success;
}





