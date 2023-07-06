/******************************************************************
Created with PROGRAMINO IDE for Arduino - 23.06.2023 | 09:35:21

File        : BbsSetting.h
Author      : Chris74
Description : Setting for your e-bike BAFANG BBSxx


library const Wheel (diameter) : Use it in your setting
---------------------    
#define WHEEL_23   
#define WHEEL_23a  
#define WHEEL_24   
#define WHEEL_24a  
#define WHEEL_25   
#define WHEEL_25a  
#define WHEEL_26   
#define WHEEL_26a  
#define WHEEL_27   
#define WHEEL_27a  
#define WHEEL_700C 
#define WHEEL_28   
#define WHEEL_28a  
#define WHEEL_29   
#define WHEEL_29a  

******************************************************************/


// ** Do not change the name of constants !! **


// --------  BASIC SETTING part ---
// --------------------------------
const byte B_LB   = 38;       // low battery voltage (min voltage 28V)
const byte B_CL   = 20;       // curent limit (max current 35A) 
const byte B_WD = WHEEL_27a;  // wheel diameter, cf Const 

// % of current, % of speed : start with level 0 and end with level 9 
byte B_LEVEL[10][2] = {
    {1,  100},
    {15, 100},
    {35, 100},
    {50, 100},
    {70, 100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {40, 100},
}; // end array


// -------- PAS SETTING part ---
// -----------------------------

const byte P_DA =   255; // designated assist level: 255 (0xFF) = by display's command (default) or level 0 to 9
const byte P_SL =   255; // speed limit : 255 (0xFF) = display's command (default) or 15 to 40 kmh

const byte P_SC =    40; // start_current in % (max 40%)
const byte P_SSM =    4; // slow_start_mode
const byte P_SDEG =   4; // start_degre (signal No.)
const byte P_WA =    25; // work_angular (angular pedal speed/wheel*10) : 255 (0xFF) =undetermined

const byte P_TS =    10; // stop delay (time_of_stop) x10ms
const byte P_CD =     8; // current_decay (1-8)
const byte P_SD =     0; // stop_decay (x10ms)
const byte P_KC =    70; // keep_current (in %)



// --------- THROTTLE SETTING part --- 
// -----------------------------------
const byte T_SV =    11; // Start voltage (*100 mV) default 11
const byte T_EV =    35; // End voltage (*100mV), default 35
const byte T_type =   1; // Mode type     : 1=mode current, 0=mode speed
const byte T_DL =     8; // Design level  : level 0-9 or by display's command = 255 (0xFF)
const byte T_SL =    40; // Speed limit   : 15 kmh to max 40 Kmh (default)  or by display's command = 255 (0xFF)
const byte T_SC =     5; // Start current : max 100% (generaly 5-20)


