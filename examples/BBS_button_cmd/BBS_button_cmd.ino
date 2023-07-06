/******************************************************************
Created with PROGRAMINO IDE for Arduino - 23.06.2023 | 09:35:21

Author                : Chris74
Description           : BAFANG BBSxx e-bike config tool, minimum example
Board                 : ESP32 dev. / Arduino Uno/Nano with limitations
Library and examples  : Github: https://github.com/Chris741233/BBS_config 

Arduino/ESP32 see diagram in "/img" or "/docs" repertory

Minimum version to connect and reprogram your Bafang BBS0x (Led and button to cmd)
(for more possibility see BBS_webserial example)
-----------------------------------------------
Once your settings are checked in the "BbsSetting.h" file (same directory as this code), 
connect the Arduino/ESP32 at the BBBS and briefly press the button. 
- If the code is transferred successfully, the LED will flash quickly several times! 
- If fails, the LED will not flash. 
Connect the serial USB port if need more info
-------

FR:
Version minimale pour connecter et reprogramer votre Bafang BBS0x (Led et bouton pour commander)
(for more possibility see webserial example)
----------------------------------------------
Une fois vos paramètres vérifiés dans le fichier "BbsSetting.h" (même dossier que ce code), 
connecter l'Arduino/ESP32 au BBBS et appuyer brièvement sur le bouton.
- Si le code est transféré avec succès, la Led clignotera rapidement plusieurs fois !
- Si échec, la Led ne clignotera pas.
Brancher le port serial USB si besoin de plus d'info


Warning : if ESP32, use a level conversion (simply diode with resistor) for RX ESP32 (5V to 3.3V) see diagram on "/img" directory !


******************************************************************/

#define USE_ESP32 1  // Choose: 1=ESP32, 0=Arduino Uno/Nano


#if USE_ESP32
    #include <HardwareSerial.h> 
#else
    #include <SoftwareSerial.h> // Arduino Uno/Nano   
#endif


// -- library for config Bafang BBSxx
#include <BBS_config.h>       // https://github.com/Chris741233/BBS_config 

// All your settings for your Bafang BBSxx (this file must be in the same folder as this sketch)
#include "BbsSetting.h" 

// -- objet BBS_config
BBS_config bbs; 


// --- GPIO and init Serial object (SerialBbs) ---- 
#if USE_ESP32
    // ESP32
    const int RX2_PIN    = 16;  // RX Serial2 (16 default esp32)
    const int TX2_PIN    = 17;  // TX Serial2 (17 default esp32)
    const int BUTTON_PIN =  2;  // Button, to send reprog  
    const int LED_PIN    = 26;  // LED control            
    
    HardwareSerial SerialBbs(2);      
#else
    // Arduino
    const int RX2_PIN    = 10;  // SoftwareSerial RX 
    const int TX2_PIN    = 11;  // SoftwareSerial TX
    const int BUTTON_PIN =  2;  // Button, to send reprog 
    const int LED_PIN    = 13;  // LED control (13=LED_BUILTIN)           
    
    SoftwareSerial SerialBbs(RX2_PIN, TX2_PIN); 
#endif

                    

// --- VAR GLOBAL ----

int buttonState;



void setup()
{
    // -- Serial console (USB) if debug
    Serial.begin(115200);
    
    // -- Serial2 BBS (BBSxx = 1200 Bd !)
    #if USE_ESP32
        SerialBbs.begin(1200, SERIAL_8N1, RX2_PIN, TX2_PIN);
    #else
        SerialBbs.begin(1200);
    #endif
    
    bbs.begin(SerialBbs); //  begin BBS object, serial2
    
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // button run prog
    pinMode(LED_PIN, OUTPUT);           // Led control 
    digitalWrite(LED_PIN, LOW);         // boot Led low 
    
    delay(500);
    
    
    // -- Generate frames (buffers) - mandatory before writing to BBS0x !
    // -- Verify or modify your BBSxx setting in "BbsSetting.h" in this directory
    
    // generate buffer Basic
    if (bbs.generateBasic(B_LEVEL, B_LB, B_CL, B_WD)) Serial.println("generate Basic OK");                        
    else Serial.println("Error generate Basic, verify your settings !");

    
    // generate buffer PAS
    if (bbs.generatePas(P_DA, P_SL, P_SC, P_SSM, P_SDEG, P_WA, P_TS, P_CD, P_SD, P_KC)) Serial.println("generate PAS OK");   
    else Serial.println("Error generate PAS, verify your settings !");
    
    
    // generate buffer Throttle
    if (bbs.generateThrottle(T_SV, T_EV, T_type, T_DL, T_SL, T_SC)) Serial.println("generate Throttle OK");
    else Serial.println("Error generate Throttle, verify your settings !"); 
    
    
    // if use serial USB, info frames buffer (debug) 
    infoBufferDEC();
    frameBasicHex();
    framePasHex();
    frameThrottleHex();
    
    delay(500);  
    
    
} // endsetup


