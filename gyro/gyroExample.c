/*
* The code is released under the GNU General Public License.
* Developed by Mark Williams
* A guide to this code can be found here; http://ozzmaker.com/2013/04/22/845/
* Created 28th April 2013
*/


#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <ncurses.h>
#include "L3G.h"
#include "LSM303.h"
#include "sensor.c"
#include "i2c-dev.h"

#define X   0
#define Y   1
#define Z   2

#define DT 0.02         // [s/loop] loop period. 20ms
#define AA 0.98         // complementary filter constant

#define A_GAIN 0.0573      // [deg/LSB]
#define G_GAIN 0.070     // [deg/s/LSB]
#define RAD_TO_DEG 57.29578
#define M_PI 3.14159265358979323846

#define KEY_ESC 27

int getKeyInst(){
    struct termios orig_info;
    struct termios new_info;
    tcgetattr(0, &orig_info); /* get current terminal attirbutes; 0 is the file descri
ptor for stdin */
    tcgetattr(0, &new_info);  /* get current terminal attirbutes; 0 is the file descrip
tor for stdin */
    new_info.c_lflag &= ~ICANON; /* disable canonical mode */
    new_info.c_lflag &= ~ECHO;   /* turn off echo*/
    new_info.c_cc[VMIN] = 0;     /* wait until at least one keystroke available */
    new_info.c_cc[VTIME] = 0;    /* no timeout */
    tcsetattr(0, TCSANOW, &new_info); /* set immediately */

    int ch = getchar();

    tcsetattr(0, TCSANOW, &orig_info); /* restore to original */
    return ch;     
}

void  INThandler(int sig)
{
        signal(sig, SIG_IGN);
        exit(0);
}

int mymillis()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;
    return (diff<0);
}

int main(int argc, char *argv[])
{
	float rate_gyr_y = 0.0;   // [deg/s]
	float rate_gyr_x = 0.0;    // [deg/s]
	float rate_gyr_z = 0.0;     // [deg/s]

	int  *Pacc_raw;
	int  *Pmag_raw;
	int  *Pgyr_raw;
	int  acc_raw[3];
	int  mag_raw[3];
	int  gyr_raw[3];

	Pacc_raw = acc_raw;
	Pmag_raw = mag_raw;
	Pgyr_raw = gyr_raw;

	float gyroXangle = 0.0;
	float gyroYangle = 0.0;
	float gyroZangle = 0.0;
	float AccYangle = 0.0;
	float AccXangle = 0.0;

	float magAngle = 0.0;

	float CFangleX = 0.0;
	float CFangleY = 0.0;

	int startInt  = mymillis();
	struct  timeval tvBegin, tvEnd,tvDiff;

	signed int acc_y = 0;
	signed int acc_x = 0;
	signed int acc_z = 0;
	signed int gyr_x = 0;
	signed int gyr_y = 0;
	signed int gyr_z = 0;

	signal(SIGINT, INThandler);

	enableIMU();

	gettimeofday(&tvBegin, NULL);

	//setup the ncurses display
	int row,col;
	initscr();	/* start the curses mode */
	getmaxyx(stdscr,row,col); /* get the number of rows and columns */
	char title[] = "Accelerometer Test";
	mvprintw(row/2,(col-strlen(title))/2,"%s",title);

	int running = 1;

	int magXmin = 0;
	int magXmax = 0;
	int magYmin = 0;
	int magYmax = 0;	

	while(running)
	{
		startInt = mymillis();

		//read ACC and GYR data
		readMAG(Pmag_raw);
		readACC(Pacc_raw);
		readGYR(Pgyr_raw);

		//Convert Gyro raw to degrees per second
		rate_gyr_x = (float) *gyr_raw * G_GAIN;
		rate_gyr_y = (float) *(gyr_raw+1) * G_GAIN;
		rate_gyr_z = (float) *(gyr_raw+2) * G_GAIN;

		//Calculate the angles from the gyro
		gyroXangle+=rate_gyr_x*DT;
		gyroYangle+=rate_gyr_y*DT;
		gyroZangle+=rate_gyr_z*DT;

		//Convert Accelerometer values to degrees
		//AccXangle = (float) (atan2(*(acc_raw+1),*(acc_raw+2))+M_PI)*RAD_TO_DEG;
		//AccYangle = (float) (atan2(*(acc_raw+2),*acc_raw)+M_PI)*RAD_TO_DEG;
		AccXangle = (float) (atan2(*(acc_raw+1),*(acc_raw+2)))*RAD_TO_DEG;
		AccYangle = (float) (atan2(*(acc_raw+2),*acc_raw)-M_PI/2.0)*RAD_TO_DEG;

		//the magnetometer must be calibrated before use.
		int x = mag_raw[0];
		int y = mag_raw[1];
		if (x < magXmin && x != -4096) magXmin = x;
		if (y < magYmin && y != -4096) magYmin = y;
		if (x > magXmax) magXmax = x;
		if (y > magYmax) magYmax = y;	

		magAngle = (float)  (atan2(*(mag_raw+2),*(mag_raw+0)-M_PI)*RAD_TO_DEG);
		//magAngle = 0.0;

		//Change the rotation value of the accelerometer to -/+ 180
		if (AccXangle >180)
		{
			AccXangle -= (float)360.0;
		}
		if (AccYangle >180)
			AccYangle -= (float)360.0;

		//Complementary filter used to combine the accelerometer and gyro values.
		//Takes a while to build up to a steady angle.
		CFangleX=AA*(CFangleX+rate_gyr_x*DT) +(1 - AA) * AccXangle;
		CFangleY=AA*(CFangleY+rate_gyr_y*DT) +(1 - AA) * AccYangle;

		mvprintw(1,0,"Gyro");
		mvprintw(2,0,"X: %7.3f\n",gyroXangle);
		mvprintw(3,0,"Y: %7.3f\n",gyroYangle);
		mvprintw(4,0,"Z: %7.3f\n",gyroZangle);

		mvprintw(1,16,"Mag Heading");
		mvprintw(2,16,"H: %7.3f\n",magAngle);
		mvprintw(4,16,"X: (%d,%d)\n",magXmin, magXmax);
		mvprintw(5,16,"Y: (%d,%d)\n",magYmin, magYmax);				
		

		mvprintw(1,32,"Acc-Angle");
		mvprintw(2,32,"X: %7.3f\n",AccXangle);
		mvprintw(3,32,"Y: %7.3f\n",AccYangle);

		mvprintw(1,48,"CF-Angle");
		mvprintw(2,48,"X: %7.3f\n",CFangleX);
		mvprintw(3,48,"Y: %7.3f\n",CFangleY);

		mvprintw(8,0,"Exec %d", mymillis()-startInt);
		//Each loop should be at least 20ms.
	    while(mymillis() - startInt < 20)
	    {
	        usleep(100);
	    }
		mvprintw(8,10,"Loop %d", mymillis()-startInt);
		refresh();	    

		//check for the ESC key and exit
		if (getKeyInst() == KEY_ESC)
			running = 0;
    }
    endwin();
    return 0;
}

