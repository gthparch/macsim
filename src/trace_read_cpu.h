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
 * File         : trace_read_cpu.h 
 * Author       : HPArch Research Group
 * Date         : 
 * SVN          : $Id: dram.h 912 2009-11-20 19:09:21Z kacear $
 * Description  : Trace handling class
 *********************************************************************************************/


#ifndef TRACE_READ_CPU_H_INCLUDED
#define TRACE_READ_CPU_H_INCLUDED


#include "uop.h"
#include "inst_info.h"
#include "trace_read.h"
#include "page_mapping.h"

#include "process_manager.h"
#include "hmc_process.h"

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Trace reader class
///
/// This class handles all trace related operations. Read instructions from the file,
/// decode, split to micro ops, uop setups ...
///////////////////////////////////////////////////////////////////////////////////////////////
class cpu_decoder_c : public trace_read_c
{
  friend class hmc_function_c;  
  public:
    /**
     * Constructor
     */
    cpu_decoder_c(macsim_c* simBase, ofstream* dprint_output);

    /**
     * Destructor
     */
    ~cpu_decoder_c();

    /**
     * Get an uop from trace
     * Called by frontend.cc
     * @param core_id - core id
     * @param uop - uop object to hold instruction information
     * @param sim_thread_id thread id
     */
    bool get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id);

    virtual inst_info_s* get_inst_info(thread_s *thread_trace_info, int core_id, int sim_thread_id);

    /**
     * GPU simulation : Read trace ahead to read synchronization information
     * @param trace_info - trace information
     * @see process_manager_c::sim_thread_schedule
     */
    void pre_read_trace(thread_s* trace_info);

    static const char *g_tr_reg_names[MAX_TR_REG]; /**< register name string */
    static const char* g_tr_opcode_names[MAX_TR_OPCODE_NAME]; /**< opcode name string */
    static const char* g_tr_cf_names[10]; /**< cf type string */
    static const char *g_optype_names[37]; /**< opcode type string */
    static const char *g_mem_type_names[20]; /**< memeory request type string */

  private:
      /**
       * Initialize the mapping between trace opcode and uop type
       */
    virtual void init_pin_convert();

    /**
     * Function to decode an instruction from the trace file into a sequence of uops
     * @param pi - raw trace format
     * @param trace_uop - micro uops storage for this instruction
     * @param core_id - core id
     * @param sim_thread_id - thread id  
     */
    inst_info_s* convert_pinuop_to_t_uop(void *pi, trace_uop_s **trace_uop, 
        int core_id, int sim_thread_id);

    /**
     * From statis instruction, add dynamic information such as load address, branch target, ...
     * @param info - instruction information from the hash table
     * @param pi - raw trace information
     * @param trace_uop - MacSim uop type
     * @param rep_offset - repetition offet
     * @param core_id - core id
     */
    void convert_dyn_uop(inst_info_s *info, void *pi, trace_uop_s *trace_uop, 
        Addr rep_offset, int core_id);

    /**
     * Dump out instruction information to the file. At most 50000 instructions will be printed
     * @param t_info - trace information
     * @param core_id - core id
     * @param thread_id - thread id
     */
    virtual void dprint_inst(void *t_info, int core_id, int thread_id);

    /**
     * After peeking trace, in case of failture, we need to rewind trace file.
     * @param core_id - core id
     * @param sim_thread_id - thread id
     * @param num_inst - number of instructions to rewind
     * @see peek_trace
     */
    bool ungetch_trace(int core_id, int sim_thread_id, int num_inst);

    /**
     * In case of GPU simulation, from our design, each uncoalcesd accesses will be one 
     * trace instruction. To make these accesses in one instruction, we need to read trace
     * file ahead.
     * @param core_id - core id
     * @param trace_info - trace information to store an instruction
     * @param sim_thread_id - thread id
     * @param inst_read - indicate instruction read successful
     * @see get_uops_from_traces
     */
    bool peek_trace(int core_id, void *trace_info, int sim_thread_id, bool *inst_read);


    //changed by Lifeng
    //HMC_Type generate_hmc_inst(const hmc_inst_s & inst_info, uint64_t hmc_vaddr, trace_info_cpu_s & ret_trace_info);
  private:
    // page mapping support
    bool m_enable_physical_mapping;     //!< use physical mapping 
    PageMapper* m_page_mapper;          //!< page mapper
};

#endif // TRACE_READ_CPU_H_INCLUDED
