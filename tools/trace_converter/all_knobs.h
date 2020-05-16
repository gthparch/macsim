#ifndef __ALL_KNOBS_H_INCLUDED__
#define __ALL_KNOBS_H_INCLUDED__

#include "global_types.h"
#include "knob.h"

class all_knobs_c {
	public:
		all_knobs_c();
		~all_knobs_c();

		void registerKnobs(KnobsContainer *container);

	public:
		

	// =========== ./knob.def ===========
		KnobTemplate< bool >* KNOB_ENABLE_INST_COUNT;
		KnobTemplate< bool >* KNOB_ENABLE_REUSE_DIST;
		KnobTemplate< bool >* KNOB_ENABLE_COUNT_STATIC_PC;

};
#endif //__ALL_KNOBS_H_INCLUDED__
