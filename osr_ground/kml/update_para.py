#!/usr/bin/python

import random
import time
import sys, os, pipes

gps = []
index = 0
t = 0.0

#Open pipe
if not os.path.exists('/tmp/rocket_pipe'):
    os.mkfifo('/tmp/rocket_pipe')
    print 'Opened rocket_pipe for reading'

while 1:
    index += 1

    #-10x^2 + 500x
    lat = 44.565 + (0.0002 * index)
    lon = -123.278 + (0.0003 * index)
    alt = (-10 * t * t) + (500 * t)
    if (alt < 0):
        alt = 0
    
    gps.append((lon, lat, alt))

    f = open('point.kml', 'w')

    pre = '<?xml version=\"1.0\" encoding=\"UTF8\"?>\n'
    pre += '<kml xmlns=\"http://earth.google.com/kml/2.1\">\n'
    pre += '<Document>\n'
    pre += '<Style id="track">\n'
    pre += '<LineStyle>\n'
    #pre += '<color>801977FF</color>\n'
    pre += '<color>ff1977ff</color>\n'
    pre += '<width>4.0</width>\n'
    pre += '</LineStyle>\n'
    pre += '</Style>\n'
    pre += '<Placemark>\n'
    pre += '<styleUrl>#track</styleUrl>\n'
    pre += '<LineString>\n'
    pre += '<altitudeMode>absolute</altitudeMode>\n'

    coords = '';
    coords += '<coordinates>'
    for x in range(0, index):
        coords += str(gps[x][0]) + ',' + str(gps[x][1]) + ","
        coords += str(gps[x][2])
        if x != (index - 1):
            coords += ' '
    
    coords += '</coordinates>\n'
    post = '</LineString>\n'
    post += '</Placemark>\n'
    post += '</Document>\n'
    post += '</kml>\n'

    output = pre + coords + post

    f.write(output)
    if t > 0 and alt == 0:
        break

    time.sleep(0.25)
    t += 0.25

f.close()

