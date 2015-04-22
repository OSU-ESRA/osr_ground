#!/usr/bin/python

import random
import time
import sys, os, pipes, fcntl

def getprefix():
    pre = '<?xml version=\"1.0\" encoding=\"UTF8\"?>\n'
    pre += '<kml xmlns=\"http://earth.google.com/kml/2.1\">\n'
    pre += '<Document>\n'
    pre += '<Style id="track">\n'
    pre += '<LineStyle>\n'
    pre += '<color>ff1977ff</color>/n'
    pre += '<width>4.0</width>\n'
    pre += '</LineStyle>\n'
    pre += '<IconStyle>\n'
    pre += '<scale>0.7</scale>\n'
    pre += '<Icon>\n'
    pre += '<href>http://maps.google.com/mapfiles/kml/paddle/ylw-circle.png</href>\n'
    pre += '<hotSpot x="32" y="1" xunits="pixels" yunits="pixels"/>\n'
    pre += '</Icon>\n'
    pre += '</IconStyle>\n'
    pre += '<LabelStyle>\n'
    pre += '<scale>0.7</scale>\n'
    pre += '</LabelStyle>\n'
    pre += '</Style>\n'
    pre += '<Placemark>\n'
    pre += '<styleUrl>#track</styleUrl>\n'
    pre += '<LineString>\n'
    pre += '<altitudeMode>absolute</altitudeMode>\n'
    return pre

def genplacemarks(places):
    placemarks = ''
    if len(places) > 0:
        for x in range(0, len(places)):
            placemarks += '<Placemark>\n'
            placemarks += '<name>' + places[x][3] + '</name>\n'
            placemarks += '<styleUrl>#track</styleUrl>\n'
            placemarks += '<Point>\n'
            placemarks += '<altitudeMode>absolute</altitudeMode>\n'
            placemarks += '<coordinates>' + str(places[x][0]) + ','
            placemarks += str(places[x][1]) + ','+ str(places[x][2])
            placemarks += '</coordinates>\n'
            placemarks += '</Point>\n'
            placemarks += '</Placemark>\n'
    return placemarks
    

gps = []
places = []
index = 0
t = 0.0
alt = altprev = 0

#Open pipe
if not os.path.exists('/tmp/rocket_avionics'):
    os.mkfifo('/tmp/rocket_avionics')
    print 'Created rocket_avionics for reading'
pipein = open('/tmp/rocket_avionics', 'r')

while 1:
    index += 1
    
    line = pipein.readline()[:-1]
    if not line:
        print 'Pipe is empty'
        f_out.close()
        break
        
    altprev = alt
    ln_split = line.split(',')
    lat = float(ln_split[0])
    lon = float(ln_split[1])
    alt = float(ln_split[2])
    
    print 'Lat: %f   Lon: %f   Alt: %f' % (lat, lon, alt)
    gps.append((lon, lat, alt))

    pre = getprefix()

    #Add coordinates
    coords = '';
    coords += '<coordinates>'
    for x in range(0, index):
        coords += str(gps[x][0]) + ',' + str(gps[x][1]) + ","
        coords += str(gps[x][2])
        if x != (index - 1):
            coords += ' '
    
    coords += '</coordinates>\n'
    coords += '</LineString>\n'
    coords += '</Placemark>\n'
    
    placemarks = genplacemarks(places)
    
    post = '</Document>\n'
    post += '</kml>\n'

    output = pre + coords + placemarks + post
    f_out = open('point_avi.kml', 'w')
    f_out.write(output)

