# Use the official image as a parent image.
FROM ubuntu:18.04

# Set the working directory.
#WORKDIR /usr/local/common

# Copy the file from your host to your current location.
COPY pin-3.13-98189-g60a6ef199-gcc-linux /usr/local/pin-3.13-98189-g60a6ef199-gcc-linux

# Run the command inside your image filesystem.
RUN cd /home &&\
    apt-get update && apt-get install -y git &&\
    git clone https://github.com/gthparch/macsim MacSim_in_docker &&\
    apt-get install -y python && apt-get install -y python3

RUN apt-get install -y python-pip &&\
    python -m pip install scons &&\
    apt-get install -y libz-dev

RUN echo 'export PIN_HOME=/usr/local/pin-3.13-98189-g60a6ef199-gcc-linux' >> ~/.bashrc
RUN echo 'export PATH=$PIN_HOME:$PATH' >> ~/.bashrc
RUN echo 'export PIN_ROOT=$PIN_HOME' >> ~/.bashrc

RUN chmod u=rxw -R /usr/local/pin-3.13-98189-g60a6ef199-gcc-linux
# Inform Docker that the container is listening on the specified port at runtime.
#EXPOSE 8080

# Run the specified command within the container.
CMD bash

# Copy the rest of your app's source code from your host to your image filesystem.
#COPY . .
