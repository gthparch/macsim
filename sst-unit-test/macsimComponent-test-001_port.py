################################################################################
# Description: 
#  - Macsim is connected to 2 individual memory controllers for instruction and
#    data memory
################################################################################
import sst
from common import *

# 0: None, 1: Stdout, 2: Stderr, 3: File
DEBUG_CORE  = 1
DEBUG_L1    = 0
DEBUG_MEM   = 1

DEBUG_LINKS = 0
DEBUG_BUS   = 0

DEBUG_LEVEL = 9
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
    "trace_file": "trace_file_list",
    "output_dir": "output_dir",
    "command_line": "--num_sim_cores=1 --num_sim_large_cores=1 --num_sim_small_cores=0 --use_memhierarchy=1 --core_type=x86",
    "frequency" : "2GHz",
    "num_cores" : "1",
    "num_links": "1",
    "mem_size" : MEM_SIZE,
    "debug": DEBUG_CORE,
    "debug_level": DEBUG_LEVEL,
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
# Links
link_bus_memctrl_i = sst.Link("link_bus_memctrl_i")
link_bus_memctrl_i.connect((macsim, "core0_icache", "50ps"), (memctrl_i, "direct_link", "50ps"))
link_bus_memctrl_d = sst.Link("link_bus_memctrl_d")
link_bus_memctrl_d.connect((macsim, "core0_dcache", "50ps"), (memctrl_d, "direct_link", "50ps"))


########################################
# Enable statistics
sst.setStatisticLoadLevel(7)
sst.setStatisticOutput("sst.statOutputConsole")
sst.enableAllStatisticsForComponentType("memHierarchy.MemController")