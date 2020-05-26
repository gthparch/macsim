#!/usr/bin/python

#########################################################################################
# Author      : Jaekyu Lee (jq.lee17@gmail.com)
# Description : Scons top-level
#########################################################################################

import sys
import os
import glob


#########################################################################################
# Build option
#########################################################################################
Import('flags')


#########################################################################################
# FLAGS
#########################################################################################


## compiler warning flags
warn_flags = [
  '-Werror',
#  '-Wunused-function',
#  '-Wreturn-type',
#  '-Wpointer-arith',
  '-Wuninitialized',
  '-Wno-write-strings'
]
warn_flags = ' '.join(warn_flags)


## Environment
env = Environment()
custom_vars = set(['AS', 'AR', 'CC', 'CXX', 'HOME', 'LD_LIBRARY_PATH', 'PATH', 'RANLIB'])

for key,val in os.environ.iteritems():
  if key in custom_vars:
    env[key] = val

env['CPPPATH']    = ['#src']
env['CPPDEFINES'] = ['LONG_COUNTERS', 'NO_MPI']
env['LIBPATH']    = ['/usr/lib', '/usr/local/lib']

## MAC OS X does not support static linking
if sys.platform != "darwin" and flags.get('qsim') != '1':
  env['LINKFLAGS']  = ['--static']
# env['CXX']        = ['icpc']


## DEBUG build
if flags['debug'] == '1':
  env['CPPFLAGS'] = '-g -std=c++14 %s' % warn_flags
## GPROF build
elif flags['gprof'] == '1':
  env['CPPFLAGS'] = '-pg -std=c++14 %s' % warn_flags
  env['CPPDEFINES'].append('NO_DEBUG')
  env['LINKFLAGS'].append('-pg')
## OPT build
else:
  env['CPPFLAGS'] = '-O3 -std=c++14 -funroll-loops %s' % warn_flags
  env['CPPDEFINES'].append('NO_DEBUG')

if flags['val'] == '1':
  env['CPPDEFINES'].append('GPU_VALIDATION')

if flags['qsim'] == '1':
  env['CPPDEFINES'] += ['USING_QSIM']
  env['CPPPATH']    += [os.environ['QSIM_PREFIX'] + "/include", '#src/rwqueue']
  env['CPPPATH']    += [os.environ['XED_HOME'] + "/include"]
  env['LIBPATH']    += [os.environ['QSIM_PREFIX'] + "/lib", os.environ['XED_HOME'] + "/lib"]

if flags['pin_3_7_trace'] == '1':
  env['CPPDEFINES'] += ['PIN_3_7_TRACE']

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


if flags['iris'] == '1':
  if flags['power'] == '1':
    env.Library('iris', IRIS_kernel_srcs + IRIS_srcs, CPPDEFINES=env['CPPDEFINES'] + ['IRIS'])
  else:
    env.Library('iris', IRIS_kernel_srcs + IRIS_srcs, CPPDEFINES=env['CPPDEFINES'] + ['IRIS'])


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


if flags['iris'] == '1':
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
  'src/DRAMSim2/AddressMapping.cpp',
  'src/DRAMSim2/BankState.cpp',
  'src/DRAMSim2/ClockDomain.cpp',
  'src/DRAMSim2/IniReader.cpp',
  'src/DRAMSim2/MemorySystem.cpp',
  'src/DRAMSim2/Rank.cpp',
  'src/DRAMSim2/TraceBasedSim.cpp',
  'src/DRAMSim2/Bank.cpp',
  'src/DRAMSim2/BusPacket.cpp',
  'src/DRAMSim2/CommandQueue.cpp',
  'src/DRAMSim2/MemoryController.cpp',
  'src/DRAMSim2/MultiChannelMemorySystem.cpp',
  'src/DRAMSim2/SimulatorObject.cpp',
  'src/DRAMSim2/Transaction.cpp',
]
#"""



