
#ifndef LIBDHB10_H
#define LIBDHB10_H

#if defined(__cplusplus)
extern "C" { 
#endif

//libdhb10serial
void drive_close(void);
char *drive_open(void);

//libdbh10...
void dbh10__stop(void);
int dbh10__start(void);
void dhb10_comunicator(void *par);


int send_speed();
int send_dist();
int send_heading();
char *recive_value();
int get_heading();
int get_left_speed();
int get_right_speed();
int get_left_distance();
int get_right_distance();


//Define the default pins to conect to
#define DHB10_DEFAULT_SERVO_L 16
#define DHB10_DEFAULT_SERVO_R 17


#define DHB10_LEN 64

//These are for example

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