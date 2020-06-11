Note:

This tool is only for converting old PIN traces to new PIN 3.13 Traces

Build:

Run scons to build the trace converter. You can specify an argument for the
architecture of the trace converter you want to build. The default architecture is
x86. The other options are *gpu*, *a64*.

$ scons arch=gpu

Clean:
$ scons --clean

Running:

Arguments
- first argument: trace file txt name 
- second argument: # number of instructions per sliced traces  (if the second
  argument is not specified, it will just write the original trace size)

Example:
```sh
trace_converter ../../sst-unit-test/traces/x86/mergesort.txt 1000
```
sliced and converted traces are:

../../sst-unit-test/traces/x86/mergesort_s0_0.raw

../../sst-unit-test/traces/x86/mergesort_s1_0.raw

../../sst-unit-test/traces/x86/mergesort_s2_0.raw

etc. 
