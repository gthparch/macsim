import sst
import os
import params

sst.setProgramOption("timebase", "1ps")
sst.setProgramOption("stopAtCycle", "10ms")

#################################################
# xbar related params go here
#################################################

local_ports = params.slices * (params.l3cache_nodes_per_slice + params.memory_controllers_per_slice + params.dsses_per_slice)

router_params = {
    "topology" : "merlin.singlerouter",
    "num_ports" : local_ports,
    "num_vns" : 1,
    "debug" : 0,
    "flit_size" : params.flit_size,
    "link_bw" : params.link_bandwidth,
    "xbar_bw" : params.xbar_bandwidth,
    "input_latency" : params.input_latency,
    "output_latency" : params.output_latency,
    "input_buf_size" : params.input_buf_size,
    "output_buf_size" : params.output_buf_size,
}

#################################################

#  num_cpus = params.slices * params.dsses_per_slice * params.cores_per_dss
#  cpu = [None for x in range(num_cpus)]
#  for cpu_id in range(num_cpus):
    #  cpu[cpu_id] = sst.Component("cpu_%d" % cpu_id, "memHierarchy.trivialCPU")
    #  cpu[cpu_id].addParams(params.cpu_params)
    #  if cpu_id == 0:
        #  cpu[cpu_id].addParams({ "num_loadstore" : params.num_loadstore })
    #  else:
        #  cpu[cpu_id].addParams({ "num_loadstore" : 0 })

#################################################

gpu = sst.Component("gpu", "macsimComponent.macsimComponent")
gpu.addParams(params.gpu_params)

#################################################

rtr = sst.Component("rtr", "merlin.hr_router")
rtr.addParams(router_params)
rtr.addParam("id", 0)

port = 0
next_network_id = 0
next_memory_ctrl_id = 0

