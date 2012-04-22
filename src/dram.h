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
 * File         : dram.h 
 * Author       : Jaekyu Lee
 * Date         : 11/3/2009
 * SVN          : $Id: dram.h 912 2009-11-20 19:09:21Z kacear $
 * Description  : Dram Controller 
 *********************************************************************************************/


#ifndef DRAM_H
#define DRAM_H


#include <list>
#include <fstream>

#include "macsim.h"
#include "global_types.h"
#include "global_defs.h"


#ifdef DRAMSIM
namespace DRAMSim {
  class MultiChannelMemorySystem;
};

//};
#endif


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief dram state enumerator
///////////////////////////////////////////////////////////////////////////////////////////////
enum DRAM_STATE {
  DRAM_INIT, /**< initialized */
  DRAM_CMD, /**< command ready */
  DRAM_CMD_WAIT, /**< wait command serviced */
  DRAM_DATA, /**< data ready to send */
  DRAM_DATA_WAIT, /**< sending data */
}; 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief dram request entry class
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct drb_entry_s {
  static int  m_unique_id;      /**< unique drb entry id */
  int         m_id;             /**< drb entry id */
  int         m_state;          /**< state */
  Addr        m_addr;           /**< request address */
  int         m_bid;            /**< bank id */
  int         m_rid;            /**< row id */
  int         m_cid;            /**< column id */
  int         m_core_id;        /**< core id */
  int         m_thread_id;      /**< thread id */
  int         m_appl_id;        /**< application id */
  bool        m_read;           /**< load/store */
  mem_req_s  *m_req;            /**< memory request pointer */
  int         m_priority;       /**< priority */
  int         m_size;           /**< size */
  Counter     m_timestamp;      /**< last touched cycle */
  Counter     m_scheduled;      /**< scheduled cycle */
  macsim_c*   m_simBase;        /**< macsim_c base class for simulation globals */
  // m_type;
  // m_core_type;

  /**
   * constructor
   *  \param simBase - Pointer to base simulation class for perf/stat counters
   */
  drb_entry_s(macsim_c* simBase);

  /**
   * set a new entry
   */
  void set(mem_req_s* req, int bid, int rid, int cid);

  /**
   * reset the entry
   */
  void reset();
} drb_entry_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Base dram scheduling class (FCFS)
///////////////////////////////////////////////////////////////////////////////////////////////
class dram_controller_c
{
  public:
    /**
     * Constructor.
     *  \param simBase - Pointer to base simulation class for perf/stat counters
     */
    dram_controller_c(macsim_c* simBase);

    /**
     * virtual destructor.
     */
    virtual ~dram_controller_c();
    
    /**
     * Initialize a dram controller with network id and controller id.
     */
    void init(int id, int noc_id);

    // controller
    /**
     * Insert a new request from the memory system.
     */
    bool insert_new_req(mem_req_s* mem_req);

    /**
     * Insert a new request to dram request buffer (DRB).
     */
    void insert_req_in_drb(mem_req_s* req, int bid, int rid, int cid);

    /**
     * Tick a cycle.
     */
    void run_a_cycle();
    
    /**
     * Create the network interface
     */
    void create_network_interface(void);
    
    /**
     * Print requests in the buffer
     */
    void print_req(void);

#ifdef DRAMSIM
    void read_callback(unsigned, uint64_t, uint64_t);
    void write_callback(unsigned, uint64_t, uint64_t);
#endif
    
  public:
    #define DRAM_REQ_PRIORITY_COUNT 12
    #define DRAM_STATE_COUNT 5
    static int dram_req_priority[DRAM_REQ_PRIORITY_COUNT]; /**< dram request priority */
    static const char* dram_state[DRAM_STATE_COUNT]; /**< dram state string */

  protected:
    /**
     * Schedule each bank.
     */
    void bank_schedule();

    /**
     * When the previous request has been serviced, pick a new one based on the policy.
     */
    void bank_schedule_new();

    /**
     * Check whether previous request has been completed.
     */
    void bank_schedule_complete();

    /**
     * Pick the highest priority entry based one the policy.
     * Each dram scheduling policy should override this function.
     */
    virtual drb_entry_s* schedule(list<drb_entry_s*>* drb_list);

    /**
     * Schedule each dram channel
     */
    void channel_schedule();

    /**
     * Pick a command from banks
     */
    void channel_schedule_cmd();

