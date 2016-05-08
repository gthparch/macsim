#-*- Makefile -*-
#
#

AM_CPPFLAGS = \
	$(BOOST_CPPFLAGS) \
	$(MPI_CPPFLAGS) \
  -I$(srcdir)/src \
  -I$(srcdir)/src/manifold/models/iris/interfaces \
  -I$(srcdir)/src/manifold/models/iris/iris_srcs/components \
  -I$(srcdir)/src/manifold/models/iris/iris_srcs/data_types \
  -I$(srcdir)/src/manifold/models/iris/iris_srcs/topology \
  -I$(srcdir)/src/manifold/kernel/include \
  -I$(srcdir)/src/manifold/kernel/include/kernel \
  -I$(srcdir)/src/orion \
  -I$(srcdir)/src/rwqueue \
  -std=c++0x \
	-DUSING_SST  \
  -DNO_DEBUG \
	$(HMC_FLAG)  \
	$(QSIM_CPPFLAGS) \
	-g


SUBDIRS = scripts

compdir = $(pkglibdir)
comp_LTLIBRARIES = libmacsimComponent.la
libmacsimComponent_la_SOURCES =                                        \
macsimEvent.h                                                          \
macsimComponent.cpp          macsimComponent.h                         \
libmacsimComponent.cc                                                  \
src/dram_sst_vaultsim.cc     src/dram_sst_vaultsim.h                   \
src/all_knobs.cc             src/all_knobs.h                           \
src/allocate.cc              src/allocate.h                            \
src/allocate_smc.cc          src/allocate_smc.h                        \
src/all_stats.cc             src/all_stats.h                           \
src/assert_macros.h                                                    \
src/bp.cc                    src/bp.h                                  \
src/bp_gshare.cc             src/bp_gshare.h                           \
src/bp_targ.cc               src/bp_targ.h                             \
src/bug_detector.cc          src/bug_detector.h                        \
src/cache.cc                 src/cache.h                               \
src/config.h                                                           \
src/core.cc                  src/core.h                                \
src/debug_macros.h                                                     \
src/dram.cc                  src/dram.h                                \
src/dram_ctrl.cc             src/dram_ctrl.h                           \
src/dram_dramsim.cc          src/dram_dramsim.h                        \
src/dyfr.cc                  src/dyfr.h                                \
src/exec.cc                  src/exec.h                                \
src/factory_class.cc         src/factory_class.h                       \
src/fetch_factory.cc         src/fetch_factory.h                       \
src/frontend.cc              src/frontend.h                            \
src/global_defs.h                                                      \
src/global_types.h                                                     \
src/inst_info.h                                                        \
src/knob.cc                  src/knob.h                                \
src/macsim.cc                src/macsim.h                              \
src/main.cc                                                            \
src/map.cc                   src/map.h                                 \
src/memory.cc                src/memory.h                              \
src/memreq_info.cc           src/memreq_info.h                         \
src/port.cc                  src/port.h                                \
src/pqueue.h                                                           \
src/pref.cc                  src/pref.h                                \
src/pref_common.cc           src/pref_common.h                         \
src/pref_factory.cc          src/pref_factory.h                        \
src/pref_stride.cc           src/pref_stride.h                         \
src/process_manager.cc       src/process_manager.h                     \
src/readonly_cache.cc        src/readonly_cache.h                      \
src/resource.cc              src/resource.h                            \
src/retire.cc                src/retire.h                              \
src/rob.cc                   src/rob.h                                 \
src/rob_smc.cc               src/rob_smc.h                             \
src/schedule.cc              src/schedule.h                            \
src/schedule_io.cc           src/schedule_io.h                         \
src/schedule_ooo.cc          src/schedule_ooo.h                        \
src/schedule_smc.cc          src/schedule_smc.h                        \
src/statistics.cc            src/statistics.h                          \
src/statsEnums.h                                                       \
src/sw_managed_cache.cc      src/sw_managed_cache.h                    \
src/trace_read.cc            src/trace_read.h                          \
src/trace_read_cpu.cc        src/trace_read_cpu.h                      \
src/trace_read_gpu.cc        src/trace_read_gpu.h                      \
src/trace_read_a64.cc        src/trace_read_a64.h                      \
src/uop.cc                   src/uop.h                                 \
src/utils.cc                 src/utils.h                               \
src/network.cc               src/network.h                             \
src/network_ring.cc          src/network_ring.h                        \
src/network_mesh.cc          src/network_mesh.h                        \
src/network_simple.cc        src/network_simple.h                      \
src/page_mapping.cc          src/page_mapping.h                        \
src/allocate_interface.h                                               \
src/exec_interface.h                                                   \
src/frontend_interface.h                                               \
src/retire_interface.h                                                 \
src/schedule_interface.h                                               \
src/hmc_process.cc           src/hmc_process.h                         \
src/trace_gen.h                                                        \
src/trace_gen_a64.cc         src/trace_gen_a64.h                       \
src/cs_disas.cc              src/cs_disas.h


EXTRA_DIST = 

libmacsimComponent_la_LDFLAGS = -module -avoid-version $(QSIM_LDFLAGS)
libmacsimComponent_la_LIBADD = -lz $(QSIM_LIBS)
