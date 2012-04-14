#!/usr/bin/python

#########################################################################################
# Author      : Jaekyu Lee (jq.lee17@gmail.com)
# Description : Scons top-level
#########################################################################################


import os


## parse argument
flags = {}
flags['debug'] = ARGUMENTS.get('debug', 0)
flags['gprof'] = ARGUMENTS.get('gprof', 0)
flags['dram']  = ARGUMENTS.get('dram', 0)
flags['power'] = ARGUMENTS.get('power', 0)


## Create stat/knobs
SConscript('scripts/SConscript')


## debug build
if flags['debug'] == '1':
  SConscript('SConscript', variant_dir='.dbg_build', duplicate=0, exports='flags')
  Clean('.', '.dbg_build')
## gprof build
elif flags['gprof'] == '1':
  SConscript('SConscript', variant_dir='.gpf_build', duplicate=0, exports='flags')
  Clean('.', '.gpf_build')
## opt build
else:
  SConscript('SConscript', variant_dir='.opt_build', duplicate=0, exports='flags')
  Clean('.', '.opt_build')

