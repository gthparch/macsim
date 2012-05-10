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
* File         : macsim.cc
* Author       : HPArch
* Date         : 1/7/2008
* SVN          : $Id: main.cc 911 2009-11-20 19:08:10Z kacear $:
* Description  : main function
*********************************************************************************************/


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/time.h>

#include "macsim.h"
#include "assert_macros.h"
#include "global_types.h"
#include "core.h"
#include "trace_read.h"
#include "frontend.h"
#include "knob.h"
#include "process_manager.h"
#include "debug_macros.h"
#include "statistics.h"
#include "memory.h"
#include "dram.h"
#include "utils.h"
#include "bug_detector.h"
#include "fetch_factory.h"
#include "pref_factory.h"
#include "noc.h"
#include "factory_class.h"
#include "dram.h"
#include "router.h"

#include "all_knobs.h"
#include "all_stats.h"
#include "statistics.h"


#ifdef POWER_EI
#include "ei_power.h"
#endif


using namespace std;


///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_SIM, ## args)


///////////////////////////////////////////////////////////////////////////////////////////////


// =======================================
// Macsim constructor
// =======================================
macsim_c::macsim_c()
{
    m_simBase = this;
    
    m_num_active_threads             = 0; 
    m_num_waiting_dispatched_threads = 0;
    m_total_num_application          = 0;
    m_process_count                  = 0; 
    m_process_count_without_repeat   = 0;
    m_all_threads                    = 0;
    m_no_threads_per_block           = 0;
    m_total_retired_block            = 0;
    m_end_simulation                 = false;
    m_repeat_done                    = false;
    m_gpu_paused                     = true;

    for (int ii = 0; ii < MAX_NUM_CORES; ++ii) {    
        m_core_cycle[ii]     = 0;
        m_core_end_trace[ii] = false;
        m_sim_end[ii]        = false;
        m_core_started[ii]   = false;
    }

    m_termination_check[0] = false;
    m_termination_check[1] = false;

    m_simulation_cycle = 0;
    m_core0_inst_count = 0;
}


// =======================================
// Macsim destructor
// =======================================
macsim_c::~macsim_c()
{
}


// =======================================
// initialize knobs
// =======================================
void macsim_c::init_knobs(int argc, char** argv)
{
    report("initialize knobs");

    // Create the knob managing class
    m_knobsContainer = new KnobsContainer();

    // Get a reference to the actual knobs for this component instance
    m_knobs = m_knobsContainer->getAllKnobs();

#ifdef USING_SST
    string  paramPath(argv[0]);
    string  tracePath(argv[1]);
    string outputPath(argv[2]);
    
    //Apply specified params.in file
    m_knobsContainer->applyParamFile(paramPath);
    
    //Specify trace_file_list and update the knob
    m_knobsContainer->updateKnob("trace_name_file", tracePath);

    //Specify output path for statistics
    m_knobsContainer->updateKnob("out", outputPath);

    //save the states of all knobs to a file
    m_knobsContainer->saveToFile(outputPath + "/params.out");

#else
    // apply values from parameters file
    m_knobsContainer->applyParamFile("params.in");

    // apply the supplied command line switches
    char* pInvalidArgument = NULL;
    if(!m_knobsContainer->applyComandLineArguments(argc, argv, &pInvalidArgument)) {
    }

    // save the states of all knobs to a file
    m_knobsContainer->saveToFile("params.out");
#endif
}


// =======================================
// register wrapper functions to allocate objects later
// =======================================
void macsim_c::register_functions(void)
{
    mem_factory_c::get()->register_class("l3_coupled_network", default_mem);
    mem_factory_c::get()->register_class("l3_decoupled_network", default_mem); 
    mem_factory_c::get()->register_class("l2_coupled_local", default_mem); 
    mem_factory_c::get()->register_class("no_cache", default_mem);
    mem_factory_c::get()->register_class("l2_decoupled_network", default_mem);
    mem_factory_c::get()->register_class("l2_decoupled_local", default_mem);
    
    dram_factory_c::get()->register_class("FRFCFS", frfcfs_controller);
    dram_factory_c::get()->register_class("FCFS", fcfs_controller);
    
    fetch_factory_c::get()->register_class("rr", fetch_factory);
    pref_factory_c::get()->register_class(pref_factory);
    bp_factory_c::get()->register_class("gshare", default_bp); 

    llc_factory_c::get()->register_class("default", default_llc);
}


