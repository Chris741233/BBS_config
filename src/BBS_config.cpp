/******************************************************************
Created with PROGRAMINO IDE for Arduino - 23.06.2023 09:40:07

Author      : Chris74
Description : ESP32/Arduino tool for setting e-bike BAFANG BBS0x
Board       : ESP32 dev or Pico / Arduino with restrictions
Source      : https://github.com/Chris741233/BBS_config
License     : MIT
Version     : 1.0.0

******************************************************************/



#include "BBS_config.h"


//  BASE CONSTRUCTOR

BBS_config::BBS_config()
{
    //
}

//  PUBLIC

void BBS_config::begin(Stream &stream)
{
    _serial = &stream;
    
}


const char * BBS_config::str_basic[28] = {"header", "location", "basic", "low_battery", "current_limit",
    "cur0", "cur1", "cur2", "cur3", "cur4", "cur5", "cur6", "cur7", "cur8", "cur9",
    "spd0", "spd1", "spd2", "spd3", "spd4", "spd5", "spd6", "spd7", "spd8", "spd9",
    "wheel_diam", "meter_type", "crc"
};


const char * BBS_config::str_pas[15] = {"header", "location", "pas", "sensor_type", "design_level", "speed_limit",
    "start_current", "slow_start", "start_deg", "work_mode", "time_stop", "current_decay", "stop_decay", "keep_current", "crc"
};

const char * BBS_config::str_throttle[10] = {"header", "location", "throttle", "start_volt", "end_volt", "type",
    "design_level", "speed_limit", "start_current", "crc"
};




bool BBS_config::generateBasic(byte niv[10][2], byte lb, byte cl, byte wd)
{
    // All in DEC !
    bool error = false;
    
    buffBasic[0]=22; // head
    buffBasic[1]=82; // location (Basic)
    buffBasic[2]=24; // mode write
    
    // low bat
    if (lb >= 28 && lb <= 60) buffBasic[3]=lb; 
    else error = true;
    
    // current limit
    if (cl > 1 && cl <= 35) buffBasic[4]=cl;
    else error = true;
    
    // current
    for(byte i=0; i<10; i++)
    {
        if (niv[i][0] <= 100) buffBasic[i+5]=niv[i][0];
        else error = true;
    } 
    // speed
    for(byte i=0; i<10; i++)
    {
        if (niv[i][1] <= 100) buffBasic[i+15]= niv[i][1];
        else error = true;
    } 
    // wheel diam (45 to 59, see constant)
    if (wd >= 45 && wd <= 59) buffBasic[25]=wd;
    else error = true;
    
    buffBasic[26]=1;  // meter type, default 1
    buffBasic[27]=0;  // temp CRC
    
    // sum of octets without first "write command" (0x16) and without last octet (CRC)
    int sum = 0;
    for (byte i = 1; i <= 26; i ++) {
        sum = sum + buffBasic[i];
    }
    buffBasic[27] = sum % 256; // modulo and update buffBasic CRC
    
    if (error) {
        // clear buff
        for (byte j = 0; j < sizeof(buffBasic); j++) {
            buffBasic[j]=0;
        }
        return false;   
    }
    else return true;
    
} //endfunc


bool BBS_config::generatePas(byte pda, byte psl, byte sc, byte ssm, byte deg, byte wma, byte ts, byte cd, byte sd, byte kc) {
    
    // All in DEC !
    bool error = false;
    
    buffPas[0]=22;   // head
    buffPas[1]=83;   // location (Pas)
    buffPas[2]=11;   // mode write
    
    buffPas[3]=3;    // pedal sensor type (3 = double signal 24)
    
    //buffPas[4]=pda;  // designated assist level (0-9 or 255/FF = by display command) 
    if (pda >= 0 && pda <=9 || pda == 255) buffPas[4]=pda;
    else error = true;
    
    //buffPas[5]=psl;  // speed limit (15-40 or 255/FF = by display command) 
    if (psl >= 15 && psl <=40 || psl == 255) buffPas[5]=psl;
    else error = true;
    
    buffPas[6]=sc;   // start current
    buffPas[7]=ssm;  // slow start mode
    buffPas[8]=deg;  // start degre
    buffPas[9]=wma;  // work mode angular
    buffPas[10]=ts;  // time of stop
    
    buffPas[11]=cd;  // current decay
    buffPas[12]=sd;  // stop decay
    buffPas[13]=kc;  // keep current
    buffPas[14]=0;   // temp CRC
    
    // sum of octets without first "write command" (22/0x16) and without last octet (CRC)
    int sum = 0;
    for (byte i = 1; i <= 13; i ++) {
        sum = sum + buffPas[i];
    }
    buffPas[14] = sum % 256; // modulo and update buffPas CRC
    
    if (error) {
        // clear buff
        for (byte j = 0; j < sizeof(buffPas); j++) {
            buffPas[j]=0;
        }
        return false;   
    }
    else return true;
    
} //endfunc


