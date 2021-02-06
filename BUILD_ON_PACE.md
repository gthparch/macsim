# Steps
* Download [conda](https://docs.conda.io/en/latest/miniconda.html) installer.
* Install and init conda (we assume it is installed on default path).
* Create a new conda environment. Because we don't have root access on PACE, we will use conda to manage all packages.
```
conda create --name macsim
```
* Activate the macsim env and install necessary packages (gcc, g++, scons, and zlib).
```
conda activate macsim
conda install gcc_linux-64 gxx_linux-64 scons zlib
```
* Set default compilation flags.
```
export CC=~/miniconda2/envs/macsim/bin/x86_64-conda_cos6-linux-gnu-gcc
export CXX=~/miniconda2/envs/macsim/bin/x86_64-conda_cos6-linux-gnu-g++
```
* Checkout `pace-release` brnach.
```
git checkout pace-release
```
* Modify path in `SConscript` to point to your local library and header file. There are three places that you need to modify, Line 217, 218, and 221.
* Run macsim build (currently only support build with --ramnulator flag).
```
./build --ramulator
```
