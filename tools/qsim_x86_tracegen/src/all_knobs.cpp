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



/**********************************************************************************************
 * File         : all_knob.cc
 * Description  : knob template 
 * This file comes from MacSim Simulator 
 * All knobs have to be added in this file 
 *********************************************************************************************/

#include <iomanip>
#include "all_knobs.h"

#include <string>

all_knobs_c::all_knobs_c() {

    const char* qsim_prefix = getenv("QSIM_PREFIX");

    if (!qsim_prefix) qsim_prefix = "/usr/local";

    Knob_trace_name = new KnobTemplate<string> ("tracename","trace");
    Knob_inst_dump = new KnobTemplate<bool> ("dump", 1);
    Knob_dump_max = new KnobTemplate<UINT32> ("dump_max", 50000);
    Knob_dump_file = new KnobTemplate<string> ("dump_file", "dump");
    Knob_num_cpus = new KnobTemplate<UINT32> ("num_cpus", 1);
    Knob_max = new KnobTemplate<UINT64> ("max", 0);
    Knob_extra_memops = new KnobTemplate<bool> ("extra_memops", 0);
    Knob_trace_mode = new KnobTemplate<string> ("tracemode","all");
    Knob_state_file = new KnobTemplate<string> ("statefile", std::string(qsim_prefix) + "/state.1");
    Knob_qsim_trace_name = new KnobTemplate<string> ("qsim_tracename", "qsim_trace");
    Knob_tar = new KnobTemplate<string> ("tar", "./examples/hello_world.tar");
    Knob_use_va = new KnobTemplate<bool> ("use_va", 0);
}

all_knobs_c::~all_knobs_c() {
	delete Knob_trace_name ;
	delete Knob_inst_dump ;
	delete Knob_dump_max ;
	delete Knob_dump_file ;
	delete Knob_num_cpus ;
	delete Knob_max ;
	delete Knob_state_file;
	delete Knob_qsim_trace_name;
	delete Knob_tar;
	delete Knob_extra_memops ;
	delete Knob_trace_mode  ;
        delete Knob_use_va;
}

void all_knobs_c::registerKnobs(KnobsContainer *container) {
	container->insertKnob( Knob_trace_name );
	container->insertKnob( Knob_inst_dump );
	container->insertKnob( Knob_dump_max );
	container->insertKnob( Knob_dump_file );
	container->insertKnob( Knob_num_cpus );
	container->insertKnob( Knob_max );
	container->insertKnob( Knob_state_file );
	container->insertKnob( Knob_qsim_trace_name );
	container->insertKnob( Knob_tar );
	container->insertKnob( Knob_extra_memops);
	container->insertKnob( Knob_trace_mode);
        container->insertKnob( Knob_use_va);
}

void all_knobs_c::display() {
	Knob_trace_name ->display(cout); cout << endl;
	Knob_inst_dump ->display(cout); cout << endl;
	Knob_dump_max ->display(cout); cout << endl;
	Knob_dump_file ->display(cout); cout << endl;
	Knob_num_cpus ->display(cout); cout << endl;
	Knob_max ->display(cout); cout << endl;
	Knob_extra_memops->display(cout); cout << endl ;
	Knob_trace_mode ->display(cout); cout << endl;
	Knob_state_file->display(cout); cout << endl;
	Knob_qsim_trace_name->display(cout); cout << endl;
	Knob_tar->display(cout); cout << endl;
        Knob_use_va->display(cout); cout << endl;
}

void all_knobs_c::print_knob_descriptions() {
    cout << setw(15)<<"KNOB NAME"<< setw(10)<<"TYPE"<<setw(90)<<"DESCRIPTION"<< setw(25)<<"DEFAULT VALUE" << endl <<endl; 
    cout << setw(15)<<"tracename"<< setw(10)<<"String"<<setw(90)<<"Base name for output trace/config file. Names are basename_[tid].raw/basename.txt"<<setw(25)<<"trace"<<endl;
    cout << setw(15)<<"dump"<< setw(10)<<"Bool"<<setw(90)<<"Generate human readable dump of traces"<<setw(25)<<"1"<<endl;

    cout << setw(15)<<"dump_max"<<setw(10)<<"Unsigned"<<setw(90)<<"Max instructions to dump if dump==True"<<setw(25)<<"50000"<<endl;
    cout << setw(15)<<"dump_file"<<setw(10)<<"String"<<setw(90)<<"Base name for dump files. Actual names are basename_[tid].dump"<<setw(25)<<"dump"<<endl;

    cout << setw(15)<<"num_cpus"<<setw(10)<<"Unsigned"<<setw(90)<<"Number of CPUS. Should be consistent with statefile"<<setw(25)<<"1"<<endl;

    cout << setw(15)<<"max"<<setw(10)<<"Unsigned"<<setw(90)<<"Max instructions to generate. Use 0 for unlimited"<<setw(25)<<"0"<<endl;

    cout << setw(15)<<"extra_memops"<<setw(10)<<"Bool"<<setw(90)<<"WIP. Knob to add children for ops that don't fit in the trace struct. Don't enable"<<setw(25)<<"0"<<endl;
    cout << setw(15)<<"tracemode"<<setw(10)<<"String"<<setw(90)<<"Mode for which which instructions should be traced. Valid values: user, kernel, all"<<setw(25)<<"all"<<endl;

     cout << setw(15)<<"use_va"<<setw(10)<<"Bool"<<setw(90)<<"WIP. Knob to enable VA support. Due to VA space collisons, it is disabled. Don't enable"<<setw(25)<<"0"<<endl;

    cout << setw(15)<<"statefile"<<setw(10)<<"String"<<setw(90)<<"Path to the input state file. State files are generated by Qsim's fast-forwarder"<<setw(25)<<"../../state.1"<<endl;

    cout << setw(15)<<"qsim_tracename"<<setw(10)<<"String"<<setw(90)<<"Name of output trace file in qsim's format"<<setw(25)<<"qsim_trace"<<endl;

     cout << setw(15)<<"tar"<<setw(10)<<"String"<<setw(90)<<"Input tar file with the application and input data. See README for details"<<setw(25)<<"./examples/example1.tar"<<endl;

    cout << endl;
}

