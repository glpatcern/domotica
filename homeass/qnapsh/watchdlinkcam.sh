#!/bin/sh
#
# watchdlinkcam.sh
#
# monitors the dlink camera folder for new files
# coming from motion detection and regenerates a symbolic
# link to the latest available picture + updates a log file
#
# Giuseppe Lo Presti, September 2018
###############################################################

cd /share/Container/homeass-config
while true; do
  inotifywait --event create dlinkcam
  # recreate symbolic link
  cd dlinkcam
  rm cam_latest.jpg
  ln -s `ls -t *jpg | head -1` cam_latest.jpg
  cd ..
  # this will update a sensor in Home Assistant
  echo '{"event_type": "SaloneCam Motion", "timestamp": "'`date +%s`'"}' >> motionevents.log
done
