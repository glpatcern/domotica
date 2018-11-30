#!/bin/sh
#
# start the docker container for InfluxDB and Grafana

docker run -d --restart=always --name influxdb-grafana \
  -p 3003:3003 -p 3004:8083 -p 8086:8086 -p 22022:22 \
  -v /share/Container/influxdb-storage:/var/lib/influxdb \
  -v /share/Container/grafana-storage:/var/lib/grafana \
  influxdb-grafana:glp
