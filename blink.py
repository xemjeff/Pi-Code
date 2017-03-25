import RPi.GPIO as GPIO
import time
PIN = 7
LOW = 0
HIGH = 1

def clockIn():
	GPIO.output(PIN,0)
	time.sleep(0.5)
	GPIO.output(PIN,1)
	time.sleep(0.125)

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(PIN, GPIO.OUT)
for i in range(1,20):
	clockIn()
GPIO.cleanup()

