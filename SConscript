#!/usr/bin/python

#########################################################################################
# Author      : Jaekyu Lee (jq.lee17@gmail.com)
# Description : Scons top-level
#########################################################################################

import os


#########################################################################################
# Build option
#########################################################################################
Import('flags')


#########################################################################################
# FLAGS
#########################################################################################

## include directories
header_dirs = [
  '-I src/manifold/kernel/include/kernel',
  '-I src/manifold/kernel/include',
  '-I src/manifold/models/iris/interfaces',
  '-I src',
  '-I src/orion'
]
header_dirs = ' '.join(header_dirs)


# 
compile_flags = [
  '-DNO_MPI',
  '-DLONG_COUNTERS'
]
compile_flags = ' '.join(compile_flags)


## compiler warning flags
warn_flags = [
  '-Werror',
  '-Wunused-function',
  '-Wreturn-type',
  '-Wpointer-arith',
  '-Wno-write-strings'
]
warn_flags = ' '.join(warn_flags)

use_vars = set([ 'AS', 'AR', 'CC', 'CXX', 'HOME', 'LD_LIBRARY_PATH', 'PATH', 'RANLIB'])
use_env = {}

for key,val in os.environ.iteritems():
  if key in use_vars:
    use_env[key] = val

env = Environment(ENV=use_env)

## Opt
if flags['debug'] == '1':
  env['CPPFLAGS'] = '-g -std=c++0x %s %s %s' % (warn_flags, header_dirs, compile_flags)
## Gprof
elif flags['gprof'] == '1':
  env['CPPFLAGS'] = '-pg -std=c++0x %s %s %s -DNO_DEBUG' % (warn_flags, header_dirs, compile_flags)
## Debug
else:
  env['CPPFLAGS'] = '-O3 -std=c++0x -funroll-loops %s %s %s -DNO_DEBUG' % (warn_flags, header_dirs, compile_flags)

env['LINKFLAGS'] = '-lz -lpthread'
env['CPPDEFINES'] = []



#########################################################################################
# libstdc++ library static linking
#########################################################################################
static_libcpp = Command('libstdc++.a',None,Action('ln -s `g++ -print-file-name=libstdc++.a` $TARGET'))


#########################################################################################
# IRIS
#########################################################################################
IRIS_kernel_srcs = [
  'src/manifold/kernel/src/clock.cc',
  'src/manifold/kernel/src/component.cc',
  'src/manifold/kernel/src/link.cc',
  'src/manifold/kernel/src/manifold.cc',
  'src/manifold/kernel/src/messenger.cc'
]

IRIS_srcs = [
  'src/manifold/models/iris/iris_srcs/data_types/networkPacket.cc',
  'src/manifold/models/iris/iris_srcs/data_types/flit.cc',
  'src/manifold/models/iris/iris_srcs/data_types/linkData.cc',
  'src/manifold/models/iris/iris_srcs/topology/twoNode.cc',
  'src/manifold/models/iris/iris_srcs/topology/ring.cc',
  'src/manifold/models/iris/iris_srcs/topology/mesh.cc',
  'src/manifold/models/iris/iris_srcs/topology/torus.cc',
  'src/manifold/models/iris/iris_srcs/topology/util.cc',
  'src/manifold/models/iris/iris_srcs/components/genericBuffer.cc',
  'src/manifold/models/iris/iris_srcs/components/genericRC.cc',
  'src/manifold/models/iris/iris_srcs/components/genericSwitchArbiter.cc',
  'src/manifold/models/iris/iris_srcs/components/genericVcAllocator.cc',
  'src/manifold/models/iris/iris_srcs/components/manifoldProcessor.cc',
  'src/manifold/models/iris/iris_srcs/components/ninterface.cc',
  'src/manifold/models/iris/iris_srcs/components/simpleRouter.cc'
]


if flags['power'] == '1':
  env.Library('iris', IRIS_kernel_srcs + IRIS_srcs, CPPDEFINES=['POWER_EI'])
else:
  env.Library('iris', IRIS_kernel_srcs + IRIS_srcs)


#########################################################################################
# ORION
#########################################################################################
ORION_srcs = [
  'src/orion/SIM_router.c',
  'src/orion/SIM_arbiter.c',
  'src/orion/SIM_crossbar.c',
  'src/orion/SIM_router_power.c',
  'src/orion/SIM_link.c',
  'src/orion/SIM_clock.c',
  'src/orion/SIM_router_area.c',
  'src/orion/SIM_array_l.c',
  'src/orion/SIM_array_m.c',
  'src/orion/SIM_cam.c',
  'src/orion/SIM_ALU.c',
  'src/orion/SIM_misc.c',
  'src/orion/SIM_permu.c',
  'src/orion/SIM_static.c',
  'src/orion/SIM_util.c',
  'src/orion/SIM_time.c'
]


