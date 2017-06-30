"""
Basic support for Somfy Tahoma covers.

For more details about this platform, please refer to the documentation at
https://home-assistant.io/components/cover.somfytahoma/

@author: glopresti@gmail.com
"""
import logging

import voluptuous as vol

# Import the device class from the component that you want to support
from homeassistant.components.cover import (CoverDevice, PLATFORM_SCHEMA)
from homeassistant.const import CONF_USERNAME, CONF_PASSWORD, CONF_FILENAME
import homeassistant.helpers.config_validation as cv

# This is the repo with the Tahoma unofficial python bindings
REQUIREMENTS = [
    'https://github.com/manuelciosici/TahomaProtocol'
    '/archive/7c2fca7cf6a8ea6eb4ad9507601c192099859cc9.zip'
    '#tahoma==0.1.0']       # the release number is actually fake

_LOGGER = logging.getLogger(__name__)

# Validation of the user's configuration
PLATFORM_SCHEMA = PLATFORM_SCHEMA.extend({
    vol.Required(CONF_USERNAME): cv.string,
    vol.Required(CONF_PASSWORD): cv.string,
    vol.Optional(CONF_FILENAME, default='tahomacookie'): cv.string,
})


def setup_platform(hass, config, add_devices, discovery_info=None):
    """Setup the Somfy Tahoma cover platform."""
    import tahoma.protocol

    # setup connection with the Tahoma box
    username = config.get(CONF_USERNAME)
    password = config.get(CONF_PASSWORD)
    cookie = config.get(CONF_FILENAME)
    try:
        # note this goes via www.tahomalink.com. The cookie is stored in the
        # given path (defaults to the current directory) for faster access
        # in case of repeated actions. It expires after 2 hours.
        hub = tahoma.protocol.Protocol(username, password, cookie)
        hub.getSetup()
    except ValueError, e:
        _LOGGER.error("Could not connect to Somfy Tahoma: %s", e)
        return False

    # Add all known devices
    devs = hub.getDevices()
    for d in devs:
        add_devices(TahomaCover(hass, hub, d))



class TahomaCover(CoverDevice):
    """Representation of a Tahoma-controlled cover."""

    def __init__(self, hass, tahoma, device):
        """Initialize the cover."""
        self._hass = hass
        self._tahoma = tahoma
        self._device = device

    def _exec_command(self, command):
        """Execute a Tahoma command."""
        from tahoma.action import Action

        _LOGGER.info("Running command %s on device %s",
                     command, _device.label)
        a = Action(self._device.url)
        a.addCommand(command)
        try:
            self._tahoma.applyActions('hass', [a])
        except ValueError, e:
            _LOGGER.error("Could not execute command: %s", e)

    @property
    def should_poll(self):
        """For now disable polling. Anyhow most Somfy RTS
           devices don't provide a state."""
        return False

    @property
    def name(self):
        """Return the name of the cover."""
        return self._device.label

    @property
    def is_closed(self):
        """Return if the cover is closed."""
        pass

    @property
    def current_cover_position(self):
        """Return current position of cover.

        None is unknown, 0 is closed, 100 is fully open.
        """
        return None

    def _query_state(self):
        """Query for the state."""
        pass

    def update(self):
        """Update device state."""
        pass

    def open_cover(self, **kwargs):
        """Open the cover."""
        self._exec_command('open')

    def close_cover(self, **kwargs):
        """Close the cover."""
        self._exec_command('close')

    def stop_cover(self, **kwargs):
        """Stop the cover."""
        self._exec_command('stop')

    def mypos_cover(self, **kwargs):
        """Move the cover to the 'My' position.

        This is a Somfy-specific feature.
        """
        self._exec_command('my')
