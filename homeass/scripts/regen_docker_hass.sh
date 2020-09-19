#!/bin/sh
#
# regen_docker_hass.sh
#
# regenerates the home assistant docker image by pulling it
# from the repo and recreating and restarting a container for it
#
# Giuseppe Lo Presti, September 2017
################################################################

DOCKERPATH="$(dirname "$(readlink -f "$0")")"/../docker

currimg=`docker images | grep homeassistant | awk '{print $3}'`
currcont=`docker ps | grep homeass | awk '{print $1}'`
echo === Stopping current container ===
docker stop $currcont

echo === Pulling new docker image ===
docker pull homeassistant/home-assistant | grep Status | grep -v 'up to date' && \
  echo === Regenerating the new custom image === && \
  docker-compose -f $DOCKERPATH/homeass.yaml build
docker rm $currcont
echo === Starting a new container with the new image ===
docker run -d -p 8123:8123/tcp -p 3200:3200/udp --name="homeassistant" --privileged \
       -v /etc/localtime:/etc/localtime:ro \
       -v /share/Container/homeass-config:/config \
       -v /mnt/HDA_ROOT:/mnt \
       -v /share/CACHEDEV1_DATA:/data \
       -v /share/ExtBackup:/extusb \
       --net=host --restart=always homeass:glp

echo === Pruning old images ===
docker container prune -f
docker image prune -f
