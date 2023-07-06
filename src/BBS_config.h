/******************************************************************
Created with PROGRAMINO IDE for Arduino - 23.06.2023 09:40:07

Author      : Chris74
Description : ESP32/Arduino tool for setting e-bike BAFANG BBS0x
Board       : ESP32 dev or Pico / Arduino with restrictions
Source      : https://github.com/Chris741233/BBS_config
License     : MIT
Version     : 1.0.0


******************************************************************/


// const Wheel diam    
#define WHEEL_23   45
#define WHEEL_23a  46
#define WHEEL_24   47
#define WHEEL_24a  48
#define WHEEL_25   49
#define WHEEL_25a  50
#define WHEEL_26   51
#define WHEEL_26a  52
#define WHEEL_27   53
#define WHEEL_27a  54
#define WHEEL_700C 55
#define WHEEL_28   56
#define WHEEL_28a  57
#define WHEEL_29   58
#define WHEEL_29a  59


#ifndef BBS_config_h
    #define BBS_config_h
    
    #include <Arduino.h>
    
    class BBS_config {
        
    private:
        
        Stream* _serial;
        void sendNiv();
        
    public:
        BBS_config();
        void begin(Stream& stream);
        
        // https://forum.arduino.cc/t/resolu-c-tableau-de-constantes-dans-class/93026/5
        static const char * str_basic[28];    // const txt frame basic
        static const char * str_pas[15];      // const txt frame pas
        static const char * str_throttle[10]; // const txt frame throttle
        
        bool generateBasic(byte niv[10][2], byte lb, byte cl, byte wd); // bool
        bool generatePas(byte pda, byte psl, byte sc, byte ssm, byte deg, byte wma, byte ts, byte cd, byte sd, byte kc);
        bool generateThrottle(byte sv, byte ev, byte mt, byte dl, byte sl, byte sc);
        
        void readBasic();
        void readPas();
        void readThrottle();
        
        bool writeBasic();
        bool writePas();
        bool writeThrottle();
        
        byte buffReadBasic[27];     // buff read BBS Basic     (28 - head)
        byte buffReadPas[14];       // buff read BBS Pas       (15 - head)
        byte buffReadThrottle[9];   // buff read BBS Throttle  (10 - head)
        
        byte buffBasic[28];     // buff generate Basic
        byte buffPas[15];       // buff generate Pas
        byte buffThrottle[10];  // buff generate Throttle
        
    };
    
#endif
