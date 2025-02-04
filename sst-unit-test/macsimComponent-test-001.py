import sst

obj = sst.Component("macsimComponent", "macsimComponent.macsimComponent")
obj.addParams({
    "clock" : "2GHz",
    "num_cores" : "4",
    "cache_size" : "512KB"
})
