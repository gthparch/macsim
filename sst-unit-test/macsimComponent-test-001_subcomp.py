import sst
from common import *

DEBUG_L1 = 1
DEBUG_MEM = 1
DEBUG_CORE = 1
DEBUG_LEVEL = 9

MEM_SIZE_S = '512MiB'
MEM_SIZE = str2bytes(MEM_SIZE_S)
MEM_START = 0
MEM_END = MEM_START + MEM_SIZE - 1

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
macsim_icache_if = macsim.setSubComponent("core0_icache", "memHierarchy.standardInterface")
macsim_dcache_if = macsim.setSubComponent("core0_dcache", "memHierarchy.standardInterface")

# Instruction Memory Controller
memctrl_i = sst.Component("memory_i", "memHierarchy.MemController")
memctrl_i.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    # "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})

# Instruction Memory
memory_i = memctrl_i.setSubComponent("backend", "memHierarchy.simpleMem")
memory_i.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})


# Data Memory Controller
memctrl_d = sst.Component("memory_d", "memHierarchy.MemController")
memctrl_d.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    "verbose" : 1,
    # "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})

# Data Memory
memory_d = memctrl_d.setSubComponent("backend", "memHierarchy.simpleMem")
memory_d.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})

link_bus_memctrl_i = sst.Link("link_bus_memctrl_i")
link_bus_memctrl_i.connect((macsim_icache_if, "lowlink", "50ps"), (memctrl_i, "highlink", "50ps"))
link_bus_memctrl_d = sst.Link("link_bus_memctrl_d")
link_bus_memctrl_d.connect((macsim_dcache_if, "lowlink", "50ps"), (memctrl_d, "highlink", "50ps"))

