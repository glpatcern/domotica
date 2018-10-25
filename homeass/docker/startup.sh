#!/bin/sh
#
# Startup script for the custom homeassistant container
#
# Giuseppe Lo Presti, October 2018
########################################################

while ! wget -q http://localhost:3004 -O /dev/null; do
  sleep 10    # QNAP is booting, wait until influxDB is up
done

python -m homeassistant --config /config

