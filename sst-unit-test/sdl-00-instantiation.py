################################################################################
# Description: 
#   - macsimComponent instantiation test 
#   - It will fail since nothing is connected
#   - It should something similar to the folowing:
#           FATAL: macsimComponent, Error: unable to configure link on port 'core0_icache'
#           SST Fatal Backtrace Information:
#           ...
################################################################################
import sst

obj = sst.Component("macsimComponent", "macsimComponent.macsimComponent")
obj.addParams({
    "param_file": "params.in",
    "trace_file": "trace_file_list",
    "output_dir": "output_dir",
    "frequency" : "2GHz",
    "num_cores" : "4",
    "cache_size" : "512KB"
})
