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
 * File         : memory.h
 * Author       : Jaekyu Lee
 * Date         : 3/4/2011
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : memory system
 *********************************************************************************************/

#ifndef MEMORY_H
#define MEMORY_H

#include <functional>

#include "memreq_info.h"
#include "pref_common.h"
#include "uop.h"
#include "macsim.h"

#define MemType_Prefetch(x) (x >= MEM_SWPREF_NTA && x <= MEM_SWPREF_T2)
#define MRT_Prefetch(x) (x >= MRT_SW_DPRF && x <= MRT_SW_DPRF_T2)

///////////////////////////////////////////////////////////////////////////////////////////////

enum COHERENCE_STATE {
  I_STATE,
  M_STATE,
  S_STATE,
};

bool IsStore(Mem_Type type);
bool IsLoad(Mem_Type type);
bool dcache_fill_line_wrapper(mem_req_s* req);

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief data cache data structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct dcache_data_s {
  bool m_dirty; /**< line dirty */
  Counter m_fetch_cycle; /**< fetched cycle */
  int m_core_id; /**< core id */
  Addr m_pc; /**< pc address */
  int m_tid; /**< thread id */
} dcache_data_s;

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Memory queue class
///////////////////////////////////////////////////////////////////////////////////////////////
class queue_c
{
  /**
   * Queue sort function based on the priority
   */
  struct sort_func {
    macsim_c* m_simBase; /**< pointer to the base simulation class */
    /**
     * Assign a base class pointer
     */
    sort_func(macsim_c* simBase) {
      m_simBase = simBase;
    };
    bool operator()(mem_req_s* a, mem_req_s* b); /**< comparison function */
  }; /**< sort function */

public:
  /**
   * Memory queue constructor
   */
  queue_c(macsim_c* simBase, int size);

  /**
   * Memory queue destructor
   */
  ~queue_c();

  /**
   * Search entries with the given address
   */
  mem_req_s* search(Addr addr, int size);

  /**
   * Search the list with the given request
   */
  bool search(mem_req_s*);

  /**
   * Delete a request
   */
  void pop(mem_req_s* req);

  /**
   * Insert a new request
   */
  bool push(mem_req_s* req);

  /**
   * Check buffer availability
   */
  bool full();

private:
  queue_c();  // Do not implement

public:
  list<mem_req_s*> m_entry; /**< queue entries */

private:
  unsigned int m_size; /**< queue size */
  macsim_c* m_simBase; /**< macsim_c base class for simulation globals */
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Data cache class
///////////////////////////////////////////////////////////////////////////////////////////////
class dcu_c
{
public:
  /**
   * data cache constructor
   */
  dcu_c(int id, Unit_Type type, int level, memory_c* mem, int, dcu_c** next,
        dcu_c** prev, macsim_c* simBase);

  /**
   * data cache destructor
   */
  ~dcu_c();

  /**
   * Initialize a data cache
   * @param next_id next-level cache id
   * @param prev_id prev-level cache id
   * @param done decide whether corresponding done_func is callable
   * @param coupled_up connected to upper-level cache
   * @param coupled_down connected to lower-level cache
   * @param disable disabled cache (in case of 2-level cache hierarchy)
   * @param has_router decide the next destination (NoC or next cache)
   */
  void init(int next_id, int prev_id, bool done, bool coupled_up,
            bool coupled_down, bool disable, bool has_router);

  /**
   * Get the cache line address
   */
  Addr base_addr(Addr addr);

  /**
   * Get the cache line size
   */
  int line_size();

  /**
   * Get the bank id
   */
  int bank_id(Addr addr);

  /**
   * Acquire data cache read port
   */
  bool get_read_port(int bank_id);

  /**
   * Access data cache
   */
  int access(uop_c* uop);

  /**
   * Insert a request to fill_queue (FILL REQ)
   */
  bool fill(mem_req_s* req);

  /**
   * Fill a cache line
   */
  bool done(mem_req_s* req);

  /**
   * Function for write ack
   */
  bool write_done(mem_req_s* req);

  /**
   * Insert a request to in_queue (NEW REQ)
   */
  bool insert(mem_req_s* req);

  /**
   * Tick a cycle
   */
  void run_a_cycle(bool);

  /**
   * Check available buffer space
   */
  bool full();

  /**
   * Access the data cache
   */
  dcache_data_s* access_cache(Addr addr, Addr* line_addr, bool update,
                              int appl_id);

  /**
   * Search a prefetch request from queues
   */
  mem_req_s* search_pref_in_queue();

