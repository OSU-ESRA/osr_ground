#!/usr/bin/python

import time
import sys
import os, pipes

if not os.path.exists('/tmp/rocket_pipe'):
    os.mkfifo('/tmp/rocket_pipe')
    print 'Created pipe for write'

pipeout = os.open('/tmp/rocket_pipe', os.O_WRONLY)
print 'Opened pipe for write'
coords = open('coords', 'r')

index = 0
t = 0.0
#-10x^2+500x
while 1:
    
    lat = 44.565 + (0.0001 * index)
    lon = -123.278 + (0.0003 * index)
    alt = (-10 * t * t) + (500 * t)
    if alt < 0:
        alt = 0
    
    line = str(lat) + ',' + str(lon) + ',' + str(alt) + '\n'
    os.write(pipeout, line)
    
    if t > 0 and alt == 0:
        print 'end of path'
        break

    index += 1
    t += 0.25
    print 'sent: %s' % line
    time.sleep(0.25)


