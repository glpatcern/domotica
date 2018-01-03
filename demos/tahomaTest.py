#!/usr/bin/python3

import json
from tahoma_api import TahomaApi

creds = open('.tahoma.credentials').read()
creds = json.loads(creds)
print('Logging in to Tahomalink')
tahoma = TahomaApi(creds['USERNAME'], creds['PASSWORD'])
tahoma.get_setup()

print('Get all registered devices')
devbylabel = {}
devs = tahoma.get_devices()
for d in devs:
    _dev = tahoma.get_device(d)
    # print info
    print('Device %s: type = %s, label = %s' % (_dev, _dev.type, _dev.label))
    print('    commands = %s' % _dev.command_definitions)
    try:
        print('    state = %s' % _dev.active_states)
    except AttributeError:
        pass
    print('\n')
