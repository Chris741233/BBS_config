/******************************************************************
Created with PROGRAMINO IDE for Arduino - 23.06.2023 | 09:35:21

Author                : Chris74
Description           : BAFANG BBSxx e-bike config tool, complete example with OTA and WebSerial !
Board                 : ESP32 dev. / ESP32 Pico
Library and examples  : Github: https://github.com/Chris741233/BBS_config 

Diagram see "/img" or "/docs" repertory

Setting and usage :
--------------------
- Install all library (url)
- Indicate your Wifi ssid and password 
- Verify your ESP32 GPIO
- Verify or modify your Bafang BBS setting in "BbsSetting.h" in this directory
and upload the sketch (first with USB and after by Wifi OTA)

- First connect ESP32 to your BBSxx controler and connect your e-bike battery 
(do not forget to connect battery and/or press button ON if exist !)
- In your navigator type "IP Esp32/webserial" expl: 192.168.1.162/webserial
- In WebSerial windows, type "h" or "help" to see all cmd

Warning : Use a level conversion (simply diode with resistor) for RX ESP32 (5V to 3.3V) see diagram on "/img" directory !


HIGO-B5-B femelle connector :
https://www.e-bike-technologies.de/index.php/en/connectors/connectors-signal/connectors-higo-mini-b-series/higo-mini-b-with-overmolded-cable/higo-b5-b-detail

******************************************************************/

const char* ssid = "Your WiFi SSID";           //  REPLACE_WITH_YOUR_SSID
const char* password = "Your WiFi Password";   //  REPLACE_WITH_YOUR_PASSWORD
// -- WebSerial expl url  :  192.168.1.162/webserial


#include <HardwareSerial.h>

// -- library for config Bafang BBSxx
#include <BBS_config.h>       // https://github.com/Chris741233/BBS_config 

// All your settings for your Bafang BBSxx (this file must be in the same folder as this sketch)
#include "BbsSetting.h"       


// github WebSerial : https://github.com/ayushsharma82/WebSerial
// see also :  https://randomnerdtutorials.com/esp32-webserial-library/

#include <WiFi.h>
#include <AsyncTCP.h>           // https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h>  // https://github.com/me-no-dev/ESPAsyncWebServer
#include <WebSerial.h>          // https://github.com/ayushsharma82/WebSerial

// OTA
#define USE_OTA 1  // use OTA ? 1=yes, 0= no : Warning, if No you must have access to USB port !

#if USE_OTA 
    #include <ESPmDNS.h>
    #include <WiFiUdp.h>
    #include <ArduinoOTA.h>
#endif


// --- If use M5Stamp Pico (Led)
/*
#include <FastLED.h>
#define NUM_LEDS 1      // How many leds in your strip
#define DATA_PIN 27
CRGB leds[NUM_LEDS];    // Define the array of leds
#define BRIGHTNESS  100 // max 255
//CRGB::Red             // Red and Green inversed ?!
//CRGB::Gray            // 'white' is too bright compared to red and blue
//CRGB::Blue
//CRGB::Black           // = Led Off
*/



// --- INIT OBJECT ----

HardwareSerial SerialBbs(2);  // init Serial2 
BBS_config bbs;               // objet Serial2 BBS_config

AsyncWebServer server(80);    // port for WebSerial

// --- GPIO ----
// Warning : Use a level conversion (simply diode with resistor) for RX ESP32 (5V to 3.3V) see diagram on "/img" directory !

const int RX2_PIN    = 18;  // RX Serial2 (ESP32 default=16) - M5 Pico=18
const int TX2_PIN    = 19;  // TX Serial2 (ESP32 default=17) - M5 Pico=19

const int LED_PIN    = 26;   // LED control - if M5 Pico, use FastLED.h (DATA_PIN=27)
//const int BUTTON_PIN = 39;   // if use M5 Pico, 39 = bouton 


// -- Global var
bool basicError = false;
bool pasError   = false;
bool throtError = false;


