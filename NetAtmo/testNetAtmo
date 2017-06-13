#!/usr/bin/python36

import json, lnetatmo

creds = json.loads(open('.netatmo.credentials').read())
authorization = lnetatmo.ClientAuth(clientId=creds['CLIENT_ID'],
	                                clientSecret=creds['CLIENT_SECRET'],
	                                username=creds['USERNAME'],
	                                password=creds['PASSWORD'])
wsdata = lnetatmo.WeatherStationData(authorization)

print ("Current temperature (inside/outside): %s / %s C, Wind: %s km/h" %
        ( wsdata.lastData()['Indoor']['Temperature'],
          wsdata.lastData()['Outdoor']['Temperature'],
          wsdata.lastData()['Veranda']['GustStrength'])
      )

data = wsdata.lastData(exclude=3600)
print('--- all JSON data ------------------------')
print(data)
print('--- variables in modules -----------------')
for module_name in data.keys():
    for variable in data[module_name]:
    	print('module %s: var %s' % (module_name, variable))

print('--- monitored conditions -----------------')
for module_name in data.keys():
    for variable in wsdata.monitoredConditions(module_name):
    	print('module %s: var %s' % (module_name, variable))

#for name in data.keys():
#	print(wsdata.moduleByName(name)['dashboard_data'])