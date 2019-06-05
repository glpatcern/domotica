#!/bin/sh
#
# refreshconfig.sh
#
# monitors the (QSync-enabled) homeass folder for changes and:
# - replicates them to the home assistant config dir
# - restarts the homeass container
#
# Giuseppe Lo Presti, July 2017
###############################################################

PATH=/share/CACHEDEV1_DATA/.qpkg/container-station/bin:$PATH
while true; do
  inotifywait --event modify ~glp/QSync/dev/DOMOTICA/homeass/*yaml ~glp/QSync/dev/DOMOTICA/homeass/automations/*
  cp ~glp/QSync/dev/DOMOTICA/homeass/*yaml /share/Container/homeass-config
  cp ~glp/QSync/dev/DOMOTICA/homeass/automations/* /share/Container/homeass-config/automations
  cp ~glp/QSync/cambio_pneumatici_auto.json /share/Container/homeass-config
  # also refresh the SSL certificate for HA and Grafana
  pushd /mnt/HDA_ROOT/.config/QcloudSSLCertificate/cert
  cat cert chain > /share/Container/homeass-config/sslcert.crt
  cp key /share/Container/homeass-config/sslkey.key
  cat cert chain > /share/Container/grafana-storage/sslcert.crt
  cp key /share/Container/grafana-storage/sslkey.key
  popd
  # if running, restart the container to reflect the change
  id=`docker ps | grep homeass | awk '{print $1}'` && \
    docker restart $id
done