// -- Main
void setup()
{
    // -- Serial console (USB) 
    Serial.begin(115200);
    Serial.println("Start Bafang BBS ESP32 tool ...");
    
    // -- Serial2 Bafang BBS
    SerialBbs.begin(1200, SERIAL_8N1, RX2_PIN, TX2_PIN);
    bbs.begin(SerialBbs); //  begin BBS serial2
    
    // -- Web serial
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    // WebSerial is accessible at "<IP Address>/webserial" in browser
    WebSerial.begin(&server);
    /* Attach Message Callback */
    WebSerial.msgCallback(recvMsg);
    server.begin();
    
    #if USE_OTA
        mode_OTA();
    #endif
    
    pinMode(LED_PIN, OUTPUT);           // Led control 
    digitalWrite(LED_PIN, LOW);         // boot Led low 
    
    /*
    pinMode(BUTTON_PIN,INPUT); // Bouton Pico
    // -- Led Pico
    FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
    FastLED.setBrightness( BRIGHTNESS );
    leds[0] = CRGB::Red;  // Red et Green sont inversé ?! 
    // (Black=off)
    FastLED.show();
    */
    
    delay(500);
    
    
    // -- Generate frames (buffers) - mandatory before writing to BBS0x !
    // -- Verify or modify your BBSxx setting in "BbsSetting.h" in this directory
    generateAllBuffers();
    
    
    delay(500);  
    
} // endsetup


void loop()
{
    
    #if USE_OTA
        ArduinoOTA.handle();
    #endif
    
} // endloop



void generateAllBuffers() {
    // generate buffer Basic
    if (bbs.generateBasic(B_LEVEL, B_LB, B_CL, B_WD)) Serial.println("generate Basic OK");                        
    else {
        basicError = true;
        Serial.println("Error generate Basic, verify your settings !");
    }
    
    // generate buffer PAS
    if (bbs.generatePas(P_DA, P_SL, P_SC, P_SSM, P_SDEG, P_WA, P_TS, P_CD, P_SD, P_KC)) Serial.println("generate PAS OK");   
    else {
        pasError = true;
        Serial.println("Error generate PAS, verify your settings !");
    }
    
    // generate buffer Throttle
    if (bbs.generateThrottle(T_SV, T_EV, T_type, T_DL, T_SL, T_SC)) Serial.println("generate Throttle OK");
    else {
        throtError = true;
        Serial.println("Error generate Throttle, verify your settings !");
    }
} //endfunc


// -- message callback of WebSerial 
void recvMsg (uint8_t *data, size_t len){
    WebSerial.println("Received Data...");
    String d = "";
    for(int i=0; i < len; i++){
        d += char(data[i]);
    }
    WebSerial.println(d);
    
    if (d == "h" || d=="help"){
        helpCmd(); // help cmd
    }
    else if (d=="i" || d=="info"){
        infoBasic();      // full info setting (DEC)  
        infoPas();
        infoThrottle();
    }
    else if (d=="hex" ){
        frameBasicHex();  // frame HEX
        framePasHex();
        frameThrottleHex();
    }
    else if (d=="led"){
        blinkLed (6, 50);  // test led
    }
    else if (d=="rb"){
        BbsReadBasic();    // read BBS Basic
    }
    else if (d=="rp"){
        BbsReadPas();      // read BBS Pas
    }
    else if (d=="rt"){
        BbsReadThrottle(); // read BBS Throttle
    }
    else if (d=="wb"){
        sendBasic();       // write Basic
    }
    else if (d=="wp"){
        sendPas();         // write Pas
    }
    else if (d=="wt"){
        sendThrottle();     // write Throttle
    }
    else if (d=="wall"){
        sendBasic();        // write All
        delay(200);
        sendPas();
        delay(200);
        sendThrottle();
    }
    
    else {
        WebSerial.println("unknown command!"); 
    }
} //endfunc



// -- send write BASIC
void sendBasic () {
    if (basicError) {
        WebSerial.println("Error in your Basic setting !");
        WebSerial.println("verify BbsSetting.h");
        return;
    }
    
    bool checkBasic = bbs.writeBasic(); 
    // if succes
    if (checkBasic) {
        WebSerial.println("SUCCES (BASIC part) !");
        frameBasicHex();
        blinkLed(4,100); // nb of blink, delay
    }
    else { 
        WebSerial.println("WRITING ERROR (BASIC part) !");
        //frameBasicDec();
        //blinkLed(1,500); // nb of blink, delay
    }
} //endfunc

