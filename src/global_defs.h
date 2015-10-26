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
* File         : global_defs.h
* Author       : Hyesoon Kim
* Date         : 12/18/2007
* CVS          : $Id: global_defs.h 890 2009-11-09 18:36:00Z nageshbl $:
* Description  :  Global defines that are intended to be included in every source file.
**********************************************************************************************/

#ifndef GLOBAL_DEFS_INCLUDED
#define GLOBAL_DEFS_INCLUDED


#include <vector>
#include <map>
#include <limits.h>
#include <queue>
#include <string>


using namespace std;


///////////////////////////////////////////////////////////////////////////////////////////////
// Class forward declarations

class all_knobs_c;
class all_stats_c;
class allocate_c;
class bp_data_c;
class bp_dir_base_c;
class bp_recovery_info_c;
class cache_c;
class cache_cohe_c;
class context_pool_c;
class core_c;
class dcu_c;
class exec_c;
class frontend_c;
class mem_factory_c;
class fetch_factory_c;
class pref_factory_c;
class dram_factory_c;
class bp_factory_c;
class pref_base_c;
class hwp_common_c;
class map_c;
class memory_c;
class macsim_c;
class port_c;
class retire_c;
class recovery_info_c;
class rob_c;
class network_c;
class schedule_c;
class tlb_c;
class uop_c;
class smc_rob_c;
class smc_allocate_c;
class readonly_cache_c;
class sw_managed_cache_c;
class dram_c;
class dram_ctrl_c;
class icache_data_c;
class frontend_s;
class thread_s;
class throttler_c;
class fbstream_c;
class bug_detector_c;
class queue_c;
class multi_key_map_c;
class inst_info_s;
class thread_queue_c;
class thread_block_queue_c;
class process_manager_c;
class extra_stat_c;
class pref_info_c;
class pc_info_c;
class redundant_filter_c;
class section_info_s;
class mem_map_entry_c;
class heartbeat_s;
class pref_sw_throttle_c;
class noc_c;
class ManifoldProcessor;
class trace_read_c;
class trace_reader_wrapper_c;
class KnobsContainer;
class ProcessorStatistics;
class CoreStatistics;
class cache_partition_framework_c;
class dyfr_c;

template <class T> class pqueue_c;
template <typename T> class hash_c;
template <class T> class pool_c;


///////////////////////////////////////////////////////////////////////////////////////////////
// Struct declarations


struct thread_trace_info_node_s;

typedef struct mem_req_s mem_req_s;
typedef struct Hash_Table_struct Hash_Table;
typedef struct l2_data_s l2_data_s;
typedef struct pref_req_info_s pref_req_info_s;
typedef struct trace_uop_s trace_uop_s;
typedef struct TraceBuffer_ TraceBuffer;
typedef struct mt_scheduler_s mt_scheduler_s;
typedef struct reconv_data_s recove_data_s;
typedef struct pref_inst_ pref_inst_s;
typedef struct FCDPData_ FCDPData;
typedef struct block_schedule_info_s block_schedule_info_s;
typedef struct process_s process_s;
typedef struct thread_s thread_s;
typedef struct Hash_Table_struct Hash_Table;
typedef struct pref_info_s pref_info_s;
typedef struct HWP_Struct HWP;
typedef struct gpu_allocq_entry_s gpu_allocq_entry_s; 
typedef struct thread_stat_s thread_stat_s;



///////////////////////////////////////////////////////////////////////////////////////////////
// Function declarations


void init_block_schedule_info(void); 
void terminate_process(process_s*);
void delete_store_hash_entry_wrapper (map_c *map, uop_c *uop);  


///////////////////////////////////////////////////////////////////////////////////////////////
// Global definitions


#define MAX_TR_OPCODE_NAME  GPU_OPCODE_LAST
#define MAX_GPU_ADDR_SPACE  GPU_ADDR_SP_LAST
#define MAX_GPU_CACHE_OP    GPU_CACHE_OP_LAST
#define MAX_GPU_CACHE_LEVEL GPU_CACHE_LAST
#define MAX_GPU_FENCE_LEVEL GPU_FENCE_LAST

#define MAX_PUP 256 
//per core
#define MAX_NUM_THREADS 3000
#define MAX_NUM_BLOCKS 64 
// across all the cores
#define MAX_NUM_TOTAL_BLOCKS 16384
#define MAX_NUM_CORES 128
// #define NUM_REG_IDS 1024
#define NUM_REG_IDS 1600 
#define NUM_INT_REGS 32
#define MAX_UOP_SRC_DEPS 10 // 6 + max 4 (store-load dependencies for each BYTE) // hyesoon 3-12-2009 
#define MAX_DRAM_BANKS 32
#define MAX_SRCS    9
#define MAX_DESTS   6
#define CACHE_MISS 1
#define CACHE_HIT 0
#define MAX_STR_LENGTH        256         /* default 256 */
#define MAX_SIMULTANEOUS_STRINGS    32      /* default 32 */ /* power of 2 */
//#define SUCCESS 1
//#define FAILURE 0
#define MAX_CTR  0xffffffffffffffffULL
#define MAX_SCTR 0x7fffffffffffffffLL
#define MAX_INT64  0x7fffffffffffffffLL
#define MAX_INT    0x7fffffff
#define MAX_UNS64  0xffffffffffffffffULL
#define MAX_UNS    0xffffffffU
#define MAX_ADDR   0xffffffffffffffffULL
#define INT64_C_M_1 0xffffffffffffffffLL
// ISA macros
#define MAX_TRACE_BUFFER_SIZE 500
#define BYTES_IN_INST           4
#define BYTES_IN_BYTE		1
#define BYTES_IN_WORD		2
#define BYTES_IN_LONGWORD	4
#define BYTES_IN_QUADWORD	8
#define BITS_IN_BYTE		8
#define BITS_IN_WORD		16
#define BITS_IN_LONGWORD	32
#define BITS_IN_QUADWORD	64

#ifndef NULL
#define NULL ((void *)0x0)
#endif

#ifdef USING_SST
#include "callback.h"
typedef SST::MacSim::CallbackBase4<void,int,uint64_t,uint64_t,int> CallbackSendInstReq;
#ifdef USE_VAULTSIM_HMC
typedef SST::MacSim::CallbackBase6<void,int,uint64_t,uint64_t,int,int,uint8_t> CallbackSendDataReq;
#else
typedef SST::MacSim::CallbackBase5<void,int,uint64_t,uint64_t,int,int> CallbackSendDataReq;
#endif
typedef SST::MacSim::CallbackBase4<void,uint64_t,uint64_t,int,int> CallbackSendCubeReq;
typedef SST::MacSim::CallbackBase2<bool,int,uint64_t> CallbackStrobeInstRespQ;
typedef SST::MacSim::CallbackBase2<bool,int,uint64_t> CallbackStrobeDataRespQ;
typedef SST::MacSim::CallbackBase1<bool,uint64_t> CallbackStrobeCubeRespQ;

#define UNIQUE_KEY(C,T,A,I) ((A<<19)|((C&0xF)<<15)|((T&0xFF)<<7)|I)
#endif //USING_SST

#endif
