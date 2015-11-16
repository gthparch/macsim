Build:

Run scons to build the trace reader. You can specify an argument for the
architecture of the trace reader you want to build. The default architecture is
x86. The other options are *gpu*, *a64*.

$ scons arch=gpu

Running:

Arguments
- first argument: trace file txt name 
- second argument: # number of instructions per sliced traces  (if the second
  argument is not specified, it will just write the original trace size)

Example:
```sh
trace_reader ../../sst-unit-test/traces/x86_sep2013/x86/spec2006/trace_simpoint/xalancbmk/pin_traces/xalancbmk.1.txt 1000
```
sliced traces are:

../../sst-unit-test/traces/x86_sep2013/x86/spec2006/trace_simpoint/xalancbmk/pin_traces/xalancbmk.1_s0_0.raw

../../sst-unit-test/traces/x86_sep2013/x86/spec2006/trace_simpoint/xalancbmk/pin_traces/xalancbmk.1_s1_0.raw

../../sst-unit-test/traces/x86_sep2013/x86/spec2006/trace_simpoint/xalancbmk/pin_traces/xalancbmk.1_s2_0.raw

etc. 
