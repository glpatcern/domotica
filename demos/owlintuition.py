#!/usr/local/bin/python

import socket
import sys
import select
from xml.etree import ElementTree as ET

#sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#sock.sendto('VERSION,EBE23F9E', ('192.168.1.56', 5100))
#readable, _, _ = select.select([srvsock], [], [], 5)
#if readable:
#  value = srvsock.recvfrom(1024)
#  print "Received:", value
#else:
#  print "Timeout after VERSION"

#sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
#sock.sendto('SET,UDP,,192.168.1.48,3200,EBE23F9E', ('192.168.1.60', 5100))
#readable, _, _ = select.select([sock], [], [], 5)
#if readable:
#    value = sock.recvfrom(1024)
#    print "Received:", value
#else:
#	print "Timeout after SET UDP"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
try:
    sock.bind((socket.gethostbyname(socket.getfqdn()), 3200))
except socket.error as err:
    print "Unable to bind on port %s: %s" % (3200, err)
    sys.exit(1)

while True:
    readable, _, _ = select.select([sock], [], [], 5)
    if readable:
        data, addr = sock.recvfrom(1024)
        xml = ET.fromstring(data)
        print data
        print "\n\n"
        etxml = ET.ElementTree(xml)
        for e in etxml.iter():
            print "%s: '%s'" % (e.tag, e.text)
#        curr1 = (xml.find(".//curr/..[@id='0']"))[0].text
#        curr2 = (xml.find(".//curr/..[@id='1']"))[0].text
#        curr3 = (xml.find(".//curr/..[@id='2']"))[0].text
#        print curr1, curr2, curr3
