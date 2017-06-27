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

//track if the port opened for comunications
int dhb10_opened     = 0;


//#define NONE 0
//#define SPD 1
//#define HEAD 2
//#define DIST 3
//int fetching = 0;


/* _dhb10_open()
 * Open the connection to the dhb-10 board
 *
 */
int _dhb10_open(void)
{  
  #ifdef HALF_DUPLEX
    dhb10 = fdserial_open(DHB10_SERVO_L, DHB10_SERVO_L, 0b1100, 19200);
    dhb10_opened = 1;
    pause(10);
  #else
    dhb10 = fdserial_open(DHB10_SERVO_R, DHB10_SERVO_L, 0b0000, 19200);
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
  dhb10_opened = 1;  
  return(0);
}


/* _dhb10_close() 
 * close the serial port to the board
 *
 */
void _dhb10_close(void)
{
  fdserial_close(dhb10);
  dhb10_opened = 0;
}



/* _dhb10_recive()
 * recive the results from the board into the 
 * reply buffer
 *
 */
int _dhb10_recive(char *reply)
{
  
  int i = 0;
  int t = CNT + CLKFREQ;
  int dt = CLKFREQ;
  int ca = 0, cta = 0;
  while(1)
  {
    cta = fdserial_rxCount(dhb10);
    if(cta)
    {
      ca = readChar(dhb10);
      reply[i] = ca;
      if(ca == '\r' || ca == 0)
      {
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
  return(0);  
}  


/* _dhb10_cmd()
 * Sends the string cmd to the board
 * 
 */
void _dhb10_cmd(char *cmd)
{
  writeLine(dhb10, cmd);
  fdserial_txFlush(dhb10);
  //clear the buffer
  //reset the flag

  //sscan(dhb10_cmd,"%s", cmd);
  //writeStr(dhb10, "Hello fdserial!\n\n");
  //sprint(dhb10_cmd, "%d %d ", 2, 4);
  //writeDec(dhb10,42);

}

/* _dhb10_rst()
 * RST
 */
void _dhb10_rst()
{
  //Reset the dist counters
  fdserial_txChar(dhb10, 'R');
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'T');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}  




/* _dhb10_speed()
 * SPD
 */
int _dhb10_speed()
{
  //fetching = SPD;
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'P');
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}

/* _dhb10_heading()
 * HEAD
 *
 */
int _dhb10_heading()
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

/* _dhb10_dist()
 * DIST
 *
 */
int _dhb10_dist()
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






