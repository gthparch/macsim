// #include "trace_generator.h"


#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
#else
#include <inttypes.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <zlib.h>
#include <ostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cassert> 
#include <string> 
#include <ctype.h> 

#include "mem_trace.h"

#define MAX2(a, b)   ((a)>(b) ? (a): (b))
using namespace std;

void init_mem_op (trace_info_cpu_s *mem_op) {

  mem_op->m_num_read_regs = 1;
  mem_op->m_num_dest_regs = 1;
  mem_op->m_src[0] = 1;
  mem_op->m_dst[0] = 2;
  mem_op->m_cf_type = 0;
  mem_op->m_has_immediate = 0;
  mem_op->m_opcode = XED_CATEGORY_DATAXFER;
  mem_op->m_has_st = 0;
  mem_op->m_is_fp = 0;
  mem_op->m_write_flg  = 0;
  mem_op->m_num_ld = 1;
  mem_op->m_size = 2;
  mem_op->m_ld_vaddr2 = 0;
  mem_op->m_st_vaddr = 0;
  mem_op->m_instruction_addr  = 0x1111;
  mem_op->m_branch_target = 0;
  mem_op->m_mem_read_size = 4;
  mem_op->m_mem_write_size = 0;
  mem_op->m_rep_dir = 0;
  mem_op->m_actually_taken = 0;
}

void init_fence_op (trace_info_cpu_s *fence_op) {

  fence_op->m_num_read_regs = 0;
  fence_op->m_num_dest_regs = 0;
  fence_op->m_src[0] = 0;
  fence_op->m_dst[0] = 0;
  fence_op->m_cf_type = 0;
  fence_op->m_has_immediate = 0;
  fence_op->m_opcode = XED_CATEGORY_MISC;
  fence_op->m_has_st = 0;
  fence_op->m_is_fp = 0;
  fence_op->m_write_flg  = 0;
  fence_op->m_num_ld = 0;
  fence_op->m_size = 0;
  fence_op->m_ld_vaddr2 = 1;
  fence_op->m_st_vaddr = 0;
  fence_op->m_instruction_addr  = 0x1111;
  fence_op->m_branch_target = 0;
  fence_op->m_mem_read_size = 0;
  fence_op->m_mem_write_size = 0;
  fence_op->m_rep_dir = 0;
  fence_op->m_actually_taken = 0;
}

void set_mem_op(trace_info_cpu_s *mem_op, uint64_t inst_addr, uint64_t addr1, bool is_write) {

  mem_op->m_instruction_addr = inst_addr;
  mem_op->m_size = 2;

  if (is_write == 0 ) {
    // read operations
    mem_op->m_has_st = 0;
    mem_op->m_num_read_regs = 1;
    mem_op->m_num_dest_regs = 1;
    mem_op->m_src[0] = 1;
    mem_op->m_dst[0] = 2;
    mem_op->m_ld_vaddr1 = addr1;
    mem_op->m_num_ld = 1;
    mem_op->m_ld_vaddr2 = 0;
    mem_op->m_st_vaddr = 0;
    mem_op->m_mem_read_size = 4;
    mem_op->m_mem_write_size = 0;
  }
  else {
    // write operations
    mem_op->m_has_st = 1;
    mem_op->m_num_read_regs = 1;
    mem_op->m_num_dest_regs = 0;
    mem_op->m_src[0] = 1;
    mem_op->m_dst[0] = 0;
    mem_op->m_ld_vaddr1 = 0;
    mem_op->m_num_ld = 0;
    mem_op->m_ld_vaddr2 = 0;
    mem_op->m_st_vaddr = addr1;
    mem_op->m_mem_read_size = 0;
    mem_op->m_mem_write_size = 4;
  }
}