void loop()
{
    
    // Button command
    buttonState = digitalRead(BUTTON_PIN); // with input_pullup high = off
    
    if (buttonState == LOW) {
        delay(100);
        Serial.println("wait ...");
        
        // you can comment and keep only the desired part !
        sendBasic ();     // BASIC part
        delay(500);
        sendPas ();       // PAS part
        delay(500);
        sendThrottle ();  // THROTTLE
        
        delay(1000);
        digitalWrite(LED_PIN, LOW);
        
    } // endif button
    
} // endloop


// -- send write BASIC
void sendBasic () {
    bool checkBasic = bbs.writeBasic(); 
    // if succes
    if (checkBasic) {
        Serial.println("SUCCES (BASIC part)");
        frameBasicHex();
        blinkLed(5,100); // nb of blink, delay
    }
    else { 
        Serial.println("ERROR (BASIC part) !");
        //frameBasicHex();
        //blinkLed(1,500); // nb of blink, delay
    }
} //endfunc

// -- send write PAS
void sendPas () {
    bool checkPas = bbs.writePas(); 
    // if succes
    if (checkPas) {
        Serial.println("SUCCES (PAS part)");
        framePasHex();
        blinkLed(5,100); // nb of blink, delay
    }
    else { 
        Serial.println("ERROR (PAS part) !");
        //framePasHex();
        //blinkLed(1,500); // nb of blink, delay
    }
} //endfunc

// -- send write THROTTLE
void sendThrottle () {
    bool checkThrottle = bbs.writeThrottle(); 
    // if succes
    if (checkThrottle) {
        Serial.println("SUCCES (THROTTLE part)");
        frameThrottleHex();
        blinkLed(5,100); // nb of blink, delay
    }
    else { 
        Serial.println("ERROR (THROTTLE part) !");
        //frameThrottleHex();
        //blinkLed(1,500); // nb of blink, delay
    }
} //endfunc


void blinkLed (int nb, int ms) {
    for(int i=1; i<=nb; i++)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(ms);
        digitalWrite(LED_PIN, LOW);
        delay(ms);
    }
} //endfunc




// ************ Debug Serial ************

// -- Serial (USB) info buffer in DEC
void infoBufferDEC()
{
    // debug generate buffers (frames for reprog BBS) :
    Serial.println("-------- BASIC DEC-------------------");
    for (byte i = 0; i < sizeof(bbs.buffBasic); i++) {
        Serial.print(i, DEC);
        Serial.print(" = ");
        Serial.println(bbs.buffBasic[i], DEC); // or HEX
    }
    Serial.println("-------- PAS DEC --------------------");
    for (byte i = 0; i < sizeof(bbs.buffPas); i++) {
        Serial.print(i, DEC);
        Serial.print(" = ");
        Serial.println(bbs.buffPas[i], DEC); // or HEX
    }
    Serial.println("-------- THROTTLE DEC ---------------");
    for (byte i = 0; i < sizeof(bbs.buffThrottle); i++) {
        Serial.print(i, DEC);
        Serial.print(" = ");
        Serial.println(bbs.buffThrottle[i], DEC); // or HEX
    }
    
} //endfunc

// -- info frame Basic in HEX (one line)
void frameBasicHex()
{
    Serial.println("-------- HEX Basic------------------");   
    for (byte i = 0; i < sizeof(bbs.buffBasic); i++) {
        Serial.print(bbs.buffBasic[i], HEX); 
        Serial.print(" ");
    }
    Serial.println();
} //endfunc


// -- info frame PAS in HEX (one line)
void framePasHex()
{
    Serial.println("-------- HEX Pas---------------------");   
    for (byte i = 0; i < sizeof(bbs.buffPas); i++) {
        Serial.print(bbs.buffPas[i], HEX); 
        Serial.print(" ");
    }
    Serial.println();
} //endfunc


// -- info frame THROTTLE in HEX (one line)
void frameThrottleHex()
{
    Serial.println("-------- HEX Throttle----------------");   
    for (byte i = 0; i < sizeof(bbs.buffThrottle); i++) {
        Serial.print(bbs.buffThrottle[i], HEX); 
        Serial.print(" ");
    }
    Serial.println();
} //endfunc

// ************ END Debug Serial ************