  /**
   * Create the network interface
   */
  bool create_network_interface(int);

  /**
   * Send a packet to the NoC
   * @param req - memory request to be sent
   * @param msg - message type
   * @param dir - direction
   */
  bool send_packet(mem_req_s* req, int msg, int dir);

  /**
   * Receive a packet from the NoC
   */
  void receive_packet(void);

  /**
   * Invalidate cache lines of the given page
   */
  void invalidate(Addr page_addr);

private:
  /**
   * data cache default constructor
   */
  dcu_c();  // do not implement

  /**
   * Process requests from in_queue
   */
  void process_in_queue();

  /**
   * Process requests from out_queue
   */
  void process_out_queue();

  /**
   * Process requests from fill_queue
   */
  void process_fill_queue();

  /**
   * Process requests from wb_queue
   */
  void process_wb_queue();

private:
  int m_id; /**< cache id */
  int m_noc_id; /**< cache network id */
  int m_level; /**< cache level (L1, L2, LLC, or memory controller) */
  bool m_disable; /**< disabled */
  bool m_bypass; /**< bypass cache */
  cache_c* m_cache; /**< cache structure */
  port_c** m_port; /**< cache port */
  int m_next_id; /**< next-level cache id */
  dcu_c** m_next; /**< next-level cache pointer */
  int m_prev_id; /**< previous-level cache id */
  dcu_c** m_prev; /**< previous-level ache pointer */
  bool m_coupled_up; /**< directly connected to upward cache w/o NoC */
  bool m_coupled_down; /**< directly connected to downward cache w/o NoC */
  bool m_has_router; /**< has network router in this level */
  bool m_done; /**< done_func can be called */
  Unit_Type m_type; /**< core type that this cache belongs to */
  int m_num_set; /**< number of cache sets */
  int m_assoc; /**< cache associativity */
  int m_line_size; /**< cache line size */
  int m_banks; /**< number of cache banks */
  int m_latency; /**< cache access latency */
  bool m_acc_sim; /**< gpu cache */
  bool m_igpu_sim; /**< intel gpu cache */
  bool m_ptx_sim; /**< gpu cache */
  queue_c* m_in_queue; /**< input queue */
  queue_c* m_wb_queue; /**< write-back queue */
  queue_c* m_fill_queue; /**< fill queue */
  queue_c* m_out_queue; /**< out queue */
  bool m_req_llc_bypass; /**< bypass llc */
  int m_num_read_port; /**< number of read ports */
  int m_num_write_port; /**< number of write ports */

  list<mem_req_s*> m_retry_queue;

  memory_c* m_memory; /**< pointer to the memory system */
  macsim_c* m_simBase; /**< macsim_c base class for simulation globals */

  // clock
  Counter m_cycle; /**< clock cycle */
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief memory system
///////////////////////////////////////////////////////////////////////////////////////////////
class memory_c
{
public:
  /**
   * Constructor
   */
  memory_c(macsim_c* simBase);

  /**
   * Destructor
   */
  virtual ~memory_c() = 0;

  /**
   * Generate a new memory request
   * @param type - memory request type
   * @param addr - memory request address
   * @param size - memory request size
   * @param cache_hit - was request a hit in L1?
   * @param with_data - does request include data as well?
   * @param delay - delay
   * @param uop - request generating uop
   * @param done_func - done function
   * @param unique_num - uop unique number
   * @param pref_info - prefetch information structure
   * @param core_id - core id
   * @param thread_id - thread id
   * @param ptx - GPU request
   * @return false, if mshr full or l2 queue full
   * @return true, otherwise
   */
  bool new_mem_req(Mem_Req_Type type, Addr addr, uns size, bool cache_hit,
                   bool with_data, uns delay, uop_c* uop,
                   function<bool(mem_req_s*)> done_func, Counter unique_num,
                   pref_req_info_s* pref_info, int core_id, int thread_id,
                   bool ptx);

  /**
   * Access first-level cache from execution stage
   * @return 0, if not accessed correctly
   * @return -1, generate a new memory request
   * @return L1_latency, if hit
   */
  int access(uop_c* uop);

  /**
   * Return base line address
   */
  Addr base_addr(int core_id, Addr addr);

  /**
   * Return cache line size
   */
  int line_size(int core_id);

  /**
   * Return cache bank id
   */
  int bank_id(int core_id, Addr addr);

  /**
   * Acquire dcache read port with the specified bank
   */
  bool get_read_port(int core_id, int bank_id);

