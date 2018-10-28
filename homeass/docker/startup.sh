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

# make sure the IOMega disk is mounted
while ! ssh admin@naslopresti 'df | grep iomega' > /dev/null; do
  sleep 10
done

# standard Home Assistant startup
python -m homeassistant --config /config

