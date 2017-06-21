import time
import picamera

with picamera.PiCamera() as camera:
	camera.resolution = (1920,1080)
	time.sleep(2)
	camera.capture('image.png', 'png')
