#!/usr/bin/python

import random
import time
import sys, os, pipes
import threading

def getprefix(port):
    pre = '<?xml version=\"1.0\" encoding=\"UTF8\"?>\n'
    pre += '<kml xmlns=\"http://earth.google.com/kml/2.1\">\n'
    pre += '<Document>\n'
    pre += '<Style id="track">\n'
    pre += '<LineStyle>\n'
    if port == 0:
        pre += '<color>ff1977ff</color>\n'
    else:
        pre += '<color>ffff4b19</color>\n'
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
    
def update(port):
    gps = []
    index = 0
    
    #Open pipe
    pipein = None
    f_out = None
    
    if port == 0:
        if not os.path.exists('/tmp/rocket_avionics'):
            os.mkfifo('/tmp/rocket_avionics')
            print 'Created rocket_avionics pipe'

        pipein = open('/tmp/rocket_avionics', 'r')
        print 'opened rocket_avionics for read'
    else:
        if not os.path.exists('/tmp/rocket_payload'):
            os.mkfifo('/tmp/rocket_payload')
            print 'Created rocket_payload pipe'
        
        pipein = open('/tmp/rocket_payload', 'r')
        print 'opened rocket_payload for read'
        
    while 1:
        index += 1
        
        line = pipein.readline()[:-1]
        if not line:
            print 'Pipe is empty'
            f_out.close()
            return
            
        ln_split = line.split(',')
        lat = float(ln_split[0])
        lon = float(ln_split[1])
        alt = float(ln_split[2])
        
        #print 'Lat: %f   Lon: %f   Alt: %f' % (lat, lon, alt)
        gps.append((lon, lat, alt))
        
        pre = getprefix(port)
        
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
        
        post = '</Document>\n'
        post += '</kml>\n'
        
        output = pre + coords  + post
        if port == 0:
            f_out = open('point_avi.kml', 'w')
        else:
            f_out = open('point_pay.kml', 'w')
        f_out.write(output)


t_avi = threading.Thread(target=update, args=(0,))
t_pay = threading.Thread(target=update, args=(1,))

t_avi.start()
print 'Started avionics thread'
t_pay.start()
print 'Started payload thread'

t_avi.join()
print 'avionics thread finished'
t_pay.join()
print 'payload thread finished'