// -- send write PAS
void sendPas () {
    if (pasError) {
        WebSerial.println("Error in your PAS setting !");
        WebSerial.println("verify BbsSetting.h");
        return;
    }
    
    bool checkPas = bbs.writePas(); 
    // if succes
    if (checkPas) {
        WebSerial.println("SUCCES (PAS part) !");
        framePasHex();
        blinkLed(4,100); // nb of blink, delay
    }
    else { 
        WebSerial.println("WRITING ERROR (PAS part) !");
        //framePasDec();
        //blinkLed(1,500); // nb of blink, delay
    }
} //endfunc

// -- send write THROTTLE
void sendThrottle () {
    if (throtError) {
        WebSerial.println("Error in your THROTTLE setting !");
        WebSerial.println("verify BbsSetting.h");
        return;
    }
    
    bool checkThrottle = bbs.writeThrottle(); 
    // if succes
    if (checkThrottle) {
        WebSerial.println("SUCCES (THROTTLE part) !");
        frameThrottleHex();
        blinkLed(4,100); // nb of blink, delay
    }
    else { 
        WebSerial.println("WRITING ERROR (THROTTLE part) !");
        //frameThrottleDec();
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



// ************** CMD WebSerial ***************
// ********************************************

// help command
void helpCmd() {
    String frame = "";
    frame += "--- BBS list command --- \n";
    frame += "- h or help = this menu \n";
    frame += "- i or info = full mysetting info + error \n";
    frame += "- hex = mysetting frames in HEX \n";
    frame += "- led = test Led control \n";
    
    frame += "- rb = read BBS BASIC \n";
    frame += "- rp = read BBS PAS \n";
    frame += "- rt = read BBS THROTTLE \n";
    
    frame += "- wb = write BASIC \n";
    frame += "- wp = write PAS \n";
    frame += "- wt = write THROTTLE \n";
    frame += "- wall = write ALL (take a little time) \n";
    
    frame += "--- to clear window, refresh navigator --- \n";
    
    WebSerial.print(frame); //  already "\n"
} //endfunc


// read BBS Basic + print
void BbsReadBasic() {
    
    bbs.readBasic();  // cmd read BBS and mem buff
    
    String frame = "";
    WebSerial.println("-------- READ BBS BASIC (Dec-Hex) ---------------");
    for (byte i = 0; i < sizeof(bbs.buffReadBasic); i++) {
        frame += i; frame += ": ";
        frame += bbs.str_basic[i+1]; // str + 1
        frame += " = ";
        frame += bbs.buffReadBasic[i];
        frame += "    0x";
        frame += String(bbs.buffReadBasic[i], HEX);
        frame += "\n";
    }
    WebSerial.print(frame); //  already "\n"
    // clear buff 
    for (byte j = 0; j < sizeof(bbs.buffReadBasic); j++) {
        bbs.buffReadBasic[j]=0;
    }
} //endfunc

// read BBS PAS + print
void BbsReadPas() {
    
    bbs.readPas();  // cmd read BBS and mem buff
    
    String frame = "";
    WebSerial.println("-------- READ BBS PAS (Dec-Hex) ---------------");
    for (byte i = 0; i < sizeof(bbs.buffReadPas); i++) {
        frame += i; frame += ": ";
        frame += bbs.str_pas[i+1]; // str + 1
        frame += " = ";
        frame += bbs.buffReadPas[i];
        frame += "    0x";
        frame += String(bbs.buffReadPas[i], HEX);
        frame += "\n";
    }
    WebSerial.print(frame); //  already "\n"
    // clear buff
    for (byte j = 0; j < sizeof(bbs.buffReadPas); j++) {
        bbs.buffReadPas[j]=0;
    }
} //endfunc

// read BBS Throttle + print
void BbsReadThrottle() {
    
    bbs.readThrottle();  // cmd read BBS and mem buff
    
    String frame = "";
    WebSerial.println("-------- READ BBS THROTTLE (Dec-Hex) ---------------");
    for (byte i = 0; i < sizeof(bbs.buffReadThrottle); i++) {
        frame += i; frame += ": ";
        frame += bbs.str_throttle[i+1]; // str + 1
        frame += " = ";
        frame += bbs.buffReadThrottle[i];
        frame += "    0x";
        frame += String(bbs.buffReadThrottle[i], HEX);
        frame += "\n";
    }
    WebSerial.print(frame); //  already "\n"
    // clear buff
    for (byte j = 0; j < sizeof(bbs.buffReadThrottle); j++) {
        bbs.buffReadThrottle[j]=0;
    }
} //endfunc


// print full info Basic with txt
void infoBasic() {
    String frame = "";
    if(basicError) WebSerial.println("-------- ERROR settig Basic ! ---------------");
    else WebSerial.println("-------- SETTING BASIC DEC ---------------");
    for (byte i = 0; i < sizeof(bbs.buffBasic); i++) {
        frame += bbs.str_basic[i];
        frame += "=";
        frame += bbs.buffBasic[i];
        frame += "\n";
    }
    WebSerial.print(frame); //  already "\n"
} //endfunc


// print full info PAS with txt
void infoPas() {
    String frame = "";
    if(pasError) WebSerial.println("-------- ERROR setting Pas ! ---------------");
    else WebSerial.println("-------- SETTING PAC DEC ---------------");
    for (byte i = 0; i < sizeof(bbs.buffPas); i++) {
        frame += bbs.str_pas[i];
        frame += "=";
        frame += bbs.buffPas[i];
        frame += "\n";
    }
    WebSerial.print(frame); //  already "\n"
} //endfunc

// print full info throttle with txt
void infoThrottle() {
    String frame = "";
    if(throtError) WebSerial.println("-------- ERROR setting Throttle ! ---------------");
    else WebSerial.println("-------- SETTING THROTTLE DEC ---------------");
    for (byte i = 0; i < sizeof(bbs.buffThrottle); i++) {
        frame += bbs.str_throttle[i];
        frame += "=";
        frame += bbs.buffThrottle[i];
        frame += "\n";
    }
    WebSerial.print(frame); // already "\n"  
} //endfunc


// print  one line info HEX Basic
void frameBasicHex()
{
    WebSerial.println("-------- SETTING HEX BASIC---------------------");
    
    String frame = ""; // string mandatory with webserial to have full frame (or bug) !
    for (byte i = 0; i < sizeof(bbs.buffBasic); i++) {
        frame += "0x";
        frame += String(bbs.buffBasic[i], HEX); 
        //frame += bbs.buffBasic[i]; // if DEC
        frame += " ";
    }
    WebSerial.println(frame);
} //endfunc



// print one line info HEX PAS
void framePasHex()
{
    WebSerial.println("-------- SETTING HEX PAS---------------------");   
    
    String frame = ""; // string mandatory with webserial to have full frame (or bug) !
    for (byte i = 0; i < sizeof(bbs.buffPas); i++) {
        frame += "0x";
        frame += String(bbs.buffPas[i], HEX); 
        //frame += bbs.buffPas[i]; // if DEC 
        frame += " ";
    }
    WebSerial.println(frame);
} //endfunc


// print one line info HEX Throttle
void frameThrottleHex()
{
    WebSerial.println("-------- SETTING HEX THROTTLE----------------");   
    
    String frame = ""; // string mandatory with webserial to have full frame (or bug) !
    for (byte i = 0; i < sizeof(bbs.buffThrottle); i++) {
        frame += "0x";
        frame += String(bbs.buffThrottle[i], HEX); 
        //frame += bbs.buffThrottle[i]; // if DEC 
        frame += " ";
    }
    WebSerial.println(frame);
} //endfunc


// ************ END WebSerial ************



// --------------- OTA ----------
// ------------------------------

#if USE_OTA
    void mode_OTA(void) {
        
        // Port defaults to 3232
        // ArduinoOTA.setPort(3232);
        
        // Hostname defaults to esp3232-[MAC]
        // ArduinoOTA.setHostname("myesp32");
        
        // No authentication by default
        // ArduinoOTA.setPassword("admin");
        
        // Password can be set with it's md5 value as well
        // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
        // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
        
        ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";
            
            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });
        
        ArduinoOTA.begin();
        
        Serial.println("OTA Ready");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
    }//endfunc
#endif











