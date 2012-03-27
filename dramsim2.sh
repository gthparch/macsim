git clone git://github.com/dramninjasUMD/DRAMSim2.git ./src/DRAMSim2
patch -p0 -i patch/dramsim2_make.patch
cd src/DRAMSim2
patch < ../../patch/dramsim2_src.patch
