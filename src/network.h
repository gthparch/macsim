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
 * File         : network.h 
 * Author       : HPArch Research Group
 * Date         : 2/18/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Interconnection network
 *********************************************************************************************/


#ifndef NETWORK_H
#define NETWORK_H


#include <vector>
#include <unordered_map>
#include <queue>
#include <list>

#include "macsim.h"


#define CPU_ROUTER 0
#define GPU_ROUTER 1
#define L3_ROUTER 2
#define MC_ROUTER 3

#define LOCAL 0
#define LEFT  1
#define RIGHT 2
#define UP    3
#define DOWN  4

#define HEAD 0
#define BODY 1
#define TAIL 2

#define INIT 0
#define IB   1
#define RC   2
#define VCA  3
#define SA   4
#define ST   5
#define LT   6

#define OLDEST_FIRST 0
#define ROUND_ROBIN  1
#define CPU_FIRST    2
#define GPU_FIRST    3

#define GPU_FRIENDLY 0
#define CPU_FRIENDLY 1
#define MIXED        2
#define INTERLEAVED  3


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Flit data structure
///////////////////////////////////////////////////////////////////////////////////////////////
class flit_c
{
  public:
    /**
     * Constructor
     */
    flit_c();

    /**
     * Destructor
     */
    ~flit_c();

    /**
     * Initialize function
     */
    void init(void);

  public:
    int        m_src; /**< msg source */
    int        m_dst; /**< msg destination */
    int        m_dir; /**< direction */
    bool       m_head; /**< header flit? */
    bool       m_tail; /**< tail flit? */
    mem_req_s* m_req; /**< pointer to the memory request */
    int        m_state; /**< current state */
    Counter    m_timestamp; /**< timestamp */
    Counter    m_rdy_cycle; /**< ready cycle */
    Counter    m_rc_changed; /**< route changed? */
    bool       m_vc_batch; /**< VCA batch */
    bool       m_sw_batch; /**< SWA batch */
    int        m_id; /**< flit id */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Credit class - for credit-based flow control
///////////////////////////////////////////////////////////////////////////////////////////////
class credit_c
{
  public:
    /**
     * Constructor
     */
    credit_c();

    /**
     * Destructor
     */
    ~credit_c();

  public:
    Counter m_rdy_cycle; /**< credit ready cycle */
    int m_port; /**< credit port */
    int m_vc; /**< credit vc */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief dummy router class
///////////////////////////////////////////////////////////////////////////////////////////////
class router_c
{
  public:
    router_c(macsim_c* simBase, int type, int id, int num_port);
    ~router_c();

  public:
    virtual bool inject_packet(mem_req_s* req);
    virtual mem_req_s* receive_req(int dir);
    virtual void pop_req(int dir);
    virtual void init(int total_router, int* total_packet, pool_c<flit_c>* flit_pool, 
        pool_c<credit_c>* credit_pool);
    virtual void set_link(int dir, router_c* link);
    virtual int get_id(void);
    virtual void set_id(int id);
    virtual void run_a_cycle(bool pll_lock);
    virtual void print_link_info() = 0;
    virtual void print(ofstream& out);
    virtual void insert_packet(flit_c* flit, int ip, int ivc);
    virtual void insert_credit(credit_c*);
    virtual router_c* get_router(int dir);

    // these functions are currently used by only network_simple_c
    virtual void reset(void);
    virtual int* get_num_packet_inserted(void);

    void set_router_map(vector<router_c*>& router_map);
    void insert_packet(mem_req_s* req);
    
    /**
     * VCA (Virtual Channel Allocation) stage
     */
    virtual void stage_vca(void);
    
    /**
     * RC (Route Calculation) stage
     */
    virtual void stage_rc(void) = 0;

    /**
     * LT (Link Traversal) stage
     */
    virtual void stage_lt(void);
    
    /**
     * SWT (Switch Traversal) stage
     */
    virtual void stage_st(void);
    
    /**
     * VC arbitration
     */
    virtual void stage_vca_pick_winner(int, int, int&, int&) = 0;

    /**
     * SW arbitration
     */
    virtual void stage_sa_pick_winner(int, int&, int&, int);