// =======================================
// memory allocation
// =======================================
void macsim_c::init_memory(void)
{
  // pool allocation
  m_thread_pool           = new pool_c<thread_s>(10, "thread_pool"); 
  m_section_pool          = new pool_c<section_info_s>(100, "section_pool"); 
  m_mem_map_entry_pool    = new pool_c<mem_map_entry_c>(200, "mem_map_pool");
  m_heartbeat_pool        = new pool_c<heartbeat_s>(10, "heartbeat_pool");
  m_bp_recovery_info_pool = new pool_c<bp_recovery_info_c>(10, "bp_recovery_info_pool");
  m_trace_node_pool       = new pool_c<thread_trace_info_node_s>(10, "thread_node_pool");
  m_uop_pool              = new pool_c<uop_c>(1000, "uop_pool");

  m_block_id_mapper = new multi_key_map_c;
  m_process_manager = new process_manager_c(m_simBase);
  m_trace_reader = new trace_read_c(m_simBase);

  // block schedule info
  block_schedule_info_s* block_schedule_info = new block_schedule_info_s;
  m_block_schedule_info[0] = block_schedule_info;

  // dummy invalid uop
  m_invalid_uop = new uop_c(m_simBase);
  m_invalid_uop->init();

  // main memory
  string memory_type = m_simBase->m_knobs->KNOB_MEMORY_TYPE->getValue();
  m_memory = mem_factory_c::get()->allocate(memory_type, m_simBase);

  // dram controller
  m_num_mc = m_simBase->m_knobs->KNOB_DRAM_NUM_MC->getValue();
  m_dram_controller = new dram_controller_c*[m_num_mc];
  int num_noc_node = *m_simBase->m_knobs->KNOB_NUM_SIM_CORES
    + *m_simBase->m_knobs->KNOB_NUM_L3;
  for (int ii = 0; ii < m_num_mc; ++ii) {
    m_dram_controller[ii] = dram_factory_c::get()->allocate(
        m_simBase->m_knobs->KNOB_DRAM_SCHEDULING_POLICY->getValue(), m_simBase);
    m_dram_controller[ii]->init(ii, num_noc_node + ii);
  }


    // interconnection network
  if (*KNOB(KNOB_ENABLE_NEW_NOC))
    m_router = new router_wrapper_c(m_simBase);

  if (*KNOB(KNOB_ENABLE_IRIS) || *KNOB(KNOB_ENABLE_NEW_NOC))
    m_memory->init();
  else
    m_noc = new noc_c(m_simBase);

    // bug detector
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE)) {
        printf("enabling bug detector\n");
        m_bug_detector = new bug_detector_c(m_simBase);
    }
    
  m_PCL = new cache_partition_framework_c(m_simBase);
}


// =======================================
// initialize output streams 
// =======================================
void macsim_c::init_output_streams()
{
    string stderr_file = *m_simBase->m_knobs->KNOB_STDERR_FILE;
    string stdout_file = *m_simBase->m_knobs->KNOB_STDOUT_FILE;
    string status_file = *m_simBase->m_knobs->KNOB_STDOUT_FILE;

    if (strcmp(stderr_file.c_str(), "NULL")) {
        g_mystderr = file_tag_fopen(stderr_file.c_str(), "w", m_simBase);

        if (!g_mystderr) {
            fprintf(stderr, "\n");
            fprintf(stderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,
                unsstr64(m_core0_inst_count), unsstr64(m_simulation_cycle));
            fprintf(stderr, "%s '%s'\n", "mystderr", stderr_file.c_str());
            breakpoint(__FILE__, __LINE__);
            exit(15);
        }
    }

    if (strcmp(stdout_file.c_str(), "NULL")){
        g_mystdout = file_tag_fopen(stdout_file.c_str(), "w", m_simBase);

        if (!g_mystdout) {
            fprintf(stderr, "\n");
            fprintf(stderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,
                unsstr64(m_core0_inst_count), unsstr64(m_simulation_cycle));

            fprintf(stderr, "%s '%s'\n", "mystdout", stdout_file.c_str()); 
            breakpoint(__FILE__, __LINE__);
            exit(15);
        }
    }

    if (!strcmp(status_file.c_str(), "NULL")) {
        g_mystatus = fopen(status_file.c_str(), "a"); 

        if (!g_mystatus) {
            fprintf(stderr, "\n");
            fprintf(stderr, "%s:%d: ASSERT FAILED (I=%s  C=%s):  ", __FILE__, __LINE__,
                unsstr64(m_core0_inst_count), unsstr64(m_simulation_cycle));
            fprintf(stderr, "%s '%s'\n", "mystatus", status_file.c_str());
            breakpoint(__FILE__, __LINE__);
            exit(15);
        }
    }
}


