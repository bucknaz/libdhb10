/*
 * This module containes all the serial stuff
 * for comunicating with the DHB-10
 *
 * Thease functions are used by libdb10cog to read from
 * and write to the DHB-10 board
 *
 *
 *
 *
 *
 *
 */


#include "simpletools.h"                      // Include simple tools
#include "fdserial.h"
#include "libdhb10.h"

//The terminal stucture for the dhb-10 port
fdserial *dhb10;

int dhb10_pin_L    = DHB10_DEFAULT_SERVO_L;
int dhb10_pin_R    = DHB10_DEFAULT_SERVO_R;
int dhb10_opened     = 0;

char dhb10_reply[DHB10_LEN];
char dhb10_cmd[DHB10_LEN];

//#define NONE 0
//#define SPD 1
//#define HEAD 2
//#define DIST 3
//int fetching = 0;

/*
 *   functions to open, close and comunicate with the dhb10_reply
 *
 */
char *drive_open(void)
{  
  memset(dhb10_reply, 0, DHB10_LEN);
  char *reply = dhb10_reply;

  #ifdef HALF_DUPLEX
    dhb10 = fdserial_open(dhb10_pin_L, dhb10_pin_L, 0b1100, 19200);
    dhb10_opened = 1;
    pause(10);
  #else
    dhb10 = fdserial_open(dhb10_pin_R, dhb10_pin_L, 0b0000, 19200);
    dhb10_opened = 1;
    pause(10);
    fdserial_txChar(dhb10, 'T');
    fdserial_txChar(dhb10, 'X');
    fdserial_txChar(dhb10, 'P');
    fdserial_txChar(dhb10, 'I');
    fdserial_txChar(dhb10, 'N');
    fdserial_txChar(dhb10, ' ');
    fdserial_txChar(dhb10, 'C');
    fdserial_txChar(dhb10, 'H');
    fdserial_txChar(dhb10, '2');
    fdserial_txChar(dhb10, '\r');    
  #endif   
  fdserial_rxFlush(dhb10);
  pause(2);    
  return reply;
}


/*
 *
 *
 */
void drive_close(void)
{
  fdserial_close(dhb10);
  dhb10_opened = 0;
}



/*
 *
 *
 */
char *recive_value()
{
  
  int i = 0;
  int t = CNT + CLKFREQ;
  int dt = CLKFREQ;
  int ca = 0, cta = 0;
  char *reply = dhb10_reply;
  memset(dhb10_reply, 0, DHB10_LEN);
  while(1)
  {
    cta = fdserial_rxCount(dhb10);
    if(cta)
    {
      ca = readChar(dhb10);
      dhb10_reply[i] = ca;
      if(ca == '\r' || ca == 0)
      {
        reply = dhb10_reply;
        break;
      }  
      i++;
    }
    if( CNT > t )
    {
      strcpy(reply, "Error, no reply from DHB-10!\r");
      break;
    }  
  } 
  return(reply);  
}  


/* send_cmd()
 * Sends the command in the dhb10_cmd buffer to the board
 * 
 */
void send_cmd()
{
  writeLine(dhb10, dhb10_cmd);
  //clear the buffer
  //reset the flag

  //sscan(dhb10_cmd,"%s", cmd);
  //writeStr(dhb10, "Hello fdserial!\n\n");
  //sprint(dhb10_cmd, "%d %d ", 2, 4);
  //writeDec(dhb10,42);

}

void dhb10_rst()
{
  sprint(dhb10_cmd, "rst\r");
}  

void dhb10_gospd(int l, int r)
{
  sprint(dhb10_cmd, "gospd %d %d\r",l,r);
}  



/* get_speed()
 * SPD
 */
int send_speed()
{
  //fetching = SPD;
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'P');
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}

/* get_heading()
 * HEAD
 *
 */
int send_heading()
{
  //fetching = HEAD;
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'H');
  fdserial_txChar(dhb10, 'E');
  fdserial_txChar(dhb10, 'A');
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}

/* get_dist()
 * DIST
 *
 */
int send_dist()
{
  //fetching = DIST;
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, 'I');
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'T');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}      






