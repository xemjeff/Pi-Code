# Reads a switch on a GPIO input

import RPi.GPIO as GPIO
PIN = 7
LOW = 0
HIGH = 1

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

oldVal=0
while True:
	v = GPIO.input(PIN)
	if v!=oldVal :
		if v==HIGH :
			print "switch HIGH"
		else :
			print "switch LOW"
		oldVal = v

GPIO.cleanup()
