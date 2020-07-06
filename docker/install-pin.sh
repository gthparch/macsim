#!/bin/bash

cd /root
#Download PIN from Intel website
wget https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.13-98189-g60a6ef199-gcc-linux.tar.gz

#Extract
tar -xvzf pin-3.13-98189-g60a6ef199-gcc-linux.tar.gz

#Set permissions
chmod u=rxw -R /root/pin-3.13-98189-g60a6ef199-gcc-linux

#Set up PIN env variables
echo 'export PIN_HOME=/root/pin-3.13-98189-g60a6ef199-gcc-linux' >> ~/.bashrc
echo 'export PATH=$PIN_HOME:$PATH' >> ~/.bashrc
echo 'export PIN_ROOT=$PIN_HOME' >> ~/.bashrc
