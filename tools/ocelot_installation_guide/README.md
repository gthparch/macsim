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
- Call *id* on the terminal on your host. Note down the uid and the gid. For example, it would show something like uid=1234(username) gid=7001(hparch)
- **sudo docker run --privileged -i -t -v /home/username:/home/username -v /trace2:/trace2 hparch/gpuocelot /bin/bash**
- This will drop you to the container's root shell. Now, we create a user here with the **same uid and gid as host**. 
  
    ```
    addgroup --gid 7001 hparch
    adduser --no-create-home --uid 1234 --gid 7001 username
    adduser username sudo
    su - username
    ```
- Now you should be able to see your nfs home
- This is a barebones ubuntu installation. So it won't have many packages that you are used to. You'll have to install them yourself using apt-get. 
- ocelot is located at /usr/local/src/gpuocelot, and it has already been installed. You can link your application to it directly.
- **Important: Root inside the privileged container has total access to the host too. Use it with caution. For development, use the user you created and use sudo for system commands. It should be possible to launch the container in unpriveleged mode, but it probably needs some SELinux changes, at least on RHEL. On Ubunutu, the unprivileged mode might just work out of the box.** 
- The docker image is updated whenever someone pushes changes to our fork of gpuocelot on github. You'll have to do a docker pull to get the latest image when that happens. 
- gpuocelot needs gcc/g++ 4.6 for compilation. However, gpu applications that link against ocelot have better compatibility with gcc/g++ 4.4. 
- When you drop into the docker container, the default gcc/g++ version is set to 4.4 since the assumption is that you are using this container to generate traces. If you want to modify and recompile ocelot itself, you should change it to 4.6 first by calling "update-alternatives --set gcc "/usr/bin/gcc-4.6" && update-alternatives --set g++ "/usr/bin/g++-4.6" as the root user. Change it back to 4.4 before you compile your applications. 

# Basic Docker Management #
- To see all your running containers, run sudo docker ps
- If you have a running container, you would see something like 

 ```
 CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
 43a1f1e80ffc        hparch/gpuocelot    "/bin/bash"         5 weeks ago         Up 4 minutes                             stupefied_feynman
 ```
- You can attach to this container by calling
- sudo docker exec -i -t stupefied_feynman bash
- This will drop you to the root shell. Change to your local user by calling su - username.
- If you stopped your container, it won't show up when you call sudo docker ps. Howerver, it will show up if you call sudo docker ps -a. The status would show "Exited". For example,

 ```
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS                     PORTS               NAMES
43a1f1e80ffc        hparch/gpuocelot    "/bin/bash"         5 weeks ago         Exited (0) 3 minutes ago                       stupefied_feynman
0ceeab137ff5        hparch/gpuocelot    "/bin/bash"         6 weeks ago         Exited (130) 6 weeks ago                       clever_mccarthy
fed37ca26a79        hparch/gpuocelot    "/bin/bash"         6 weeks ago         Exited (0) 6 weeks ago                         kickass_leakey
 ```
- Before you can attach to an exited container, you need to start it first. For example, sudo docker start stupefied_feynman
- Then you can follow the same instructions as before to attach to it.
- If you want to delete old containers, first stop them, and then call sudo docker rm container_name. For example, sudo docker rm kickass_leakey in the previous list of exited containers. 
- If you want to delete the main docker image from which the containers were created, first stop and remove all the containers, and then call sudo docker rmi image_name.
- Use docker's documentation for more details.

# 2. Compile ocelot natively
- Start with an Ubuntu 14.04 system, and mimic the steps from *apt-key adv* onwards in the dockerfile located at https://github.com/gthparch/gpuocelot/blob/master/docker/Dockerfile.
- If you already have conflicting versions of packages, uninstall them first.

