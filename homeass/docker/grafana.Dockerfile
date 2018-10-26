#
# LoPresti smart home - docker files for Home Assistant
#
##############################################################

FROM philhawthorne/docker-influxdb-grafana

MAINTAINER Giuseppe Lo Presti @glpatcern

# Change config file to point to external volume
RUN cp /etc/grafana/grafana.ini /etc_grafana.ini.orig && \
    rm /etc/grafana/grafana.ini && \
    ln -s /var/lib/grafana/grafana.ini /etc/grafana/

# Execute original entrypoint command
CMD ["/usr/bin/supervisord"]

