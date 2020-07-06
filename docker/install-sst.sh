#!/bin/bash

## env setting
mkdir -p /root/sst_main
echo "export SST_TOP=/root/sst_main" >> ~/.sstrc
echo "export SST_ROOT=\$SST_TOP/devel/sst" >> ~/.sstrc
echo "export SST_CORE_HOME=\$SST_ROOT/local/sstcore" >> ~/.sstrc
echo "export SST_CORE_ROOT=\$SST_ROOT/scratch/src/sstcore" >> ~/.sstrc
echo "export PATH=\$SST_CORE_HOME/bin:\$PATH" >> ~/.sstrc
echo "export SST_ELEMENTS_HOME=\$SST_ROOT/local/sstelements" >> ~/.sstrc
echo "export SST_ELEMENTS_ROOT=\$SST_ROOT/scratch/src/sst-elements" >> ~/.sstrc
echo "export PATH=\$SST_ELEMENTS_HOME/bin:\$PATH" >> ~/.sstrc
source ~/.sstrc

## prepare workspace
mkdir -p ${SST_ROOT}/scratch/src
mkdir -p ${SST_ROOT}/local 

## download source and build
cd ${SST_ROOT}/scratch/src 
git clone https://github.com/sstsimulator/sst-core.git
git clone https://github.com/sstsimulator/sst-elements.git
cd ${SST_ROOT}/scratch/src/sst-elements/src/sst/elements
git clone https://github.com/gthparch/macsim.git macsimComponent 


### build sst-core 
cd ${SST_ROOT}/scratch/src/sst-core 
./autogen.sh && ./configure --disable-mpi --prefix=$SST_CORE_HOME && make all -j && make install

cd ${SST_ROOT}/scratch/src/sst-elements 
cd ${SST_ROOT}/scratch/src/sst-elements/src/sst/elements
### we want to put .ignore files to sub-directories that we don't need. Otherwise it will compile all unnecessary components. we need very few 

echo "" >> .ignore 
for f in * ; do if [ -d "$f" ]; then  cp .ignore $f/.; fi; done
rm  .ignore  macsimComponent/.ignore memHierarchy/.ignore simpleElementExample/.ignore

### build sst elements 
cd ${SST_ROOT}/scratch/src/sst-elements
./autogen.sh && ./configure --prefix=$SST_ELEMENTS_HOME --with-sst-core=$SST_CORE_HOME && make all -j && make install


#### testing 
# which sst     --> should print $SST_CORE_HOME/bin/sst 
# sst --version --> should print SST-Core Version (9.1.0)
# sst-info      --> should print a bunch of sst info

## Functionality check
sst $SST_ELEMENTS_ROOT/src/sst/elements/simpleElementExample/tests/test_simpleRNGComponent_mersenne.py

# 
#ln -s /usr/bin/libtoolize /usr/bin/libtool