    /**
     * Local packet injection
     */
    virtual void local_packet_injection(void);

    /**
     * Process pending credits to model credit traversal latency
     */
    virtual void process_pending_credit(void);

    /**
     * SWA (Switch Allocation) stage
     */
    virtual void stage_sa(void);

    /**
     * Get output virtual channel occupancy per type
     */
    virtual int get_ovc_occupancy(int port, bool type);

    /**
     * Check channel (for statistics)
     */
    virtual void check_channel(void);

    /**
     * Check starvation
     */
    virtual void check_starvation(void);

  private:
    router_c(); // do not implement

  protected:
    macsim_c* m_simBase; /**< pointer to simulation base class */

    int m_num_vc; /**< number of virtual channels */
    int m_num_port; /**< number of ports */
    
    string m_topology; /**< router topology */
    int m_type; /**< router type */
    int m_id; /**< router id */
    int m_total_router; /**< number of total routers */

    // configurations
    int m_link_latency; /**< link latency */
    int m_link_width; /**< link width */
    int* m_total_packet; /**< number of total packets (global) */
    bool m_enable_vc_partition; /**< enable virtual channel partition */
    int m_num_vc_cpu; /**< number of vcs for CPU */
    int m_next_vc; /**<id of next vc for local packet injection */
 
    // pools for data structure
    pool_c<flit_c>* m_flit_pool; /**< flit data structure pool */
    pool_c<credit_c>* m_credit_pool; /**< credit pool */
    
    // arbitration
    int m_arbitration_policy; /**< arbitration policy */

    // link
    router_c* m_link[5]; /**< links */
    unordered_map<int, int> m_opposite_dir; /**< opposite direction map */

    // buffers
    list<mem_req_s*>* m_injection_buffer; /**< injection queue */
    int m_injection_buffer_max_size; /**< max injection queue size */
    queue<mem_req_s*>* m_req_buffer; /**< ejection queue */

    int m_buffer_max_size; /**< input/output buffer max size */

    // per input port
    list<flit_c*>** m_input_buffer; /**< input buffer */
    bool**** m_route; /**< route information */
    int** m_route_fixed; /**< determined rc for the packet */
    int** m_output_port_id; /**< output port id */
    int** m_output_vc_id; /**< output vc id */
    
    // per output port
    list<flit_c*>** m_output_buffer; /**< output buffer */
    bool** m_output_vc_avail; /**< output vc availability */
    int** m_credit; /**< credit counter for the flow control */
    Counter* m_link_avail; /**< link availability */

    // per switch
    Counter* m_sw_avail; /**< switch availability */

    // credit-based flow control
    list<credit_c*>* m_pending_credit; /**< pending credit to model latency */

    vector<router_c*> m_router_map; /**< link to other routers */

    // clock
    Counter m_cycle; /**< router clock */

};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Network interface
///////////////////////////////////////////////////////////////////////////////////////////////
class network_c
{
  public:
    /**
     * Constructor
     */
    network_c(macsim_c* simBase);

    /**
     * Destructor
     */
    virtual ~network_c() = 0;

  private:
    network_c();

  public:
    virtual void init(int num_cpu, int num_gpu, int num_l3, int num_mc) = 0;

    virtual bool send(mem_req_s* req, int src_level, int src_id,
        int dst_level, int dst_id);

    virtual mem_req_s* receive(int level, int id);

    virtual void receive_pop(int level, int id);

    virtual void run_a_cycle(bool pll_lock) = 0;

    virtual void print() = 0;

  protected:
    macsim_c* m_simBase;
    string m_topology; /**< topology */
    
    int m_num_router; /**< number of routers */
    int m_total_packet; /**< number of total packets */
    int m_num_cpu;
    int m_num_gpu;
    int m_num_l3;
    int m_num_mc;

    pool_c<flit_c>* m_flit_pool; /**< flit data structure pool */
    pool_c<credit_c>* m_credit_pool; /**< credit pool */
    
    vector<router_c*> m_router; /**< all routers */
    unordered_map<int, int> m_router_map;

    Counter m_cycle; /**< clock cycle */

};




#endif
