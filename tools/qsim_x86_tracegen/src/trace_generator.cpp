/*
Copyright (c) <2013>, <Georgia Institute of Technology> All rights reserved.

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

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <set>
#include <getopt.h>

#include <qsim.h>
#include <qsim-load.h>
#include <qsim-x86-regs.h>

#include "distorm.h"
#include "trace_generator.h"
#include "knob.h"
#include "all_knobs.h"


/////////////////////////////////////////////////////////////////////////////////////////
/// global variables
/////////////////////////////////////////////////////////////////////////////////////////

// knob variables
KnobsContainer *g_knobsContainer = NULL; /* < knob container > */
all_knobs_c    *g_knobs = NULL; /* < all knob variables > */

void init_knobs(int argc, char** argv)
{ 
	// Create the knob managing class
	g_knobsContainer = new KnobsContainer();

	// Get a reference to the actual knobs for this component instance
	g_knobs = g_knobsContainer->getAllKnobs();

        // Hack to check if -h or --help is passed. Regular knobs are processed by macsim's functions right now. (Macsim's knob parsing follows --knob_name=knob_value template. So -h or --help don't fit in there.) TODO: Consolidate this process. Macim's knob processing is quite unwieldy. 

        static struct option long_options[] =
        {
                {"help", no_argument, NULL, 'h'},
                {NULL, 0, NULL, 0}
        };
        
        char ch;
        // Suppress errors. They'll get caught later by macsim's functions. 
        opterr = 0;
        // loop over all of the options but check only for 'h'
        while ((ch = getopt_long(argc, argv, "h", long_options, NULL)) != -1){
            switch (ch){
                case 'h':
                    cout << endl << "Usage: "<<argv[0]<<" --knob_name=knob_value" <<endl<<endl;
                    g_knobs ->  print_knob_descriptions();
                    exit (1);
            }
        }

	// apply the supplied command line switches
	char* pInvalidArgument = NULL;
	g_knobsContainer->applyComandLineArguments(argc, argv, &pInvalidArgument);

	cout << "Knob assignments:" << endl;
        g_knobs->display();
	cout << endl;
}

void free_knobs(){
	if(g_knobsContainer) delete g_knobsContainer ;
}