// =======================================
// initialize cores 
// =======================================
void macsim_c::init_cores(int num_max_core)
{
    int num_large_cores        = *KNOB(KNOB_NUM_SIM_LARGE_CORES);
    int num_large_medium_cores = *KNOB(KNOB_NUM_SIM_LARGE_CORES) + *KNOB(KNOB_NUM_SIM_MEDIUM_CORES);

    report("initialize cores (" << num_large_cores << "/" 
            << (num_large_medium_cores - num_large_cores) << "/" 
      << *KNOB(KNOB_NUM_SIM_SMALL_CORES) <<")");

    ASSERT(num_max_core == (*KNOB(KNOB_NUM_SIM_SMALL_CORES) + \
        *KNOB(KNOB_NUM_SIM_MEDIUM_CORES) + *KNOB(KNOB_NUM_SIM_LARGE_CORES)));


    // based on the core type, add cores into type-specific core pools
    
    // large coresno_mcs
    for (int ii = 0; ii < num_large_cores; ++ii) { 
        m_core_pointers[ii] = new core_c(ii, m_simBase, UNIT_LARGE);
        m_core_pointers[ii]->init();
        m_core_pointers[ii]->pref_init();

        // insert to the core type pool
        if (static_cast<string>(*m_simBase->m_knobs->KNOB_LARGE_CORE_TYPE) == "ptx")
            m_ptx_core_pool.push(ii);
        else
            m_x86_core_pool.push(ii);
    }

    // medium cores
    int total_core = num_large_cores;
    for (int ii = 0; ii < *KNOB(KNOB_NUM_SIM_MEDIUM_CORES); ++ii) { 
        m_core_pointers[ii + num_large_cores] = new core_c(ii + num_large_cores, m_simBase, UNIT_MEDIUM);
        m_core_pointers[ii + num_large_cores]->init();
        m_core_pointers[ii + num_large_cores]->pref_init();

        // insert to the core type pool
        if (static_cast<string>(*m_simBase->m_knobs->KNOB_MEDIUM_CORE_TYPE) == "ptx")
            m_ptx_core_pool.push(ii + total_core);
        else
            m_x86_core_pool.push(ii + total_core);
    }

    // small cores
    total_core += *m_simBase->m_knobs->KNOB_NUM_SIM_MEDIUM_CORES;
    for (int ii = 0; ii < *KNOB(KNOB_NUM_SIM_SMALL_CORES); ++ii) { 
        m_core_pointers[ii + num_large_medium_cores] = 
            new core_c(ii + num_large_medium_cores, m_simBase, UNIT_SMALL);
        m_core_pointers[ii + num_large_medium_cores]->init();
        m_core_pointers[ii + num_large_medium_cores]->pref_init();

        // insert to the core type pool
        if (static_cast<string>(*m_simBase->m_knobs->KNOB_CORE_TYPE) == "ptx")
            m_ptx_core_pool.push(ii + total_core);
        else
            m_x86_core_pool.push(ii + total_core);
    }
}


