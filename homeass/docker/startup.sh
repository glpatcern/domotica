#!/bin/sh
#
# Startup script for the custom homeassistant container
#
# Giuseppe Lo Presti, October 2018
########################################################

# make sure influxDB is up
while ! wget -q http://localhost:3004 -O /dev/null; do
  sleep 10
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

