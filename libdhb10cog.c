/*
 * This module containes all the cog related stuff
 * for comunicating with the DHB-10
 *
 * The main program will interface though thease 
 * functions to control the DHB-10 driver board
 *  
 * 
 *  
 *  
 *
 *
 */
/*    Comands that return values
 **SPD  => 20 -50
 **HEAD => 320
 **DIST => 1230 1520
 *HWVER => 1
 *VER => 1
 *Comands to control the motors
 *TURN 200 50
 *ARC 50 40 180
 *GO 90 -20
 **GOSPD 50 40
 *MOVE 20 -60 40
 *TRVL 200 80
 *TRVL 250 80 45
 *RST
 *    Comands for Configueration
 *PULSE
 *SETLF OFF
 *DEC
 *HEX
 *ECHO ON
 *VERB 1
 *RXPIN CH1
 *TXPIN CH2
 *BAUD 57600
 *SCALE 100
 *PACE 100
 *HOLD 200
 *KIP 60
 *KIT 50
 *KIMAX 75
 *KI 85
 *KP 50
 *ACC 500
 *RAMP 80
 *LZ 10
 *DZ 2
 *PPR 280
 *
 */

#include "simpletools.h"                      // Include simple tools
#include "fdserial.h"
#include "libdhb10.h"

//For maintaining the cog
static int input_lockId;
static int output_lockId;
static volatile int dbh10_cog;               // Global var for cogs to share
static unsigned int dbh10_stack[128];    // Stack vars for other cog



#define CMD_NONE 0
#define CMD_GOSPD 1
#define CMD_RESET 2
#define CMD_STOP 3
#define CMD_SEND 4

//Special comands use when stopping the cog
#define COG_STOP 98
#define COG_STOPPED 99


//Flag to indicate command ready
static volatile int cmd_ready=CMD_NONE;

//A place to store Values returned by cog 
static volatile int heading = 0;
static volatile int left_spd = 0;
static volatile int right_spd = 0;
static volatile int left_dist = 0;
static volatile int right_dist = 0;

static volatile int min_cycles = 0;
static volatile int max_cycles = 0;
static volatile int cur_cycles = 0;
static volatile  unsigned int startcnt; //the count at the start of the loop
static volatile  unsigned int endcnt; //the count at the end of the loop


//These are values sent to the cog
static volatile int s_left = 0;  //place to xfer the left go spd
static volatile int s_right = 0; //place to xfer the right go spd


//Track how many errors since last succsessfull send
static volatile int Error_cnt=0;
static volatile int loop_cnt=0;
static char last_error[DHB10_LEN]; //used to store the last error msg
//Command and reply buffers 
static char dhb10_reply[DHB10_LEN];//only used inside the cog
static char dhb10_cmd[DHB10_LEN]; //used to pass commands to the cog (locking)

/////////
//The terminal stucture for the dhb-10 port
fdserial *dhb10;

//track if the port opened for comunications
int dhb10_opened     = 0;
static char digit_str[10] = "0123456789";



//////


//Functions for interacting with the cog
/*
 *  get_heading()
 *  Returns the most recently fetched heading value
 */
int get_heading(int *Heading)
{
  int e;
  while (lockset(output_lockId) != 0) { /*spin lock*/ }
  *Heading = heading;
  e = Error_cnt;
  lockclr(output_lockId);  
  return(e);
}  

/*
 *  get_speed()
 *  Returns the most recently fetched left speed
 */
int get_speed(int *Left,int *Right)
{
  int e;
  while (lockset(output_lockId) != 0) { /*spin lock*/ }
  *Left = left_spd;
  *Right = right_spd;
  e= Error_cnt;
  lockclr(output_lockId);
  return(e);
}  

  
/*
 *  get_left_distance()
 *  Returns the most recently fetched left distince value
 */
int get_distance(int *Left,int *Right)
{
  int e;
  while (lockset(output_lockId) != 0) { /*spin lock*/ }
  *Left = left_dist;
  *Right = right_dist;
  e= Error_cnt;
  lockclr(output_lockId);  
  return(e);
}
  
int get_last_error(char *e)
{
  int errs=0;
  while (lockset(output_lockId) != 0) { /*spin lock*/ }
  strcpy(e,last_error);
  errs = Error_cnt;
  //memset(last_error, 0, DHB10_LEN);
  //Error_cnt=0;
  lockclr(output_lockId);
  return( errs ); 
}  

