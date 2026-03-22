# MacSim

## Introduction

MacSim is a trace-based cycle-level GPGPU simulator developed by [HPArch](https://sites.gatech.edu/hparch/) at Georgia Institute of Technology.

* It simulates x86, ARM64, NVIDIA PTX and Intel GEN GPU instructions and can be configured as
  either a trace driven or execution-driven cycle level simulator. It models
  detailed micro-architectural behaviors, including pipeline stages,
  multi-threading, and memory systems.
* MacSim is capable of simulating a variety of architectures, such as Intel's
  Sandy Bridge, Skylake (both CPUs and GPUs) and NVIDIA's Fermi. It can simulate homogeneous ISA multicore
  simulations as well as heterogeneous ISA multicore simulations. It also
  supports asymmetric multicore configurations (small cores + medium cores + big
  cores) and SMT or MT architectures as well.
* Currently interconnection network model (based on IRIS) and power model (based
  on McPAT) are connected.
* MacSim is also one of the components of SST, so multiple MacSim simulators
  can run concurrently.
* The project has been supported by Intel, NSF, Sandia National Lab.

## Table of Contents
- [Note](#note)
- [Intel GEN GPU Architecture](#intel-gen-gpu-architecture)
- [Documentation](#documentation)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Downloading Traces](#downloading-traces)
- [Generating Your Own Traces](#generating-your-own-traces)
- [Known Bugs](#known-bugs)
- [People](#people)
- [Q & A](#q--a)
- [Tutorial](#tutorial)
- [SST+MacSim](#sstmacsim)

## Note

* If you're interested in the Intel's integrated GPU model in MacSim, please refer to [intel_gpu](https://github.com/gthparch/macsim/tree/intel_gpu) branch.

* We've developed a power model for GPU architecture using McPAT. Please refer
  to the following paper for more detailed
  information. [Power Modeling for GPU Architecture using McPAT](http://www.cercs.gatech.edu/tech-reports/tr2013/git-cercs-13-10.pdf)
  Modeling for GPU Architecture using McPAT.pdf) by Jieun Lim, Nagesh
  B. Lakshminarayana, Hyesoon Kim, William Song, Sudhakar Yalamanchili, Wonyong
  Sung, from Transactions on Design Automation of Electronic Systems (TODAES)
  Vol. 19, No. 3.
* We've characterised the performance of Intel's integrated GPUs using MacSim. Please refer to the following paper for more detailed information. [Performance Characterisation and Simulation of Intel's Integrated GPU Architecture (ISPASS'18)](http://comparch.gatech.edu/hparch/papers/gera_ispass18.pdf)

## Intel GEN GPU Architecture
* Intel GEN9 GPU Architecture: ![](http://comparch.gatech.edu/hparch/images/intel_gen9_arch.png)

## Documentation

Please see [MacSim documentation file](https://github.com/gthparch/macsim/blob/master/doc/macsim.pdf) for more detailed descriptions.

## Installation

### Prerequisites

- **zlib** (development library)
  ```bash
  # Ubuntu/Debian
  sudo apt install zlib1g-dev
  # RHEL/CentOS/Fedora
  sudo dnf install zlib-devel
  ```

- **Python >= 3.11** and **SCons** (build tool)
  ```bash
  uv venv
  uv pip install scons
  ```

  Optionally, activate the virtual environment so you can omit `uv run`:
  ```bash
  source .venv/bin/activate
  ```

### Clone and Build

```bash
git clone https://github.com/gthparch/macsim.git --recursive
cd macsim
./build.py --ramulator -j 32

# Or without activating the virtual environment:
uv run ./build.py --ramulator -j 32
```

For more build options, see `./build.py --help`.

## Quick Start

This section walks you through downloading a trace, setting up the simulation, and running it.

### 1. Download a Sample Trace

```bash
uv pip install gdown
gdown -O macsim_traces.tar.gz 1rpAgIMGJnrnXwDSiaM3S7hBysFoVhyO1
tar -xzf macsim_traces.tar.gz
rm macsim_traces.tar.gz
```

This will extract sample traces from the [Rodinia benchmark suite](https://github.com/yuhc/gpu-rodinia) into a `macsim_traces/` directory.

### 2. Set Up a Run Directory

You need three files in the same directory to run a simulation:
- `macsim` — the binary executable
- `params.in` — GPU configuration
- `trace_file_list` — list of paths to GPU traces

Copy them from the build output:

```bash
mkdir run
cp bin/macsim bin/params.in bin/trace_file_list run/
cd run
```

### 3. Set Up the Trace Path

Edit `trace_file_list`. The first line is the number of traces, and the second line is the path to the trace:

```
1
/absolute/path/to/macsim_traces/hotspot/r512h2i2/kernel_config.txt
```

### 4. Run

```bash
./macsim
```

Simulation results will appear in the current directory. For example, check `general.stat.out` for the total cycle count:

```bash
grep CYC_COUNT_TOT general.stat.out
```

> **Note:** The parameter file must be named `params.in`. The macsim binary looks for this exact filename in the current directory.

## Downloading Traces

### Publicly Available Traces

| Dataset | Download |
|---------|----------|
| Rodinia | [Download](https://www.dropbox.com/scl/fi/qyqk9yuxaut0f9490k5n3/pytorch_nvbit.tar.gz?rlkey=dgq53t37k38izawacgxdkqxsw&st=fbvchdmw&dl=0) |
| PyTorch | [Download](https://www.dropbox.com/scl/fi/otaiy3gnmkcrexy66hkez/rodinia_nvbit.tar.gz?rlkey=w2pa56a0ik42zydl0incogc99&st=y3ki6xyy&dl=0) |
| YOLOPv2 | [Download](https://www.dropbox.com/scl/fi/srmp7cp2uw6lup34j4keg/yolopv2.tar.gz?rlkey=s5pg7dhdub7jofit3omy446n3&st=d6dfq6uy&dl=0) |
| GPT2 | [Download](https://www.dropbox.com/scl/fi/qn72hfwyeo5qq120kyade/gpt2_nvbit.tar.gz?rlkey=pal8q77bwf4iarypfts2osus3&st=cmjslv8o&dl=0) |
| GEMMA | [Download](https://www.dropbox.com/scl/fi/ewcyrogwv7odc6soi9v6n/gemma_nvbit.tar.gz?rlkey=arifvlad3kj9tcw6ogze7n04m&st=66fbac0t&dl=0) |

## Generating Your Own Traces

> **Warning:** The trace generation tool is experimental — use at your own risk.

To generate traces for your own CUDA workloads, use the [MacSim Tracer](https://github.com/gthparch/Macsim_tracer).

Simply prepend `CUDA_INJECTION64_PATH` to your original command. For example:

```bash
CUDA_INJECTION64_PATH=/path/to/main.so python3 your_cuda_program.py
```

Available environment variables:

| Variable | Description | Default |
|----------|-------------|---------|
| `TRACE_PATH` | Path to save trace files | `./` |
| `KERNEL_BEGIN` | First kernel to trace | `0` |
| `KERNEL_END` | Last kernel to trace | `UINT32_MAX` |
| `INSTR_BEGIN` | First instruction to trace per kernel | `0` |
| `INSTR_END` | Last instruction to trace per kernel | `UINT32_MAX` |
| `COMPRESSOR_PATH` | Path to the compressor binary | (built with tracer) |
| `DEBUG_TRACE` | Generate human-readable debug traces | `0` |
| `OVERWRITE` | Overwrite existing traces | `0` |
| `TOOL_VERBOSE` | Enable verbose output | `0` |

See the [MacSim Tracer README](https://github.com/gthparch/Macsim_tracer) for full installation and usage instructions.

## Known Bugs

1. **`src/memory.cc:1043: ASSERT FAILED`** — Happens with FasterTransformer traces + too many cores (40+). **Solution:** Reduce the number of cores.

2. **`src/factory_class.cc:77: ASSERT FAILED`** — Happens when `params.in` file is missing or has a wrong name. **Solution:** Use `params.in` as the config file name.

3. **`src/process_manager.cc:826: ASSERT FAILED ... error opening trace file`** — Too many trace files open simultaneously. **Solution:** Add `ulimit -n 16384` to your `~/.bashrc`.

## People

* Prof. Hyesoon Kim (Project Leader) at Georgia Tech
Hparch research group
(http://hparch.gatech.edu/people.hparch)

## Q & A

If you have a question, please use github issue ticket.

## Tutorial

* We had a tutorial in HPCA-2012. Please visit [here](http://comparch.gatech.edu/hparch/OcelotMacsim_tutorial.html) for the slides.
* We had a tutorial in ISCA-2012, Please visit [here](http://comparch.gatech.edu/hparch/isca12_gt.html) for the slides.

## SST+MacSim

* Here are two example configurations of SST+MacSim.
  * A multi-socket system with cache coherence model: ![](http://comparch.gatech.edu/hparch/images/sst+macsim_conf_1.png)
  * A CPU+GPU heterogeneous system with shared memory: ![](http://comparch.gatech.edu/hparch/images/sst+macsim_conf_2.png)
