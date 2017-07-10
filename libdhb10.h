
#ifndef LIBDHB10_H
#define LIBDHB10_H

#if defined(__cplusplus)
extern "C" { 
#endif

//Define the default pins to conect to
#define DHB10_SERVO_L 16
#define DHB10_SERVO_R 17

#define DHB10_LEN 64

/*
  We can Run in full or half duplex as well as faster
  or slower loop time. The chart below is the max mesured
  Loop time for the diferent configs not including the 
  delay time.
  
  duplex    full half 
  highspeed  5ms    6ms
  lowspeed   9ms    10ms 


  When changing the duplex or speed, It it important to restart the
  DHB-10 board as it will remain in configured as the last run
  until restarted
*/


// Uncoment the below line to run half duplex
//#define HALF_DUPLEX 1

//Comment out the below line to run the loop a little slower
#define HIGH_SPEED_SERIAL

//Uncomment out the below line to run with the timming function enabled
//#define DHB10_COG_TIMMING

// These define the total loop time delays
#if defined HIGH_SPEED_SERIAL
#define DHB_LOOP_DELAY 80
#else
#define DHB_LOOP_DELAY 60
#endif

//Functions to start and stop the cog
int dbh10_cog_start(void);
void dbh10_cog_stop(void);

//Use these function to retrieve values from the DHB-10
int dhb10_busy(void);
void dhb10_wait_ready(void);
int get_heading(int *Heading);
int get_speed(int *Left,int *Right);
int get_distance(int *Left,int *Right);
int get_last_error(char *e);
#if defined DHB10_COG_TIMMING
void get_cycles(unsigned int *cur,unsigned int *max,unsigned int *min);
#endif

//Use these to send comande to the DHB-10 Board
void dhb10_gospd(int l, int r);
void dhb10_stop();
void dhb10_rst();


// This is the main cog function and should not be called 
// directly
void _dhb10_comunicator(void *par);

//Our lower level serial recive function
int _dhb10_recive(char *reply);


//These are copied from the lib for examples
//And ideas of what we may add in the future

#define SIDE_TERM 0

#define ARD_DEFAULT_SPEEDLIMIT 200
#define ARD_DEFAULT_ACCEL_LIMIT 512
#define ARD_DEFAULT_DONE_REPS 16
#define ARD_DEFAULT_RAMPSTEP 25      
#define ARD_DEFAULT_INCREMENT 1
#define ARD_DEFAULT_FEEDBACK 1

#define ARD_DEFAULT_DEADZONE 1

#define ARD_DEFAULT_GOTOBLOCK 1
#define ARD_DEFAULT_RAMP_INTERVAL 20

#define ARD_DEFAULT_OFFSET 0
#define ARD_DEFAULT_CYCLES 0
#define ARD_DEFAULT_TRAMPSTEPPREV 0
#define ARD_DEFAULT_INITSPEED 0
#define ARD_DEFAULT_MODE 0
#define ARD_DEFAULT_RAMPMODE 0
#define ARD_DEFAULT_BLOCKSPEED 0
#define ARD_DEFAULT_BLOCKSPEEDPREV 0

#define ARD_MODE_VERBOSE 1
#define ARD_MODE_CONCISE 0
#define ARD_DEFAULT_REPLYMODE ARD_MODE_VERBOSE


#if defined(__cplusplus)                     
}
#endif /* __cplusplus */

#endif /* LIBDHB10_H */