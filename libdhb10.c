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
#include <propeller.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>



void run_test()
{
  int ls,rs,ld,rd,h,e;
  #if defined DHB10_COG_TIMMING
  unsigned int Ccur,Cmax,Cmin;
  #endif
  char results[100];
    
  dhb10_wait_ready();
  dhb10_gospd(50,50);

  pause(500);
  dhb10_stop();
    
  e = get_speed(&ls,&rs);
  e += get_distance(&ld,&rd);     
  e += get_heading(&h);
  
  if(e)
  {
    e = get_last_error(results);
    #if defined DHB10_COG_TIMMING
    printf("ls:%d\trs:%d\tld:%d\trd:%d,\th:%d\tmin:%0.3f ms\tmax:%0.3f ms\tcur:%f ms\t(%d %s)\n",
                                  ls,rs,ld,rd,h,min_tm,max_tm,cur_tm,e,results );
    #else
    printf("ls:%d\trs:%d\tld:%d\trd:%d,\th:%d\t(%d %s)\n",
                                  ls,rs,ld,rd,h,e,results );

    #endif
  }
  else
 {   
    #if defined DHB10_COG_TIMMING
      printf("ls:%d\trs:%d\tld:%d\trd:%d,\th:%d\tmin:%0.3f ms\tmax:%0.3f ms\tcur:%0.3f ms\n",
                                   ls,rs,ld,rd,h,min_tm,max_tm,cur_tm ); 
    #else
      printf("ls:%d\trs:%d\tld:%d\trd:%d,\th:%d\t\n",ls,rs,ld,rd,h ); 
    #endif                                 
  }  
  #if defined DHB10_COG_TIMMING
  get_cycles(&Ccur,&Cmax,&Cmin);
  max_tm = Cmax/(CLKFREQ/1000);
  min_tm = Cmin/(CLKFREQ/1000);
  cur_tm = Ccur/(CLKFREQ/1000);
  #endif

  return;
}  


int main()
{
 
  print("Starting Cog\n");
  dbh10_cog_start();  //Start the cog for comunicating with the DBH-10
  
  dhb10_stop();//first cmd alway give invalid
  while(dhb10_busy()){pause(1);}
  dhb10_stop();
  dhb10_wait_ready();
  dhb10_rst();
  print("Running test\n");
  run_test();
  dhb10_wait_ready();

  dhb10_stop();//first cmd alway give invalid
  dhb10_wait_ready();
  
  print("Stopping the cog\n");
  dbh10_cog_stop();
 
  print("wait a sec\n");
  pause(1000);

  print("Starting the Cog\n");
  dbh10_cog_start();  //Start the cog for comunicating with the DBH-10

  print("Running the test a second time\n");
  run_test();
  dhb10_wait_ready();
  dhb10_stop();//first cmd alway give invalid
  print("\nDone\n");  
}  