void validate_knobs(){
	if (!KNOB(Knob_trace_mode)->getValue().compare("user") && !KNOB(Knob_trace_mode)->getValue().compare("kernel") && !KNOB(Knob_trace_mode)->getValue().compare("all")){
		cerr << "Invalid value --trace_mode=" << KNOB(Knob_trace_mode)->getValue() <<". Valid values are user, kernel or all." << endl;
		exit(1);	
	}
	
	// more checks to be added
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Sanity check
/////////////////////////////////////////////////////////////////////////////////////////
void sanity_check(void)
{ 
	// ----------------------------------------
	// check whether there have been changes in the pin XED enumerators
	// ----------------------------------------
	for (int ii = 0; ii < XED_CATEGORY_LAST; ++ii) {
		if (tr_opcode_names[ii] != xed_category_enum_t2str((xed_category_enum_t)ii)) {
			cout << ii << " " << tr_opcode_names[ii] << " " << xed_category_enum_t2str((xed_category_enum_t)ii) << "\n";
			cout << "-> Changes in XED_CATEGORY!\n";
			exit(1);
		}
	}
} 


// Is floating point register

inline bool XED_REG_is_fr (xed_reg_enum_t reg){
	return (	
			xed_reg_class(reg) == XED_REG_CLASS_X87 ||
			xed_reg_class(reg) == XED_REG_CLASS_XMM ||
			xed_reg_class(reg) == XED_REG_CLASS_YMM ||
			xed_reg_class(reg) == XED_REG_CLASS_MXCSR 
	       );
}



// Merge 13,32 and 64 bit versions of registers into one macro.

xed_reg_enum_t XED_REG_FullRegName(xed_reg_enum_t reg){
	switch(reg){
		case XED_REG_FLAGS: 
		case XED_REG_EFLAGS: 
			return XED_REG_RFLAGS;

		case XED_REG_AX: 
		case XED_REG_EAX:
		case XED_REG_AL:
		case XED_REG_AH: 
			return XED_REG_RAX; 

		case XED_REG_CX: 
		case XED_REG_ECX:
		case XED_REG_CL: 
		case XED_REG_CH: 
			return XED_REG_RCX; 

		case XED_REG_DX: 
		case XED_REG_EDX:
		case XED_REG_DL: 
		case XED_REG_DH: 
			return XED_REG_RDX; 

		case XED_REG_BX: 
		case XED_REG_EBX: 
		case XED_REG_BL: 
		case XED_REG_BH: 	    
			return XED_REG_RBX; 

		case XED_REG_SP: 
		case XED_REG_ESP: 
		case XED_REG_SPL: 
			return XED_REG_RSP; 

		case XED_REG_BP: 
		case XED_REG_EBP: 
		case XED_REG_BPL:
			return XED_REG_RBP; 

		case XED_REG_SI: 
		case XED_REG_ESI: 
		case XED_REG_SIL: 
			return XED_REG_RSI; 

		case XED_REG_DI: 
		case XED_REG_EDI: 
		case XED_REG_DIL: 
			return XED_REG_RDI; 

		case XED_REG_R8W: 
		case XED_REG_R8D: 
		case XED_REG_R8B: 
			return XED_REG_R8; 

		case XED_REG_R9W: 
		case XED_REG_R9D:
		case XED_REG_R9B: 
			return XED_REG_R9; 

		case XED_REG_R10W: 
		case XED_REG_R10D:
		case XED_REG_R10B: 
			return XED_REG_R10; 

		case XED_REG_R11W: 
		case XED_REG_R11D:
		case XED_REG_R11B: 
			return XED_REG_R11; 

		case XED_REG_R12W: 
		case XED_REG_R12D:
		case XED_REG_R12B: 
			return XED_REG_R12; 

		case XED_REG_R13W: 
		case XED_REG_R13D:
		case XED_REG_R13B: 
			return XED_REG_R13; 

		case XED_REG_R14W: 
		case XED_REG_R14D:
		case XED_REG_R14B: 
			return XED_REG_R14; 

		case XED_REG_R15W:   
		case XED_REG_R15D:
		case XED_REG_R15B: 
			return XED_REG_R15;

		case XED_REG_EIP: 
		case XED_REG_IP: 
			return XED_REG_RIP; 

		default:
			return reg;	

	};
}

bool check_trace_mode(Qsim::OSDomain::cpu_prot mode){
	if(
			((mode == Qsim::OSDomain::PROT_USER) && !KNOB(Knob_trace_mode)->getValue().compare("user")) ||
			((mode == Qsim::OSDomain::PROT_KERN) && !KNOB(Knob_trace_mode)->getValue().compare("kernel")) ||	
			!KNOB(Knob_trace_mode)->getValue().compare("all")
	  ){
		return true;
	}else {
		return false;
	}
}

using Qsim::OSDomain;

class TraceWriter {
	public:
		TraceWriter(OSDomain &osd, ostream &qsim_tracefile, const char * in) : 
			osd(osd), qsim_tracefile(qsim_tracefile), finished(false), infile(in), thread_count(0), g_total_inst_count(0)
	{
		for (unsigned i=0 ; i < MAX_THREADS; i++){
			prev_iaddr[i] = make_pair(0,0);
			curr_iaddr[i] = make_pair(0,0);
			g_enable_cpu_instrument[i] = true;
			g_inst_print_count[i] = 0;
			g_inst_count[i]= 0;
			g_inst_offset[i] = 0;
		}

		Qsim::load_file(osd, in);

		osd.set_inst_cb(this, &TraceWriter::inst_cb);
		osd.set_mem_cb(this, &TraceWriter::mem_cb);
		osd.set_app_end_cb(this, &TraceWriter::app_end_cb);
	}

		bool hasFinished() { return finished; }
		void inst_cb(int c, uint64_t v, uint64_t p, uint8_t l, const uint8_t *b, enum inst_type t);
		void mem_cb(int c, uint64_t v, uint64_t p, uint8_t s, int w);
		int int_cb(int c, uint8_t v);
		void io_cb(int c, uint64_t p, uint8_t s, int w, uint32_t v);
		int atomic_cb(int c);
		int app_end_cb(int c);
		void ThreadStart(unsigned cpu_id);
		void ThreadEnd(THREADID tid);
		void finish(void);
		void write_inst(va_pa_pair iaddr, THREADID tid);
		void write_inst_to_file(ofstream* file, Inst_info *t_info, THREADID tid);

	private:
		OSDomain &osd;
		ostream &qsim_tracefile;
		bool finished;
		std::ifstream infile;
		static const char *itype_str[];
		UINT32 thread_count ;
		va_pa_pair curr_iaddr[MAX_THREADS];
		va_pa_pair prev_iaddr[MAX_THREADS];
		Qsim::OSDomain::cpu_prot prev_mode[MAX_THREADS]; 
		UINT64 g_inst_count[MAX_THREADS];
		UINT64 g_inst_offset[MAX_THREADS];
		UINT64 g_total_inst_count;
		unsigned int g_inst_print_count[MAX_THREADS];
		bool g_enable_cpu_instrument[MAX_THREADS];
		map<va_pa_pair, Inst_info*> g_inst_storage[MAX_THREADS];
		Trace_info* trace_info_array[MAX_THREADS];
		queue<child_memop> child_q[MAX_THREADS];
		map <THREADID, THREADID> cpu_to_tid;
		map <THREADID, THREADID> tid_to_cpu;

};

const char *TraceWriter::itype_str[] = {
  "QSIM_INST_NULL",
  "QSIM_INST_INTBASIC",
  "QSIM_INST_INTMUL",
  "QSIM_INST_INTDIV",
  "QSIM_INST_STACK",
  "QSIM_INST_BR",
  "QSIM_INST_CALL",
  "QSIM_INST_RET",
  "QSIM_INST_TRAP",
  "QSIM_INST_FPBASIC",
  "QSIM_INST_FPMUL",
  "QSIM_INST_FPDIV"
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Instruction callback Function
/////////////////////////////////////////////////////////////////////////////////////////

void TraceWriter::inst_cb(int c, uint64_t v, uint64_t p, uint8_t l, const uint8_t *b, enum inst_type t){

	if (g_enable_cpu_instrument[c]){

		Qsim::OSDomain::cpu_prot curr_mode = osd.get_prot(c);

		if ((cpu_to_tid.find(c) == cpu_to_tid.end()) && (check_trace_mode(curr_mode))){
			if((check_trace_mode(curr_mode))){	
				ThreadStart(c);  
			}
		}
		
		
		if(cpu_to_tid.find(c) != cpu_to_tid.end()){	  

			THREADID tid = cpu_to_tid[c];

			const va_pa_pair iaddr = make_pair(v,p);
			curr_iaddr[tid] = iaddr;

			////////////////////////////////////////////////////////////////////////////////////////
                        ADDRINT prev_addr = KNOB(Knob_use_va)->getValue() ? prev_iaddr[tid].first: prev_iaddr[tid].second;
                        ADDRINT curr_addr = KNOB(Knob_use_va)->getValue() ? curr_iaddr[tid].first: curr_iaddr[tid].second;

			if (prev_addr){

				assert(g_inst_storage[tid].find(prev_iaddr[tid]) != g_inst_storage[tid].end());

				uint8_t prev_cf_type = g_inst_storage[tid][prev_iaddr[tid]]->cf_type;

				if(prev_cf_type != NOT_CF){
					switch(prev_cf_type){
						case CF_IBR:
						case CF_BR:
						case CF_ICALL:
						case CF_CALL:
						case CF_RET:
						case CF_ICO:
							g_inst_storage[tid][prev_iaddr[tid]] -> branch_target = curr_addr;
							g_inst_storage[tid][prev_iaddr[tid]] -> actually_taken = true;
							break;
						case CF_CBR:
							assert(g_inst_storage[tid][prev_iaddr[tid]] -> branch_target);
							if (g_inst_storage[tid][prev_iaddr[tid]] -> branch_target == curr_addr){
								g_inst_storage[tid][prev_iaddr[tid]] -> actually_taken = true;
							}else{
								g_inst_storage[tid][prev_iaddr[tid]] -> actually_taken = false;
							}
							break;
						case CF_ICBR:
							if (prev_addr + g_inst_storage[tid][prev_iaddr[tid]] -> size == curr_addr){
								//TODO: Find g_inst_storage[tid][prev_iaddr[tid]] -> branch_target for branches that are not taken. Edit: x86 doesn't have indirect conditional branches. Other ISAs might.
								g_inst_storage[tid][prev_iaddr[tid]] -> actually_taken = false;
							}else{
								g_inst_storage[tid][prev_iaddr[tid]] -> branch_target = curr_addr;
								g_inst_storage[tid][prev_iaddr[tid]] -> actually_taken = true;
							}
							break;
						default:
							assert(0);
					}
				}

				if(check_trace_mode(prev_mode[tid])){
					write_inst(prev_iaddr[tid],tid);

					if(KNOB(Knob_extra_memops)->getValue()){

						while (!child_q[tid].empty()){

							Inst_info* prev_inst = g_inst_storage[tid][prev_iaddr[tid]];
							child_memop dm = child_q[tid].front();
							if(dm.is_read){
								prev_inst -> num_ld = 1;
								prev_inst -> has_st = 0;
								prev_inst -> ld_vaddr1 = dm.addr;
								prev_inst -> mem_read_size = dm.size;
							}else{
								prev_inst -> num_ld = 0;
								prev_inst -> has_st = 1;
								prev_inst -> st_vaddr = dm.addr;
								prev_inst -> mem_write_size = dm.size;
							}

							child_q[tid].pop();

							// Change opcode before printing
							uint8_t prev_opcode = prev_inst -> opcode ;
							prev_inst -> opcode = CPU_MEM_EXT_OP; 
							write_inst(prev_iaddr[tid],tid);
							// Restore opcode
							prev_inst -> opcode = prev_opcode ;
						}
					}


					prev_iaddr[tid] = make_pair(0,0);

					g_inst_count[tid] ++ ; 	  
					g_total_inst_count ++ ;  

				}

				// If total max met, disable all threads. 
				// The callbacks will still get called till this iteration of while (!tw.hasFinished()) completes.
				if (KNOB(Knob_max)->getValue() !=0 && g_inst_count[tid] == KNOB(Knob_max)->getValue()){
					for (UINT32 ii = 0; ii < KNOB(Knob_num_cpus) -> getValue(); ++ii) {
						g_enable_cpu_instrument[ii] = 0;
					}
				}  
			}

			// ------------------------------------------------------------
			// add a static instruction (per thread id) if it doesn't exist
			// ------------------------------------------------------------
			if (g_inst_storage[tid].find(iaddr) == g_inst_storage[tid].end()) {

                                Inst_info *info = new Inst_info;
				memset(info,0,sizeof(*info));

				Trace_info* trace_info = trace_info_array[tid];
				set<xed_reg_enum_t> src_regs;
				set<xed_reg_enum_t> dst_regs;
				src_regs.clear();
				dst_regs.clear();

				xed_machine_mode_enum_t mmode;
				xed_address_width_enum_t stack_addr_width;
				// Hard coded right now since qsim only supports ia32
				xed_bool_t long_mode = 0;
				// The state of the machine -- required for decoding
				if (long_mode) {
					mmode=XED_MACHINE_MODE_LONG_64;
					stack_addr_width = XED_ADDRESS_WIDTH_64b;
				}
				else {
					mmode=XED_MACHINE_MODE_LEGACY_32;
					stack_addr_width = XED_ADDRESS_WIDTH_32b;
				}

				xed_error_enum_t xed_error;
				xed_uint_t noperands;
				const xed_inst_t* xi =  0;
				xed_decoded_inst_t xedd;
				xed_decoded_inst_zero(&xedd);
				xed_decoded_inst_set_mode(&xedd, mmode, stack_addr_width);
				xed_error = xed_decode(&xedd, XED_STATIC_CAST(const xed_uint8_t*,b), l);
                                
                                //TODO: There are some stray magic instruction calls that are getting trapped here. Fix them.
				switch(xed_error)
				{
					case XED_ERROR_NONE:
						break;
					case XED_ERROR_BUFFER_TOO_SHORT:
						cerr << "CPU ["<<c<<"]: Warning: Skipping instruction at address " << hex << "(VA: " <<iaddr.first << " PA: "<< iaddr.second << "). Not enough bytes provided" << endl;
						break;
					case XED_ERROR_GENERAL_ERROR:
						cerr << "CPU ["<<c<<"]: Warning: Skipping instruction at address " << hex << "(VA: " <<iaddr.first << " PA: "<< iaddr.second << "). Could not decode given input" << endl;
						break;
					default:
						cerr << "CPU ["<<c<<"]: Warning: Skipping instruction at address " << hex << "(VA: " <<iaddr.first << " PA: "<< iaddr.second << "). Unhandled error code " << xed_error_enum_t2str(xed_error) << endl;
						break;
				}

				if(xed_error == XED_ERROR_NONE){
					xi = xed_decoded_inst_inst(&xedd);
					xed_uint_t memops = xed_decoded_inst_number_of_memory_operands(&xedd);
					noperands = xed_inst_noperands(xi);  

					// ----------------------------------------
					// check all operands
					// ----------------------------------------

					for (UINT32 ii = 0; ii < noperands; ++ii) {
						// ----------------------------------------
						// operand - Register
						// add source and destination registers
						// ----------------------------------------
						const xed_operand_t* op = xed_inst_operand(xi,ii);
						xed_operand_enum_t opname = xed_operand_name(op);
						if (xed_operand_is_register(opname) || xed_operand_is_memory_addressing_register(opname)) {
							xed_reg_enum_t reg = xed_decoded_inst_get_reg(&xedd, opname);

							if((xed_reg_class(reg) !=  XED_REG_CLASS_PSEUDO) && (xed_reg_class(reg) != XED_REG_CLASS_PSEUDOX87) ){
								if (xed_operand_read (op)) {
									src_regs.insert(reg);
								}
								if (xed_operand_written(op)) {
									dst_regs.insert(reg);
								}
							}
						}
					}

					for (UINT32 ii = 0; ii < memops; ++ii) {

						// ----------------------------------------
						// operand - Memory
						// ----------------------------------------
						if (xed_decoded_inst_mem_read(&xedd,ii) || xed_decoded_inst_mem_written(&xedd,ii)){

							// Can have 2 loads per ins in some cases.(CMPS). This refers to explicit loads in the instruction.
							if (xed_decoded_inst_mem_read(&xedd,ii)) info->num_ld ++;
							assert(info->num_ld <=2 );	

							if (xed_decoded_inst_mem_written(&xedd,ii)) {
								//Can have only one st per ins
								assert(!info->has_st);
								info->has_st = 1;
							}

							// ---------------------------------------------------
							// Add base, index, or segment registers if they exist
							// --------------------------------------------------
							xed_reg_enum_t base = xed_decoded_inst_get_base_reg(&xedd,ii);
							if (base != XED_REG_INVALID) {
								src_regs.insert(base);
							}

							xed_reg_enum_t indx = xed_decoded_inst_get_index_reg(&xedd,ii);
							if (ii == 0 && indx != XED_REG_INVALID) {
								src_regs.insert(indx);
							}	    

							xed_reg_enum_t seg = xed_decoded_inst_get_seg_reg(&xedd,ii);
							if (seg != XED_REG_INVALID) {
								src_regs.insert(seg);
							}
						}else{
							//TODO: LEA instructions reach here. LEA is typically not a memory read, but in some cases (immediate operand) its behaviour can be similar to MOV. Not sure if LEA with imm reaches here.
						}			
					}

					// ----------------------------------------
					// handle source registers
					// ----------------------------------------

					if (!src_regs.empty()) {
						info->num_read_regs = src_regs.size();
						assert(info->num_read_regs < MAX_SRC_NUM);

						set<xed_reg_enum_t>::iterator begin(src_regs.begin()), end(src_regs.end());
						uint8_t *ptr = info->src;

						while (begin != end) {

							if (XED_REG_is_fr(*begin)){
								info->is_fp = true;
							}

							*ptr = XED_REG_FullRegName(*begin);
							*ptr = *begin;
							++ptr;
							++begin;
						}

					}


					// ----------------------------------------
					// destination registers
					// ----------------------------------------
					if (!dst_regs.empty()) {
						info->num_dest_regs=dst_regs.size();
						assert(info->num_dest_regs < MAX_DST_NUM);
						set<xed_reg_enum_t>::iterator begin(dst_regs.begin()), end(dst_regs.end());
						uint8_t *ptr = info->dst;
						while (begin != end) {

							if (xed_reg_class(*begin) == XED_REG_CLASS_FLAGS)
								info->write_flg = 1;

							if (XED_REG_is_fr(*begin))
								info->is_fp = 1;

							*ptr = XED_REG_FullRegName(*begin);
							*ptr = *begin;
							++ptr;
							++begin;
						}
					}

					// ----------------------------------------
					// instruction size
					// ----------------------------------------
					info->size = l;

					// ----------------------------------------
					// PC address
					// ----------------------------------------
					info->instruction_addr = KNOB(Knob_use_va)->getValue()? v : p;

					// ----------------------------------------
					// set the opcode
					// ----------------------------------------
					xed_iclass_enum_t inst_iclass = xed_decoded_inst_get_iclass(&xedd);
					xed_category_enum_t inst_category = xed_decoded_inst_get_category(&xedd);
					string inst_iclass_str(xed_iclass_enum_t2str(inst_iclass));

					if (inst_category == XED_CATEGORY_NOP || inst_category == XED_CATEGORY_WIDENOP ) {
						info->opcode = TR_NOP;      
					}
					// ----------------------------------------
					// opcode : multiply
					// ----------------------------------------
					else if (inst_iclass_str.find("MUL") != string::npos) {
						if (inst_iclass == XED_ICLASS_IMUL || inst_iclass == XED_ICLASS_MUL) {
							info->opcode = TR_MUL;
						} else {
							info->opcode = TR_FMUL;
						}
					}

					// ----------------------------------------
					// opcode : multiply
					// ----------------------------------------
					else if (inst_iclass_str.find("DIV") != string::npos) {
						if (inst_iclass == XED_ICLASS_DIV || inst_iclass == XED_ICLASS_IDIV) {
							info->opcode = TR_DIV;
						} else {
							info->opcode = TR_FDIV;
						}
					}

					// ----------------------------------------
					// opcode : prefetch
                                        // src: http://software.intel.com/en-us/articles/use-software-data-prefetch-on-32-bit-intel-architecture
					// ----------------------------------------
					else if (inst_iclass == XED_ICLASS_PREFETCHNTA) {
						info->opcode = PREFETCH_NTA;
                                                info->mem_read_size = 128;
					}
					else if (inst_iclass == XED_ICLASS_PREFETCHT0) {
						info->opcode = PREFETCH_T0;
					        info->mem_read_size = 128;
                                        }
					else if (inst_iclass == XED_ICLASS_PREFETCHT1) {
						info->opcode = PREFETCH_T1;
					        info->mem_read_size = 128;
                                        }
					else if (inst_iclass == XED_ICLASS_PREFETCHT2) {
						info->opcode = PREFETCH_T2;
					        info->mem_read_size = 128;
                                        }

					// ----------------------------------------
					// opcode : others
					// ----------------------------------------
					else {
						info->opcode = (uint8_t) (inst_category);
                                        }
                                        
					// ----------------------------------------
					// Branch instruction - set branch type
					// ----------------------------------------
					xed_uint_t disp_bits = xed_decoded_inst_get_branch_displacement_width(&xedd);
					if (disp_bits) {
						//Direct branch
						xed_int32_t disp = xed_decoded_inst_get_branch_displacement(&xedd);
						info->branch_target =(KNOB(Knob_use_va)->getValue() ? iaddr.first: iaddr.second) + l + disp ;
					}

					if (inst_category == XED_CATEGORY_UNCOND_BR ){
						if (disp_bits){
							info->cf_type = CF_BR;
						}else{
							info->cf_type = CF_IBR;
						}
					}else if (inst_category == XED_CATEGORY_COND_BR){
						if (disp_bits){
							info->cf_type = CF_CBR;
						}else{
							info->cf_type = CF_ICBR;
						}
					}else if (inst_category == XED_CATEGORY_CALL ) {
						if (disp_bits){
							info->cf_type = CF_CALL;
						}else{
							info->cf_type = CF_ICALL;
						}
					}else if (inst_category == XED_CATEGORY_RET) { 
						info->cf_type = CF_RET;
					}else if (inst_category == XED_CATEGORY_INTERRUPT){ 
						info->cf_type = CF_ICO;
					}else {
						assert(!disp_bits);
						info->cf_type = NOT_CF;
					}


					g_inst_storage[tid][iaddr] = info;

					prev_iaddr[tid] = curr_iaddr[tid];
					prev_mode[tid] = curr_mode;

				}else{
					delete info;
					prev_iaddr[tid] = make_pair(0,0);
				}
			}else{
				prev_iaddr[tid] = curr_iaddr[tid];
				prev_mode[tid] = curr_mode;
			}

		}
	}

	// Carried over from QSIM's trace generator and left as is. 

	_DecodedInst inst[15];
	unsigned int shouldBeOne;
	distorm_decode(0, b, l, Decode32Bits, inst, 15, &shouldBeOne);


	qsim_tracefile << std::dec << c << ": Inst@(0x" << std::hex << v << "/0x" << p 
		<< ", tid=" << std::dec << osd.get_tid(c) << ", "
		<< ((osd.get_prot(c) == Qsim::OSDomain::PROT_USER)?"USR":"KRN")
		<< (osd.idle(c)?"[IDLE]":"[ACTIVE]")
		<< "): " << std::hex;

	//while (l--) qsim_tracefile << ' ' << std::setw(2) << std::setfill('0') 
	//                      << (unsigned)*(b++);

	if (shouldBeOne != 1) qsim_tracefile << "[Decoding Error]";
	else qsim_tracefile << inst[0].mnemonic.p << ' ' << inst[0].operands.p;

	qsim_tracefile << " (" << itype_str[t] << ")\n";
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Memory callback Function
/////////////////////////////////////////////////////////////////////////////////////////

void TraceWriter::mem_cb(int c, uint64_t v, uint64_t p, uint8_t s, int w) {

	if (g_enable_cpu_instrument[c] && (cpu_to_tid.find(c) != cpu_to_tid.end())){

		THREADID tid = cpu_to_tid[c];

		//This check is needed to skip instructions that weren't decoded successfully for some reason.
		if(g_inst_storage[tid].find(curr_iaddr[tid]) != g_inst_storage[tid].end()){

			
                        Inst_info *info = g_inst_storage[tid][curr_iaddr[tid]];
                        
                        // Set the direction flag. Useful for MOVS/STOS 
                        info -> rep_dir =  ((osd.get_reg(c, QSIM_X86_RFLAGS) & (0x400)) >> 10); 
		        
                        if ((info -> num_ld) && !w){

				if(!info->ld_vaddr1) {
					assert(!info->ld_vaddr2);
					info->ld_vaddr1 = KNOB(Knob_use_va)->getValue() ? v: p;
					info->mem_read_size = s;

				}else{
					if(info -> num_ld == 2  && !info->ld_vaddr2){
						// There is only one size field. So, both loads should be of the same size, as in CMPS. 
						assert(info->mem_read_size == s);
						info->ld_vaddr2 = KNOB(Knob_use_va)->getValue() ? v: p;
					}

					// Note: The trace structure has room for only 2 ld addresses and 1 st addr per ins.
					// Certain instructions can have multiple loads and stores. 
					// Skip logging those right now unless the user explicitly enables them.

					if(KNOB(Knob_extra_memops)->getValue()){
						// Push a new child ins with the same PC address and print it later to the trace.

						child_memop dm;
						dm.addr = KNOB(Knob_use_va)->getValue() ? v: p;
						dm.size = s;
						dm.is_read = true; 
						child_q[tid].push(dm);
					}
				}

			}else if((info -> has_st) && w){
				if(!info->st_vaddr){
					info->st_vaddr = KNOB(Knob_use_va)->getValue() ? v: p;
					info->mem_write_size = s;
				}else{
					// Note: The trace structure has room for only 2 ld addresses and 1 st addr per ins.
					// Certain instructions can have multiple loads and stores. 
					// Skip logging those right now unless the user explicitly enables them.

					if(KNOB(Knob_extra_memops)->getValue()){
						// Push a new child ins with the same PC address and print it later to the trace.

						child_memop dm;
						dm.addr = KNOB(Knob_use_va)->getValue() ? v: p;
						dm.size = s;
						dm.is_read = false; 
						child_q[tid].push(dm);
					}
				}
			}else{
                                // We reach here when the instruction does not have explicit memory traffic, but it implicitly does mem RW/WR.
                                // For example, some JMP or CALL instructions which trigger state save/restore. 

                                // Note: The trace structure has room for only 2 ld addresses and 1 st addr per ins.
				// Certain instructions can have multiple loads and stores. 
				// Skip logging those right now unless the user explicitly enables them.

				if(KNOB(Knob_extra_memops)->getValue()){
					// Push a new child ins with the same PC address and print it later to the trace.

					child_memop dm;
					dm.addr = KNOB(Knob_use_va)->getValue() ? v: p;
					dm.size = s;

					if(w){
						dm.is_read = false; 
					}else{
						dm.is_read = true; 
					}

					child_q[tid].push(dm);
				}
			}
		}

	}

	// Carried over from QSIM's trace generator and left as is. 
	qsim_tracefile << std::dec << c << ":  " << (w?"WR":"RD") << "(0x" << std::hex
		<< v << "/0x" << p << "): " << std::dec << (unsigned)(s*8) 
		<< " bits.\n";

}

/////////////////////////////////////////////////////////////////////////////////////////
/// Interrupt callback Function
/////////////////////////////////////////////////////////////////////////////////////////

int TraceWriter::int_cb(int c, uint8_t v) {
	// Carried over from QSIM's trace generator and left as is. 

	qsim_tracefile << std::dec << c << ": Interrupt 0x" << std::hex << std::setw(2)
		<< std::setfill('0') << (unsigned)v << '\n';
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
/// IO callback Function
/////////////////////////////////////////////////////////////////////////////////////////

void TraceWriter::io_cb(int c, uint64_t p, uint8_t s, int w, uint32_t v) {
	// Carried over from QSIM's trace generator and left as is. 

	qsim_tracefile << std::dec << c << ": I/O " << (w?"RD":"WR") << ": (0x" 
		<< std::hex << p << "): " << std::dec << (unsigned)(s*8) 
		<< " bits.\n";
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Atomic callback Function
/////////////////////////////////////////////////////////////////////////////////////////

int TraceWriter::atomic_cb(int c) {
	qsim_tracefile << std::dec << c << ": Atomic\n";
	return 0;
}
	
/////////////////////////////////////////////////////////////////////////////////////////
/// Application end callback Function
/////////////////////////////////////////////////////////////////////////////////////////

int TraceWriter::app_end_cb(int c)   { 

    std::cout << "App end cb called." << std::endl;
	if(!finished){
		finished = true; 

		for (UINT32 ii = 0; ii < thread_count; ++ii) { 
			ThreadEnd(ii);
		}

		finish();
	}
	return 1; 
}
	
/////////////////////////////////////////////////////////////////////////////////////////
/// Thread Start Function
/// 1. Set trace file for each thread
/// 2. Set debug (dump) files for each thread
/////////////////////////////////////////////////////////////////////////////////////////

void TraceWriter::ThreadStart(unsigned cpu_id)
{
	assert(cpu_to_tid.find(cpu_id) == cpu_to_tid.end());
	cpu_to_tid[cpu_id] = thread_count;
	THREADID tid = cpu_to_tid[cpu_id];
	assert(tid_to_cpu.find(tid) == tid_to_cpu.end());
	tid_to_cpu[tid] = cpu_id;

	thread_count ++;

	cout << "CPU [" << cpu_id << "] -> Trace thread [" << tid << "] begins." << endl;

	struct Trace_info* trace_info = NULL;

	trace_info = new Trace_info;
	if (trace_info == NULL) {
		cerr << "could not allocate memory\n";
		return;
	}
	memset(trace_info, 0, sizeof(Trace_info));

	stringstream sstream;
	sstream << KNOB(Knob_trace_name) -> getValue() << "_" << tid << ".raw";
	string file_name;
	sstream >> file_name;

	gzFile trace_stream = NULL;
	trace_stream = gzopen(file_name.c_str(), "w");

	// DEBUG
	char debug_file_name[256] = {'\0'};

	ofstream* debug_stream = NULL;
	sprintf(debug_file_name, "%s_%d.dump", (KNOB(Knob_dump_file)->getValue()).c_str(), tid);
	debug_stream = new ofstream(debug_file_name);

	if (trace_stream != NULL) {
		trace_info->trace_stream = trace_stream;
		trace_info->bytes_accumulated = 0;

		if (KNOB(Knob_inst_dump)->getValue())
			trace_info->debug_stream = debug_stream;

		assert(tid < MAX_THREADS);

		trace_info_array[tid] = trace_info;

		g_inst_offset[tid] = g_inst_count[0];
	}
	else {
		cerr << "error opening file for writing\n";
		exit(1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Thread end function
/////////////////////////////////////////////////////////////////////////////////////////

void TraceWriter::ThreadEnd(THREADID tid)
{
	Trace_info* trace_info = NULL;
	trace_info = trace_info_array[tid];

	/**< dump out instructions that are not written yet when a thread is terminated */
	if (trace_info->bytes_accumulated > 0) {
		if (trace_info->bytes_accumulated != gzwrite(trace_info->trace_stream, trace_info->trace_buf, trace_info->bytes_accumulated)) {
			cerr << "TID " << tid << " Error when writting instruction " << g_inst_count[tid] << endl;
		}
	}

	/**< close trace file */
	gzclose(trace_info->trace_stream);

	/**< close dump file */
	if (KNOB(Knob_inst_dump)->getValue()){
		trace_info->debug_stream->close();
		delete trace_info->debug_stream;
	}

	/**< delete thread trace info */
	delete trace_info;
	
	/**< delete g_inst_storage */
	for(map<va_pa_pair, Inst_info*>::iterator it = g_inst_storage[tid].begin(); it != g_inst_storage[tid].end(); it ++ ){
		delete(it->second);
	}
	
	g_inst_storage[tid].erase(g_inst_storage[tid].begin(),g_inst_storage[tid].end());

}

/////////////////////////////////////////////////////////////////////////////////////////
/// Finalization
/// Create configuration file
/// Final print to standard output
/////////////////////////////////////////////////////////////////////////////////////////

void TraceWriter::finish(void)
{
	/**< Create configuration file */
	string config_file_name = KNOB(Knob_trace_name)->getValue() + ".txt";
	ofstream configFile;

	configFile.open(config_file_name.c_str());
        configFile << "x86\n";
        configFile << thread_count << endl;
	for (unsigned int ii = 0; ii < thread_count; ++ii) {
		// thread_id thread_inst_start_count (relative to main thread)
		configFile << ii << " " << g_inst_offset[ii] << endl;
	}
	configFile.close();


	/**< Final print to standard output */
	for (unsigned ii = 0; ii < thread_count; ++ii) {
		cout << "-> tid " << ii << " inst count " << g_inst_count[ii]
			<< " (from " << g_inst_offset[ii] << ")\n";
	}
	cout << "-> Exiting..." << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Write an instruction to the buffer 
/////////////////////////////////////////////////////////////////////////////////////////

void TraceWriter::write_inst(va_pa_pair iaddr, THREADID tid)
{
	Trace_info* trace_info = trace_info_array[tid];
	assert(g_inst_storage[tid].find(iaddr) != g_inst_storage[tid].end());
	Inst_info* static_inst = g_inst_storage[tid][iaddr];

	// Handle special cases for macsim before writing instructions.
        // INVLPG gets num_ld but no mem callback.

        if (static_inst -> opcode == XED_CATEGORY_SYSTEM){
            if (static_inst -> num_ld && static_inst -> mem_read_size == 0){
                static_inst -> mem_read_size = 4;
            } 
        } 
        
        memcpy(trace_info->trace_buf + trace_info->bytes_accumulated, static_inst, sizeof(Inst_info));
	trace_info->bytes_accumulated += sizeof(Inst_info);
	// ----------------------------------------
	// once accumulated instructions exceed the buffer size, write instructions to the file
	// ----------------------------------------
	if (trace_info->bytes_accumulated == BUF_SIZE) {
		int write_size = gzwrite(trace_info->trace_stream, trace_info->trace_buf, BUF_SIZE);
		if (write_size != BUF_SIZE) {
			cerr << "-> TID - " << tid << " Error when writing instruction " << g_inst_count[tid] << " write_size:" << write_size << " " << BUF_SIZE << endl;
			exit(1);
		}
		trace_info->bytes_accumulated = 0;
	}


	// ----------------------------------------
	// dump out an instruction to the text file
	// ----------------------------------------
	if (KNOB(Knob_inst_dump)->getValue()) {
		write_inst_to_file(trace_info->debug_stream, static_inst, tid);
	}

	
	//Reset dynamic fields.These will get generated again in mem_cb or at the beginning of the inst_cb for branches.

	static_inst->ld_vaddr1      = 0;
	static_inst->ld_vaddr2      = 0;
	static_inst->st_vaddr       = 0;
	static_inst->actually_taken = 0;
        // Reset read size only if it's not a prefetch instruction. Prefetches don't generate actual mem callbacks.
        if (static_inst->opcode != PREFETCH_NTA && static_inst->opcode != PREFETCH_T0 && static_inst->opcode != PREFETCH_T1 && static_inst->opcode != PREFETCH_T2){
            static_inst->mem_read_size  = 0;
        }
        static_inst->mem_write_size = 0;
	//TODO: Need to get the rep dir first. Need to read flag reg at runtime. Add a call in mem_cb to read the flag. Edit: Done. 
	static_inst->rep_dir 	    = 0;
	if (
			static_inst->cf_type == CF_IBR ||
			static_inst->cf_type == CF_ICBR ||
			static_inst->cf_type == CF_ICALL ||
			static_inst->cf_type == CF_RET ||
			static_inst->cf_type == CF_ICO 
	   ){
		// Reset only if the target is dynamically determined
		static_inst->branch_target = 0;
	}
	
}

/////////////////////////////////////////////////////////////////////////////////////////
// Dump an instruction to a text file
/////////////////////////////////////////////////////////////////////////////////////////

void TraceWriter::write_inst_to_file(ofstream* file, Inst_info *t_info, THREADID tid)
{
	if (g_inst_print_count[tid] > KNOB(Knob_dump_max) ->getValue())
		return ;

	g_inst_print_count[tid]++;

	(*file) << "*** begin of the data strcture *** " <<endl;
	(*file) << "t_info->uop_opcode " <<tr_opcode_names[(uint32_t) t_info->opcode]  << endl;
	(*file) << "t_info->num_read_regs: " << hex <<  (uint32_t) t_info->num_read_regs << endl;
	(*file) << "t_info->num_dest_regs: " << hex << (uint32_t) t_info->num_dest_regs << endl;
	for (UINT32 ii = 0; ii < t_info->num_read_regs; ++ii) {
		if (t_info->src[ii] != 0) {
			(*file) << "t_info->src" << ii << ": " << (xed_reg_enum_t2str((xed_reg_enum_t)t_info->src[ii])) << endl;
		}
	}
	for (UINT32 ii = 0; ii < t_info->num_dest_regs; ++ii) {
		if (t_info->dst[ii] != 0) {
			(*file) << "t_info->dst" << ii << ": " << hex << xed_reg_enum_t2str((xed_reg_enum_t)t_info->dst[ii]) << endl;
		}
	}
	(*file) << "t_info->cf_type: " << hex << tr_cf_names[(uint32_t) t_info->cf_type] << endl;
	(*file) << "t_info->has_immediate: " << hex << (uint32_t) t_info->has_immediate << endl;
	(*file) << "t_info->r_dir:" << (uint32_t) t_info->rep_dir << endl;
	(*file) << "t_info->has_st: " << hex << (uint32_t) t_info->has_st << endl;
	(*file) << "t_info->num_ld: " << hex << (uint32_t) t_info->num_ld << endl;
	(*file) << "t_info->mem_read_size: " << hex << (uint32_t) t_info->mem_read_size << endl;
	(*file) << "t_info->mem_write_size: " << hex << (uint32_t) t_info->mem_write_size << endl;
	(*file) << "t_info->is_fp: " << (uint32_t) t_info->is_fp << endl;
	(*file) << "t_info->ld_vaddr1: " << hex << (uint64_t) t_info->ld_vaddr1 << endl;
	(*file) << "t_info->ld_vaddr2: " << hex << (uint64_t) t_info->ld_vaddr2 << endl;
	(*file) << "t_info->st_vaddr: " << hex << (uint64_t) t_info->st_vaddr << endl;
	(*file) << "t_info->instruction_addr: " << hex << (uint64_t) t_info->instruction_addr << endl;
	(*file) << "t_info->branch_target: " << hex << (uint64_t) t_info->branch_target << endl;
	(*file) << "t_info->actually_taken: " << hex << (uint32_t) t_info->actually_taken << endl;
	(*file) << "t_info->write_flg: " << hex << (uint32_t) t_info->write_flg << endl;
	(*file) << "*** end of the data strcture *** " << endl << endl;
}

int main(int argc, char** argv) {

        // initialize knobs
	init_knobs(argc, argv);
	validate_knobs();
	sanity_check();
	// initialize the XED tables -- one time.
	xed_tables_init();

	ofstream *outfile(NULL);

	// Check if input file exists
	ifstream input_tar(KNOB(Knob_tar->getValue().c_str()));
	if (!input_tar){
		cerr <<"Error: " << KNOB(Knob_tar->getValue().c_str()) << " doesn't exist." <<endl; 
		exit(1);
	}
	
	// Read the qsim trace file as a parameter.
	outfile = new ofstream(KNOB(Knob_qsim_trace_name)->getValue().c_str());
    
	OSDomain *osd_p(NULL);
	// Create new OSDomain from saved state.
    osd_p = new OSDomain(KNOB(Knob_num_cpus)->getValue(), KNOB(Knob_state_file)->getValue().c_str());
    OSDomain &osd(*osd_p);
	unsigned n_cpus = osd.get_n();

	if (KNOB(Knob_num_cpus)->getValue() != n_cpus){
		cerr << "Warning: Statefile reports "<< n_cpus <<" cpus whereas --num_cpus ="<< KNOB(Knob_num_cpus)->getValue()<<". Statefile's value will be used." << endl;
		KNOB(Knob_num_cpus)->setValue(n_cpus);
	}

	osd.connect_console(std::cout);

	// Attach a TraceWriter if a trace file is given.
	TraceWriter tw(osd, outfile?*outfile:std::cout, KNOB(Knob_tar->getValue().c_str()) );

	// The main loop: run until 'finished' is true.
	while (!tw.hasFinished()) {
        osd.run(0, 1000);
	}

	if (outfile) { outfile->close(); }
	delete outfile;
	
	free_knobs();
	return 0;
}