int get_cycles(unsigned int *cur,unsigned int *max,unsigned int *min)
{
  int e;
   while (lockset(output_lockId) != 0) { /*spin lock*/ } 
   *min = min_cycles;
   *max = max_cycles;
   *cur = cur_cycles;
   max_cycles = 0;
   min_cycles = CLKFREQ;//some large number
   e= loop_cnt;
   loop_cnt=0;
   lockclr(output_lockId);   
   return(e);
}  
// Comands to send to the DHB-10 Board

/*
 *  dhb10_gospd()
 *  Send speed to the board
 */
void dhb10_gospd(int l, int r)
{
  while (lockset(input_lockId) != 0) { /*spin lock*/ }
  s_left = l;
  s_right = r;  
  cmd_ready = CMD_GOSPD;     
  lockclr(input_lockId);
}  


/*
 *  dbh10_stop()
 */
void dhb10_stop()
{
  while (lockset(input_lockId) != 0) { /*spin lock*/ }
  cmd_ready = CMD_STOP;     
  lockclr(input_lockId);
}  

/*
 *  dhb10_rst()
 */
void dhb10_rst()
{
  while (lockset(input_lockId) != 0) { /*spin lock*/ }
  cmd_ready = CMD_RESET;     
  lockclr(input_lockId);
}  




// Function to start and stop the Cog
// And the Cog it's self

/*
 *  dbh10_start()
 *  Start the cog running
 *  uses fdserial so 2 cogs are consumed
 */
int dbh10_cog_start(void)
{
  input_lockId = locknew(); // get a new lock
  lockclr(input_lockId); //Don't know if this is needed or not


  output_lockId = locknew(); // get a new lock
  lockclr(output_lockId); //Don't know if this is needed or not
  
  dbh10_cog_stop();
 
  //Clear the comand and replay buffers
  memset(dhb10_reply, 0, DHB10_LEN/2);
  memset(dhb10_cmd, 0, DHB10_LEN/2);

  cmd_ready = 0;//Just starting no commands to send
  
  // if_dhb10_comunicator
  // case_dhb10_comunicator
  dbh10_cog = 1 + cogstart(case_dhb10_comunicator, NULL, dbh10_stack, sizeof(dbh10_stack));
  return(dbh10_cog);
}

/*
 *  dbh10_cog_stop()
 *  shut the cog off and free resources
 */
void dbh10_cog_stop(void)
{
  if(dbh10_cog)
  {
    //If the cog is spinning due to a command error
    //it will not ever process the COG_STOPPED command
    //So we will hang the program here
    while(cmd_ready != COG_STOPPED)
    { 
      //while (lockset(input_lockId) != 0) { /*spin lock*/ }  
      cmd_ready = CMD_STOP;//flag cog to close serial port
      //lockclr(input_lockId);
      pause(100);//Dont use 100% cpu clocks
    }       
      
    cogstop(dbh10_cog -1);
    dbh10_cog = 0;
    lockclr(input_lockId); //Don't know if this is needed or not can't hurt
    lockclr(output_lockId); //Don't know if this is needed or not can't hurt
    lockret(input_lockId);
    lockret(output_lockId);
  }    
}



//This is the main cog function 

//This is the cog that we run
/*
 * The cog will lock the output set the value and unlock the output.
 * The cog will lock the input at the start of the loop.
 * 
 * If the is no command ready it will unlock the input and 
 * and perform a fetch for the speed, heading or distance.
 * If we get an error trying to fetch it will increment the 
 * error count and save the error message in last_error
 * 
 * If a command is ready
 * It will get a local copy of the inputs
 * and send the command to the board.
 * It will not accept new input or fetch new values
 * untill the last command has been 
 * send successfully
 * 
 */



