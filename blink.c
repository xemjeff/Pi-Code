#include <wiringPi.h>
int main (void)
{
  #define PIN 7
  wiringPiSetup () ;
  pinMode(PIN, OUTPUT) ;
  for (;;)
  {
    digitalWrite (PIN,  LOW) ; delay (500) ;
    digitalWrite (PIN, HIGH) ; delay (125) ;
  }
  return 0 ;
}
