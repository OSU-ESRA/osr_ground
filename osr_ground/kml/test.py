#!/usr/bin/env python
import sys
import time
from pygame import mixer


c = ''

mixer.init()
nuke = mixer.Sound('nuclear.wav')

nuke.play()

time.sleep(2)
