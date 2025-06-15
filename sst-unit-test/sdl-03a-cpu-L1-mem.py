################################################################################
# Description:
#  - Macsim is connected to L1 caches, followed by a bus and memory controller.
#  - Macsim ports (stdInterface) are loaded as subcomponents explictly.
################################################################################
import sst
from common import *

# 0: None, 1: Stdout, 2: Stderr, 3: File
DEBUG_CORE  = 1
DEBUG_L1    = 0
DEBUG_MEM   = 1

DEBUG_LINKS = 0
DEBUG_BUS   = 0

DEBUG_LEVEL = 5
VERBOSE     = 10

########################################
# System Parameters
MEM_SIZE_S = '512MiB'
MEM_SIZE = str2bytes(MEM_SIZE_S)
MEM_START = 0
MEM_END = MEM_START + MEM_SIZE - 1


########################################
# Macsim
macsim = sst.Component("macsimComponent", "macsimComponent.macsimComponent")
macsim.addParams({
    "param_file": "params.in",
    "trace_file": "trace_file_list_cpu",
    "output_dir": "output_dir",
    "command_line": "--num_sim_cores=1 --num_sim_large_cores=1 --num_sim_small_cores=0 --use_memhierarchy=1 --core_type=x86",
    "frequency" : "2GHz",
    "num_cores" : "1",
    "num_link": "1",
    "mem_size" : MEM_SIZE,
    "debug": DEBUG_CORE,
    "debug_level": DEBUG_LEVEL,
})
macsim_icache_if = macsim.setSubComponent("macsim0_core0_icache", "memHierarchy.standardInterface")
macsim_icache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_dcache_if = macsim.setSubComponent("macsim0_core0_dcache", "memHierarchy.standardInterface")
macsim_dcache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})


########################################
# L1 Caches
macsim0_core0_icache = sst.Component("macsim0_core0_icache", "memHierarchy.Cache")
macsim0_core0_icache.addParams({
    "access_latency_cycles" : "3",
    "cache_frequency" : "3.5Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MSI",
    "associativity" : "4",
    "cache_line_size" : "128",
    "debug" : DEBUG_L1,
    "debug_level" : DEBUG_LEVEL,
    "verbose" : VERBOSE,
    "L1" : "1",
    "cache_size" : "2KiB"
})

macsim0_core0_dcache = sst.Component("macsim0_core0_dcache", "memHierarchy.Cache")
macsim0_core0_dcache.addParams({
    "access_latency_cycles" : "3",
    "cache_frequency" : "3.5Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MSI",
    "associativity" : "4",
    "cache_line_size" : "64",
    "debug" : DEBUG_L1,
    "debug_level" : DEBUG_LEVEL,
    "verbose" : VERBOSE,
    "L1" : "1",
    "cache_size" : "2KiB"
})


########################################
# Bus between L1 caches and memory controller
mem_bus = sst.Component("mem_bus", "memHierarchy.Bus")
mem_bus.addParams({
    "debug" : DEBUG_LINKS,
    "debug_level" : DEBUG_LEVEL,
    "bus_frequency" : "4 Ghz"
})


########################################
# Memory Controller
memctrl = sst.Component("memctrl", "memHierarchy.MemController")
memctrl.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    "verbose" : VERBOSE,
    # "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})
memory = memctrl.setSubComponent("backend", "memHierarchy.simpleMem")
memory.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})

########################################
# Links

# Macsim::core0_icache -> L1 ICache
link_macsim_icache = sst.Link("link_macsim_icache")
link_macsim_icache.connect( (macsim_icache_if, "port", "1000ps"), (macsim0_core0_icache, "high_network_0", "1000ps") )

# Macsim::core0_dcache -> L1 DCache
link_macsim_dcache = sst.Link("link_macsim_dcache")
link_macsim_dcache.connect( (macsim_dcache_if, "port", "1000ps"), (macsim0_core0_dcache, "high_network_0", "1000ps") )

# L1 I/DCache -> Bus
link_icache_bus = sst.Link("link_icache_bus")
link_icache_bus.connect( (macsim0_core0_icache, "low_network_0", "50ps"), (mem_bus, "high_network_0", "50ps") )
link_dcache_bus = sst.Link("link_dcache_bus")
link_dcache_bus.connect( (macsim0_core0_dcache, "low_network_0", "50ps"), (mem_bus, "high_network_1", "50ps") )

# Bus -> Memory
link_bus_mem = sst.Link("link_bus_mem")
link_bus_mem.connect( (mem_bus, "low_network_0", "50ps"), (memctrl, "direct_link", "50ps") )


########################################
# Enable statistics
sst.setStatisticLoadLevel(7)
sst.setStatisticOutput("sst.statOutputConsole")