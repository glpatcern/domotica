#!/usr/bin/python3

import json
import tahoma.protocol

creds = open('.tahoma.credentials').read()
creds = json.loads(creds)
print('Logging in to Tahomalink')
tahoma = tahoma.protocol.Protocol(creds['USERNAME'], creds['PASSWORD'], 'tahomacookie')
tahoma.getSetup()

#print('User:', tahoma.getUser())

print('All registered devices')
devs = tahoma.getDevices()
for d in devs.keys():
	print('Device %s: type = %s, label = %s' % (d, devs[d].type, devs[d].label))

