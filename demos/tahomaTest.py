#!/usr/bin/python3

import sys
import json
from tahoma_api import TahomaApi, Action

# From the Tahoma HA platform
def apply_action(controller, cmd_name, dev_url, *args):
    """Apply Action to Device."""
    action = Action(dev_url)
    action.add_command(cmd_name, *args)
    controller.apply_actions('Test', [action])


creds = open('.tahoma.credentials').read()
creds = json.loads(creds)
print('Logging in to Tahomalink')
tahoma = TahomaApi(creds['USERNAME'], creds['PASSWORD'])
tahoma.get_setup()

print('Get all registered devices')
devurlbylabel = {}
devs = tahoma.get_devices()
for d in devs:
    _dev = tahoma.get_device(d)
    devurlbylabel[_dev.label] = _dev.url
    # print info
    print('Device label = %s, type = %s (url = %s)' % (_dev.label, _dev.type, _dev.url))
    print('    commands = %s' % _dev.command_definitions)
    try:
        print('    state = %s' % _dev.active_states)
    except AttributeError:
        pass
    print('\n')

print('Get all registered scenes')
agroups = tahoma.get_action_groups()
for ag in agroups:
    print('Scene %s: oid = %s\n' % (ag.name, ag.oid))

if len(sys.argv) != 3:
    sys.exit(0)

devname = sys.argv[1]
cmd = sys.argv[2]
print cmd, devurlbylabel[devname]
apply_action(tahoma, cmd, devurlbylabel[devname])
