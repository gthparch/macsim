################################################################################
# Description:
#  - Macsim is connected to L1 caches, followed by a bus and memory controller.
#  - Macsim ports (stdInterface) are loaded as subcomponents explictly.
################################################################################
import sst
from common import *

# 0: None, 1: Stdout, 2: Stderr, 3: File
DEBUG_CORE  = 1
DEBUG_L1    = 1
DEBUG_MEM   = 1

DEBUG_LINKS = 0
DEBUG_BUS   = 0

DEBUG_LEVEL = 0
VERBOSE     = 0

########################################
# System Parameters
MEM_SIZE_S = '512MiB'
MEM_SIZE = str2bytes(MEM_SIZE_S)
MEM_START = 0
MEM_END = MEM_START + MEM_SIZE - 1


########################################
# Macsim 0
macsim_0 = sst.Component("macsimComponent0", "macsimComponent.macsimComponent")
macsim_0.addParams({
    "param_file": "params.in",
    "trace_file": "trace_file_list_gpu",
    "output_dir": "output_dir",
    "command_line": "--num_sim_cores=2 --num_sim_large_cores=0 --num_sim_small_cores=2 --use_memhierarchy=1 --core_type=nvbit",
    "frequency" : "2GHz",
    "num_link": "2",
    "mem_size" : MEM_SIZE,
    "debug": DEBUG_CORE,
    "debug_level": DEBUG_LEVEL,
    "nvbit_core": True,
    "component_num": 0,
})

########################################
# Macsim 1
macsim_1 = sst.Component("macsimComponent1", "macsimComponent.macsimComponent")
macsim_1.addParams({
    "param_file": "params.in",
    "trace_file": "trace_file_list",
    "output_dir": "output_dir",
    "command_line": "--num_sim_cores=2 --num_sim_large_cores=0 --num_sim_small_cores=2 --use_memhierarchy=1 --core_type=nvbit",
    "frequency" : "2GHz",
    "num_link": "2",
    "mem_size" : MEM_SIZE,
    "debug": DEBUG_CORE,
    "debug_level": DEBUG_LEVEL,
    "nvbit_core": True,
    "component_num": 1,
})

########################################
# Macsim 0
# Core 0 interface
macsim_0_icache_if = macsim_0.setSubComponent("macsim0_core0_icache", "memHierarchy.standardInterface")
macsim_0_icache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_0_dcache_if = macsim_0.setSubComponent("macsim0_core0_dcache", "memHierarchy.standardInterface")
macsim_0_dcache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_0_ccache_if = macsim_0.setSubComponent("macsim0_core0_ccache", "memHierarchy.standardInterface")
macsim_0_ccache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': 10
})
macsim_0_tcache_if = macsim_0.setSubComponent("macsim0_core0_tcache", "memHierarchy.standardInterface")
macsim_0_tcache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': 10
})

# Core 1 Interfaces
macsim_0_icache_if_1 = macsim_0.setSubComponent("macsim0_core1_icache", "memHierarchy.standardInterface")
macsim_0_icache_if_1.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_0_dcache_if_1 = macsim_0.setSubComponent("macsim0_core1_dcache", "memHierarchy.standardInterface")
macsim_0_dcache_if_1.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_0_ccache_if_1 = macsim_0.setSubComponent("macsim0_core1_ccache", "memHierarchy.standardInterface")
macsim_0_ccache_if_1.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': 10
})
macsim_0_tcache_if_1 = macsim_0.setSubComponent("macsim0_core1_tcache", "memHierarchy.standardInterface")
macsim_0_tcache_if_1.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': 10
})

########################################
# Macsim 1
# Core 0 interface
macsim_1_icache_if = macsim_1.setSubComponent("macsim1_core0_icache", "memHierarchy.standardInterface")
macsim_1_icache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_1_dcache_if = macsim_1.setSubComponent("macsim1_core0_dcache", "memHierarchy.standardInterface")
macsim_1_dcache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_1_ccache_if = macsim_1.setSubComponent("macsim1_core0_ccache", "memHierarchy.standardInterface")
macsim_1_ccache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': 10
})
macsim_1_tcache_if = macsim_1.setSubComponent("macsim1_core0_tcache", "memHierarchy.standardInterface")
macsim_1_tcache_if.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': 10
})

