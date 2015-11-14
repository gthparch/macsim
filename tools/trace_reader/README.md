- first argument: trace file txt name 
- second argument: # number of instructions per sliced traces  (if the second argument is not specified, it will just write the original trace size)

Example:
```sh
trace_reader ../../sst-unit-test/traces/x86_sep2013/x86/spec2006/trace_simpoint/xalancbmk/pin_traces/xalancbmk.1.txt 1000
```
sliced traces are:

../../sst-unit-test/traces/x86_sep2013/x86/spec2006/trace_simpoint/xalancbmk/pin_traces/xalancbmk.1_s0_0.raw
../../sst-unit-test/traces/x86_sep2013/x86/spec2006/trace_simpoint/xalancbmk/pin_traces/xalancbmk.1_s1_0.raw
../../sst-unit-test/traces/x86_sep2013/x86/spec2006/trace_simpoint/xalancbmk/pin_traces/xalancbmk.1_s2_0.raw

etc. 

change trace format 

(default setting is X86) 

Change the paramter in Sconscript file 
uncomment one of the right trace type (TODO to change it as a variable) 
env['CPPDEFINES'] = 'X86_TRACE'
#env['CPPDEFINES'] = 'GPU_TRACE'
#env['CPPDEFINES'] = 'ARM64_TRACE'