bool BBS_config::generateThrottle(byte sv, byte ev, byte mt, byte dl, byte sl, byte sc) {
    
    // All in DEC !
    bool error = false;
    
    buffThrottle[0]=22; // head
    buffThrottle[1]=84; // location (Throttle)
    buffThrottle[2]=6;  // mode write
    
    buffThrottle[3]=sv; // Start voltage (*100 mV) default 11
    buffThrottle[4]=ev; // End voltage (*100mV), default 35
    buffThrottle[5]=mt; // Mode type (1=mode current, 0=mode speed)
    
    //buffThrottle[6]=dl; // Design level (level 0-9 or by diplay=255)
    if (dl >= 0 && dl <=9 || dl == 255)  buffThrottle[6]=dl;
    else error = true;
    
    //buffThrottle[7]=sl; // Speed limit  (15 to max 40 Kmh  or by diplay=255)
    if (sl >= 15 && sl <=40 || sl == 255)  buffThrottle[7]=sl;
    else error = true;
    
    //buffThrottle[8]=sc; // Start current (max 100%)
    if(sc <= 100) buffThrottle[8]=sc;
    else error = true;
    
    buffThrottle[9]=0;  // temp CRC
    
    // sum of octets without first "write command" (22/0x16) and without last octet (CRC)
    int sum = 0;
    for (byte i = 1; i <= 8; i ++) {
        sum = sum + buffThrottle[i];
    }
    buffThrottle[9] = sum % 256; // modulo and update buffPas CRC
    
    if (error) {
        // clear buff
        for (byte j = 0; j < sizeof(buffThrottle); j++) {
            buffThrottle[j]=0;
        }
        return false;   
    }
    else return true;
    
} //endfunc


void BBS_config::readBasic()
{
    // clean serial buffer serial by security (no more bug) 
    if (_serial->available() > 0) String clean_str = _serial->readString(); 
    delay(5);
    
    byte read_basic[2] = {0x11,0x52}; // cmd read Basic
    _serial->write(read_basic, 2);
    delay(1000);            // Delay mandatory : 1000  OK, less to test
    
    if (_serial->available() > 0) _serial->readBytes(buffReadBasic, 27);  // 28-head
    
} //endfunc

bool BBS_config::writeBasic()
{
    const byte MAX_BYTE = 5;    // 3 + marge, test avec nb. superieur au retour attendu (3 bytes), OK avec methode if, len et readbyte
    byte buff[MAX_BYTE];
    uint16_t rlen = 0;          // nb de bytes recu dans readBytes
    
    // clean serial buffer serial by security (no more bug) 
    if (_serial->available() > 0) String clean_str = _serial->readString(); 
    delay(5);
    
    // write buffer (verif sum first 3 byte, 22+82+24=128, DEC)
    if (buffBasic[0] + buffBasic[1] + buffBasic[2] == 128  ) { 
        _serial->write(buffBasic, sizeof buffBasic);
        delay(1000);   // Delay mandatory : 1000  OK, less to test
    }
    
    
    if (_serial->available() > 0) {
        rlen = _serial->readBytes(buff, MAX_BYTE); // read the serial buffer, rlen = nb. de bytes recu
        //for(uint16_t i = 0; i < rlen; i++) tft.print(buff[i], HEX); tft.print("-");
    }
    
    // -- Controle retour erreur prog Basic 0x52, 0x18, 0x6A
    if (buff[0] == 0x52 && buff[1] == 0x18 && buff[2] == 0x6A) {
        //Serial.println(buff[0], HEX); Serial.println(buff[1], HEX); Serial.println(buff[2], HEX);
        
        sendNiv(); // send Niv_0 to validate new prog : Mandatory if no reset !
        
        return true;
    }
    else {
        //sendNiv(); // debug
        return false;
    }
} //endfunc


