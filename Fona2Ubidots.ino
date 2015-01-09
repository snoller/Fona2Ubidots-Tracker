+#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include <Wire.h>
#include "SFE_ISL29125.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdlib.h>

SFE_ISL29125 RGB_sensor;

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define FONA_KEY 7
#define FONA_PS 8
#define led 13

SoftwareSerial myfona = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(&myfona, FONA_RST);


void setup() {
  //Serial.begin(9600);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(100);
  digitalWrite(led, LOW);
  delay(100);
     if (RGB_sensor.init())
  {
    for(int i=0;i<2;i++){
      unsigned int trash1 = RGB_sensor.readRed();
      unsigned int trash2 = RGB_sensor.readGreen();
      unsigned int trash3 = RGB_sensor.readBlue();
      delay(1000);
      }
  }
  fona.setGPRSNetworkSettings(F("eseye.com"), F("user"), F("pass"));
  pinMode(FONA_KEY, OUTPUT);
  TurnOffFona();
  delay(1000);
  wdt_reset();
  wdt_enable(WDTO_8S);
 }


void loop() {
  digitalWrite(led, HIGH);
  SendGPS();
  wdt_reset();
  delay(1000);
  SendMeasurements();
  wdt_reset();
  digitalWrite(led, LOW);
  delay(100);
  TurnOffFona();
  delay(100);
sleepabit(3600);
}

void SendMeasurements()
{
  unsigned int red = map(RGB_sensor.readRed(),0,60000,0,255);
  unsigned int green = map(RGB_sensor.readGreen(),0,60000,0,255);;
  unsigned int blue = map(RGB_sensor.readBlue(),0,60000,0,255);;
  char value_red[3+1];
  char value_green[3+1];
  char value_blue[3+1];
  sprintf(value_red,"%d",red);
  sprintf(value_green,"%d",green);
  sprintf(value_blue,"%d",blue);
  TurnOnFona();
  delay(100);
  fona.begin(4800);                   
  delay(1000);
  wdt_reset();
  uint16_t vbat;
  fona.getBattPercent(&vbat);
  char value_bat[20];
  sprintf(value_bat,"%d",vbat);
  GetConnected();                              
  Send2ubidots("XXXXvar1",value_red,"XXXvar2",value_green,"XXXvar3",value_blue,"XXXvar4",value_bat);
  GetDisconnected();
  wdt_reset();  
  TurnOffFona(); 
}


void SendGPS()
{
  wdt_reset();
  char replybuffer[80];
  uint16_t returncode;
  TurnOnFona();
  wdt_reset();
  fona.begin(4800);
  delay(3500);
  wdt_reset();
  fona.enableGPRS(false); 
  delay(3000);
  wdt_reset();
  fona.enableGPRS(true);
  delay(3000);
  wdt_reset();
  uint16_t vbat;
  fona.getBattPercent(&vbat);
  char value_bat[20];
  sprintf(value_bat,"%d",vbat);
       if (!fona.getGSMLoc(&returncode, replybuffer, 250))
         Serial.println(F("Failed!")); 
       
       if (returncode == 0) {
         //parse lat and lon
           char *lat;
           char *lon;
           char delimiter[] = ",";
           char *ptr;
           ptr = strtok(replybuffer, delimiter);
           int h = 0;
           while(ptr != NULL) {
             if(h==0)
              lat=ptr;
             if(h==1)
              lon=ptr;
             ptr = strtok(NULL, delimiter);
             h++;
            }
            wdt_reset();
            Send2ubidots_gps("XXXvarGPS",value_bat,lon,lat);
       } else {
       }
   wdt_reset();
   delay(1000);
  TurnOffFona();
}

