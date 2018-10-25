#
# LoPresti smart home - docker files for Home Assistant
#
##############################################################

FROM philhawthorne/docker-influxdb-grafana

MAINTAINER Giuseppe Lo Presti @glpatcern

# Change config file to point to external volume
RUN mv  /etc/grafana/grafana.ini /etc/grafana/grafana.ini.orig && \
    ln -s  /var/lib/grafana/grafana.ini /etc/grafana

CMD
