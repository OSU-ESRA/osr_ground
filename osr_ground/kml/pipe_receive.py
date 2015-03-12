#!/usr/bin/python

import time
import sys
import os, pipes, fcntl


if not os.path.exists('/tmp/rocket_pipe'):
    os.mkfifo('/tmp/rocket_pipe')
    print 'Created pipe for read'

pipein = open('/tmp/rocket_pipe', 'r')
print 'Opened pipe for read'
#fcntl.fcntl(pipein, fcntl.F_SETFL, os.O_NONBLOCK)

init = pipein.readline()[:-1]
if not init == 'INITIALIZATION TEST':
    print 'Failed initialization check'
    sys.exit(0)
print 'Passed initialization check'

while 1:
    line =  pipein.readline()[:-1]
    if not line:
        print 'Pipe is empty'
        break

    gps = line.split(',')
    lat = float(gps[0])
    lon = float(gps[1])
    alt = float(gps[2])

    print 'Lat: %f   Lon: %f   Alt: %f' % (lat, lon, alt)