//Dummy main function  
//Can be used for testing/timming the library 
int loopin_main()                                    // Main function
{
  char results[100];
  print("Hello!\n");                            // Display test message
  int ls,rs,ld,rd,h,e, t=0;
  #if defined DHB10_COG_TIMMING
  unsigned int Ccur,Cmax,Cmin;
  #endif
  float max_tm,min_tm,cur_tm;
  float tm=0.0;
  volatile unsigned int startcnt; //the count at the start of the loop
  volatile unsigned int endcnt; //the count at the end of the loop   

  volatile unsigned int cycles,start_cycles=0,end_cycles;
  unsigned int ticks = CNT;// + sixteen;

  dbh10_cog_start();  //Start the cog for comunicating with the DBH-10
  pause(100);
  dhb10_stop();//first cmd alway give invalid
  pause(10);
  dhb10_stop();
  dhb10_rst();
  pause(100);

  while(1)
  {
  t++;
  start_cycles=CNT;  
  startcnt = CNT;//for timing howlong the cod takes
  
  if(t>11)
    t=0;  
  else if(t==1)
    dhb10_rst();// takes 39 ms with overhead
  else if(t==2)
    dhb10_gospd(50,50);
  else if(t==3)
    dhb10_gospd(50,50);
  else if(t==4)
    dhb10_gospd(50,50);
  else if(t==5)
    dhb10_gospd(50,50);
  else if(t==6)
    dhb10_stop();//Speed roughfly halfs ever 100ms till stopped
  else if(t==7)
    dhb10_stop();//Speed roughfly halfs ever 100ms till stopped
  else if(t==8)
    dhb10_stop();//Speed roughfly halfs ever 100ms till stopped
  else if(t==9)
    dhb10_stop();//Speed roughfly halfs ever 100ms till stopped
  else if(t==10)
    dhb10_stop();//Speed roughfly halfs ever 100ms till stopped

  e = get_speed(&ls,&rs);
  e += get_distance(&ld,&rd);     
  e += get_heading(&h);
  #if defined DHB10_COG_TIMMING
  get_cycles(&Ccur,&Cmax,&Cmin);
  max_tm = Cmax/(CLKFREQ/1000);
  min_tm = Cmin/(CLKFREQ/1000);
  cur_tm = Ccur/(CLKFREQ/1000);
  #endif
  
  if(e)
  {
    e = get_last_error(results);
    #if defined DHB10_COG_TIMMING
    printf("ls:%d\trs:%d\tld:%d\trd:%d,\th:%d\tmin:%0.3f ms\tmax:%0.3f ms\tcur:%f ms\ttm:%0.3f ms\t=%d\t(%d %s)\n",
                                  ls,rs,ld,rd,h,min_tm,max_tm,cur_tm,tm,t,e,results );
    #else
    printf("ls:%d\trs:%d\tld:%d\trd:%d,\th:%d\ttm:%0.3f ms\t=%d\t(%d %s)\n",
                                  ls,rs,ld,rd,h,tm,t,e,results );

    #endif
    //printf("cur:%f ms (%d %s)\n",cur_tm,e,results );
  }
  else
 {   
    #if defined DHB10_COG_TIMMING
    //33ms loops max 
      printf("ls:%d\trs:%d\tld:%d\trd:%d,\th:%d\tmin:%0.3f ms\tmax:%0.3f ms\tcur:%0.3f ms\ttm:%0.3f ms\t%d\n",
                                   ls,rs,ld,rd,h,min_tm,max_tm,cur_tm,tm,t ); 
    #else
      printf("ls:%d\trs:%d\tld:%d\trd:%d,\th:%d\ttm:%0.3f ms\t%d\n",ls,rs,ld,rd,h,tm,t ); 
    #endif
                                 
    //16ms loop max
    //printf("ls:%d\trs:%d\tld:%d\trd:%d,\ttm:%0.3f ms\n",ls,rs,ld,rd,tm );
          

    //printf("%d\n",ld );                                   
  }  
  
  if (!strcmp(results,"ERROR Motor or encoder error"))
  {
    while(1){pause(1000);} //spinn here
  }  
   
   endcnt = CNT;
   
   
//can we loop every 16 ms? nop looks like 32 - 43 ms

//the dhb10 cog runs at 50 hz, 20ms
// cog running at 70hz 14.28ms
  //pause(50-tm);//The cog runs every 62ms so no need to go faster than that
/*
  hz    ms
  10 = 100
  15 = 66.66
  20 = 50
  25 = 40
  30 = 33.33
  35 = 28.5
  36 = 27.777
  37 = 27.02
  38 = 26.31
  39 = 25.4
  40 = 25    to fast for this loop
  45 = 22.22
  50 =20
  55 = 18.18
  60 = 16.666
  65 = 15.38
  70 = 14.28
  75 = 13.33
  80 = 12.5
  85 = 11.76
  90 = 11.11
  
  
*/
  if( (CLKFREQ/15) > (endcnt - startcnt) )
  {
   ticks = (CLKFREQ/15) - (endcnt - startcnt);
   waitcnt(ticks + CNT);//loop at 30hz every 33.33 ms
  }
  else
  {
     printf("p");
     //pause(1);    
  }      
  //ticks = (CLKFREQ/60) - (endcnt - startcnt);
  //waitcnt(ticks + CNT);//loop at 30hz every 33.33 ms

  end_cycles = CNT;
  cycles = end_cycles-start_cycles; // = 2222528
  tm = cycles/(CLKFREQ/1000);//get the time in ms

  
  }  
 
}

/*
    CLKFREQ cycles in a sec
    CLKFREQ/1000 cycles in a mssec
    1/CLKFREQ = time per cycle
    cycles * (1/CLKFREQ) = execution time
    so with a 80mhz xtal we get
    1/80000000 gives us 0.0000000125 sec per clock
    2222528 * 0000000125 gives us 27.7816 ms time befor 
    the rst command clears the values and we can read them   
    pause(time in ms) by default can be changed
        
*/

/*
 *The DHB-10 will only process input every 
 *PROCESSRATE   = _clkfreq / 16000  16ms
 *if no char is waiting at the port it will wait 16ms before checking again
 *
 */


