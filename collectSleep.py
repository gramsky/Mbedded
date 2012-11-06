import serial

ser = serial.Serial('/dev/tty.usbserial-A501DGRP')
ser.baudrate = 9600

while 1:
   try:
      if ser.inWaiting():
         val = ser.read(ser.inWaiting())
         print val
   except:
      pass
