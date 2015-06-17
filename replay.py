#!/usr/bin/python

import sys, os
import time

class Message(object):
    def __init__(self, tp, tm):
        self.typ = tp
        self.time = tm

class Location(Message):
    def __init__(self, time, latitude, longitude, altitude):
        super(Location, self).__init__('location', time)
        self.lat = latitude
        self.lon = longitude
        self.alt = altitude

class Attitude(Message):
    def __init__(self, time, d):
        super(Attitude, self).__init__('attitude', time)
        self.dcm = d


if len(sys.argv) < 2:
    print 'Usage: replay.py <log file name>'
    sys.exit(1)

print str(sys.argv)

f = open(sys.argv[1], 'r')
if not f:
    print 'Error opening file'
    sys.exit(1)

if not os.path.exists('/tmp/rocket_payload'):
    os.mkfifo('/tmp/rocket_payload')

if not os.path.exists('/tmp/rocket_instrument'):
    os.mkfifo('/tmp/rocket_instrument')

pipe_gps = os.open('/tmp/rocket_payload', os.O_WRONLY)
pipe_qfl = os.open('/tmp/rocket_instrument', os.O_WRONLY)
print 'Opened pipe for write'

ms_start = 1000 * time.time()
start_time = 0
messages = []

while 1:
    line = f.readline()
    if not line:
        break

    words = line.split(':')
    
    msg_time = int(words[0].split(' ')[0])
    if start_time == 0:
        start_time = msg_time
    
    label = words[0].split('<')[1].split('>')[0]
    
    #print msg_time
    if label == 'location':
        data = words[1].split('\n')[0].split(' ')[1:]
        messages.append(Location(msg_time, data[0], data[1], data[2]))
        
    elif label == 'attitude':
        data = words[1].split('\n')[0].split(' ')[1:]
        messages.append(Attitude(msg_time, data))


time1 = 1000 * time.time()
i = 0
while i < len(messages):
    elapsed = int(1000 * time.time() - time1)
    while elapsed != (messages[i].time - start_time):
        elapsed = int(1000 * time.time() - time1)

    line = ''
    if messages[i].typ == 'location':
        line = 'altitude,'+str(messages[i].alt)+','+str(messages[i].time)+'\n'
    elif messages[i].typ == 'attitude':
        line = 'attitude,' + str(messages[i].dcm[0]) + ',' + str(messages[i].dcm[1]) + ',' + str(messages[i].dcm[2]) + ',' + str(messages[i].dcm[3]) + ',' + str(messages[i].dcm[4]) + ',' + str(messages[i].dcm[5]) + ',' + str(messages[i].dcm[6]) + ',' + str(messages[i].dcm[7]) + ',' + str(messages[i].dcm[8])
    
    if messages[i].typ == 'location':
        os.write(pipe_gps, line)
    elif messages[i].typ == 'attitude':
        os.write(pipe_qfl, line)
    
    i += 1
