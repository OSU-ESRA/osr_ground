#!/usr/bin/python

import time
import sys
import os, pipes
import threading

def send(port):
    pipeout = None
    
    if port == 0:
        print 'starting avionics thread'
        if not os.path.exists('/tmp/rocket_avionics'):
            os.mkfifo('/tmp/rocket_avionics')
            print 'Created rocket_avionics'
    
        pipeout = os.open('/tmp/rocket_avionics', os.O_WRONLY)
        print 'Opened rocket_avionics for write'
    else:
        print 'starting payload thread'
        if not os.path.exists('/tmp/rocket_payload'):
            os.mkfifo('/tmp/rocket_payload')
            print 'Created rocket_payload'
    
        pipeout = os.open('/tmp/rocket_payload', os.O_WRONLY)
        print 'Opened rocket_payload'

    index = 0
    t = 0.0
    v = 400 + 100 * port

    while 1:
        lat = 44.565 + (0.0001 * index)
        lon = -123.278 + (0.0003 * index)
        alt = (-10 * t * t) + (v * t)
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


send_avi = threading.Thread(target=send, args=(0,))
send_pay = threading.Thread(target=send, args=(1,))

send_avi.start()
send_pay.start()

send_avi.join()
send_pay.join()
