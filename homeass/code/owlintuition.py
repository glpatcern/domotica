"""
Support for OWL Intuition Power Meter.

For more details about this platform, please refer to the documentation at
https://home-assistant.io/components/sensor.owlintuition/
"""
import logging
import voluptuous as vol

from homeassistant.components.sensor import PLATFORM_SCHEMA
from homeassistant.const import (CONF_NAME, CONF_PORT,
  CONF_MONITORED_CONDITIONS, CONF_MODE)
from homeassistant.exceptions import TemplateError
from homeassistant.helpers.entity import Entity
import homeassistant.helpers.config_validation as cv
from homeassistant.util import Throttle

from datetime import timedelta
from xml.etree import ElementTree as ET
import socket
import select

_LOGGER = logging.getLogger(__name__)

DEFAULT_NAME = 'OWL Meter'
MODE_MONO = 'monophase'
MODE_TRI = 'triphase'

SENSOR_BATTERY = 'battery'
SENSOR_RADIO = 'radio'
SENSOR_POWER = 'power'
SENSOR_ENERGY_TODAY = 'energy_today'

SENSOR_TYPES = {
    SENSOR_BATTERY: ['Battery', None, 'mdi:battery'],
    SENSOR_RADIO: ['Radio', 'dBm', 'mdi:signal'],
    SENSOR_POWER: ['Power', 'W', 'mdi:flash'],
    SENSOR_ENERGY_TODAY: ['Energy Today', 'kWh', 'mdi:flash']
}

DEFAULT_MONITORED = [SENSOR_BATTERY, SENSOR_POWER, SENSOR_ENERGY_TODAY]

PLATFORM_SCHEMA = PLATFORM_SCHEMA.extend({
    vol.Required(CONF_PORT): cv.port,
    vol.Optional(CONF_MONITORED_CONDITIONS, default=DEFAULT_MONITORED):
        vol.All(cv.ensure_list, [vol.In(SENSOR_TYPES)]),
    vol.Optional(CONF_NAME, default=DEFAULT_NAME): cv.string,
    vol.Optional(CONF_MODE, default=MODE_MONO):
        vol.In([MODE_MONO, MODE_TRI])
})

MIN_TIME_BETWEEN_UPDATES = timedelta(seconds=55)
SOCK_TIMEOUT = 65


def setup_platform(hass, config, add_devices, discovery_info=None):
    """Set up the OWL Intuition Sensors."""
    data = OwlIntuitionData(config)

    dev = []
    for v in config.get(CONF_MONITORED_CONDITIONS):
        dev.append(OwlIntuitionSensor(hass, config, v, data))
    if config.get(CONF_MODE) == MODE_TRI:
        for phase in range(1, 4):
            dev.append(OwlIntuitionSensor(hass, config,
                                          SENSOR_POWER, data, phase))
            dev.append(OwlIntuitionSensor(hass, config,
                                          SENSOR_ENERGY_TODAY, data, phase))

    add_devices(dev, True)


class OwlIntuitionSensor(Entity):
    """Implementation of the OWL Intuition Power Meter sensors."""

    def __init__(self, hass, config, sensor_type, data, phase=0):
        """Set all the config values if they exist and get initial state."""
        self._hass = hass
        self._sensor_type = sensor_type
        self._data = data
        self._phase = phase
        self._state = None
        if(phase > 0):
            self._name = '{} {} P{}'.format(
                config.get(CONF_NAME),
                SENSOR_TYPES[sensor_type][0],
                phase)
        else:
            self._name = '{} {}'.format(
                config.get(CONF_NAME),
                SENSOR_TYPES[sensor_type][0])

    @property
    def name(self):
        """Return the name of this sensor."""
        return self._name

    @property
    def unit_of_measurement(self):
        """Return the unit of measurement of this entity."""
        return SENSOR_TYPES[self._sensor_type][1]

    @property
    def icon(self):
        """Return the icon for this entity."""
        return SENSOR_TYPES[self._sensor_type][2]

    @property
    def state(self):
        """Return the state of the device."""
        return self._state

    def update(self):
        """Retrieve the latest value for this sensor."""
        self._data.update()
        xml = self._data.getXmlData()
        if xml is None:
            self._state = None
        elif self._sensor_type == SENSOR_BATTERY:
            # strip off the '%'
            batt_lvl = int(xml.find("battery").attrib['level'][:-1])
            if batt_lvl > 90:
                self._state = 'High'
            elif batt_lvl > 30:
                self._state = 'Medium'
            elif batt_lvl > 10:
                self._state = 'Low'
            else:
                self._state = 'Very Low'
        elif self._sensor_type == SENSOR_RADIO:
            self._state = int(xml.find('signal').attrib['rssi'])
        elif self._phase == 0:
            if self._sensor_type == SENSOR_POWER:
                self._state = int(float(xml.find('property').find('current').
                                  find('watts').text))
            elif self._sensor_type == SENSOR_ENERGY_TODAY:
                self._state = round(float(xml.find('property').find('day').
                                    find('wh').text)/1000, 2)
        else:
            if self._sensor_type == SENSOR_POWER:
                self._state = int(float(xml.find('channels').
                                  findall('chan')[self._phase-1].
                                  find('curr').text))
            elif self._sensor_type == SENSOR_ENERGY_TODAY:
                self._state = round(float(xml.find('channels').
                                    findall('chan')[self._phase-1].
                                    find('day').text)/1000, 2)


class OwlIntuitionData(object):
    """Listen to updates via UDP from the OWL Intuition station"""

    def __init__(self, config):
        """Initialize the data gathering class"""
        self._config = config
        self._xml = None

    def getXmlData(self):
        """Return the last retrieved full XML tree"""
        return self._xml

    # Updates are sent every 60 seconds
    @Throttle(MIN_TIME_BETWEEN_UPDATES)
    def update(self):
        """Retrieve the latest data by listening to the UDP message"""
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.settimeout(SOCK_TIMEOUT)
            try:
                sock.bind((socket.gethostbyname(socket.getfqdn()),
                           self._config.get(CONF_PORT)))
            except socket.error as err:
                _LOGGER.error("Unable to bind on port %s: %s",
                              self._config.get(CONF_PORT), err)
                return

            readable, _, _ = select.select([sock], [], [], SOCK_TIMEOUT)
            if not readable:
                _LOGGER.warning(
                    "Timeout (%s second(s)) waiting for data on port %s.",
                    SOCK_TIMEOUT, self._config.get(CONF_PORT))
                return

            data, _ = sock.recvfrom(1024)
            self._xml = ET.fromstring(data)