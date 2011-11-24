#ifndef TRACE_READER_H
#define TRACE_READER_H

#include "trace_read.h"
#include "global_types.h"
#include <vector>
#include <unordered_map>


extern all_knobs_c* g_knobs;


class trace_reader_c
{
  public:
    trace_reader_c();
    virtual ~trace_reader_c();
    virtual void inst_event(trace_info_s* inst);
    virtual void print();
    virtual void reset();

    void init();
    static trace_reader_c Singleton;

  protected:
    std::vector<trace_reader_c*> m_tracer;
    std::string m_name;
};


class reuse_distance_c : public trace_reader_c
{
  public:
    reuse_distance_c();
    ~reuse_distance_c();
    void inst_event(trace_info_s* inst);
    void print();
    void reset();

  private:
    int m_self_counter;
    std::unordered_map<Addr, int> m_reuse_map;
    std::unordered_map<Addr, int> m_reuse_pc_map;
    std::map<Addr, bool> m_static_pc;
    std::unordered_map<Addr, std::unordered_map<Addr, std::unordered_map<int, bool> *> *> m_result;
};


class static_pc_c : public trace_reader_c
{
  public:
    static_pc_c();
    ~static_pc_c();
    void inst_event(trace_info_s* inst);


  private:
    std::unordered_map<Addr, bool> m_static_pc;
    std::unordered_map<Addr, bool> m_static_mem_pc;
    uint64_t m_total_inst_count;
    uint64_t m_total_store_count;
};


#endif
