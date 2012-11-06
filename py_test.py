import serial
import time
import _mysql
from Tkinter import *

#s = serial.Serial('/dev/tty.usbserial-A501DGRP',9600)
s = serial.Serial('/dev/tty.usbmodemfd141',9600)
db = _mysql.connect(host="localhost",user="root",passwd="",db="Mbedded",unix_socket="/Applications/XAMPP/xamppfiles/var/mysql/mysql.sock")

values = []
chunk = ''

def printReport(values):
   # Function prints a report based on the values that
   #    return from the Arduino

   numEvents = len(values)  # Number of 'events' the arduino collected
   lastMinute = -1             # placeholder for previous minute marker
   
   print "You woke up ", numEvents, " number of times last night"
   print "Listed below are the times you woke up from the start of your sleep"

   for time in values:
      minute = int(time)

      # need to account for wrapping that takes place on Arduino
      #    As we can only store values < 256, and number that is 
      #    less than the previous has been wrapped
      if minute < lastMinute:
         minute = minute + 256
      
      print minute, " minutes into sleep"

      lastMinute = minute
   



# loop through the stream of data from the arduino.  Numbers are delimter
#    by '.' and stream ends with a "#"
while 1:
   # must handle 'exceptions' - IE blank data....
   try:
      if s.inWaiting():
         val = s.read(s.inWaiting())
         for char in val:
            if char.isdigit():
               chunk += char


            # handle delimiter
            if char is '.':
               # get previous value (sum of chunks)
               values.append(int(chunk))
               chunk=''


            # handle end of stream
            if char is '#':
               #print "Dumping...."
    
               # get last value (sum of chunks)
               values.append(int(chunk))

               printReport(values)

               # Reset temp values...
               values = []
               chunk = ''

   except:
      pass
