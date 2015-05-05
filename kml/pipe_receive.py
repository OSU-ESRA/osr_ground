#!/usr/bin/python

import time
import sys
import os, pipes, fcntl


if not os.path.exists('/tmp/rocket_instrument'):
    os.mkfifo('/tmp/rocket_instrument')
    print 'Created pipe for read'

pipein = open('/tmp/rocket_instrument', 'r')
print 'Opened pipe for read'
#fcntl.fcntl(pipein, fcntl.F_SETFL, os.O_NONBLOCK)

while 1:
    line =  pipein.readline()[:-1]
    if not line:
        while not line:
            line =  pipein.readline()[:-1]
            time.sleep(0.1)

    print line