  /**
   * Tick a cycle for the memory system
   */
  void run_a_cycle(bool);

  /**
   * Tick a cycle for L1/L2 (core, private) caches
   */
  void run_a_cycle_core(int, bool);

  /**
   * Tick a cycle for LLC cache
   */
  void run_a_cycle_uncore(bool);

  /**
   * Deallocate completed memory request
   */
  void free_req(int core_id, mem_req_s* req);

  /**
   * Deallocate completed write request
   */
  void free_write_req(mem_req_s* req);

  /**
   * Receive a message from NoC
   */
  bool receive(int src, int dst, int msg, mem_req_s* req);

  /**
   * Using level and its id, get network id
   */
  int get_dst_id(int level, int id);

  /**
   * Using level and its id, get destination router id
   */
  int get_dst_router_id(int level, int id);

  /**
   * Using network id, get level and its id
   */
  void get_level_id(int noc_id, int* level, int* id);

  /**
   * Cache line fill function
   */
  bool done(mem_req_s* req);

  /**
   * Function for write ack
   */
  bool write_done(mem_req_s* req);

  /**
   * Access L1 data cache
   */
  dcache_data_s* access_cache(int core_id, Addr addr, Addr* line_addr,
                              bool update, int appl_id);

  /**
   * Generate a new write-back request
   */
  mem_req_s* new_wb_req(Addr addr, int size, bool ptx, dcache_data_s* data,
                        int level);

  /**
   * Print all entries in MSHR
   */
  void print_mshr(void);

  /**
   * Check available entry in mshr
   */
  int get_num_avail_entry(int core_id);

  /**
   * Initialize the memory system. \n
   * Setup interconnection network interface.
   */
  void init(void);

  /**
   * Handle coherence traffic (currently empty, will support coherence soon)
   */
  void handle_coherence(int level, bool hit, bool store, Addr addr,
                        dcu_c* cache);

  /**
   * Invalidate cache lines of the given page
   */
  virtual void invalidate(Addr page_addr);

public:
  static int m_unique_id; /**< unique memory request id */

  int* m_iris_node_id; /**< noc id for iris network nodes */

protected:
  /**
   * Allocate a new request from free list
   */
  mem_req_s* allocate_new_entry(int core_id);

  /**
   * Initialize a new request
   */
  void init_new_req(mem_req_s* req, Mem_Req_Type type, Addr addr, int size,
                    bool with_data, int delay, uop_c* uop,
                    function<bool(mem_req_s*)> done_func, Counter unique_num,
                    Counter priority, int core_id, int thread_id, bool ptx);

  /**
   * Adjust a new request. In case of finding matching entry, we need to adjust
   * fields of the matching request
   */
  void adjust_req(mem_req_s* req, Mem_Req_Type type, Addr addr, int size,
                  int delay, uop_c* uop, function<bool(mem_req_s*)> done_func,
                  Counter unique_num, Counter priority, int core_id,
                  int thread_id, bool ptx);

  /**
   * Search a request from queues
   */
  mem_req_s* search_req(int core_id, Addr addr, int size);

  /**
   * Set the level of each cache level
   */
  virtual void set_cache_id(mem_req_s* req);

  /**
   * When MSHR is full, try to evict a prefetch request
   */
  mem_req_s* evict_prefetch(int core_id);

  /**
   * Flush all prefetches in MSHR
   */
  void flush_prefetch(int core_id);

protected:
  dcu_c** m_l1_cache; /**< L1 caches */
  dcu_c** m_l2_cache; /**< L2 caches */
  dcu_c** m_l3_cache; /**< L3 caches */
  dcu_c** m_llc_cache; /**< LLC caches */
  list<mem_req_s*>* m_mshr; /**< mshr entry per L1 cache */
  list<mem_req_s*>* m_mshr_free_list; /**< mshr entry free list */
  int m_num_core; /**< number of cores */
  int m_num_cpu;
  int m_num_gpu;
  int m_num_l3; /**< number of L3 caches */
  int m_num_llc; /**< number of LLC caches */
  int m_num_mc; /**< number of memory controllers */
  int m_noc_index_base[MEM_LAST]; /**< component id of each memory hierarchy */
  int m_noc_id_base[MEM_LAST]; /**< noc id base per level */
  Counter m_stop_prefetch; /**< when set, no prefetches will be inserted */
  int m_l3_interleave_factor; /**< mask bit for L3 id */
  int m_llc_interleave_factor; /**< mask bit for LLC id */
  int m_dram_interleave_factor; /**< mask bit for dram id */
  macsim_c* m_simBase; /**< macsim_c base class for simulation globals */
  long m_page_size;
  bool m_igpu_sim; /**< intel gpu */

