# Use the official image as a parent image.
FROM ubuntu:18.04

# Set the working directory.
#WORKDIR /usr/local/common

# Copy PIN.
COPY pin-3.13-98189-g60a6ef199-gcc-linux /root/pin-3.13-98189-g60a6ef199-gcc-linux
RUN chmod u=rxw -R /root/pin-3.13-98189-g60a6ef199-gcc-linux

#Copy the script that will install SST
COPY install-sst.sh /root/install-sst.sh
RUN chmod u=rxw /root/install-sst.sh

# Run these commands inside your image filesystem.
RUN cd /root &&\
    apt-get update && apt-get install -y git &&\
    git clone https://github.com/gthparch/macsim MacSim_no_sst &&\
    apt-get install -y python && apt-get install -y python3

RUN apt-get install -y python-pip &&\
    python -m pip install scons &&\
    apt-get install -y libz-dev

RUN echo 'export PIN_HOME=/root/pin-3.13-98189-g60a6ef199-gcc-linux' >> ~/.bashrc
RUN echo 'export PATH=$PIN_HOME:$PATH' >> ~/.bashrc
RUN echo 'export PIN_ROOT=$PIN_HOME' >> ~/.bashrc

# Install SST aka call the script
# Note this scirpt will also perfrom a functionality check on the installed SST
# See the script for more details
RUN apt-get update -y && apt-get dist-upgrade -y &&\
    apt-get update -y && apt-get install libtool-bin -y &&\
    apt-get install m4 -y && apt-get install automake -y &&\
    apt-get install autoconf -y
RUN apt-get install zlib1g-dev -y
 
RUN cd /root/ && ./install-sst.sh

# Inform Docker that the container is listening on the specified port at runtime.
#EXPOSE 8080

# Run the specified command within the container.
CMD bash

# Copy the rest of your app's source code from your host to your image filesystem.
#COPY . .
