/**********************************************************************************************
* File         : macsim.h
* Author       : Jaekyu Lee
* Date         : 3/25/2011
* SVN          : $Id: main.cc 911 2009-11-20 19:08:10Z kacear $:
* Description  : MacSim Wrapper Class
*********************************************************************************************/

#ifndef MACSIM_H_INCLUDED
#define MACSIM_H_INCLUDED


#include <unordered_map>

#include "global_defs.h"
#include "global_types.h"

#include "manifold/kernel/include/kernel/clock.h"

#ifndef DONT_INCLUDE_MANIFOLD       //to prevent circular link dependencies
#include "manifold/models/iris/interfaces/topology.h"
#include "manifold/models/iris/interfaces/irisInterface.h"
#include "manifold/models/iris/interfaces/irisTerminal.h"
#include "manifold/models/iris/iris_srcs/components/manifoldProcessor.h"
#include "manifold/models/iris/iris_srcs/topology/ring.h"
#include "manifold/models/iris/iris_srcs/topology/torus.h"
#include "manifold/kernel/include/kernel/manifold.h"
#include "manifold/kernel/include/kernel/component.h"
#include "manifold/models/iris/iris_srcs/components/ninterface.h"
#include "manifold/models/iris/iris_srcs/components/simpleRouter.h"
#endif

class Topology;

class macsim_c
{
	public:
		macsim_c();
		~macsim_c();

		void initialize(int argc, char** argv);
		int  run_a_cycle();
		void finalize();

		void init_knobs(int argc, char** argv);
		void register_functions(void);
		void init_sim(void);
		void init_memory(void);
		void init_output_streams(void);
		void init_cores(int num_core);
		void init_network(string topology);
		void open_traces(string trace_file);
		void deallocate_memory(void);
		void compute_power(void);
		void fini_sim(void);

	public:
		int m_num_active_threads; // number of active threads
		int m_num_waiting_dispatched_threads; // number of threads waiting for begin dispatched
		int m_total_num_application; // total number of applications
		int m_process_count; // number of processes
		int m_process_count_without_repeat; // number of processes without repeat
		int m_all_threads; // number of all threads
		int m_no_threads_per_block; // number of threads per block
		int m_total_retired_block; //number of retired blocks total

		int m_num_running_core; //set to any non0 number before simulation start

		bool m_repeat_done; // in trace repeat mode, indicate repetition done


		FILE *g_mystdout;	// default output stream
		FILE *g_mystderr;	// default error stream
		FILE *g_mystatus; // default status stream
		Counter m_simulation_cycle; // simulation cycle count
		Counter m_core0_inst_count; // core 0 inst count for debug/assert
		Counter m_core_cycle[MAX_NUM_CORES];// core cycle count
		int m_end_simulation; //flag to end simulation

		all_stats_c*         m_allStats;
		ProcessorStatistics* m_ProcessorStats;
		CoreStatistics*      m_coreStatsTemplate;
		map<string, ofstream*> m_AllStatsOutputStreams;
		ofstream *m_core_stat_files[MAX_NUM_CORES]; // core statistics files

		KnobsContainer *m_knobsContainer;
		all_knobs_c    *m_knobs;

		bool m_core_end_trace[MAX_NUM_CORES]; // core end trace flag
		bool m_sim_end[MAX_NUM_CORES]; // core sim end flag
		bool m_core_started[MAX_NUM_CORES]; // core started flag

		core_c *m_core_pointers[MAX_NUM_CORES]; // core pointers
		memory_c* m_memory; // main memory
		noc_c* m_noc; // interconnection network
		queue<int> m_x86_core_pool; // x86 cores pool
		queue<int> m_ptx_core_pool; // GPU cores pool
		bug_detector_c *m_bug_detector; // bug detector
		multi_key_map_c* m_block_id_mapper; // block id mapper
		process_manager_c* m_process_manager; // process manager

		uop_c *m_invalid_uop; // invalide uop pointer (for uop pool maintenance)
	
		ei_power_c* m_ei_power; // energy introspector

		pool_c<thread_s>* m_thread_pool; // thread data pool
		pool_c<section_info_s>* m_section_pool; // section data pool
		pool_c<mem_map_entry_c>* m_mem_map_entry_pool; // memory dependence data pool
		pool_c<heartbeat_s>* m_heartbeat_pool; // heartbeat data pool
		pool_c<bp_recovery_info_c>* m_bp_recovery_info_pool; // bp recovery information pool
		pool_c<thread_trace_info_node_s>* m_trace_node_pool; // trace node pool
		pool_c<uop_c> *m_uop_pool; // uop pool

		unordered_map<int, hash_c<inst_info_s>*> m_inst_info_hash; // decoded instruction map
		unordered_map<int, block_schedule_info_s*> m_block_schedule_info; // block schedule info
		unordered_map<int, process_s*> m_sim_processes; // process map
		unordered_map<int, thread_stat_s*> m_thread_stats; // thread stat map

		struct timeval m_begin_sim; // simulation start time
		struct timeval m_end_sim; // simulation termination time

		trace_read_c* m_trace_reader;
                
		// interconnect
	public:
		void init_iris_config(map<string, string> &params);
	public:
		map<string, string> m_iris_params;
		Topology* m_iris_network; //IRIS
		Topology* tp;
		uint no_nodes;
		int Mytid;
		FILE* log_file;

		vector<ManifoldProcessor*> m_macsim_terminals;
    cache_partition_framework_c* m_PCL;

		manifold::kernel::Clock* master_clock;// = new manifold::kernel::Clock(1); //clock has to be global or static
	private:
		macsim_c* m_simBase; // self-reference for macro usage
};

#endif
