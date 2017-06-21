#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdbool.h>

#include <termios.h>
#include <pigpio.h>

#define NEUTRAL 1500
#define REVERSE 1000
#define FORWARD 2000
#define INCREMENT 10

#define GPIO_DIR_CHANNEL 15
#define GPIO_CHANNEL 18
#define PWM_FREQUENCY 200 
#define MINSPEED 0
#define MAXSPEED 255
#define FWD 0
#define REV 1

char getKey(){
  struct termios orig_info;
  struct termios new_info;
  tcgetattr(0, &orig_info);          /* get current terminal attirbutes; 0 is the file descriptor for stdin */
  tcgetattr(0, &new_info);          /* get current terminal attirbutes; 0 is the file descriptor for stdin */
  new_info.c_lflag &= ~ICANON;      /* disable canonical mode */
  new_info.c_lflag &= ~ECHO;        /* turn off echo*/
  new_info.c_cc[VMIN] = 1;          /* wait until at least one keystroke available */
  new_info.c_cc[VTIME] = 0;         /* no timeout */
  tcsetattr(0, TCSANOW, &new_info); /* set immediately */

  char ch = getchar();  //should happen immediately now
  tcsetattr(0, TCSANOW, &orig_info); /* restore to original */
  return ch;     
}


int main(int argc, char *argv[])
{
   bool done = false;
   int speed;
   if (gpioInitialise() < 0)
   {
      printf("gpio initialization failed. run as sudo?\n");
      return -1;
   }

   gpioSetMode(GPIO_DIR_CHANNEL, PI_OUTPUT);
   gpioSetPWMfrequency(GPIO_CHANNEL, PWM_FREQUENCY);

   speed = MAXSPEED/2;

   printf("Keys: [+]faster  [-]slower  (f)orward  (r)everse  (n)eutral  (q)uit\n");
   while(!done) {
      char k = getKey();
      switch(k) {
       case 's' :
         speed = 0;
         printf("Stop\n");
         break;
       case 'f' :
         printf("Forward\n");
         gpioWrite(GPIO_DIR_CHANNEL, FWD);
         break;
       case 'r' :
         printf("Reverse:\n");
         gpioWrite(GPIO_DIR_CHANNEL, REV);
         break;
       case '+' :
         speed += INCREMENT;
         if (speed > MAXSPEED) speed = MAXSPEED;
         printf("+faster %3.2f\n", (float) speed/MAXSPEED);
         break;
       case '-' :
         speed -= INCREMENT;
         if (speed < MINSPEED) speed = MINSPEED;
         printf("+slower: %3.2f\n", (float) speed/MAXSPEED);
         break;
       case 'q' :
         printf("Quit\n");
         done = true;
         break;
      }
      gpioPWM(GPIO_CHANNEL, speed);
   }
   gpioTerminate();   
   return 0;
}