    /**
     * Pick a data ready bank
     */
    void channel_schedule_data();

    /**
     * Check whether data bus of a channel is available
     */
    bool avail_data_bus(int);

    /**
     * Acquire data bus access
     */
    Counter acquire_data_bus(int, int, bool gpu_req);

    /**
     * When a buffer is full, flush all prefetches in the buffer
     */
    void flush_prefetch(int bid);

    /**
     * Check the progress of dram controller.
     * Although there are requests, if no request has been serviced for certain cycles
     * raise exception.
     */
    void progress_check();

    /**
     * Send a packet to the NoC
     */
    void send_packet(void);

    /**
     * Receive a packet from the NoC
     */
    void receive_packet(void);

    /**
     * Function to do any book-keeping that might be needed by scheduling policies
     */
    virtual void on_insert(mem_req_s* req, int bid, int rid, int cid);

    /**
     * Function to do any book-keeping that might be needed by scheduling policies
     */
    virtual void on_complete(drb_entry_s* req);

    /**
     * Function to do any book-keeping that might be needed by scheduling policies
     */
    virtual void on_run_a_cycle();

  protected:
    list<drb_entry_s*> *m_buffer; /**< Dram request buffer (DRB) */
    list<drb_entry_s*> *m_buffer_free_list; /**< DRB free list */
    drb_entry_s** m_current_list; /**< Currently servicing request in each DRB */
    int* m_current_rid; /**< Current open row id */
    Counter* m_bank_ready; /**< bank ready cycle */
    Counter* m_data_ready; /**< data ready cycle */
    Counter* m_data_avail; /**< data avail cycle */
    Counter* m_bank_timestamp; /**< last touched cycle of a bank */
    int m_bus_width; /**< dram data bus width */
    int* m_byte_avail;  /**< number of available bytes of data bus */
    Counter* m_dbus_ready; /**< bus ready cycle */
    int m_num_bank; /**< number of dram banks */
    int m_num_channel; /**< number of dram channels */
    int m_num_bank_per_channel;  /**< number of banks per channel */
    uns m_cid_mask; /**< column id mask */
    uns m_bid_mask; /**< bank id mask */
    uns m_bid_shift; /**< bank id shift */
    uns m_rid_shift; /**< row id shift */
    uns m_bid_xor_shift; /**< bank id xor factor */

    int m_id; /**< dram controller id */
    int m_noc_id; /**< network id */
    int m_num_completed_in_last_cycle; /**< number of requests completed in last cycle */
    int m_starvation_cycle; /**< number of cycles without completed requests*/
    int m_total_req; /**< total pending requests */

    // latency
    Counter m_cycle; /**< dram clock cycle */
    int m_activate_latency; /**< activate latency */
    int m_precharge_latency; /**< precharge latency */
    int m_column_latency; /**< column access latency */

    // interconnection
#ifdef IRIS
    ManifoldProcessor* m_terminal; /**< connects to Iris interface->router */
#endif
    router_c* m_router; /**< router */
    list<mem_req_s*>* m_output_buffer; /**< output buffer */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

#ifdef DRAMSIM
    list<mem_req_s*>* m_pending_request; /**< pending request */
    DRAMSim::MultiChannelMemorySystem* dramsim_instance; /**< dramsim2 instance */
#endif
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief FR-FCFS dram scheduling
///////////////////////////////////////////////////////////////////////////////////////////////
class dc_frfcfs_c : public dram_controller_c
{
  /**
   * \brief dc_frfcfs_c sort function
   */
  class sort_func {
    public:
      /**
       * Constructor
       */

      sort_func(dc_frfcfs_c *);
      /**
       * Operator ()
       */
      bool operator() (const drb_entry_s* req_a, const drb_entry_s* req_b);

      dc_frfcfs_c* m_parent; /**< parent dram scheduler pointer */
  };

  friend class sort_func;

  public:
    /**
     * Constructor
     */
    dc_frfcfs_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~dc_frfcfs_c();

    /**
     * Overloaded schedule function
     * @param drb_list - dram request buffer to be scheduled
     */
    drb_entry_s* schedule(list<drb_entry_s*> *drb_list);

  private:
    class sort_func* m_sort; /**< sort function */
};


// wrapper function to allocate a dram scheduler
dram_controller_c* fcfs_controller(macsim_c* simBase);
dram_controller_c* frfcfs_controller(macsim_c* simBase);

#endif
