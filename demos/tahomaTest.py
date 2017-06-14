#/usr/bin/python3

import json
import tahoma.protocol


creds = open('.tahoma.credentials').read()
print('Parsing credentials')
creds = json.loads(creds)
print('Logging in to Tahomalink')
tahoma = tahoma.protocol.Protocol(creds['USERNAME'], creds['PASSWORD'])

print(tahoma.getUser())
#print(tahoma.getSetup())
#print(tahoma.getDevices())
