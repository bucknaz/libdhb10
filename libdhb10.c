/*
  libdhb10
  A library to encapsulate comunicating with the dhb-10
  driver board from parallax
  
  Uses 2 cogs. ! for the serial bitbang and one for 
  control an managment

*/
#include "simpletools.h"                      // Include simple tools
#include "libdhb10.h"


int main()                                    // Main function
{
  char results[100];
  print("Hello!");                            // Display test message
  int ls,rs,ld,rd,h;
  
  dbh10_start();  //Start the cog for comunicating with the DBH-10
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



