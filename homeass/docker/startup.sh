#!/bin/bash
#
# Startup script for the custom homeassistant container
#
# Giuseppe Lo Presti, October 2018
########################################################

# make sure influxDB is up
n=0
while ! wget -q http://localhost:3004 -O /dev/null; do
  sleep 10
  let "n=n+1"
  if [ "$n" -eq 60 ]; then
    # after (and every) 10 mins attempt to restart it
    ssh hass@naslopresti ./qnap_restart_influx.sh
    let "n=0"
  fi
done

# make sure the IOMega disk is mounted, but give up after 5 mins
n=0
while ! ssh hass@naslopresti 'df | grep iomega' > /dev/null; do
  sleep 5
  let "n=n+1"
  if [ "$n" -eq 60 ]; then
    break
  fi
done

# standard Home Assistant startup
python -m homeassistant --config /config

