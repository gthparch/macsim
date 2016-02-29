There are two paths to using ocelot for trace generation:
1. Use a prebuilt Docker image. This can be used on any linux system
2. Compile ocelot natively. Since ocelot has specific dependencies on old packages, you might have some package conflicts. 

# 1. Use the docker image
- Install docker. For RHEL 7, sudo yum install docker. For Ubuntu, follow the steps in https://docs.docker.com/engine/installation/linux/ubuntulinux/
- The ocelot docker image is about 4.5 GB. So make sure that your local hard drive has space. 
- **sudo service docker start**
- **sudo docker pull hparch/gpuocelot** 
- This will download the image, which could take about 10 minutes.
- A docker image is like the blue print. A container is a running instance created from the image. 
- *sudo docker images* will list all the images on the system
- *sudo docker ps -a* will show all the containers on the system. 
- *sudo docker stop container_id* stops a running container, *sudo docker rm container_id* removes it, and *sudo docker rmi image_id* removes the docker image. Refer to docker documentation for other details. 
- Now that we have the image, we need to launch a container from the image. We'll expose the /home directory and /trace2 directory from the host to the container in this example. You can customize this according to your needs.
- Call *id* on the terminal on your host. Note down the uid and the gid. For example, it would show something like uid=6074(username) gid=7001(hparch)
- **sudo docker run --privileged -i -t -v /home/username:/home/username -v /trace2:/trace2 hparch/gpuocelot /bin/bash**
- This will drop you to the container's root shell. Now, we create a user here with the same uid and gid as host. 
    ```
    addgroup --gid 7001 hparch
    adduser --no-create-home --uid 6074 --gid 7001 username
    adduser username sudo
    su - username
    ```
- Now you should be able to see your nfs home
- This is a barebones ubuntu installation. So it won't have many packages that you are used to. You'll have to install them yourself using apt-get. 
- ocelot is located at /usr/local/src/gpuocelot, and it has already been installed. You can link your application to it directly.
- **Important: Root inside the container has total access to the host too. Use it with caution. For development, use the user you created and use sudo for system commands.** 
- Docker will save the state if you exit. You can restart the container by calling sudo docker start container_id, followed by sudo docker attach container_id. Again, google or read up on docker documentation for these details.  
- The docker image is updated whenever someone pushes changes to our fork of gpuocelot on github. You'll have to do a docker pull to get the latest image when that happens. 

# 2. Compile ocelot natively
- Start with an Ubuntu 14.04 system, and mimic the steps in the dockerfile located at https://github.com/gthparch/gpuocelot/blob/master/docker/Dockerfile.
- If you already have conflicting versions of packages, uninstall them first.

