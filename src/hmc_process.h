#ifndef HMC_PROCESS_H
#define HMC_PROCESS_H
#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <map>

#include "hmc_types.h"
#include "trace_read.h"

typedef struct hmc_inst_s
{
    uint64_t caller_pc;
    uint64_t func_pc;
    uint64_t ret_pc;
    uint64_t addr_pc;
    string name;
} hmc_inst_s;


class hmc_function_c
{
public:

    // get HMC instruction information
    static void hmc_info_read(string file_name_base,
                              map<uint64_t, hmc_inst_s> & m_hmc_info);

    // generate hmc inst
    static HMC_Type generate_hmc_inst(const hmc_inst_s & inst_info,
                                      uint64_t hmc_vaddr,
                                      trace_info_cpu_s & ret_trace_info);
    
    // fetch uops from traces and replace hmc functions 
    //  with generated hmc inst.
    static bool get_uops_from_traces_with_hmc_inst(
        void * ptr,
        int core_id,
        uop_c *uop,
        int sim_thread_id);


};


#endif