#ifdef IRIS
// =======================================
// initialize IRIS parameters (with config file, preferrably)
// =======================================
void macsim_c::init_iris_config(map<string, string> &params)  //passed g_iris_params here
{
  params["topology"] = KNOB(KNOB_IRIS_TOPOLOGY)->getValue();

  // todo
  // 1. mapping function
  // 2. # port for torus
  // 3. rc method
  // 4. other knobs
  if (params["topology"] == "ring") {
    params["no_ports"] = "3";
    stringstream sstr;
    for (int ii = 0; ii < 64; ++ii) {
      sstr << ii;
      if (ii != 63)
        sstr << ',';
    }
    sstr >> params["mapping"];
    params["rc_method"] = "RING_ROUTING";
  }
  else if (params["topology"] == "mesh" || params["topology"] == "torus") {
    params["grid_size"] = KNOB(KNOB_IRIS_GRIDSIZE)->getValue();
    params["no_ports"]  = "5"; // check for torus
    params["rc_method"] = "XY_ROUTING_HETERO";
    params["mapping"]  = "0,1,2,5,6,7,8,9,10,13,14,15,16,17,18,21,22,23,24,25,26,29,30,31,32,33,34,37,38,39,40,41,42,45,46,47,48,49,50,53,54,55,56,57,58,61,62,63,11,12,19,20,27,28,35,36,43,44,51,52,3,4,59,60,";                                                             
    //original spinal order "0,1,2,48,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,3,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66";

//"0,1,2,60,61,3,4,5,6,7,8,48,49,9,10,11,12,13,14,50,51,15,16,17,18,19,20,52,53,21,22,23,24,25,26,54,55,27,28,29,30,31,32,56,57,33,34,35,36,37,38,58,59,39,40,41,42,43,44,62,63,45,46,47";
    //"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16";
  }//else if (params["topology"] == "spinalMesh") 
  
  //FIXME - always uses spinalMesh for testing
  if (0)
  {
    params["grid_size"] = KNOB(KNOB_IRIS_GRIDSIZE)->getValue();
    params["no_ports"]  = "5"; // 4 for north/south, 2 for west/east, 1 for interface
    params["mapping"]  = "0,1,2,60,61,3,4,5,6,7,8,48,49,9,10,11,12,13,14,50,51,15,16,17,18,19,20,52,53,21,22,23,24,25,26,54,55,27,28,29,30,31,32,56,57,33,34,35,36,37,38,58,59,39,40,41,42,43,44,62,63,45,46,47";
    //"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66";
    cout << "reading mesh topology in init iris config" << endl;
    params["rc_method"] = "XY_ROUTING_HETERO"; //doesn't work in param file, might need to use quotes
  }
  params["no_vcs"]         = KNOB(KNOB_IRIS_NUM_VC)->getValue();
  params["credits"]        = KNOB(KNOB_IRIS_CREDIT)->getValue();
  params["int_buff_width"] = KNOB(KNOB_IRIS_INT_BUFF_WIDTH)->getValue();
  params["link_width"]     = KNOB(KNOB_IRIS_LINK_WIDTH)->getValue();

  string s;
  ToString(s, KNOB(KNOB_DRAM_NUM_MC)->getValue());
  params["no_mcs"] = s; 

  //number of nodes depends on MacSim configuration (# of L1+L3(acts as L2))+MC nodes)
  ToString(s, m_macsim_terminals.size());
  params["no_nodes"] = s;
          
  report("Routing method: " << params["rc_method"] << "\n");       
#if FIXME
  //if (! key.compare("-iris:self_assign_dest_id"))
   //           params.insert(pair<string,string>("self_assign_dest_id",value));
          if (! key.compare("-mc:resp_payload_len"))
              params.insert(pair<string,string>("resp_payload_len",value));
          if (! key.compare("-mc:memory_latency"))
              params.insert(pair<string,string>("memory_latency",value));
#endif
}
#endif


// =======================================
// IRIS
// initialize Iris/manifold network
// =======================================
void macsim_c::init_network(void)
{
#ifdef IRIS
  assert(*KNOB(KNOB_ENABLE_IRIS) == true && *KNOB(KNOB_ENABLE_NEW_NOC) == false);
  init_iris_config(m_iris_params);

  map<std::string, std::string>:: iterator it;

  it = m_iris_params.find("topology");
  if ((it->second).compare("ring") == 0) {
    m_iris_network = new Ring(m_simBase);
  } 
  else if ((it->second).compare("mesh") == 0) {
    m_iris_network = new Mesh(m_simBase);
  } 
  else if ((it->second).compare("torus") == 0) {
    m_iris_network = new Torus(m_simBase);
  } 

  //initialize iris network
  m_iris_network->parse_config(m_iris_params);
  m_simBase->no_nodes = m_macsim_terminals.size();
  report("number of macsim terminals: " << m_macsim_terminals.size() << "\n");

#define MAPI i //((Mesh*)m_iris_network)->mapping[i]

  for (unsigned int i=0; i<m_macsim_terminals.size(); i++) {
    //create component id
    manifold::kernel::CompId_t interface_id = manifold::kernel::Component::Create<NInterface>(0,m_simBase);
    manifold::kernel::CompId_t router_id = manifold::kernel::Component::Create<SimpleRouter>(0,m_simBase);

    //create component
    NInterface* interface = manifold::kernel::Component::GetComponent<NInterface>(interface_id);
    SimpleRouter* rr =manifold::kernel::Component::GetComponent<SimpleRouter>(router_id);

    //set node id
    interface->node_id = i;
    rr->node_id = i;

    //register clock
    manifold::kernel::Clock::Register<NInterface>(interface, &NInterface::tick, &NInterface::tock);
    manifold::kernel::Clock::Register<SimpleRouter>(rr, &SimpleRouter::tick, &SimpleRouter::tock);

    //push back
    m_iris_network->terminals.push_back(m_macsim_terminals[MAPI]);
    m_iris_network->terminal_ids.push_back(m_macsim_terminals[MAPI]->GetComponentId());
    m_iris_network->interfaces.push_back(interface);
    m_iris_network->interface_ids.push_back(interface_id);
    m_iris_network->routers.push_back(rr);
    m_iris_network->router_ids.push_back(router_id);

    //init
    m_macsim_terminals[MAPI]->parse_config(m_iris_params);
    m_macsim_terminals[MAPI]->init();
    interface->parse_config(m_iris_params);
    interface->init();
    rr->parse_config(m_iris_params);
    rr->init();
  }

  //initialize router outports
  for (unsigned int i = 0; i < m_macsim_terminals.size(); i++)
    m_iris_network->set_router_outports(i);

  m_iris_network->connect_interface_terminal();
  m_iris_network->connect_interface_routers();
  m_iris_network->connect_routers();

#ifdef POWER_EI
  //initialize power stats
  avg_power     = 0;
  total_energy  = 0;
  total_packets = 0;
#endif
#else
  if (*KNOB(KNOB_ENABLE_NEW_NOC)) {
    m_router->init();
  }
#endif
}


