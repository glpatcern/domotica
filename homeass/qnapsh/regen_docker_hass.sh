#!/bin/sh
#
# regen_docker_hass.sh
#
# regenerates the home assistant docker image by pulling it
# from the repo and recreating and restarting a container for it
#
# Giuseppe Lo Presti, September 2017
################################################################

currimg=`docker images | grep homeassistant | awk '{print $3}'`
currcont=`docker ps | grep homeass | awk '{print $1}'`
docker stop $currcont

echo === Pulling new docker image ===
docker pull homeassistant/home-assistant | grep Status | grep -v 'up to date' && \
  echo === Removing previous image === && \
  docker rmi -f $currimg
docker rm $currcont
echo === Starting a new container with the new image ===
docker run -d -p 8123:8123 --name="home-assistant" --privileged -v /share/Container/homeass-config:/config \
       -v /mnt/RTRR_CIFS/iomega/admin/backup:/mnt  -v /mnt/HDA_ROOT/.logs:/media \
       --net=host --restart=always homeassistant/home-assistant

