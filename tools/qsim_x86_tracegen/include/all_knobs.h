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
 * File         : all_knob.h 
 * Description  : knob template 
 * This file comes from MacSim Simulator 
 * All knobs have to be added in this file  and all_knobs.h 
 *********************************************************************************************/


#ifndef __ALL_KNOBS_H_INCLUDED__
#define __ALL_KNOBS_H_INCLUDED__

//#include "global_types.h"
#include "knob.h"

#define KNOB(var) g_knobs->var

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief knob variables holder
///////////////////////////////////////////////////////////////////////////////////////////////
class all_knobs_c {
	public:
		/**
		 * Constructor
		 */
		all_knobs_c();

		/**
		 * Destructor
		 */
		~all_knobs_c();

		/**
		 * Register Knob Variables
		 */
		void registerKnobs(KnobsContainer *container);

		/**
		 * Display All the Knob Variables
		 */
		void display();

                /**
                  Print knob descriptions
                  */
                void print_knob_descriptions();

	public:

		// Macsim knobs
		KnobTemplate<string>*	Knob_trace_name ;
		KnobTemplate<bool>* Knob_inst_dump ;
		KnobTemplate<UINT32>* Knob_dump_max ;
		KnobTemplate<string>* Knob_dump_file ;
		KnobTemplate<UINT32>* Knob_num_cpus ;
		KnobTemplate<UINT64>* Knob_max ;
		KnobTemplate<bool>* Knob_extra_memops ;
		KnobTemplate<string>* Knob_trace_mode ;
		KnobTemplate<bool>* Knob_use_va;
                // QSIM knobs
		KnobTemplate<string>* Knob_state_file;
		KnobTemplate<string>* Knob_qsim_trace_name;
		KnobTemplate<string>* Knob_tar;

};
#endif //__ALL_KNOBS_H_INCLUDED__