void BBS_config::readPas()
{
    // clean serial buffer serial by security (no more bug) 
    if (_serial->available() > 0) String clean_str = _serial->readString(); 
    delay(5);
    
    byte read_pas[2] = {0x11,0x53}; // cmd read Pas
    _serial->write(read_pas, 2);
    delay(1000);            // Delay mandatory : 1000  OK, less to test
    
    if (_serial->available() > 0) _serial->readBytes(buffReadPas, 14); // 15-head
} //endfunc


bool BBS_config::writePas()
{
    const byte MAX_BYTE = 5;    // 3+ marge, test avec nb. superieur au retour attendu (3 bytes), OK avec methode if, len et readbyte
    byte buff[MAX_BYTE];
    uint16_t rlen = 0;          // nb de bytes recu dans readBytes
    
    // clean serial buffer serial by security (no more bug) 
    if (_serial->available() > 0) String clean_str = _serial->readString(); 
    delay(5);
    
    // write buffer (verif sum first 3 byte, 22+83+11=116, DEC)
    if (buffPas[0] + buffPas[1] + buffPas[2] == 116  ) { 
        _serial->write(buffPas, sizeof buffPas);
        delay(1000);   // Delay mandatory : 1000  OK, less to test
    }
    
    if (_serial->available() > 0) {
        rlen = _serial->readBytes(buff, MAX_BYTE); // read the serial buffer, rlen = nb. de bytes recu
        //for(uint16_t i = 0; i < rlen; i++) tft.print(buff[i], HEX); tft.print("-");
    }
    
    // -- Controle retour erreur prog PAS 0x53, 0x0B, 0x5E
    if (buff[0] == 0x53 && buff[1] == 0x0B && buff[2] == 0x5E) {
        //Serial.println(buff[0], HEX); Serial.println(buff[1], HEX); Serial.println(buff[2], HEX);
        
        sendNiv(); // send Niv_0 to validate new prog : Mandatory if no reset !
        
        return true;
    }
    else {
        
        return false;
    }
} //endfunc


void BBS_config::readThrottle()
{
    // clean serial buffer serial by security (no more bug) 
    if (_serial->available() > 0) String clean_str = _serial->readString(); 
    delay(5);
    
    byte read_throt[2] = {0x11,0x54}; // cmd read throttle
    _serial->write(read_throt, 2);
    delay(1000);            // Delay mandatory : 1000  OK, less to test
    
    if (_serial->available() > 0) _serial->readBytes(buffReadThrottle, 9); // 10-head
} //endfunc


bool BBS_config::writeThrottle()
{
    const byte MAX_BYTE = 5;    // 3+ marge, test avec nb. superieur au retour attendu (3 bytes), OK avec methode if, len et readbyte
    byte buff[MAX_BYTE];
    uint16_t rlen = 0;          // nb de bytes recu dans readBytes
    
    // clean serial buffer serial by security (no more bug) 
    if (_serial->available() > 0) String clean_str = _serial->readString(); 
    delay(5);
    
    // write buffer (verif sum first 3 byte, 22+84+6=112, DEC)
    if (buffThrottle[0] + buffThrottle[1] + buffThrottle[2] == 112  ) { 
        _serial->write(buffThrottle, sizeof buffThrottle);
        delay(1000);   // Delay mandatory : 1000  OK, less to test
    }
    
    if (_serial->available() > 0) {
        rlen = _serial->readBytes(buff, MAX_BYTE); // read the serial buffer, rlen = nb. de bytes recu
        //for(uint16_t i = 0; i < rlen; i++) tft.print(buff[i], HEX); tft.print("-");
    }
    
    // -- Controle retour erreur prog Throttle 0x54, 0x06, 0x5A
    if (buff[0] == 0x54 && buff[1] == 0x06 && buff[2] == 0x5A) {
        //Serial.println(buff[0], HEX); Serial.println(buff[1], HEX); Serial.println(buff[2], HEX);
        
        sendNiv(); // send Niv_0 to validate new prog : Mandatory if no reset !
        
        return true;
    }
    else {
        
        return false;
    }
} //endfunc


// PRIVATE

// send Niv to validate new prog
void BBS_config::sendNiv() 
{
    const byte NIV_0[] = {0x16, 0x0b, 0x00, 0x21};
    const byte NIV_1[] = {0x16, 0x0b, 0x01, 0x22};
    for (byte i = 0; i <= 3; i++) _serial->write(NIV_0[i]); // send niv_0
    delay(50);
    for (byte j = 0; j <= 3; j++) _serial->write(NIV_1[j]); // replace niv_1 by default
    
} //endfunc