// =======================================
// initialize simulation
// =======================================
void macsim_c::init_sim(void)
{
  report("initialize simulation");

  // start measuring time
  gettimeofday(&m_begin_sim, NULL);

  // make sure all the variable sizes are what we expect
  ASSERTU(sizeof(uns8)  == 1);
  ASSERTU(sizeof(uns16) == 2);
  ASSERTU(sizeof(uns32) == 4);
  ASSERTU(sizeof(uns64) == 8);
  ASSERTU(sizeof(int8)  == 1);
  ASSERTU(sizeof(int16) == 2);
  ASSERTU(sizeof(int32) == 4);
  ASSERTU(sizeof(int64) == 8);
}


#ifdef POWER_EI
// =======================================
// =======================================
void macsim_c::compute_power(void)
{
    m_ei_power = new ei_power_c(m_simBase);
    m_ei_power->ei_config_gen_top();    // to make config file for EI
    m_ei_power->ei_main();
    
    delete m_ei_power;
}
#endif


// =======================================
// open traces from trace_file_list file
// =======================================
void macsim_c::open_traces(string trace_list)
{
  fstream tracefile(trace_list.c_str(), ios::in);

  tracefile >> m_total_num_application;

  // create processes
  for (int ii = 0; ii < m_total_num_application; ++ii) {
    string line;
    tracefile >> line;
    m_process_manager->create_process(line);
    m_process_manager->sim_thread_schedule();
  }
}


