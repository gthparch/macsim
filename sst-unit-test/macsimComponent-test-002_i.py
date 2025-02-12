import sst

################################################################################
# Parameters
DEBUG_L1 = 1
DEBUG_MEM = 1
DEBUG_CORE = 1
DEBUG_LEVEL = 10

MEM_SIZE_S = '512MiB'

MEM_SIZE = str2bytes(MEM_SIZE_S)
MEM_START = 0
MEM_END = MEM_START + MEM_SIZE - 1

print("MEM_SIZE: %d (%s): 0x%x - 0x%x" % (MEM_SIZE, MEM_SIZE_S, MEM_START, MEM_END))
   

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
    "mem_size" : MEM_SIZE,
    "debug_level": "9",
})

l1_icache = sst.Component("l1_icache", "memHierarchy.Cache")
l1_icache.addParams({
      "access_latency_cycles" : "4",
      "cache_frequency" : "2 Ghz",
      "replacement_policy" : "lru",
      "coherence_protocol" : "MSI",
      "associativity" : "4",
      "cache_line_size" : "64",
      "debug_level" : DEBUG_LEVEL,
      "L1" : "1",
      "debug" : DEBUG_L1,
      "cache_size" : "64 KB"
})

l1_dcache = sst.Component("l1_dcache", "memHierarchy.Cache")
l1_dcache.addParams({
      "access_latency_cycles" : "4",
      "cache_frequency" : "2 Ghz",
      "replacement_policy" : "lru",
      "coherence_protocol" : "MSI",
      "associativity" : "4",
      "cache_line_size" : "64",
      "debug_level" : DEBUG_LEVEL,
      "L1" : "1",
      "debug" : DEBUG_L1,
      "cache_size" : "64 KB"
})

# Bus between caches and memory controller
mem_bus = sst.Component("mem_bus", "memHierarchy.Bus")
mem_bus.addParams({
     "debug" : "8",
     "bus_frequency" : "4 Ghz"
})

# Memory Controller
memctrl = sst.Component("memory", "memHierarchy.MemController")
memctrl.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    # "addr_range_start" : MEM_START,
    "addr_range_end" : MEM_END,
})

# Memory
memory = memctrl.setSubComponent("backend", "memHierarchy.simpleMem")
memory.addParams({
    "access_time" : "1000ns",
    "mem_size" : MEM_SIZE_S
})


########################################
# Enable statistics
sst.setStatisticLoadLevel(7)
sst.setStatisticOutput("sst.statOutputConsole")

########################################
# Links

# Macsim::icache -> l1_icache
link_msi_l1icache = sst.Link("link_msi_l1icache")
link_msi_l1icache.connect((macsim, "core0_icache", "1000ps"), (l1_icache, "highlink", "1000ps"))

# Macsim::dcache -> l1_dcache
link_msd_l1dcache = sst.Link("link_msd_l1dcache")
link_msd_l1dcache.connect((macsim, "core0_dcache", "1000ps"), (l1_dcache, "highlink", "1000ps"))

# L1_icache -> Bus
link_l1icache_bus = sst.Link("link_l1icache_bus")
link_l1icache_bus.connect((l1_icache, "lowlink", "50ps"), (mem_bus, "highlink0", "50ps"))

# L1_dcache -> Bus
# link_l1dcache_bus = sst.Link("link_l1dcache_bus")
# link_l1dcache_bus.connect((l1_dcache, "lowlink", "50ps"), (mem_bus, "highlink1", "50ps"))

# Bus -> memctrl
link_bus_memctrl = sst.Link("link_bus_memctrl")
link_bus_memctrl.connect((mem_bus, "lowlink0", "50ps"), (memctrl, "highlink", "50ps"))