void case_dhb10_comunicator(void *par)
{
  
 int state = 0; //For the state mach
 int loops = 0; // track howmany time though the loop to slow down reading the state
 int t; //general use int
 //local copyies of our inputs
 int cmd = CMD_NONE;
 int left = 0;
 int right = 0;
 
 int start_cycles=0,end_cycles;
 unsigned int ticks = CNT;// + sixteen;
  
 _dhb10_open(); //Open the serial port

 while(1)
 {
  // We will continuosly poll the board for for 3 value sets when 
  // we are just spinning waiting for somthing to do   

   startcnt = CNT;//used to calculate the waitcnt time
   
   start_cycles = CNT; //for reporting timeing
      

   if(cmd == CMD_NONE)
   {
      //Lock the inputs untill we done with them
      while (lockset(input_lockId) != 0) { ; }   
      cmd = cmd_ready;
      left = s_left;
      right = s_right;  
      cmd_ready =  CMD_NONE;
      lockclr(input_lockId);
   }   
   

   switch(cmd)
   {
     
   case CMD_NONE:
     while (lockset(output_lockId) != 0) { ; }  
     switch(state)
     {
       case 0:
        _dhb10_speed();        
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
          Error_cnt++;
        }
        else
        { 
          state++;    
          Error_cnt=0;     
          sscan(dhb10_reply, "%d%d", &left_spd, &right_spd);
/*  can we replace scanf ?? 
          t = 0;
          left_spd = 0;         
          right_spd = 0;
          while (isspace(dhb10_reply[t]))
            t++;
          while(isdigit(dhb10_reply[t]))
          {
            left_spd *= 10;
            left_spd += (dhb10_reply[t] + '0');
            t++
          }
              
          while (isspace(dhb10_reply[t]))
            t++;
          while(isdigit(dhb10_reply[t]))
          {
            right_spd *= 10;
            right_spd += (dhb10_reply[t] + '0');
            t++
          }
*/
        }        
        break;

       case 1:
        _dhb10_heading();
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
          Error_cnt++;
        }
        else
        { 
          state++; 
          Error_cnt=0;     
          sscan(dhb10_reply, "%d", &heading);
/*  can we replace scanf ?? 
          t = 0;
          heading = 0;         
          while (isspace(dhb10_reply[t]))
            t++;
          while(isdigit(dhb10_reply[t]))
          {
            heading *= 10;
            heading += (dhb10_reply[t] + '0');
            t++
          }              
*/
        }        
        break;

       case 2:
        _dhb10_dist();
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
          Error_cnt++;
        }
        else
        { 
          state = 0; 
          Error_cnt=0;  
          sscan(dhb10_reply, "%d%d", &left_dist, &right_dist);
/*  can we replace scanf ?? 
          t = 0;
          left_dist = 0;         
          right_dist = 0;
          while (isspace(dhb10_reply[t]))
            t++;
          while(isdigit(dhb10_reply[t]))
          {
            left_dist *= 10;
            left_dist += (dhb10_reply[t] + '0');
            t++
          }
              
          while (isspace(dhb10_reply[t]))
            t++;
          while(isdigit(dhb10_reply[t]))
          {
            right_dist *= 10;
            right_dist += (dhb10_reply[t] + '0');
            t++
          }
*/
        }          
        break;
        
       default:
        state = 0; 
        break;        
     }//End switch  
     lockclr(output_lockId);    
     break;         
    
   case CMD_GOSPD: //Close the terminal for orderly shutdown
        _dhb10_gospd(left,right);
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
            Error_cnt++;
        }
        else
        {
          Error_cnt=0; 
          cmd = CMD_NONE;          
        }        
      break;
          
   case CMD_RESET://Reset the distance and heading counters
        _dhb10_rst();
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
            Error_cnt++;
        }
        else
        {
          Error_cnt=0; 
          cmd = CMD_NONE;          
          state = 2;
        }        
      break;
        
   case CMD_STOP://gospd 0 0 
        _dhb10_stop();
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
            Error_cnt++;
        }
        else
        {
          Error_cnt=0; 
          cmd = CMD_NONE;          
        }        
        break;
              
   //We should try to use predefined commands instead                  
   case CMD_SEND: 
        //send command and fetch results
        _dhb10_cmd(dhb10_cmd);
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
            Error_cnt++;
        }
        else
        {
          Error_cnt=0; 
          cmd = CMD_NONE;          
        }
        break;                 

   case COG_STOP:
        _dhb10_close();
        cmd_ready = COG_STOPPED; //to indicate we are done
        while(1){pause(1000);}//spin here forever
        break;

      
   //We are doing nothing this time though
   default:
        cmd = CMD_NONE;          
        cmd_ready = CMD_NONE;// Let them know we are done
        break;   
  }//end switch(cmd)
  
        
   //If we got an error save the error message         
   if(Error_cnt != 0)
   {
      while (lockset(output_lockId) != 0) { ; } 
      loop_cnt++;
      memcpy(last_error,dhb10_reply,DHB10_LEN);
      lockclr(output_lockId);
   }
   else
   {
     while (lockset(output_lockId) != 0) { ; } 
     loop_cnt++;
     lockclr(output_lockId);
   }            

   endcnt = CNT; //used to calculate wait times

   //Track the time it takes to get though the loop 
   end_cycles = CNT; //get the total time in the loop
   
   //ticks = (CLKFREQ/50) - (endcnt - startcnt);
   //waitcnt(ticks + CNT);//loop at 50hz every 20 ms
 
   ticks = (CLKFREQ/70) - (endcnt - startcnt);
   waitcnt(ticks + CNT);//loop at 50hz every 20 ms
   //70 14.28ms
   
