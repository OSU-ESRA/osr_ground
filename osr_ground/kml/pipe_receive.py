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

while 1:
    line =  pipein.readline()[:-1]
    if not line:
        print 'Pipe is empty'
        break

    gps = line.split(',')
    lat = gps[0]
    lon = gps[1]
    alt = gps[2]
    
    print 'Lat: %s' % lat
    print 'Lon: %s' % lon
    print 'Alt: %s' % alt