for next_slice in xrange(params.slices):
    for next_dss in range(params.dsses_per_slice):
        l2 = sst.Component("s%d_dss%d_l2" % (next_slice, next_dss), "memHierarchy.Cache")
        l2.addParams(params.l2_params)
        l2.addParams({"network_address" : next_network_id})
        next_network_id = next_network_id + 1

        bus = sst.Component("s%d_dss%d_bus" % (next_slice, next_dss), "memHierarchy.Bus")
        bus.addParams({"bus_frequency" : params.clock, "debug" : 0, "debug_level" : 9})

        for next_core in range(params.cores_per_dss):
            core_id = next_slice * params.dsses_per_slice * params.cores_per_dss + next_dss * params.cores_per_dss + next_core

            #################################################

            #  l1 = sst.Component("s%d_dss%d_core%d_l1cache" % (next_slice, next_dss, next_core), "memHierarchy.Cache")
            #  l1.addParams(params.l1_params)

            #  link = sst.Link("s%d_dss%d_core%d:l1" % (next_slice, next_dss, next_core))
            #  link.connect((cpu[core_id], "mem_link", params.link_latency), (l1, "high_network_0", params.link_latency))

            #  link = sst.Link("s%d_dss%d_core%d_l1:bus" % (next_slice, next_dss, next_core))
            #  link.connect((l1, "low_network_0", params.link_latency), (bus, "high_network_%d" % next_core, params.link_latency))

            #################################################

            l1i = sst.Component("s%d_dss%d_core%d_l1icache" % (next_slice, next_dss, next_core), "memHierarchy.Cache")
            l1i.addParams(params.l1_params)

            l1d = sst.Component("s%d_dss%d_core%d_l1dcache" % (next_slice, next_dss, next_core), "memHierarchy.Cache")
            l1d.addParams(params.l1_params)

            link = sst.Link("s%d_dss%d_core%d:l1i" % (next_slice, next_dss, next_core))
            link.connect((gpu, "core%d-icache" % core_id, params.link_latency), (l1i, "high_network_0", params.link_latency))

            link = sst.Link("s%d_dss%d_core%d:l1d" % (next_slice, next_dss, next_core))
            link.connect((gpu, "core%d-dcache" % core_id, params.link_latency), (l1d, "high_network_0", params.link_latency))

            link = sst.Link("s%d_dss%d_core%d_l1i:bus" % (next_slice, next_dss, next_core))
            link.connect((l1i, "low_network_0", params.link_latency), (bus, "high_network_%d" % (2*next_core+0), params.link_latency))

            link = sst.Link("s%d_dss%d_core%d_l1d:bus" % (next_slice, next_dss, next_core))
            link.connect((l1d, "low_network_0", params.link_latency), (bus, "high_network_%d" % (2*next_core+1), params.link_latency))

            #################################################

        link = sst.Link("s%d_dss%d_bus:l2" % (next_slice, next_dss)) 
        link.connect((bus, "low_network_0", params.link_latency), (l2, "high_network_0", params.link_latency))
        
        link = sst.Link("s%d_dss%d_l2:rtr" % (next_slice, next_dss))
        link.connect((l2, "cache", params.link_latency), (rtr, "port%d" % port, params.link_latency))
        port = port + 1

    for next_l3_cache_block in range(params.l3cache_nodes_per_slice):
        l3idx = (next_slice * params.l3cache_nodes_per_slice) + next_l3_cache_block
        l3cache = sst.Component("l3cache" + str(l3idx), "memHierarchy.Cache")
        l3cache.addParams(params.l3_params)
        l3cache.addParams({"slice_id" : str(l3idx)})
        l3cache.addParams({"network_address" : next_network_id})
        next_network_id = next_network_id + 1
        
        link = sst.Link("l3_ring_link_" + str(l3idx))
        link.connect((l3cache, "directory", params.link_latency), (rtr, "port%d" % port, params.link_latency))
        port = port + 1

    for next_mem_ctrl in range(params.memory_controllers_per_slice):	
        num_memory_controllers = params.slices * params.memory_controllers_per_slice
        local_size = params.memory_capacity / num_memory_controllers
        
        mem = sst.Component("memory_" + str(next_memory_ctrl_id), "memHierarchy.MemController")
        mem.addParams(params.mem_params)
        
        dc = sst.Component("dc_" + str(next_memory_ctrl_id), "memHierarchy.DirectoryController")
        dc.addParams(params.dc_params)
        dc.addParams({"addr_range_start" : next_memory_ctrl_id * params.mem_interleave_size}) 
        dc.addParams({"addr_range_end" : (params.memory_capacity * 1024 * 1024) - (num_memory_controllers * params.mem_interleave_size) + (next_memory_ctrl_id * params.mem_interleave_size)})
        dc.addParams({"network_address" : next_network_id}) 
        next_network_id = next_network_id + 1

        link = sst.Link("mem_link_" + str(next_memory_ctrl_id))
        link.connect((mem, "direct_link", params.link_latency), (dc, "memory", params.link_latency))
        
        link = sst.Link("dc_link_" + str(next_memory_ctrl_id))
        link.connect((dc, "network", params.link_latency), (rtr, "port%d" % port, params.link_latency)) 
        port = port + 1
        
        next_memory_ctrl_id = next_memory_ctrl_id + 1




# Enable SST Statistics Outputs for this simulation
sst.setStatisticLoadLevel(4)
sst.enableAllStatisticsForAllComponents({"type":"sst.AccumulatorStatistic"})
sst.enableAllStatisticsForComponentType("macsimComponent.macsimComponent")
sst.enableAllStatisticsForComponentType("memHierarchy.Cache")
sst.enableAllStatisticsForComponentType("memHierarchy.MemController")

sst.setStatisticOutput("sst.statOutputCSV")
sst.setStatisticOutputOptions({
    "filepath" : "sst-xbar.stat.csv",
    "separator" : ",",
    "outputtopheader" : 1,
    "outputsimtime" : 1,
    "outputrank" : 1
})
