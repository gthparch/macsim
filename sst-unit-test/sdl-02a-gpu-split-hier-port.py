################################################################################
# Description: 
#  - Macsim is connected to 2 individual memory controllers for instruction and
#    data memory
################################################################################
import sst
from common import *

# 0: None, 1: Stdout, 2: Stderr, 3: File
DEBUG_CORE          = 1
DEBUG_CORE_LINKS    = 0
DEBUG_L1            = 0
DEBUG_MEM           = 0

DEBUG_LINKS         = 0
DEBUG_BUS           = 0

DEBUG_LEVEL         = 0
VERBOSE             = 0

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
    "trace_file": "trace_file_list_gpu",
    "output_dir": "output_dir",
    "command_line": "--num_sim_cores=1 --num_sim_large_cores=0 --num_sim_small_cores=1 --use_memhierarchy=1 --core_type=nvbit",
    "frequency" : "2GHz",
    "num_cores" : "1",
    "num_link": "1",
    "mem_size" : MEM_SIZE,
    "debug": DEBUG_CORE,
    "debug_level": DEBUG_LEVEL,
    "nvbit_core": True,
})


########################################
# Instruction Memory Controller
memctrl_i = sst.Component("memory_i", "memHierarchy.MemController")
memctrl_i.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    # "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})
memory_i = memctrl_i.setSubComponent("backend", "memHierarchy.simpleMem")
memory_i.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})


########################################
# Data Memory Controller
memctrl_d = sst.Component("memory_d", "memHierarchy.MemController")
memctrl_d.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    "verbose" : VERBOSE,
    "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})
memory_d = memctrl_d.setSubComponent("backend", "memHierarchy.simpleMem")
memory_d.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})

########################################
# Constant Memory Controller
memctrl_c = sst.Component("memory_c", "memHierarchy.MemController")
memctrl_c.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    "verbose" : VERBOSE,
    "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})
memory_c = memctrl_c.setSubComponent("backend", "memHierarchy.simpleMem")
memory_c.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})

########################################
# Texture Memory Controller
memctrl_t = sst.Component("memory_t", "memHierarchy.MemController")
memctrl_t.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    "verbose" : VERBOSE,
    "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})
memory_t = memctrl_t.setSubComponent("backend", "memHierarchy.simpleMem")
memory_t.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})


########################################
# Links
link_bus_memctrl_i = sst.Link("link_bus_memctrl_i")
link_bus_memctrl_i.connect((macsim, "macsim0_core0_icache", "50ps"), (memctrl_i, "direct_link", "50ps"))
link_bus_memctrl_d = sst.Link("link_bus_memctrl_d")
link_bus_memctrl_d.connect((macsim, "macsim0_core0_dcache", "50ps"), (memctrl_d, "direct_link", "50ps"))
link_bus_memctrl_c = sst.Link("link_bus_memctrl_c")
link_bus_memctrl_c.connect((macsim, "macsim0_core0_ccache", "50ps"), (memctrl_c, "direct_link", "50ps"))
link_bus_memctrl_t = sst.Link("link_bus_memctrl_t")
link_bus_memctrl_t.connect((macsim, "macsim0_core0_tcache", "50ps"), (memctrl_t, "direct_link", "50ps"))


########################################
# Enable statistics
sst.setStatisticLoadLevel(7)
sst.setStatisticOutput("sst.statOutputConsole")
# sst.setStatisticOutput("sst.statOutputCSV", {"filepath" : "./sst_stats.csv", "separator" : ", " } )

sst.enableAllStatisticsForComponentType("memHierarchy.MemController")
