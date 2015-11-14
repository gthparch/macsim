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


#include <cassert>
#include <fstream>
#include <zlib.h>
#include <cstring>

#include "all_knobs.h"
#include "knob.h"
#include "trace_read.h"
#include "trace_reader.h"
  

all_knobs_c* g_knobs;

int read_trace(string trace_path, int truncate_size)
{
  string base_filename = trace_path.substr(0, trace_path.find_last_of("."));
  ifstream trace_file(trace_path.c_str());

  /*
  if (trace_file.fail()) {
    cout << "> error: trace file does not exist!\n";
    exit(0);
  }
  */

  int num_thread;
  string type;
  int max_block_per_core;
  int inst_count = 0;
  int load_count = 0;

  // read number of threads and type of trace
// trace_file >> num_thread >> type;
  trace_file >> type>> num_thread ;
  if (type == "newptx") {
    trace_file >> max_block_per_core;
#ifndef GPU_TRACE
    assert(0);
#endif 
  }
  else if (type == "x86")  { 
#ifndef X86_TRACE
    assert(0);
#endif 
  }
  else if (type == "a64") { 
#ifndef ARM64_TRACE
    assert(0);
#endif 
 
  }


  // open each thread trace file
  for (int ii = 0; ii < num_thread; ++ii) {
    int tid;
    int start_inst_count;
    int cur_file_inst_count = 0; 
    int slice_file_num = 0; 

    inst_count = 0; //reset inst count for each thread

    // set up thread trace file name
    trace_file >> tid >> start_inst_count;

    stringstream sstr;
    stringstream wsstr;

    sstr << base_filename << "_" << tid << ".raw";
    wsstr << base_filename << "_s" << slice_file_num << "_" << tid << ".raw";
    string thread_filename;
    sstr >> thread_filename;
    
    string wthread_filename;
    wsstr >> wthread_filename;

    cout << "thread_filename: " << thread_filename.c_str() << endl; 
    cout << "thread_write_filename: " << wthread_filename.c_str() << endl; 
    // open thread trace file
    gzFile gztrace = gzopen(thread_filename.c_str(), "r");
    gzFile gzwtrace = gzopen(wthread_filename.c_str(), "w");

    const int trace_buffer_size = 100000;
    char trace_buffer[trace_buffer_size * TRACE_SIZE];



    trace_reader_c::Singleton.reset();
    while (1) {
      int byte_read = gzread(gztrace, trace_buffer, trace_buffer_size * TRACE_SIZE);

      byte_read /= TRACE_SIZE;
      inst_count += byte_read;

      for (int jj = 0; jj < byte_read; ++jj) {


#if defined(GPU_TRACE) 
	trace_info_gpu_small_s trace_info; 
#elif defined(ARM64_TRACE)  
	trace_info_a64_s trace_info; 
#else 
	trace_info_cpu_s trace_info;
#endif


        memcpy(&trace_info, &trace_buffer[jj*TRACE_SIZE], TRACE_SIZE);
        trace_reader_c::Singleton.inst_event(&trace_info);

#if defined (GPU_TRACE) 
	if (trace_info.m_is_load >=1) 
#else 
        if (trace_info.m_num_ld >= 1)
#endif 
          ++load_count;

	
      } 


      /* generate multiple files of traces */ 
      
      gzwrite(gzwtrace,trace_buffer,(byte_read * TRACE_SIZE)); 
      cur_file_inst_count  += byte_read; 

      if (truncate_size != 0 ) { 
	
	if (cur_file_inst_count >= truncate_size) { 
	  gzclose(gzwtrace); 
	  // open a new file for next file 
	  cur_file_inst_count = 0; 
	  slice_file_num = slice_file_num + 1; 
	  stringstream wsstr_;
	  wsstr_ << base_filename << "_s" << slice_file_num << "_" << tid << ".raw";
	  string wthread_filename_; 
	  wsstr_ >> wthread_filename_;
	  cout << "new thread_write_filename: " << wthread_filename_.c_str() << endl; 
	  gzwtrace = gzopen(wthread_filename_.c_str(), "w");
	}
	
      }
      
      if (byte_read != trace_buffer_size) {
        break;
      } 
    }
    
    gzclose(gztrace);
    gzclose(gzwtrace);

  }
  cout << "> trace_path: " << trace_path << " inst_count: " << inst_count << " load_count:" << load_count << "\n";

  return inst_count;
}


void register_trace_reader(void)
{
  trace_reader_c::Singleton.init();
}


int main(int argc, char* argv[])
{
  KnobsContainer* knob_container = new KnobsContainer();
  g_knobs = knob_container->getAllKnobs();
  knob_container->applyParamFile("params.in");
  char* pInvalidArgument = NULL;
  knob_container->applyComandLineArguments(argc-1, &argv[1], &pInvalidArgument);
  int truncate_size = 0; 

  register_trace_reader();

  if (argc < 2) {
    cout << "> error: specify trace path\n";
    exit(0);
  }

  if (argc  == 3) { 
    truncate_size = atoi(argv[2]); 
  }
  string trace_path(argv[1]);
  cout << "> trace_path: " << trace_path << " trunkcate_size: " << truncate_size << "\n";

  string base_filename = trace_path.substr(0, trace_path.find_last_of("."));
  ifstream trace_file(trace_path.c_str());

  if (trace_file.fail()) {
    cout << "> error: trace file does not exist!\n";
    exit(0);
  }

  int num_thread;
  string type;
  int max_block_per_core;

  // read number of threads and type of trace
  trace_file >> num_thread >> type;

  int64_t inst_count = 0;
  /* if (type == "newptx") {
    while (trace_file >> trace_path) {
      inst_count += read_trace(trace_path);
    }
  }
  else {
  */
    inst_count += read_trace(trace_path, truncate_size);
    trace_file.close();
  // }


  cout << "> Total instruction count: " << inst_count << "\n";


  return 0;
}
