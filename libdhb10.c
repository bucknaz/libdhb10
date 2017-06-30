/*
  libdhb10
  A library to encapsulate comunicating with the dhb-10
  driver board from parallax
  
  Uses 2 cogs. ! for the serial bitbang and one for 
  control an managment

*/
#include "simpletools.h"                      // Include simple tools
#include "libdhb10.h"
#include "stdio.h"
static char tdigit_str[10] = "0123456789";


int main()                                    // Main function
{
  char results[100];
  print("Hello!\n");                            // Display test message
  int ls,rs,ld,rd,h;
 
  //emitNumber(12);
  
  char obuf[11];//buffer for 10 digits
  char *t;
  t = obuf;
  int u=36;
  do {
    *t++ = tdigit_str[u % 10];//16 for hex etc
    u /= 10;
  } while (u > 0);
  
  while (t != obuf) {
    print("%c",*--t);
  }
  
  print("\n");  
  print("done\n");
  
  while(1){pause(500);}
  
  
  dbh10_cog_start();  //Start the cog for comunicating with the DBH-10
  while(1)
  {
  ls = get_left_speed();
  rs = get_right_speed();
  ld = get_left_distance();
  rd = get_right_distance();
  h  = get_heading();
  printf("%d %d %d %d %d\n",ls,rs,ld,rd,h );
  
  pause(500);
  }  
 
}

//Dummy main function above 



void emitNumber(unsigned long u)
{
  static char obuf[32];
  char *t;
  t = obuf;
  do {
    *t++ = "0123456789abcdef"[u % 10];//16 for hex etc
    u /= 10;
  } while (u > 0);

  while (t != obuf) {
    putchar(*--t);
  }
}

