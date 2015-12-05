HebrewTime Pebble watchface
===========================

A Hebrew hour is defined as 1/12th of the time between sunset and sunrise, or 
1/12th of the time between sunrise and sunset. This is a analogue watchface for 
Pebble smartwatches that shows the positions of the 24 Hebrew hours, along with 
a hand to sweep over the watchface once every 24 hours. Thus the first orange 
dot is sunrise, the first blue dot sunset. As the season changes, the inter-dot
distance will grow/shrink, to represent the growing/shrinking Hebrew hours of 
day and night.

Pebble C originally stolen from [initialneil/PebbleFace-IvyTick](https://github.com/initialneil/PebbleFace-IvyTick),
hacked for this purpose, then heavily refactored. PebbleKit JS originally stolen 
from [NOAA Solar Calculator](http://www.esrl.noaa.gov/gmd/grad/solcalc/), 
lightly refactored.

I wrote this for my friend Joe (Happy Hanukkah 5776!), and because I think the
idea of Hebrew time is really cool and captures seasonal variation in day 
length well, helping me value my daytime in winter more.

Public domain as far as I care.

Vaguely tested in CloudPebble emulator, no guarantees this will actually work 
for you. :3