void Send2ubidots(char *variable1, char *value1, char *variable2, char *value2, char *variable3, char *value3, char *variable4, char *value4)
{
   wdt_reset();
   int num;
   num = strlen(value1)+strlen(value2)+strlen(value3)+strlen(value4)+strlen(variable1)+strlen(variable2)+strlen(variable3)+strlen(variable4)+15+11+17+11+17+11+17+11+2+1;
   char sendstring[num];
   sprintf(sendstring,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","[{\"variable\": \"",variable1,"\", \"value\":",value1,"}, {\"variable\": \"",variable2,"\", \"value\":",value2,"},{\"variable\": \"",variable3,"\", \"value\":",value3,"},{\"variable\": \"",variable4,"\", \"value\":",value4,"}]");   
   char numchar[get_int_len(num)+1];
   itoa(num,numchar,10);
   //wdt_reset();
   char lchar[15+1+strlen(numchar)+1]; //string below + whitespace + num
   sprintf(lchar,"%s %s","Content-Length:",numchar);
  
  if (fona.println("AT+CIPSHUT")) {
       delay(100);
    }
  if (fona.println("AT+CIPMUX=0")) {
      delay(100);
      wdt_reset();
    }
    if (fona.println("at+cipcsgp=1,\"eseye.com\",\"user\",\"pass\"")) { //your APN setting go here
      delay(100);
    }
  if (SendATCommand("AT+CIPSTART=\"tcp\",\"things.ubidots.com\",\"80\"", 'C', 'T')) {
      delay(100);
      wdt_reset();
    }
  if (SendATCommand("AT+CIPSEND", '\n', '>')) {
    delay(100);
    }
  fona.println("POST /api/v1.6/collections/values HTTP/1.1");
  fona.println("Content-Type: application/json");
  fona.println(lchar);
  fona.println("X-Auth-Token: XXXtoken");   //your token goes here                                  
  fona.println("Host: things.ubidots.com");
  fona.println();
  fona.println(sendstring);
  fona.println();
  fona.println((char)26);                                       
  if (SendATCommand("", '2', '0')) {                            
  }
  else {
  }
  if (SendATCommand("AT+CIPCLOSE", 'G', 'M')) {
  }
  wdt_reset();
}

void Send2ubidots_gps(char *variable1, char *value1, char *longi, char *lat)
{
  wdt_reset();
  wdt_disable();  
  int num;
  num = strlen(longi)+strlen(value1)+strlen(lat)+9+21+9+2+1;
  char sendstring[num];
  sprintf(sendstring,"%s%s%s%s%s%s%s","{\"value\":",value1,", \"context\": {\"lat\": ",longi,", \"lng\": ",lat,"}}");
  char numchar[get_int_len(num)+1];
  itoa(num,numchar,10);
  char varichar[25+16+strlen(variable1)+2];
  sprintf(varichar,"%s%s%s","POST /api/v1.6/variables/",variable1,"/values HTTP/1.1");
  char lchar[15+1+strlen(numchar)+2]; //string below + whitespace + num
  sprintf(lchar,"%s %s","Content-Length:",numchar);
  wdt_reset();
    
  if (fona.println("AT+CIPSHUT")) {
       delay(100);
    }
  if (fona.println("AT+CIPMUX=0")) {
      delay(100);
      wdt_reset();
    }
    if (fona.println("at+cipcsgp=1,\"eseye.com\",\"user\",\"pass\"")) {
      delay(100);
      wdt_reset();
    }
    //wdt_reset();
  if (SendATCommand("AT+CIPSTART=\"tcp\",\"things.ubidots.com\",\"80\"", 'C', 'T')) {
      delay(100);
    }
   //wdt_reset();
  if (SendATCommand("AT+CIPSEND", '\n', '>')) {
    delay(100);
    }
    fona.println(varichar);
    fona.println("Content-Type: application/json");
    fona.println(lchar);
    fona.println("X-Auth-Token: XXXX");            //your token goes here                        
    fona.println("Host: things.ubidots.com");
    fona.println();
    fona.println(sendstring);
    fona.println();
    fona.println((char)26);                                       
    wdt_reset();
  if (SendATCommand("", '2', '0')) {                            
  }
  else {
    //Serial.println("Send Timed out, will retry at next interval");
  }
  if (SendATCommand("AT+CIPCLOSE", 'G', 'M')) {
    //wdt_reset();
  }
  wdt_reset();
  //wdt_disable();
}
 
    

boolean SendATCommand(char Command[], char Value1, char Value2) {
  unsigned char buffer[64];                                  
  unsigned long TimeOut = 20000;    
  int count = 0;
  int complete = 0;
  unsigned long commandClock = millis();                      
  fona.println(Command);
  //wdt_reset();
  while (!complete && commandClock <= millis() + TimeOut)      
  {
    while (!fona.available() && commandClock <= millis() + TimeOut);
    while (fona.available()) {                                
      buffer[count++] = fona.read();                          
      if (count == 64) break;
    }
    //Serial.write(buffer, count);                          
    for (int i = 0; i <= count; i++) {
      if (buffer[i] == Value1 && buffer[i + 1] == Value2) complete = 1;
    }
  }
  //wdt_reset();
  if (complete == 1) return 1;                             
  else return 0;
}



void GetConnected()
{
  uint8_t n = 0;
  do
  {
    n = fona.getNetworkStatus();  // Read the Network / Cellular Status
    //Serial.print(F("Network status "));
    //Serial.print(n);
    //Serial.print(F(": "));
    if (n == 0);// Serial.println(F("Not registered"));
    if (n == 1);// Serial.println(F("Registered (home)"));
    if (n == 2);// Serial.println(F("Not registered (searching)"));
    if (n == 3);// Serial.println(F("Denied"));
    if (n == 4);// Serial.println(F("Unknown"));
    if (n == 5);// Serial.println(F("Registered roaming"));
    //wdt_reset();
  } 
  while (n != 5);
  //wdt_reset();
}

void GetDisconnected()
{
  fona.enableGPRS(false);
  //Serial.println(F("GPRS Serivces Stopped"));
}

void TurnOnFona()
{
  //Serial.println("Turning on Fona: ");
  while(digitalRead(FONA_PS)==LOW)
    {
    digitalWrite(FONA_KEY, LOW);
    }
    digitalWrite(FONA_KEY, HIGH);
}

 void TurnOffFona()
{
  //Serial.println("Turning off Fona ");
  while(digitalRead(FONA_PS)==HIGH)
  {
    digitalWrite(FONA_KEY, LOW);
    //delay(100);
    }
    digitalWrite(FONA_KEY, HIGH); 
}

int get_int_len (int value){
  int l=1;
  while(value>9){ l++; value/=10; }
  return l;
}

void sleepabit(int howlong)
  {
  int i2 = 0;  
  delay(100);  
  while (i2 < (howlong/8))
    {  
    cli();  
    delay(100); 
    // disable ADC
    //ADCSRA = 0;
    //prepare interrupts
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    // Set Watchdog settings:
    WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
    sei();
    wdt_reset();  
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
    sleep_enable();
    // turn off brown-out enable in software
    //MCUCR = bit (BODS) | bit (BODSE);
    //MCUCR = bit (BODS); 
    sleep_cpu ();  
    // cancel sleep as a precaution
    sleep_disable();
    i2++;
    }
   wdt_disable(); 
  }
// watchdog interrupt
ISR (WDT_vect) 
{
  //i++;
   //Serial.println("waking up...");
}  // end of WDT_vect

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

 
