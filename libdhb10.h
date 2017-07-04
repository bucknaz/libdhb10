
#ifndef LIBDHB10_H
#define LIBDHB10_H

#if defined(__cplusplus)
extern "C" { 
#endif



//Public interface
int dbh10_cog_start(void);
void dbh10_cog_stop(void);

int get_heading(int *Heading);
int get_speed(int *Left,int *Right);
int get_distance(int *Left,int *Right);
int get_last_error(char *e);
int get_cycles(unsigned int *cur,unsigned int *max,unsigned int *min);
void dhb10_gospd(int l, int r);
void dhb10_stop();
void dhb10_rst();


//Cog function should not be calle directly
void case_dhb10_comunicator(void *par);
void if_dhb10_comunicator(void *par);


//Our lower level serial interface called by cog
int _dhb10_open(void);
void _dhb10_close(void);
int _dhb10_recive(char *reply);
void _dhb10_cmd(char *cmd);
void _dhb10_rst(void);
void _dhb10_speed(void);
void _dhb10_heading(void);
void _dhb10_dist(void);
void _dhb10_gospd(int l,int r);
void _dhb10_stop(void);





//Define the default pins to conect to
#define DHB10_SERVO_L 16
#define DHB10_SERVO_R 17


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