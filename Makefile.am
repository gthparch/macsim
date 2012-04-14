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
  -std=c++0x \
	-DUSING_SST

SUBDIRS = scripts

compdir = $(pkglibdir)
comp_LTLIBRARIES = libmacsimComponent.la
libmacsimComponent_la_SOURCES =                                        \
macsimComponent.cpp          macsimComponent.h                         \
                                                                       \
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
src/noc.cc                   src/noc.h                                 \
src/port.cc                  src/port.h                                \
src/pqueue.h                                                           \
src/pref.cc                  src/pref.h                                \
src/pref_common.cc           src/pref_common.h                         \
src/pref_factory.cc          src/pref_factory.h                        \
src/pref_stride.cc           src/pref_stride.h                         \
src/process_manager.cc       src/process_manager.h                     \
src/readonly_cache.cc        src/readonly_cache.h                      \
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
src/uop.cc                   src/uop.h                                 \
src/utils.cc                 src/utils.h                               \
src/ei_power.cc              src/ei_power.h                            \
src/router.cc                src/router.h                              \
                                                                       \
src/allocate_interface.h                                               \
src/exec_interface.h                                                   \
src/frontend_interface.h                                               \
src/retire_interface.h                                                 \
src/schedule_interface.h                                               \
                                                                       \
src/manifold/kernel/src/clock.cc                                       \
src/manifold/kernel/src/component.cc                                   \
src/manifold/kernel/src/link.cc                                        \
src/manifold/kernel/src/manifold.cc                                    \
src/manifold/kernel/src/messenger.cc                                   \
                                                                       \
src/manifold/kernel/src/kernel/include/kernel/clock.h                  \
src/manifold/kernel/src/kernel/include/kernel/common-defs.h            \
src/manifold/kernel/src/kernel/include/kernel/component-decl.h         \
src/manifold/kernel/src/kernel/include/kernel/component.h              \
src/manifold/kernel/src/kernel/include/kernel/link-decl.h              \
src/manifold/kernel/src/kernel/include/kernel/link.h                   \
src/manifold/kernel/src/kernel/include/kernel/manifold-decl.h          \
src/manifold/kernel/src/kernel/include/kernel/manifold.h               \
src/manifold/kernel/src/kernel/include/kernel/message.h                \
src/manifold/kernel/src/kernel/include/kernel/messenger.h              \
                                                                       \
src/manifold/models/iris/interfaces/genericHeader.h                    \
src/manifold/models/iris/interfaces/irisInterface.h                    \
src/manifold/models/iris/interfaces/irisRouter.h                       \
src/manifold/models/iris/interfaces/irisTerminal.h                     \
src/manifold/models/iris/interfaces/MersenneTwister.h                  \
src/manifold/models/iris/interfaces/topology.h                         \
                                                                       \
src/manifold/models/iris/iris_srcs/components/genericBuffer.cc         \
src/manifold/models/iris/iris_srcs/components/genericBuffer.h          \
src/manifold/models/iris/iris_srcs/components/genericRC.cc             \
src/manifold/models/iris/iris_srcs/components/genericRC.h              \
src/manifold/models/iris/iris_srcs/components/genericSwitchArbiter.cc  \
src/manifold/models/iris/iris_srcs/components/genericSwitchArbiter.h   \
src/manifold/models/iris/iris_srcs/components/genericVcAllocator.cc    \
src/manifold/models/iris/iris_srcs/components/genericVcAllocator.h     \
src/manifold/models/iris/iris_srcs/components/manifoldProcessor.cc     \
src/manifold/models/iris/iris_srcs/components/manifoldProcessor.h      \
src/manifold/models/iris/iris_srcs/components/ninterface.cc            \
src/manifold/models/iris/iris_srcs/components/ninterface.h             \
src/manifold/models/iris/iris_srcs/components/simpleRouter.cc          \
src/manifold/models/iris/iris_srcs/components/simpleRouter.h           \
                                                                       \
src/manifold/models/iris/iris_srcs/data_types/flit.cc                  \
src/manifold/models/iris/iris_srcs/data_types/flit.h                   \
src/manifold/models/iris/iris_srcs/data_types/linkData.cc              \
src/manifold/models/iris/iris_srcs/data_types/linkData.h               \
src/manifold/models/iris/iris_srcs/data_types/networkPacket.cc         \
src/manifold/models/iris/iris_srcs/data_types/networkPacket.h          \
                                                                       \
src/manifold/models/iris/iris_srcs/topology/ring.cc                    \
src/manifold/models/iris/iris_srcs/topology/ring.h                     \
src/manifold/models/iris/iris_srcs/topology/twoNode.cc                 \
src/manifold/models/iris/iris_srcs/topology/twoNode.h                  \
src/manifold/models/iris/iris_srcs/topology/util.cc                    \
                                                                       \
src/orion/SIM_router.c                                                 \
src/orion/SIM_router.h                                                 


EXTRA_DIST = 

libmacsimComponent_la_LDFLAGS = -module -avoid-version