//   end_cycles = CNT; //get the total time in the loop

   //report them back             
   while (lockset(output_lockId) != 0) { ; }  
   cur_cycles =  end_cycles - start_cycles;//not including delay
   if(cur_cycles > max_cycles)
      max_cycles = cur_cycles;
   if(cur_cycles < min_cycles)
      min_cycles = cur_cycles;  
   lockclr(output_lockId);   
 }//End While    
}  





/*
void if_dhb10_comunicator(void *par)
{
  
 int state = 0; //For the state mach
 int loops = 0; // track howmany time though the loop to slow down reading the state
 
 //local copyies of our inputs
 int cmd = CMD_NONE;
 int left = 0;
 int right = 0;
 
 int start_cycles=0,end_cycles;
 unsigned int ticks = CNT;// + sixteen;
  
 _dhb10_open(); //Open the serial port

 while(1)
 {
  // We will continuosly poll the board for for 3 value sets when 
  // we are just spinning waiting for somthing to do   

   startcnt = CNT;//for timing howlong the cod takes
   start_cycles = CNT; //for reporting timeing
      
   //If we are not busy with a command see if we have a new cmd

   if(cmd == CMD_NONE)
   {
      //Lock the inputs untill we done with them
      while (lockset(input_lockId) != 0) { ; }   
      cmd = cmd_ready;
      left = s_left;
      right = s_right;  
      cmd_ready =  CMD_NONE;
      lockclr(input_lockId);
   }   
   
   if(cmd == CMD_NONE)
   {
     while (lockset(output_lockId) != 0) { ; }  
     switch(state)
     {
       case 0:
        _dhb10_speed();        
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
          Error_cnt++;
        }
        else
        { 
          state++;    
          Error_cnt=0;     
          sscan(dhb10_reply, "%d%d", &left_spd, &right_spd);
        }        
        break;

       case 1:
        _dhb10_heading();
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
          Error_cnt++;
        }
        else
        { 
          state++; 
          Error_cnt=0;     
          sscan(dhb10_reply, "%d", &heading);
        }        
        break;

       case 2:
        _dhb10_dist();
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
          Error_cnt++;
        }
        else
        { 
          state = 0; 
          Error_cnt=0;  
          sscan(dhb10_reply, "%d%d", &left_dist, &right_dist);
        }          
        break;
        
       default:
        state = 0; 
        break;        
     }//End switch  
     lockclr(output_lockId);    
   }            
    
   else if(cmd == CMD_GOSPD) //Close the terminal for orderly shutdown
   {
        _dhb10_gospd(left,right);
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
            Error_cnt++;
        }
        else
        {
          Error_cnt=0; 
          cmd = CMD_NONE;          
        }        
   }
   
          
   else if(cmd == CMD_RESET)//Reset the distance and heading counters
   {
        _dhb10_rst();
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
            Error_cnt++;
        }
        else
        {
          Error_cnt=0; 
          cmd = CMD_NONE;          
          state = 2;
        } 
   }            
    
        
   else if(cmd == CMD_STOP)//gospd 0 0 
   {
        _dhb10_stop();
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
            Error_cnt++;
        }
        else
        {
          Error_cnt=0; 
          cmd = CMD_NONE;          
        }        
   }   
              
   //We should try to use predefined commands instead                  
   else if(cmd == CMD_SEND)
   { 
        //send command and fetch results
        _dhb10_cmd(dhb10_cmd);
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
            Error_cnt++;
        }
        else
        {
          Error_cnt=0; 
          cmd = CMD_NONE;          
        }
    }                

    else if(cmd == COG_STOP)
    {
        _dhb10_close();
        cmd_ready = COG_STOPPED; //to indicate we are done
        while(1){pause(1000);}//spin here forever
    }
          
    //We are doing nothing this time though
    else
    {
        cmd = CMD_NONE;          
        cmd_ready = CMD_NONE;// Let them know we are done
    }        
  
        
   //If we got an error save the error message         
   if(Error_cnt != 0)
   {
      while (lockset(output_lockId) != 0) { ; } 
      loop_cnt++;
      memcpy(last_error,dhb10_reply,DHB10_LEN);
      lockclr(output_lockId);
   }
   else
   {
     while (lockset(output_lockId) != 0) { ; } 
     loop_cnt++;
     lockclr(output_lockId);
   }    
        
   //Track the time it takes to get though the loop
   //min max and current
   //after the code determin the time to wait
   endcnt = CNT;
   end_cycles = CNT; //get the total time in the loop
   
   ticks = (CLKFREQ/50) - (endcnt -startcnt);
   waitcnt(ticks + CNT);//pause for 62.5 ms
   
//   end_cycles = CNT; //get the total time in the loop (62 - 63ms)

   //report them back             
   while (lockset(output_lockId) != 0) { ; }  
   cur_cycles =  end_cycles - start_cycles;//not including delay
   if(cur_cycles > max_cycles)
      max_cycles = cur_cycles;
   if(cur_cycles < min_cycles)
      min_cycles = cur_cycles;  
   lockclr(output_lockId);   
 }//End While    
}  

*/




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
  if(dhb10_opened)
  {
    fdserial_close(dhb10);
  }  
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
  int dt = 0;
  
  int ca = 0, cta = 0;
  reply[i] = 0;
  if(!dhb10_opened){return(0);}
  //pause(1);
  while(1)
  {
    cta = fdserial_rxCount(dhb10);
    if(cta)
    {
      ca = readChar(dhb10);
      if(ca == '\r')
        ca = 0;
      reply[i] = ca;
      if(ca == '\r' || ca == 0)
      {
        break;
      }  
      i++;
      dt=0;
    }else{
      pause(1);
      dt++;
    }    
    //should not take more than 4 but we will use 5 anyway
    if( dt > 5 )
    {
      strcpy(reply, "Error, no reply from DHB-10!");
      break;
    }  
  } 
  return(0);  
}  


