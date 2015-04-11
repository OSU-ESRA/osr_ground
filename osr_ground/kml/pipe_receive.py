#!/usr/bin/python

import time
import sys
import os, pipes, fcntl


if not os.path.exists('/tmp/rocket_payload'):
    os.mkfifo('/tmp/rocket_payload')
    print 'Created pipe for read'

pipein = open('/tmp/rocket_payload', 'r')
print 'Opened pipe for read'
#fcntl.fcntl(pipein, fcntl.F_SETFL, os.O_NONBLOCK)

while 1:
    line =  pipein.readline()[:-1]
    if not line:
        print 'Pipe is empty'
        while not line:
            line =  pipein.readline()[:-1]
            time.sleep(0.1)
        #break

    gps = line.split(',')
    lat = float(gps[0])
    lon = float(gps[1])
    alt = float(gps[2])

    print 'Lat: %f   Lon: %f   Alt: %f' % (lat, lon, alt)
