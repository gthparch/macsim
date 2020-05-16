#include "all_knobs.h"

all_knobs_c::all_knobs_c() {
	

	// =========== ./knob.def ===========
	KNOB_ENABLE_INST_COUNT = new KnobTemplate< bool > ("enable_inst_count", false);
	KNOB_ENABLE_REUSE_DIST = new KnobTemplate< bool > ("enable_reuse_dist", false);
	KNOB_ENABLE_COUNT_STATIC_PC = new KnobTemplate< bool > ("enable_count_static_pc", false);
}

all_knobs_c::~all_knobs_c() {
	delete KNOB_ENABLE_INST_COUNT;
	delete KNOB_ENABLE_REUSE_DIST;
	delete KNOB_ENABLE_COUNT_STATIC_PC;
}

void all_knobs_c::registerKnobs(KnobsContainer *container) {
	

	// =========== ./knob.def ===========
	container->insertKnob( KNOB_ENABLE_INST_COUNT );
	container->insertKnob( KNOB_ENABLE_REUSE_DIST );
	container->insertKnob( KNOB_ENABLE_COUNT_STATIC_PC );
}