int main(int argc, char *argv[])
{
  int threadnum = 1; 
  /*
  ifstream fname='trace1.txt'; 
  trace_generation_mode = 0; 
  if (argc < 2) 
    printf("Usage: %s <dram-read-file>. Defalt is generating stream memory addresses \n"); 
  if (argc >= 3) {
    fname = 
    fname =(argv[1]); 
    fname << argv[1]; 
    trace_generation_mode = 1; 
  }
  if (argc > = 4)  { 
    fname = argv[1];
    fname << argv[1]; 
    threadnum = strtol(argv[2], NULL, 10); 
    trace_generation_mode = 1; 
  }
  */
  printf("generating traces \n");

  // read arguments 
  // number of threads 
  // input address trace file name 
  // output trace file name 
  // trace generation mode: 0 (default) sequential stream 1: read from a file 
  int trace_generation_mode = 1; 

  for (int threadid = 0; threadid < threadnum; threadid++) {

    stringstream sstream;
    sstream << "coh_trace.1" << "_" << threadid << ".raw";
    string file_name;
    sstream >> file_name;

    gzFile trace_output = NULL;
    // trace_output = gzopen("test_trace.1_0.raw", "w");
    trace_output = gzopen(file_name.c_str(), "w");

    trace_info_cpu_s *mem_trace = (trace_info_cpu_s *) malloc(sizeof(trace_info_cpu_s));
    trace_info_cpu_s *fence_trace = (trace_info_cpu_s *) malloc(sizeof(trace_info_cpu_s));

    memset(mem_trace, 0, sizeof(trace_info_cpu_s));
    memset(fence_trace, 0, sizeof(trace_info_cpu_s));

    init_mem_op(mem_trace);
    init_fence_op(fence_trace);

    //   char fname[256] = "trace1.txt"; 
    char fname[256] = "hmc_dram.trace"; 

    switch(trace_generation_mode) { 
    case 0: 
      /* default stream trace generation part */ 
      for (int ii = 0; ii < 1000; ii++) {
	uint64_t t_addr;
	static uint64_t inst_addr;
	
	if (inst_addr ==1024) inst_addr = 0;
	
#define INDEP_TRACE
#ifdef INDEP_TRACE
	t_addr = ii *64  + threadid*1024*1024;
#endif
#ifdef DEP_TRACE
	t_addr = ii*64;
#endif
	
	set_mem_op(mem_trace, (++inst_addr)*2, t_addr, 0);  // 0: read operation 1: write operation //
	gzwrite(trace_output, mem_trace,  sizeof(trace_info_cpu_s));
	
	if (((inst_addr%4) != 0) && (t_addr > 1024))  {
	  set_mem_op(mem_trace, (++inst_addr)*2+1, t_addr-1024,  1);
	  gzwrite(trace_output, mem_trace,  sizeof(trace_info_cpu_s));
	}
	
	if (((inst_addr%8) != 0))  {
	  gzwrite(trace_output, fence_trace,  sizeof(trace_info_cpu_s)); 
	}
      }

    case 1: {
      // file open 
      // while 
      ifstream myfile (fname);
      string line;
      uint64_t t_addr;  
      size_t pos; 
      int req_type = 0; 
      static uint64_t inst_addr;
      
      if (myfile.is_open()) { 
	while (getline(myfile, line)) {
	  // cout << line << '\n'; 
	  // t_addr = strtol(line.c_str(), &pos, 16); 
	  t_addr = std::stoul(line, &pos, 16); 
	  pos = line.find_first_not_of(' ', pos+1); 
	  
	  if (pos == string::npos || line.substr(pos)[0] == 'R') 
	    req_type = 0; // read 
	  else if (line.substr(pos)[0] == 'W') 
	    req_type = 1; // write 
	  cout << "addr:" << hex << t_addr; 
	  if (req_type == 1 ) cout << " R" << endl; 
	  else cout << " W" << endl; 

	  if (inst_addr > 1024) inst_addr = 0; 
	  if (req_type  == 0 ) set_mem_op(mem_trace, (++inst_addr)*2, t_addr, req_type);  // 0: read operation 1: write operation //
	  else set_mem_op(mem_trace, (++inst_addr)*2+1, t_addr, req_type);  // 0: read operation 1: write operation //
	  gzwrite(trace_output, mem_trace,  sizeof(trace_info_cpu_s));

	}
	myfile.close();
      }
      else 
	cout << " Unable to open file\n"; 
    }

    }

    gzclose(trace_output);

  }
}
