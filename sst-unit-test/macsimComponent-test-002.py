import sst

DEBUG_L1 = 0
DEBUG_MEM = 0
DEBUG_LEVEL = 10
verbose = 2

################################################################################
# Macsim -> L1 ICache -> (MemController) Memory
################################################################################
macsim = sst.Component("cpu0", "macsimComponent.macsimComponent")
macsim.addParams({
    "param_file": "params.in",
    "trace_file": "trace_file_list",
    "command_line": "--num_sim_cores=1 --num_sim_large_cores=1 --num_sim_small_cores=0",
    "output_dir": "output_dir",
    "frequency" : "2GHz",
    "num_cores" : "1",
    # "num_links": "1",
    "cache_size" : "512KB"
})

# ICache
core0_l1_icache = sst.Component("core0_icache", "memHierarchy.Cache")
core0_l1_icache.addParams({
    "access_latency_cycles" : "3",
    "cache_frequency" : "3.5Ghz",
    "replacement_policy" : "lru",
    "coherence_protocol" : "MSI",
    "associativity" : "4",
    "cache_line_size" : "64",
    "debug" : DEBUG_L1,
    "debug_level" : DEBUG_LEVEL,
    "verbose" : verbose,
    "L1" : "1",
    "cache_size" : "2KiB"
})


# Memory Controller
memctrl = sst.Component("memory", "memHierarchy.MemController")
memctrl.addParams({
    "debug" : DEBUG_MEM,
    "debug_level" : DEBUG_LEVEL,
    "clock" : "1GHz",
    "verbose" : verbose,
    "addr_range_end" : 512*1024*1024-1,
})

# Memory
memory = memctrl.setSubComponent("backend", "memHierarchy.simpleMem")
memory.addParams({
    "access_time" : "1000ns",
    "mem_size" : "512MiB"
})

# Macsim::core0_icache -> L1 ICache
link_macsim_icache = sst.Link("link_macsim_icache")
link_macsim_icache.connect( (macsim, "core0_icache", "1000ps"), (core0_l1_icache, "high_network_0", "1000ps") )

# L1 ICache -> Memory
link_icache_mem = sst.Link("link_icache_mem")
link_icache_mem.connect( (core0_l1_icache, "low_network_0", "50ps"), (memctrl, "direct_link", "50ps") )