env.Library('orion', ORION_srcs, CPPFLAGS='')


#########################################################################################
# EI
#########################################################################################
EI_srcs = [
  'src/energy_introspector/parameters.cc',
  'src/energy_introspector/parser.cc',
  'src/energy_introspector/string_ops.cc',
  'src/energy_introspector/energy_introspector.cc',
  'src/energy_introspector/ENERGYLIB_McPAT.cc',
  'src/energy_introspector/ENERGYLIB_IntSim.cc',
  'src/energy_introspector/THERMALLIB_HotSpot.cc',
  'src/energy_introspector/RELIABILITYLIB_RAMP.cc',
  'src/energy_introspector/SENSORLIB_RNG.cc'
]


if flags['power'] == '1':
  env.Library('ei', EI_srcs)


#########################################################################################
# DRAMSIM2
#########################################################################################
DRAMSIM2_srcs = [
  'src/DRAMSim2/Transaction.cpp',
  'src/DRAMSim2/AddressMapping.cpp',
  'src/DRAMSim2/Bank.cpp',
  'src/DRAMSim2/BankState.cpp',
  'src/DRAMSim2/BusPacket.cpp',
  'src/DRAMSim2/CommandQueue.cpp',
  'src/DRAMSim2/IniReader.cpp',
  'src/DRAMSim2/MemoryController.cpp',
  'src/DRAMSim2/MemorySystem.cpp',
  'src/DRAMSim2/MultiChannelMemorySystem.cpp',
  'src/DRAMSim2/Rank.cpp',
  'src/DRAMSim2/SimulatorObject.cpp',
  'src/DRAMSim2/TraceBasedSim.cpp',
  'src/DRAMSim2/Transaction.cpp'
]


if flags['dram'] == '1':
  if not os.path.exists('src/DRAMSim2'):
    os.system('git clone git://github.com/dramninjasUMD/DRAMSim2.git src/DRAMSim2')
  env.Library('dramsim', DRAMSIM2_srcs, CPPDEFINES=['NO_STORAGE', 'DEBUG_BUILD', 'LOG_OUTPUT'])


#########################################################################################
# MACSIM
#########################################################################################
macsim_src = [
  'src/all_knobs.cc',
  'src/all_stats.cc',
  'src/allocate.cc',
  'src/allocate_smc.cc',
  'src/bp.cc',
  'src/bp_gshare.cc',
  'src/bp_targ.cc',
  'src/bug_detector.cc',
  'src/cache.cc',
  'src/core.cc',
  'src/dram.cc',
  'src/exec.cc',
  'src/factory_class.cc',
  'src/fetch_factory.cc',
  'src/frontend.cc',
  'src/knob.cc',
  'src/macsim.cc',
  'src/main.cc',
  'src/map.cc',
  'src/memory.cc',
  'src/memreq_info.cc',
  'src/noc.cc',
  'src/port.cc',
  'src/pref.cc',
  'src/pref_common.cc',
  'src/pref_factory.cc',
  'src/pref_stride.cc',
  'src/process_manager.cc',
  'src/readonly_cache.cc',
  'src/retire.cc',
  'src/rob.cc',
  'src/rob_smc.cc',
  'src/router.cc',
  'src/schedule.cc',
  'src/schedule_io.cc',
  'src/schedule_ooo.cc',
  'src/schedule_smc.cc',
  'src/statistics.cc',
  'src/sw_managed_cache.cc',
  'src/trace_read.cc',
  'src/uop.cc',
  'src/utils.cc',
]


if flags['power'] == '1':
  macsim_src.append('src/ei_power.cc')


#########################################################################################
# Libraries
#########################################################################################
libraries = ['iris', 'orion', static_libcpp]


if flags['power'] == '1':
  libraries.append('ei')
  env['CPPDEFINES'].append('POWER_EI')

if flags['dram'] == '1':
  libraries.append('dramsim')
  env['CPPDEFINES'].append('DRAMSIM')
  env['CPPFLAGS'] += ' -I src/DRAMSim2'


env.Program(
    'macsim',
    macsim_src, 
    LIBS=libraries, 
    #LIBPATH=['.', '/usr/lib', '/usr/local/lib']
)


#########################################################################################
# Clean
#########################################################################################
if GetOption('clean'):
  os.system('rm -f ../bin/macsim')

