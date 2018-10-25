#
# LoPresti smart home - docker files for Home Assistant
#
##############################################################

FROM homeassistant/home-assistant:latest

MAINTAINER Giuseppe Lo Presti @glpatcern

# Copy SSH key and known hosts cache
ADD ./id_rsa /root/.ssh/id_rsa
ADD ./known_hosts /root/.ssh/known_hosts

# Copy startup script
ADD ./startup.sh /usr/src/app

# Volumes and ports are specified in the regen_docker_hass.sh script

# Override startup command
WORKDIR /usr/src/app
CMD [ "./startup.sh" ]