# Core 1 Interfaces
macsim_1_icache_if_1 = macsim_1.setSubComponent("macsim1_core1_icache", "memHierarchy.standardInterface")
macsim_1_icache_if_1.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_1_dcache_if_1 = macsim_1.setSubComponent("macsim1_core1_dcache", "memHierarchy.standardInterface")
macsim_1_dcache_if_1.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': VERBOSE
})
macsim_1_ccache_if_1 = macsim_1.setSubComponent("macsim1_core1_ccache", "memHierarchy.standardInterface")
macsim_1_ccache_if_1.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': 10
})
macsim_1_tcache_if_1 = macsim_1.setSubComponent("macsim1_core1_tcache", "memHierarchy.standardInterface")
macsim_1_tcache_if_1.addParams({
    'debug': DEBUG_LINKS,
    'debug_level': DEBUG_LEVEL,
    'verbose': 10
})


########################################
# Macsim 0
# Core 0 L1 Caches
macsim_0_core0_icache = sst.Component("macsim0_core0_icache", "memHierarchy.Cache")
macsim_0_core0_icache.addParams({
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

macsim_0_core0_dcache = sst.Component("macsim0_core0_dcache", "memHierarchy.Cache")
macsim_0_core0_dcache.addParams({
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
# ########################################
# # Const Caches
macsim_0_core0_ccache = sst.Component("macsim0_core0_ccache", "memHierarchy.Cache")
macsim_0_core0_ccache.addParams({
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

macsim_0_core0_tcache = sst.Component("macsim0_core0_tcache", "memHierarchy.Cache")
macsim_0_core0_tcache.addParams({
    "access_latency_cycles" : "3",
    "cache_frequency" : "3.5Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MSI",
    "associativity" : "4",
    "cache_line_size" : "64",  #set this to be 128 to align with int block_size = KNOB(KNOB_L1_SMALL_LINE_SIZE)->getValue(); 
    "debug" : DEBUG_L1,
    "debug_level" : DEBUG_LEVEL,
    "verbose" : VERBOSE,
    "L1" : "1",
    "cache_size" : "2KiB"
})

########################################
# Macsim 0
# Core 1 L1 Caches
macsim_0_core1_icache = sst.Component("macsim0_core1_icache", "memHierarchy.Cache")
macsim_0_core1_icache.addParams({
    "access_latency_cycles": "3",
    "cache_frequency": "3.5Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MSI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": DEBUG_L1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "2KiB"
})

macsim_0_core1_dcache = sst.Component("macsim0_core1_dcache", "memHierarchy.Cache")
macsim_0_core1_dcache.addParams({
    "access_latency_cycles": "3",
    "cache_frequency": "3.5Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MSI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": DEBUG_L1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "2KiB"
})

macsim_0_core1_ccache = sst.Component("macsim0_core1_ccache", "memHierarchy.Cache")
macsim_0_core1_ccache.addParams({
    "access_latency_cycles": "3",
    "cache_frequency": "3.5Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MSI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": DEBUG_L1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "2KiB"
})

macsim_0_core1_tcache = sst.Component("macsim0_core1_tcache", "memHierarchy.Cache")
macsim_0_core1_tcache.addParams({
    "access_latency_cycles": "3",
    "cache_frequency": "3.5Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MSI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": DEBUG_L1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "2KiB"
})



########################################
# Macsim 1
# Core 0 L1 Caches
macsim_1_core0_icache = sst.Component("macsim1_core0_icache", "memHierarchy.Cache")
macsim_1_core0_icache.addParams({
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

macsim_1_core0_dcache = sst.Component("macsim1_core0_dcache", "memHierarchy.Cache")
macsim_1_core0_dcache.addParams({
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
# ########################################
# # Const Caches
macsim_1_core0_ccache = sst.Component("macsim1_core0_ccache", "memHierarchy.Cache")
macsim_1_core0_ccache.addParams({
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

macsim_1_core0_tcache = sst.Component("macsim1_core0_tcache", "memHierarchy.Cache")
macsim_1_core0_tcache.addParams({
    "access_latency_cycles" : "3",
    "cache_frequency" : "3.5Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MSI",
    "associativity" : "4",
    "cache_line_size" : "64",  #set this to be 128 to align with int block_size = KNOB(KNOB_L1_SMALL_LINE_SIZE)->getValue(); 
    "debug" : DEBUG_L1,
    "debug_level" : DEBUG_LEVEL,
    "verbose" : VERBOSE,
    "L1" : "1",
    "cache_size" : "2KiB"
})

########################################
# Macsim 1
# Core 1 L1 Caches
macsim_1_core1_icache = sst.Component("macsim1_core1_icache", "memHierarchy.Cache")
macsim_1_core1_icache.addParams({
    "access_latency_cycles": "3",
    "cache_frequency": "3.5Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MSI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": DEBUG_L1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "2KiB"
})

macsim_1_core1_dcache = sst.Component("macsim1_core1_dcache", "memHierarchy.Cache")
macsim_1_core1_dcache.addParams({
    "access_latency_cycles": "3",
    "cache_frequency": "3.5Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MSI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": DEBUG_L1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "2KiB"
})

macsim_1_core1_ccache = sst.Component("macsim1_core1_ccache", "memHierarchy.Cache")
macsim_1_core1_ccache.addParams({
    "access_latency_cycles": "3",
    "cache_frequency": "3.5Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MSI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": DEBUG_L1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "2KiB"
})

macsim_1_core1_tcache = sst.Component("macsim1_core1_tcache", "memHierarchy.Cache")
macsim_1_core1_tcache.addParams({
    "access_latency_cycles": "3",
    "cache_frequency": "3.5Ghz",
    "replacement_policy": "lru",
    "coherence_protocol": "MSI",
    "associativity": "4",
    "cache_line_size": "64",
    "debug": DEBUG_L1,
    "debug_level": DEBUG_LEVEL,
    "verbose": VERBOSE,
    "L1": "1",
    "cache_size": "2KiB"
})

########################################
# L2 caches 
gpu_l2cache = sst.Component("l2cache", "memHierarchy.Cache")
gpu_l2cache.addParams({
    "access_latency_cycles" : "8",
    "cache_frequency" : "4Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MSI",
    "associativity" : "8",
    "cache_line_size" : "64", 
    "debug" : DEBUG_L1,
    "debug_level" : DEBUG_LEVEL,
    "verbose" : VERBOSE,
    "L1" : "0",
    "cache_size" : "64KiB"
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
# Macsim 0 Links

# Core 0 Links
# Macsim::core0_icache -> L1 ICache
link_macsim_0_icache = sst.Link("link_macsim_0_icache")
link_macsim_0_icache.connect( (macsim_0_icache_if, "port", "1000ps"), (macsim_0_core0_icache, "high_network_0", "1000ps") )
# Macsim::core0_dcache -> L1 DCache
link_macsim_0_dcache = sst.Link("link_macsim_0_dcache")
link_macsim_0_dcache.connect( (macsim_0_dcache_if, "port", "1000ps"), (macsim_0_core0_dcache, "high_network_0", "1000ps") )
# # Macsim::core0_ccache -> L1 CCache
link_macsim_0_ccache = sst.Link("link_macsim_0_ccache")
link_macsim_0_ccache.connect( (macsim_0_ccache_if, "port", "1000ps"), (macsim_0_core0_ccache, "high_network_0", "1000ps") )
# # Macsim::core0_tcache -> L1 TCache
link_macsim_0_tcache = sst.Link("link_macsim_0_tcache")
link_macsim_0_tcache.connect( (macsim_0_tcache_if, "port", "1000ps"), (macsim_0_core0_tcache, "high_network_0", "1000ps") )

# Core 1 Links
# Macsim::core1_icache -> L1 ICache
link_macsim_0_icache_1 = sst.Link("link_macsim_0_icache_1")
link_macsim_0_icache_1.connect((macsim_0_icache_if_1, "port", "1000ps"), (macsim_0_core1_icache, "high_network_0", "1000ps"))
# Macsim::core1_dcache -> L1 DCache
link_macsim_0_dcache_1 = sst.Link("link_macsim_0_dcache_1")
link_macsim_0_dcache_1.connect((macsim_0_dcache_if_1, "port", "1000ps"), (macsim_0_core1_dcache, "high_network_0", "1000ps"))
# # Macsim::core1_ccache -> L1 CCache
link_macsim_0_ccache_1 = sst.Link("link_macsim_0_ccache_1")
link_macsim_0_ccache_1.connect((macsim_0_ccache_if_1, "port", "1000ps"), (macsim_0_core1_ccache, "high_network_0", "1000ps"))
 # Macsim::core1_tcache -> L1 TCache
link_macsim_0_tcache_1 = sst.Link("link_macsim_0_tcache_1")
link_macsim_0_tcache_1.connect((macsim_0_tcache_if_1, "port", "1000ps"), (macsim_0_core1_tcache, "high_network_0", "1000ps"))

# L1 I/DCache -> Bus
macsim_0_link_icache_bus = sst.Link("macsim_0_link_icache_bus")
macsim_0_link_icache_bus.connect( (macsim_0_core0_icache, "low_network_0", "50ps"), (mem_bus, "high_network_0", "50ps") )
macsim_0_link_dcache_bus = sst.Link("macsim_0_link_dcache_bus")
macsim_0_link_dcache_bus.connect( (macsim_0_core0_dcache, "low_network_0", "50ps"), (mem_bus, "high_network_1", "50ps") )
macsim_0_link_ccache_bus = sst.Link("macsim_0_link_ccache_bus")
macsim_0_link_ccache_bus.connect( (macsim_0_core0_ccache, "low_network_0", "50ps"), (mem_bus, "high_network_2", "50ps") )
macsim_0_link_tcache_bus = sst.Link("macsim_0_link_tcache_bus")
macsim_0_link_tcache_bus.connect( (macsim_0_core0_tcache, "low_network_0", "50ps"), (mem_bus, "high_network_3", "50ps") )

macsim_0_link_icache_bus_1 = sst.Link("macsim_0_link_icache_bus_1")
macsim_0_link_icache_bus_1.connect((macsim_0_core1_icache, "low_network_0", "50ps"), (mem_bus, "high_network_4", "50ps"))
macsim_0_link_dcache_bus_1 = sst.Link("macsim_0_link_dcache_bus_1")
macsim_0_link_dcache_bus_1.connect((macsim_0_core1_dcache, "low_network_0", "50ps"), (mem_bus, "high_network_5", "50ps"))
macsim_0_link_ccache_bus_1 = sst.Link("macsim_0_link_ccache_bus_1")
macsim_0_link_ccache_bus_1.connect((macsim_0_core1_ccache, "low_network_0", "50ps"), (mem_bus, "high_network_6", "50ps"))
macsim_0_link_tcache_bus_1 = sst.Link("macsim_0_link_tcache_bus_1")
macsim_0_link_tcache_bus_1.connect((macsim_0_core1_tcache, "low_network_0", "50ps"), (mem_bus, "high_network_7", "50ps"))


########################################
# Macsim 1 Links

# Core 0 Links
# Macsim::core0_icache -> L1 ICache
link_macsim_1_icache = sst.Link("link_macsim_1_icache")
link_macsim_1_icache.connect( (macsim_1_icache_if, "port", "1000ps"), (macsim_1_core0_icache, "high_network_0", "1000ps") )
# Macsim::core0_dcache -> L1 DCache
link_macsim_1_dcache = sst.Link("link_macsim_1_dcache")
link_macsim_1_dcache.connect( (macsim_1_dcache_if, "port", "1000ps"), (macsim_1_core0_dcache, "high_network_0", "1000ps") )
# # Macsim::core0_ccache -> L1 CCache
link_macsim_1_ccache = sst.Link("link_macsim_1_ccache")
link_macsim_1_ccache.connect( (macsim_1_ccache_if, "port", "1000ps"), (macsim_1_core0_ccache, "high_network_0", "1000ps") )
# # Macsim::core0_tcache -> L1 TCache
link_macsim_1_tcache = sst.Link("link_macsim_1_tcache")
link_macsim_1_tcache.connect( (macsim_1_tcache_if, "port", "1000ps"), (macsim_1_core0_tcache, "high_network_0", "1000ps") )

# Core 1 Links
# Macsim::core1_icache -> L1 ICache
link_macsim_1_icache_1 = sst.Link("link_macsim_1_icache_1")
link_macsim_1_icache_1.connect((macsim_1_icache_if_1, "port", "1000ps"), (macsim_1_core1_icache, "high_network_0", "1000ps"))
# Macsim::core1_dcache -> L1 DCache
link_macsim_1_dcache_1 = sst.Link("link_macsim_1_dcache_1")
link_macsim_1_dcache_1.connect((macsim_1_dcache_if_1, "port", "1000ps"), (macsim_1_core1_dcache, "high_network_0", "1000ps"))
# # Macsim::core1_ccache -> L1 CCache
link_macsim_1_ccache_1 = sst.Link("link_macsim_1_ccache_1")
link_macsim_1_ccache_1.connect((macsim_1_ccache_if_1, "port", "1000ps"), (macsim_1_core1_ccache, "high_network_0", "1000ps"))
 # Macsim::core1_tcache -> L1 TCache
link_macsim_1_tcache_1 = sst.Link("link_macsim_1_tcache_1")
link_macsim_1_tcache_1.connect((macsim_1_tcache_if_1, "port", "1000ps"), (macsim_1_core1_tcache, "high_network_0", "1000ps"))

# L1 I/DCache -> Bus
macsim_1_link_icache_bus = sst.Link("macsim_1_link_icache_bus")
macsim_1_link_icache_bus.connect( (macsim_1_core0_icache, "low_network_0", "50ps"), (mem_bus, "high_network_8", "50ps") )
macsim_1_link_dcache_bus = sst.Link("macsim_1_link_dcache_bus")
macsim_1_link_dcache_bus.connect( (macsim_1_core0_dcache, "low_network_0", "50ps"), (mem_bus, "high_network_9", "50ps") )
macsim_1_link_ccache_bus = sst.Link("macsim_1_link_ccache_bus")
macsim_1_link_ccache_bus.connect( (macsim_1_core0_ccache, "low_network_0", "50ps"), (mem_bus, "high_network_10", "50ps") )
macsim_1_link_tcache_bus = sst.Link("macsim_1_link_tcache_bus")
macsim_1_link_tcache_bus.connect( (macsim_1_core0_tcache, "low_network_0", "50ps"), (mem_bus, "high_network_11", "50ps") )

macsim_1_link_icache_bus_1 = sst.Link("macsim_1_link_icache_bus_1")
macsim_1_link_icache_bus_1.connect((macsim_1_core1_icache, "low_network_0", "50ps"), (mem_bus, "high_network_12", "50ps"))
macsim_1_link_dcache_bus_1 = sst.Link("macsim_1_link_dcache_bus_1")
macsim_1_link_dcache_bus_1.connect((macsim_1_core1_dcache, "low_network_0", "50ps"), (mem_bus, "high_network_13", "50ps"))
macsim_1_link_ccache_bus_1 = sst.Link("macsim_1_link_ccache_bus_1")
macsim_1_link_ccache_bus_1.connect((macsim_1_core1_ccache, "low_network_0", "50ps"), (mem_bus, "high_network_14", "50ps"))
macsim_1_link_tcache_bus_1 = sst.Link("macsim_1_link_tcache_bus_1")
macsim_1_link_tcache_bus_1.connect((macsim_1_core1_tcache, "low_network_0", "50ps"), (mem_bus, "high_network_15", "50ps"))

# Bus -> Memory
link_bus_L2 = sst.Link("link_bus_L2")
link_bus_L2.connect( (mem_bus, "low_network_0", "50ps"), (gpu_l2cache, "high_network_0", "50ps") )
link_bus_mem = sst.Link("link_bus_mem")
link_bus_mem.connect( (gpu_l2cache, "low_network_0", "50ps"), (memctrl, "direct_link", "50ps") )


########################################
# Enable statistics
sst.setStatisticLoadLevel(7)
sst.setStatisticOutput("sst.statOutputConsole")