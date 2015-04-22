#!/usr/bin/python

import time
import sys
import os, pipes

if not os.path.exists('/tmp/rocket_avionics'):
    os.mkfifo('/tmp/rocket_avionics')
    print 'Created pipe for write'

pipeout = os.open('/tmp/rocket_avionics', os.O_WRONLY)
print 'Opened pipe for write'

index = 0
t = 0.0

#-9x^2+400x
while 1:
    
    lat = 44.565 + (0.0001 * index)
    lon = -123.278 + (0.0003 * index)
    alt = (-9 * t * t) + (400 * t)
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


