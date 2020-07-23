clock = "1020MHz"
cache_clock = "1020MHz"
memory_clock = "1674MHz"
coherence_protocol = "MESI"
replacement_policy = "LRU"

slices = 8
dsses_per_slice = 2
cores_per_dss = 2
l3cache_nodes_per_slice = 1
memory_controllers_per_slice = 1
macsim_cores = 24

link_latency = "50ps"
link_bandwidth = "16GB/s"
xbar_bandwidth = "32GB/s"
flit_size = "64B"
input_latency = "20ns"
output_latency = "20ns"
input_buf_size = "2KB"
output_buf_size = "2KB"

cache_line_size = 64

memory_network_bandwidth = "32GB/s"

mem_interleave_size = cache_line_size #1073741824 #2097152 #64 #4096	# Do 4K page level interleaving
memory_capacity = 16384 	# Size of memory in MBs

num_loadstore = 32768
cpu_params = {
    "do_write" : 1,
    #  "num_loadstore" : 100000,
    "commFreq" : 100,
    #  "memSize" : memory_capacity * 1024 * 1024,
    "memSize" : 512 * 1024,
    "startAddr" : cache_line_size * 1,
    "step" : cache_line_size * slices,
    "numOutstandingRequests" : 1,
    "debug" : 0
}

num_cores = slices * dsses_per_slice * cores_per_dss

gpu_params = {
    "mem_size" : memory_capacity * 1024 * 1024,
    "command_line" : "--use_memhierarchy=1 --num_sim_cores=%d --num_sim_large_cores=%d" % (macsim_cores, macsim_cores),
    "param_file" : "params.in",
    "trace_file" : "trace_file_list",
    "ptx_core" : 0,
    "num_link" : num_cores,
    "frequency" : clock,
    "output_dir" : ".",
    "debug" : 0,
    "debug_addr" : 0,
    "debug_level" : 5,
}

l1_params = {
    "L1": 1,
    "cache_frequency": clock,
    "coherence_protocol": coherence_protocol,
    "replacement_policy": replacement_policy,
    "cache_size": "1KB",
    "maxRequestDelay" : "1000000",
    "associativity": 8,
    "cache_line_size": cache_line_size,
    "access_latency_cycles": 1,
    "debug": 0,
    "debug_level" : 9,
}

l2_params = {
    "L1": 0,
    "cache_frequency": clock,
    "coherence_protocol": coherence_protocol,
    "replacement_policy": replacement_policy,
    "cache_size": "1MB",
    "associativity": 16,
    "cache_line_size": cache_line_size,
    "access_latency_cycles": 10,
    "mshr_num_entries" : 128,
    "debug": 0,
    "debug_level" : 9,
}

l3_params = {
    "L1" : 0,
    "cache_frequency" : cache_clock,
    "replacement_policy" : replacement_policy,
    "coherence_protocol" : coherence_protocol,
    "cache_size" : "2MB",
    "associativity" : 32,
    "cache_line_size" : cache_line_size,
    "access_latency_cycles" : 30,
    "mshr_num_entries" : 1024,
    "num_cache_slices" : str(slices * l3cache_nodes_per_slice),
    "slice_allocation_policy" : "rr",
    "debug" : 0,
    "debug_level" : 9,
}

dc_params = {
    "coherence_protocol" : coherence_protocol,
    "network_bw" : memory_network_bandwidth,
    "interleave_size" : str(mem_interleave_size) + "B",
    "interleave_step" : str((slices * memory_controllers_per_slice) * mem_interleave_size) + "B",
    "entry_cache_size" : 256*1024*1024, #Entry cache size of mem/blocksize
    "clock" : memory_clock,
    "debug" : 0,
}

mem_params = {
    "coherence_protocol" : coherence_protocol,
    "backend.access_time" : "100ns",
    "rangeStart" : 0,
    "backend.mem_size" : memory_capacity / (slices * memory_controllers_per_slice),
    "clock" : memory_clock,
}
