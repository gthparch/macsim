import sst

################################################################################
# Util functions

KIB = 1024
MIB = 1024 * KIB
GIB = 1024 * MIB

def bytes2str(nbytes):
    if nbytes >= GIB:
        return "%dGiB" % (nbytes / GIB)
    elif nbytes >= MIB:
        return "%dMiB" % (nbytes / MIB)
    elif nbytes >= KIB:
        return "%dKiB" % (nbytes / KIB)
    else:
        return "%dB" % nbytes

def str2bytes(s):
    if s.endswith("GiB"):
        return int(s[:-3]) * GIB
    elif s.endswith("MiB"):
        return int(s[:-3]) * MIB
    elif s.endswith("KiB"):
        return int(s[:-3]) * KIB
    else:
        return int(s)

################################################################################
# Parameters
VERBOSE = 2
DEBUG_CORE = 8
DEBUG_L1 = 0
DEBUG_MEM = 0
DEBUG_LEVEL = 10
MEM_SIZE_S = '512MiB'

MEM_SIZE = str2bytes(MEM_SIZE_S)
MEM_START = 0
MEM_END = MEM_START + MEM_SIZE - 1

print("MEM_SIZE: %s" % MEM_SIZE)
print("MEM_START: %s" % MEM_START)
print("MEM_END: %s" % MEM_END)
   

################################################################################
# Macsim -> L1 Caches -> (MemController) Memory
################################################################################
macsim = sst.Component("macsim", "macsimComponent.macsimComponent")
macsim.addParams({
    "param_file": "params.in",
    "trace_file": "trace_file_list",
    "output_dir": "output_dir",
    "command_line": "--num_sim_cores=1 --num_sim_large_cores=1 --num_sim_small_cores=0 --use_memhierarchy=1 --core_type=x86",
    "frequency" : "2GHz",
    "num_cores" : "1",
    "num_links": "1",
    "m_mem_size" : MEM_SIZE,
    "debug_level": DEBUG_CORE
})

mem_bus = sst.Component("mem_bus", "memHierarchy.Bus")
mem_bus.addParams({
     "debug" : "0",
     "bus_frequency" : "4 Ghz"
})

link_coreicache_bus0 = sst.Link("link_coreicache_bus0")
link_coreicache_bus0.connect( (macsim, "core0_icache", "1000ps"), (mem_bus, "high_network_0", "1000ps") )
link_coredcache_bus1 = sst.Link("link_coredcache_bus1")
link_coredcache_bus1.connect( (macsim, "core0_dcache", "1000ps"), (mem_bus, "high_network_1", "1000ps") )

# Memory Controller
memctrl = sst.Component("memory", "memHierarchy.MemController")
memctrl.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "2GHz",
    "verbose" : VERBOSE,
    "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})
# Memory
memory = memctrl.setSubComponent("backend", "memHierarchy.simpleMem")
memory.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})


# Bus -> memctrl
link_bus_memctrl = sst.Link("link_bus_memctrl")
link_bus_memctrl.connect( (mem_bus, "low_network_0", "50ps"), (memctrl, "direct_link", "50ps") )