  // cache coherence
  unordered_map<Addr, vector<bool>*>
    m_tag_directory; /**< oracle cache coherence table */
  unordered_map<Addr, bool>
    m_td_pending_req; /**< pending requests in tag directory */

  Counter m_cycle; /**< clock cycle */
  pool_c<mem_req_s>*
    m_mem_req_pool; /**< pool for write requests in ptx simulations */
};

#define NEW_MEMORY_CLASS(x) \
  class x : public memory_c \
  {                         \
  public:                   \
    x();                    \
    ~x();                   \
  };

#define NEW_MEMORY_CLASS_ID(x)         \
  class x : public memory_c            \
  {                                    \
  public:                              \
    x();                               \
    ~x();                              \
                                       \
  private:                             \
    void set_cache_id(mem_req_s* req); \
  };

/*
 * coupled cache : when access between two caches, no need to communicate via network router
 *                 if ids are matched. (L2[3] to LLC[3] : direct, L2[3] to LLC[5] : via noc
 * local cache : even though ids are not matched, there is command link to all caches
 */

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 3-Level, Coupled architecture (Intel Sandy Bridge)
///////////////////////////////////////////////////////////////////////////////////////////////
class llc_coupled_network_c : public memory_c
{
public:
  /**
   * constructor
   */
  llc_coupled_network_c(macsim_c* simBase);

  /**
   * Destructor
   */
  ~llc_coupled_network_c();
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 3-Level, Decoupled architecture (2D topology)
///////////////////////////////////////////////////////////////////////////////////////////////
class llc_decoupled_network_c : public memory_c
{
public:
  /**
   * Constructor
   */
  llc_decoupled_network_c(macsim_c* simBase);

  /**
   * Destructor
   */
  ~llc_decoupled_network_c();
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 2-Level, Local architecture (Intel Core 2)
///////////////////////////////////////////////////////////////////////////////////////////////
class l2_coupled_local_c : public memory_c
{
public:
  /**
   * Constructor
   */
  l2_coupled_local_c(macsim_c* simBase);

  /**
   * Destructor
   */
  ~l2_coupled_local_c();

private:
  /**
   * Set the level of each cache level
   */
  void set_cache_id(mem_req_s* req);
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief  No cache architecture (NVIDIA G80, G200)
///////////////////////////////////////////////////////////////////////////////////////////////
class no_cache_c : public memory_c
{
public:
  /**
   * Constructor
   */
  no_cache_c(macsim_c* simBase);

  /**
   * Destructor
   */
  ~no_cache_c();

private:
  /**
   * Set the level of each cache level
   */
  void set_cache_id(mem_req_s* req);
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 2-Level, L2 is accessed via NoC (NVIDIA Fermi)
///////////////////////////////////////////////////////////////////////////////////////////////
class l2_decoupled_network_c : public memory_c
{
public:
  /**
   * Constructor
   */
  l2_decoupled_network_c(macsim_c* simBase);

  /**
   * Destructor
   */
  ~l2_decoupled_network_c();

  /**
   * Invalidate cache lines of the given page
   */
  void invalidate(Addr page_addr);

private:
  /**
   * Set the level of each cache level
   */
  void set_cache_id(mem_req_s* req);
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 2-Level, L2 is accessed locally (Intel Core 2)
///////////////////////////////////////////////////////////////////////////////////////////////
class l2_decoupled_local_c : public memory_c
{
public:
  /**
   * Constructor
   */
  l2_decoupled_local_c(macsim_c* simBase);

  /**
   * Destructor
   */
  ~l2_decoupled_local_c();

private:
  /**
   * Set the level of each cache level
   */
  void set_cache_id(mem_req_s* req);
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief 2-Level, L3 & LLC are accessed via NoC (no L1, L2) (Intel GPU)
///////////////////////////////////////////////////////////////////////////////////////////////
class igpu_network_c : public memory_c
{
public:
  /**
   * constructor
   */
  igpu_network_c(macsim_c* simBase);

  /**
   * Destructor
   */
  ~igpu_network_c();

private:
  /**
   * Set the level of each cache level
   */
  void set_cache_id(mem_req_s* req);

  /**
   * Invalidate cache lines of the given page
   */
  void invalidate(Addr page_addr);
};
#endif