/* _dhb10_cmd()
 * Sends the string contained in cmd to the DHB-10 board
 * 
 */
void _dhb10_cmd(char *cmd)
{
  if(!dhb10_opened){return;}
  writeLine(dhb10, cmd);
  fdserial_txFlush(dhb10);
}



/* _dhb10_rst()
 * Send the RST command to the DHB-10 board
 *  Resets the distance counters
 */
void _dhb10_rst(void)
{
  if(!dhb10_opened){return;}
  fdserial_txChar(dhb10, 'R');
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'T');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}  




/* _dhb10_speed()
 * Send the SPD command to the DHB-10 board
 */
void _dhb10_speed(void)
{
  if(!dhb10_opened){return;}
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'P');
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}


/* _dhb10_heading()
 * Send a HEAD command to the DHB-10 board
 *
 */
void _dhb10_heading(void)
{
  if(!dhb10_opened){return;}
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'H');
  fdserial_txChar(dhb10, 'E');
  fdserial_txChar(dhb10, 'A');
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}

/* _dhb10_dist()
 * Send the DIST comand to the DHB-10 board
 *
 */
void _dhb10_dist(void)
{
  if(!dhb10_opened){return;}
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, 'I');
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'T');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}      

/* _dhb10_gospd()
 * Send a GOSPD command to the DHB-10 board
 * with the values in l and r
 */
void _dhb10_gospd(int l,int r)
{
  char obuf[11];//buffer for 10 digits
  char *t;
  t = obuf;
  
  if(!dhb10_opened){return;}
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'G');
  fdserial_txChar(dhb10, 'O');
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'P');
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, ' ');

  do {
    *t++ = digit_str[l % 10];//16 for hex etc
    l /= 10;
  } while (l > 0);
  
  while (t != obuf) {
    fdserial_txChar(dhb10,*--t);
  }
  fdserial_txChar(dhb10, ' ');
  do {
    *t++ = digit_str[r % 10];//16 for hex etc
    r /= 10;
  } while (r > 0);
  
  while (t != obuf) {
    fdserial_txChar(dhb10,*--t);
  }
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}      


/* _dhb10_stop()
 * Send GOSPD 0 0 to the DHB-10 board
 *
 */
void _dhb10_stop(void)
{
  char obuf[11];//buffer for 10 digits
  char *t;
  t = obuf;
  if(!dhb10_opened){return;}
  fdserial_rxFlush(dhb10);
  fdserial_txChar(dhb10, 'G');
  fdserial_txChar(dhb10, 'O');
  fdserial_txChar(dhb10, 'S');
  fdserial_txChar(dhb10, 'P');
  fdserial_txChar(dhb10, 'D');
  fdserial_txChar(dhb10, ' ');
  fdserial_txChar(dhb10, '0');
  fdserial_txChar(dhb10, ' ');
  fdserial_txChar(dhb10, '0');
  fdserial_txChar(dhb10, '\r');
  fdserial_txFlush(dhb10);
}



