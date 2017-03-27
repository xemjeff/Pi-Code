#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 
 
int main()
{
 char mesg[]="Accelerometer Test";		/* message to be appeared on the screen */
 int row,col;				/* to store the number of rows and *
					 * the number of colums of the screen */
 initscr();				/* start the curses mode */
 getmaxyx(stdscr,row,col);		/* get the number of rows and columns */
 mvprintw(row/2,(col-strlen(mesg))/2,"%s",mesg);
                                	/* print the message at the center of the screen */

 mvprintw(1,0,"Gyro");
 mvprintw(2,0,"X: %4.2f\n",73.25);
 mvprintw(3,0,"Y: %4.2f\n",-14.82);
 mvprintw(4,0,"Z: %4.2f\n",124.37);
 mvprintw(1,16,"Acc-Angle");
 mvprintw(2,16,"X: %4.2f\n",73.25);
 mvprintw(3,16,"Y: %4.2f\n",-14.82);
 mvprintw(4,16,"Z: %4.2f\n",124.37);
 mvprintw(1,32,"CF-Angle");
 mvprintw(2,32,"X: %4.2f\n",73.25);
 mvprintw(3,32,"Y: %4.2f\n",-14.82);
 mvprintw(4,32,"Z: %4.2f\n",124.37);
 mvprintw(6,0,"Loop Time %3.1f", 20.0);
 refresh();
 getch();
 endwin();

 return 0;
}