if flags['dram'] == '1':
  env.Library('dramsim', DRAMSIM2_srcs, CPPDEFINES=['NO_STORAGE', 'DEBUG_BUILD', 'LOG_OUTPUT'])


#########################################################################################
# Ramulator
#########################################################################################
ramulator_srcs = [
  'src/ramulator_wrapper.cc',
  'src/dram_ramulator.cc',
  'src/ramulator/src/Config.cpp',
  'src/ramulator/src/Controller.cpp',
  'src/ramulator/src/DDR3.cpp',
  'src/ramulator/src/DDR4.cpp',
  'src/ramulator/src/GDDR5.cpp',
  'src/ramulator/src/Gem5Wrapper.cpp',
  'src/ramulator/src/HBM.cpp',
  'src/ramulator/src/LPDDR3.cpp',
  'src/ramulator/src/LPDDR4.cpp',
  'src/ramulator/src/MemoryFactory.cpp',
  'src/ramulator/src/SALP.cpp',
  'src/ramulator/src/WideIO.cpp',
  'src/ramulator/src/WideIO2.cpp',
  'src/ramulator/src/TLDRAM.cpp',
  'src/ramulator/src/ALDRAM.cpp',
  'src/ramulator/src/StatType.cpp',
]

if flags['ramulator'] == '1':
  env['CPPFLAGS'] += ' -Wno-missing-field-initializers '
  env['CPPFLAGS'] += ' -Wno-unused-variable '
  env['CPPFLAGS'] += ' -Wno-reorder '
  env['CPPDEFINES'] += ['RAMULATOR']
  env['CPPPATH'] += ['#src/ramulator']
  env['LIBPATH'] += [Dir('.')]
  env.Library('ramulator', ramulator_srcs, CPPDEFINES=['RAMULATOR'])

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
  'src/dram_ctrl.cc',
  'src/dram_dramsim.cc',
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
  'src/schedule.cc',
  'src/schedule_io.cc',
  'src/schedule_ooo.cc',
  'src/schedule_smc.cc',
  'src/schedule_igpu.cc',
  'src/statistics.cc',
  'src/sw_managed_cache.cc',
  'src/trace_read.cc',
  'src/uop.cc',
  'src/utils.cc',
  'src/network.cc',
  'src/network_ring.cc',
  'src/network_mesh.cc',
  'src/network_simple.cc',
  'src/trace_read_cpu.cc',
  'src/trace_read_gpu.cc',
  'src/trace_read_a64.cc',
  'src/trace_read_igpu.cc',
  'src/page_mapping.cc',
  'src/dyfr.cc',
  'src/hmc_process.cc',
  'src/trace_gen_a64.cc',
  'src/trace_gen_x86.cc',
  'src/cs_disas.cc',
  'src/resource.cc',
  'src/mmu.cc'
]



#########################################################################################
# Libraries
#########################################################################################
libraries = ['z']

if flags['power'] == '1':
  libraries.append('pthread')

if flags['dram'] == '1':
  libraries.append('dramsim')
  env['CPPDEFINES'].append('DRAMSIM')
  env['CPPPATH'] += ['#src/DRAMSim2']
  env['LIBPATH'] += [Dir('.')]

if flags['iris'] == '1':
  libraries.append('iris')
  libraries.append('orion')
  env['CPPDEFINES'].append('IRIS')
  env['CPPPATH'] += ['#src/manifold/kernel/include/kernel']
  env['CPPPATH'] += ['#src/manifold/kernel/include']
  env['CPPPATH'] += ['#src/manifold/models/iris/interfaces']
  env['CPPPATH'] += ['#src/orion']

if flags['qsim'] == '1':
  libraries += ['xed', 'qsim', 'capstone', 'pthread', 'dl']

if flags['ramulator'] == '1':
  libraries.append('ramulator')

env.Program(
    'macsim',
    macsim_src, 
    LIBS=libraries, 
)


#########################################################################################
# Clean
#########################################################################################
if GetOption('clean'):
  os.system('rm -f ../bin/macsim')

