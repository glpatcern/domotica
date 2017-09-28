currimg=`docker images | grep homeassistant | awk '{print $3}'`
currcont=`docker ps | grep homeass | awk '{print $1}'`
docker stop $currcont
echo === Pulling new docker image ===
docker pull homeassistant/home-assistant
echo === Removing previous image ===
docker rmi -f $currimg && docker rm $currcont
echo === Starting a new container with the new image ===
docker run -d -p 8123:8123 --name="home-assistant" -v /share/Container/homeass-config:/config --net=host  --restart=always --privileged homeassistant/home-assistant

