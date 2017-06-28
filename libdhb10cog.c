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
 *SPD  => 20 -50
 *HEAD => 320
 *DIST => 1230 1520
 *HWVER => 1
 *VER => 1
 *Comands to control the motors
 *TURN 200 50
 *ARC 50 40 180
 *GO 90 -20
 *GOSPD 50 40
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
#include "libdhb10.h"


//For maintaining the cog
static int lockId;
static volatile int dbh10_cog;               // Global var for cogs to share
static unsigned int dbh10_stack[40 + 25];    // Stack vars for other cog


//Command and reply buffers 
char dhb10_reply[DHB10_LEN];//only used inside the cog
char dhb10_cmd[DHB10_LEN]; //used to pass commands to the cog (locking)

//Flag to indicate command ready
static volatile int cmd_ready=0;

//A place to store Values returned by cog 
static volatile int heading = 0;
static volatile int left_spd = 0;
static volatile int right_spd = 0;
static volatile int left_dist = 0;
static volatile int right_dist = 0;



//Functions for interacting with the cog
int get_heading()
{
  int V;
  while (lockset(lockId) != 0) { /*spin lock*/ }
  V = heading;
  lockclr(lockId);  
  return(V);
}  

int get_left_speed()
{
  int V;
  while (lockset(lockId) != 0) { /*spin lock*/ }
  V = left_spd;
  lockclr(lockId);
  return(V);
}  

int get_right_speed()
{
  int V;
  while (lockset(lockId) != 0) { /*spin lock*/ }
  V = right_spd;
  lockclr(lockId);  
  return(V);
}
  
int get_left_distance()
{
  int V;
  while (lockset(lockId) != 0) { /*spin lock*/ }
  V = left_dist;
  lockclr(lockId);  
  return(V);
}
  
int get_right_distance()
{
  int V;
  while (lockset(lockId) != 0) { /*spin lock*/ }
  V = right_dist;
  lockclr(lockId);
  return(V);
}  


void dhb10_gospd(int l, int r)
{
  while (lockset(lockId) != 0) { /*spin lock*/ }
  sprint(dhb10_cmd, "gospd %d %d\r",l,r);
  cmd_ready = 1;     
  lockclr(lockId);
}  

//lockId = locknew();
//lockret(lockId);
//while (lockset(lockId) != 0) { /*spin lock*/ }
//lockclr(rbuf->lockId);


/*
 *  dbh10_start()
 *  Start the cog running
 *  uses fdserial so 2 cogs are consumed
 */
int dbh10_start(void)
{
  lockId = locknew(); // get a new lock
  lockclr(lockId); //Don't know if this is needed or not
  dbh10_stop();
  cmd_ready = 0;//Just starting no commands to send
  dbh10_cog = 1 + cogstart(dhb10_comunicator, NULL, dbh10_stack, sizeof(dbh10_stack));
  return(dbh10_cog);
}

/*
 *  dbh10_stop()
 *  shut the cog off and free resources
 */
void dbh10_stop(void)
{
  if(dbh10_cog)
  {
    while(cmd_ready != 99)
    { 
      while (lockset(lockId) != 0) { /*spin lock*/ }  
      cmd_ready = 98;//flag cog to close serial port
      lockclr(lockId);
      pause(100);//Dont use 100% cpu clocks
    }         
    cogstop(dbh10_cog -1);
    dbh10_cog = 0;
    lockclr(lockId); //Don't know if this is needed or not can't hurt
    lockret(lockId);
  }    
}



//This is the main cog function 


//This is the cog that we run
void dhb10_comunicator(void *par)
{
 int state = 0; //For the state mach
 int local_cmd_ready;
 //Clear the comand and replay buffers
 memset(dhb10_reply, 0, DHB10_LEN);
 memset(dhb10_cmd, 0, DHB10_LEN);
 _dhb10_open(); 
 
 while(1)
 {
  // We will continuosly poll the board for for 3 value sets when 
  // we are just spinning waiting for somthing to do   

   //Fetch a local copy of cmd_ready
   while (lockset(lockId) != 0) { /*spin lock*/ } 
   local_cmd_ready = cmd_ready;
   lockclr(lockId);
   
   if(!local_cmd_ready)
   {
     switch(state)
     {
       case 0:
        _dhb10_speed();        
        memset(dhb10_reply, 0, DHB10_LEN);//clear the reply string
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
          ;
        }
        else
        { 
          state++;         
          while (lockset(lockId) != 0) { /*spin lock*/ }         
          sscan(dhb10_reply, "%d%d", &left_spd, &right_spd);
          lockclr(lockId);
        }        
        break;

       case 1:
        _dhb10_heading();
        memset(dhb10_reply, 0, DHB10_LEN);//clear the reply string
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
          ;
        }
        else
        { 
          state++; 
          while (lockset(lockId) != 0) { /*spin lock*/ }         
          sscan(dhb10_reply, "%d", &heading);
          lockclr(lockId);
        }        
        break;

       case 2:
        _dhb10_dist();
        memset(dhb10_reply, 0, DHB10_LEN);//clear the reply string
        _dhb10_recive(dhb10_reply);
        if(dhb10_reply[0] == 'E')
        {
         ; 
        }
        else
        { 
          state = 0; 
          while (lockset(lockId) != 0) { /*spin lock*/ }         
          sscan(dhb10_reply, "%d%d", &left_dist, &right_dist);
          lockclr(lockId);
        }          
        break;
        
       default:
        state = 0; 
        break;
        
     }//End switch      
    }//End if(!cmd_ready) 
    else if(local_cmd_ready == 98)//Close the terminal for orderly shutdown
    {
      _dhb10_close();
      while (lockset(lockId) != 0) { /*spin lock*/ }  
      cmd_ready = 99; //to indicate we are done
      lockclr(lockId);
      while(1){pause(1000);}//spin here forever
    }           
    else
    {
      //send command and fetch results
      while (lockset(lockId) != 0) { /*spin lock*/ }
      _dhb10_cmd(dhb10_cmd);
      cmd_ready = 0; 
      lockclr(lockId);//Clear the lock and allow others to setup command
      
      memset(dhb10_reply, 0, DHB10_LEN);//clear the reply string
      _dhb10_recive(dhb10_reply);
      if(dhb10_reply[0] == 'E')
      {
       ; 
      }
      else
      {   
            
      }          
      
    }//End if(!cmd_ready) else
          
   pause(100); 

 }//End While    
}  