// =======================================
// deallocate memory
// =======================================
void macsim_c::deallocate_memory(void)
{
  // memory deallocation
  delete m_thread_pool;
  delete m_section_pool; 
  delete m_mem_map_entry_pool;
  delete m_heartbeat_pool;
  delete m_bp_recovery_info_pool;
  delete m_trace_node_pool;
  delete m_uop_pool;
  delete m_invalid_uop;
  delete m_memory;

  if (*KNOB(KNOB_ENABLE_IRIS) == false)
    delete m_noc;

  if (*KNOB(KNOB_ENABLE_NEW_NOC))
    delete m_router;

  if (*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
    delete m_bug_detector;

  // deallocate cores
  int num_large_cores        = *KNOB(KNOB_NUM_SIM_LARGE_CORES);
  int num_large_medium_cores = *KNOB(KNOB_NUM_SIM_LARGE_CORES) + *KNOB(KNOB_NUM_SIM_MEDIUM_CORES);
  for (int ii = 0; ii < num_large_cores; ++ii) {
    delete m_core_pointers[ii];
    m_core_pointers[ii] = NULL;
  }

  for (int ii = 0; ii < *KNOB(KNOB_NUM_SIM_MEDIUM_CORES); ++ii) {
    delete m_core_pointers[ii + num_large_cores];
    m_core_pointers[ii + num_large_cores] = NULL;
  }

  for (int ii = 0; ii < *KNOB(KNOB_NUM_SIM_SMALL_CORES); ++ii) {
    delete m_core_pointers[ii + num_large_medium_cores];
    m_core_pointers[ii + num_large_medium_cores] = NULL;
  }
}


// =======================================
// finalize simulation 
// =======================================
void macsim_c::fini_sim(void)
{
  report("finalize simulation");

  // execution time calculation
  gettimeofday(&m_end_sim, NULL);
  REPORT("elapsed time:%.1f seconds\n", \
      ((m_end_sim.tv_sec - m_begin_sim.tv_sec)*1000000 + \
       m_end_sim.tv_usec - m_begin_sim.tv_usec)/1000000.0);

  int second = static_cast<int>(
      ((m_end_sim.tv_sec - m_begin_sim.tv_sec)*1000000 + 
       m_end_sim.tv_usec - m_begin_sim.tv_usec)/1000000.0);
  STAT_EVENT_N(EXE_TIME, second);

#ifdef POWER_EI
  // compute power if enable_energy_introspector is enabled
  if (*KNOB(KNOB_ENABLE_ENERGY_INTROSPECTOR)) {
    compute_power();
  }
#endif


#ifdef IRIS
  ofstream irisTraceFile;
  string logName = "iris_log_mesh_stall";

  irisTraceFile.open(logName.c_str());
  if (irisTraceFile.is_open())
  {
    cout << "printing to file " << logName << endl;

    //mapping info
    irisTraceFile << ":Topology mapping:" << endl;

    const char* message_class_type[] = {"INVALID", "PROC", "L1", "L2", "L3", "MC"};
    for(unsigned int i=0; i<m_macsim_terminals.size(); i++)
    {
      int node_id = ((Ring*)m_iris_network)->mapping[i];
      int type = m_simBase->m_macsim_terminals.at(node_id)->mclass;

      irisTraceFile << message_class_type[type] << "," 
        << m_macsim_terminals[node_id]->ptx << "," << node_id << ",";
    }
    irisTraceFile << endl;

    //summary info
    irisTraceFile << ":Per Node Summary: node id, total packets out, total flits in, total flits out, cycles: ib, sa, vca, st, average buffer size, average packet latency, average flit latency,  " << endl;
    for(unsigned int i=0; i<m_iris_network->routers.size(); i++)
    {
      irisTraceFile << m_iris_network->routers[i]->print_csv_stats();
    }

    //detailed, router/packet state vs time info
    irisTraceFile << ":Detail info: clock cycle, req ID, mem state, msg type, current node, dst node, component (R=Router), input buffer port0.vc0, port0,vc1, port1,vc0, etc..." << endl;
    irisTraceFile << m_simBase->network_trace.str();
  }
  irisTraceFile.close();

  for (unsigned int ii = 0; ii < m_iris_network->routers.size(); ++ii) {
    //        m_iris_network->routers[i]->print_stats();
    m_iris_network->routers[ii]->power_stats();
  }
#ifdef POWER_EI
  cout << "Average Network power: " << avg_power << "W\n"
    << "Total Network Energy: " << total_energy << "J\n"
    << "Total packets " << total_packets << "\n";
#endif
#endif
}



int get_gcd(int a, int b)
{
  if (b == 0)
    return a;

  return get_gcd(b, a % b);
}

int get_lcm(int a, int b)
{
  return (a * b) / get_gcd(a, b);
}


// =======================================
//Initialization before simulation run
// =======================================
void macsim_c::initialize(int argc, char** argv) 
{
  g_mystdout = stdout;
  g_mystderr = stderr;
  g_mystatus = NULL;

  // initialize knobs
  init_knobs(argc, argv);

  // initialize stats
  m_coreStatsTemplate = new CoreStatistics(m_simBase);
  m_ProcessorStats = new ProcessorStatistics(m_simBase);

  m_allStats = new all_stats_c(m_ProcessorStats);
  m_allStats->initialize(m_ProcessorStats, m_coreStatsTemplate);

  init_per_core_stats(*m_simBase->m_knobs->KNOB_NUM_SIM_CORES, m_simBase);
  cout << "number of cores" << *m_simBase->m_knobs->KNOB_NUM_SIM_CORES << "\n";

  // register wrapper functions
  register_functions();

  // initialize simulation
  init_sim();

#ifdef IRIS
  master_clock = new manifold::kernel::Clock(1); //clock has to be global or static
#endif

  // init memory
  init_memory();

  // initialize some of my output streams to the standards */
  init_output_streams();

  // initialize cores
  init_cores(*KNOB(KNOB_NUM_SIM_CORES));


#ifdef IRIS
  REPORT("Initializing sim IRIS\n");
  manifold::kernel::Manifold::Init(0, NULL);

  // initialize interconnect network
  init_network();
#else
  if (*KNOB(KNOB_ENABLE_NEW_NOC)) {
    report("Initializing new noc\n");
    init_network();
  }
#endif

  // initialize clocks
  init_clock_domain();


  // open traces
  string trace_name_list = static_cast<string>(*KNOB(KNOB_TRACE_NAME_FILE));
  open_traces(trace_name_list);

  // any number other than 0, to pass the first simulation loop iteration
  m_num_running_core = 10000; 
}


// =======================================
// To maintain different clock frequency for CPU, GPU, NOC, L3, MC
// =======================================
void macsim_c::init_clock_domain(void)
{
  m_clock_internal = 0;
  float domain_f[5];
  domain_f[0] = *KNOB(KNOB_CLOCK_CPU);
  domain_f[1] = *KNOB(KNOB_CLOCK_GPU);
  domain_f[2] = *KNOB(KNOB_CLOCK_L3);
  domain_f[3] = *KNOB(KNOB_CLOCK_NOC);
  domain_f[4] = *KNOB(KNOB_CLOCK_MC);


  // allow only .x format
  for (int ii = 0; ii < 1; ++ii) {
    bool found = false;
    for (int jj = 0; jj < 5; ++jj) {
      int int_cast = static_cast<int>(domain_f[jj]);
      float float_cast = static_cast<float>(int_cast);
      if (domain_f[jj] != float_cast) {
        found = true;
        break;
      }
    }

    if (found) {
      for (int jj = 0; jj < 5; ++jj) {
        domain_f[jj] *= 10;
      }
    }
    else {
      break;
    }
  }

  int domain_i[5];
  for (int ii = 0; ii < 5; ++ii)
    domain_i[ii] = static_cast<int>(domain_f[ii]);

  m_clock_lcm = get_lcm(domain_i[0], domain_i[1]);
  for (int ii = 2; ii < 5; ++ii)
    m_clock_lcm = get_lcm(m_clock_lcm, domain_i[ii]);

  for (int ii = 0; ii < 5; ++ii)
    m_clock_divisor[ii] = m_clock_lcm / domain_i[ii];

  report("Clock LCM           : " << m_clock_lcm);
  report("CPU clock frequency : " << *KNOB(KNOB_CLOCK_CPU) << " GHz " << m_clock_divisor[0]);
  report("GPU clock frequency : " << *KNOB(KNOB_CLOCK_GPU) << " GHz " << m_clock_divisor[1]);
  report("L3  clock frequency : " << *KNOB(KNOB_CLOCK_L3)  << " GHz " << m_clock_divisor[2]);
  report("NOC clock frequency : " << *KNOB(KNOB_CLOCK_NOC) << " GHz " << m_clock_divisor[3]);
  report("MC  clock frequency : " << *KNOB(KNOB_CLOCK_MC)  << " GHz " << m_clock_divisor[4]);

}

#define CLOCK_CPU 0
#define CLOCK_GPU 1
#define CLOCK_L3  2
#define CLOCK_NOC 3
#define CLOCK_MC  4

// =======================================
// Single cycle step of simulation state : returns running status
// =======================================
int macsim_c::run_a_cycle() 
{
  // termination condition check
  // 1. no active threads in the system
  // 2. repetition has been done (in case of multiple applications simulation)
  // 3. no core has been executed in the last cycle;
  if (m_termination_check[0] && m_termination_check[1] && 
      (m_num_active_threads == 0 || m_repeat_done || m_num_running_core == 0))  {
    return 0; //simulation finished
  }

  if (m_termination_check[0] && m_termination_check[1]) {
    m_termination_check[0] = false;
    m_termination_check[1] = false;
    m_num_running_core = 0;
  }

  int pivot = m_core_cycle[0]+1;


  // interconnection
  if (m_clock_internal % m_clock_divisor[CLOCK_NOC] == 0) {
#ifdef IRIS
    manifold::kernel::Manifold::Run((double) m_simulation_cycle);       //IRIS
    manifold::kernel::Manifold::Run((double) m_simulation_cycle);       //IRIS for half tick?
#else
    if (*KNOB(KNOB_ENABLE_NEW_NOC)) {
      m_router->run_a_cycle();
    }
    else {
      // run interconnection network
      m_noc->run_a_cycle();
    }
#endif
  }

  // run memory system
  if (m_clock_internal % m_clock_divisor[CLOCK_L3] == 0) {
    m_memory->run_a_cycle();
  }
  
  // run dram controllers
  if (m_clock_internal % m_clock_divisor[CLOCK_MC] == 0) {
    for (int ii = 0; ii < m_num_mc; ++ii) {
      m_dram_controller[ii]->run_a_cycle();
    }
  }


  // core execution loop
  for (int kk = 0; kk < *KNOB(KNOB_NUM_SIM_CORES); ++kk) {
    // use pivot to randomize core run_cycle pattern 
    int ii = (kk+pivot) % *m_simBase->m_knobs->KNOB_NUM_SIM_CORES;

    core_c *core = m_core_pointers[ii];
    string core_type = core->get_core_type();
    if (core_type == "ptx" && m_clock_internal % m_clock_divisor[CLOCK_GPU] != 0) {
      continue;
    }
    else if (core_type != "ptx" && m_clock_internal % m_clock_divisor[CLOCK_CPU] != 0) {
      continue;
    }

    // increment core cycle
    core->inc_core_cycle_count();
    m_core_cycle[ii]++;

    // core ended or not started    
    if (m_sim_end[ii] || !m_core_started[ii]) {
      continue;
    }

    // check whether all ops in this core have been completed.
    if (core->m_running_thread_num == 0 && (core->m_unique_scheduled_thread_num >= 1)) { 
      if (m_num_waiting_dispatched_threads == 0)  {
        m_sim_end[ii] = true;
      }
    }

    // active core : running a cycle and update stats
    if (!m_sim_end[ii])  {
      // run a cycle
      m_memory->run_a_cycle_core(ii);
      core->run_a_cycle();

      m_num_running_core++;
      STAT_CORE_EVENT(ii, CYC_COUNT);
      STAT_CORE_EVENT(ii, NUM_SAMPLES);
      STAT_CORE_EVENT_N(ii, NUM_ACTIVE_BLOCKS, core->m_running_block_num);
      STAT_CORE_EVENT_N(ii, NUM_ACTIVE_THREADS, core->m_running_thread_num);
    }

    // checking for threads 
    if (m_sim_end[ii] != true) {
      // when KNOB_MAX_INSTS is set, execute each thread for KNOB_MAX_INSTS instructions
      if (*m_simBase->m_knobs->KNOB_MAX_INSTS && 
          core->m_num_thread_reach_end == core->m_unique_scheduled_thread_num) {
        m_sim_end[ii] = true;
      }
      // when KNOB_SIM_CYCLE_COUNT is set, execute only KNOB_SIM_CYCLE_COUNT cycles
      else if (*KNOB(KNOB_SIM_CYCLE_COUNT) && m_simulation_cycle >= *KNOB(KNOB_SIM_CYCLE_COUNT)) {
        m_sim_end[ii] = true;
      }
    }

    if (!m_sim_end[ii]) { 
      // advance queues to prepare for the next cycle
      core->advance_queues();

      // check heartbeat
      core->check_heartbeat(false);

      // forward progress check in every 10000 cycles
      if (!(m_core_cycle[ii] % 10000))  
        core->check_forward_progress();
    } 

    // when a core has been completed, do last print heartbeat 
    if (m_sim_end[ii] || m_core_end_trace[ii]) 
      core->check_heartbeat(true);
  }


  // increase simulation cycle
  m_simulation_cycle++;
  STAT_EVENT(CYC_COUNT_TOT);
    
  if (m_clock_internal % m_clock_divisor[CLOCK_CPU] == 0) {
    STAT_EVENT(CYCLE_CPU);
    m_termination_check[0] = true;
  }

  if (m_clock_internal % m_clock_divisor[CLOCK_GPU] == 0) {
    STAT_EVENT(CYCLE_GPU);
    m_termination_check[1] = true;
  }

  m_clock_internal = (m_clock_internal + 1) % m_clock_lcm;

  return 1; //simulation not finished
}


// =======================================
// Create a router
// =======================================
router_c* macsim_c::create_router(int type)
{
  return m_router->create_router(type);
}


// =======================================
// Simulation end cleanup
// =======================================
void macsim_c::finalize()
{
  // deallocate memory
  deallocate_memory();

  // finalize simulation
  fini_sim();

  // dump out stat files at the end of simulation
  m_ProcessorStats->saveStats();

  cout << "Done\n";
}

