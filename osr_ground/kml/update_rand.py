#!/usr/bin/python

import random
import time
import sys, os, pipes

gps = []
index = 0

#Open pipe
if not os.path.exists('/tmp/rocket_pipe'):
    os.mkfifo('/tmp/rocket_pipe')
    print 'Opened rocket_pipe for reading'

while 1:
    index += 1
    
    #Set refresh time for Google Earth
    now = time.time()
    future = time.gmtime(now + 1)
    y = future[0]
    mo = future[1]
    d = future[2]
    h = future[3]
    mi = future[4]
    s = future[5]
    iso8601 = '%04d-%02d-%02dT:%02d:%02d:%02dZ' % (y,mo,d,h,mi,s)

    #Random coordinates
    lat = 44.565 + (0.0005 * random.random())
    lon = -123.278 + (index * 0.001 + 0.0005*random.random())
    alt = 0 + (index * 100)
    
    gps.append((lon, lat, alt))

    f = open('point.kml', 'w')

    pre = '<?xml version=\"1.0\" encoding=\"UTF8\"?>\n'
    pre += '<kml xmlns=\"http://earth.google.com/kml/2.1\">\n'
    pre += '<NetworkLinkControl>\n'
    pre += '<expires>' + iso8601 + '</expires>\n'
    pre += '</NetworkLinkControl>\n'
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
    time.sleep(1)
    #if (index > 5):
    #    break

f.close()

