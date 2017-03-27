import time
import picamera

with picamera.PiCamera() as camera:
	camera.resolution = (800,800)
	time.sleep(2)
	camera.capture('image.png', 'png');
