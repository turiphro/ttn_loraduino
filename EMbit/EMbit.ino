#include <ArduinoJson.h>
#include "Arduino.h"
#include "EmbitBee.h"

using namespace std;

/* Define device (choose one) */
#define DEVICE_TATU
//#define DEVICE_MBILI

/* Define the environment variables. */
#define JSON_SIZE      256
#define SLEEP_TIME     120000
#define WAKE_TIME      1000
#define WATER_LED      LED1
#define WAKE_LED       LED2

#ifdef DEVICE_TATU
#define DEVICE_NAME    "Tatu"
#define WATER_SENSOR   6
#define BEE_POWER      20
#endif

#ifdef DEVICE_MBILI
#define DEVICE_NAME    "Mbili"
#define WATER_SENSOR   4
//byte NetAddr[4] = {0xAA, 0xFF, 0x01, 0x01}; // not sure if this works (see below)
#endif

/* Keep track of each individual measurement */
uint32_t counter = 0; 

void initJson(JsonObject& root)
{
  root["name"] = DEVICE_NAME;
}

/* long strings cause trouble in emBit lib ??*/
void toAirTest()
{ char buff[]="Boot maakt water en gaat helemaal nat worden. Snel gaan hozen, voordat het te laat is.";
  int len = sizeof(buff);
  Serial.write(embitbee.write((uint8_t*) buff, len)); }

void toAirInit()
{ char buff[]="Boot wordt bemeten.";
  int len = sizeof(buff);
  Serial.write(embitbee.write((uint8_t*) buff, len-1)); 
  Serial.flush(); }

void toAirDroog()
{ char buff[]="Boot droog.";
  int len = sizeof(buff);
  Serial.write(embitbee.write((uint8_t*) buff, len-1));
  Serial.flush(); }

void toAirNat()
{ char buff[]="Boot maakt water.";
  int len = sizeof(buff);
  Serial.write(embitbee.write((uint8_t*) buff, len-1)); 
  Serial.flush(); }


/* Call this to send off your json */
void toAir(JsonObject& root)
{
  char buff[JSON_SIZE];
  root.printTo(buff, sizeof(buff));
  int len = root.measureLength();
  embitbee.write((uint8_t*) buff, len);
  //Serial.flush();
}

  

/* Initialize the json, peripherals and communication. 
   The green led should blink once to signal everything
   is okay. */
void setup() 
{
  pinMode(WATER_LED,    OUTPUT);
  pinMode(WAKE_LED,     OUTPUT);
  pinMode(WATER_SENSOR, INPUT);
  
#ifdef DEVICE_TATU
  pinMode(BEE_POWER,    OUTPUT);
  digitalWrite(BEE_POWER, HIGH);
#endif
  
  digitalWrite(WATER_LED, LOW);
  digitalWrite(WAKE_LED,  HIGH);

  Serial.begin(9600);
  Serial1.begin(9600);  
  Serial.println("Booting");
  embitbee.init();
  //memcpy(embitbee.NETWORK_address.Address, NetAddr, sizeof(NetAddr)); // doesn't work?
  Serial.println("Initialised");
  
  delay(WAKE_TIME);
  digitalWrite(WAKE_LED,  LOW);
}


bool measure() {
  // get average of 10 measurements
  int trues = 0;
  int falses = 0;
  for (int i=0; i<10; i++) {
    if (digitalRead(WATER_SENSOR))
      trues++;
    else
      falses++;
    delay(1);
  }
  bool inWater = (trues > falses);

  return inWater;
}

/* Read the water sensor and send the result to 
   the server. The counter can be used to check for double
   transmissions. Since we have no internal temperature sensor
   in the 1248P this value is zero. The power level measurement
   can be added later. */
void send_measurement(bool inWater)
{
  StaticJsonBuffer<JSON_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  initJson(root);

  root["count"] = counter++;
  root["water"] = inWater;
  //root["temp"]  = 0;
  //root["power"] = 0;
  toAir(root); 
  //if (inWater) toAirNat(); else toAirDroog();
  }




/* Perform periodic measurements. 
   TODO: put the processor in deep sleep in between. */
int state = 0;
long last_timestamp = 0;
bool inWater = 0;

void loop() 
{ 
  /*
  bool reading = digitalRead(WATER_SENSOR);
  digitalWrite(WATER_LED, reading);
  Serial.write(reading);
  */
  inWater = measure();
  digitalWrite(WATER_LED, inWater);
  
  switch (state) {
    case 0:
      last_timestamp = millis();
      state = 1;
      break;
    case 1:
      // SLEEPING
      if (millis() - last_timestamp > SLEEP_TIME) {
        state = 2;
        last_timestamp = millis();
        Serial.println("STATE 2");
      }
      break;

    case 2:
      // SENDING
      digitalWrite(WAKE_LED, HIGH);
      Serial.println("SENDING");
      Serial.println(inWater);
      send_measurement(inWater);
      Serial.println("Sent.");
      state = 3;
      last_timestamp = millis();
      break;

    case 3:
      // WAKING
      if (millis() - last_timestamp > WAKE_TIME) {
        Serial.println("STATE 1");
        state = 1;
        last_timestamp = millis();
        digitalWrite(WAKE_LED,  LOW);
      }
    
    default:
      state = 0;
      break;
  }
}





/* ========================================================================================================================= */
/*  Simple test code to get the bee working */
#if false
void toAir(String strMessage)
{ String strSep = ": ";
  String strComplete =  strID + strSep + strMessage;
  uint8_t data[]="Boot maakt water";
  Serial.write(embitbee.write((uint8_t*) strComplete.c_str(), strComplete.length()));
  //Serial.write(embitbee.write(data, sizeof(data)));
  //Serial.flush();
  }

void setup() 
{ Serial.begin(9600);
  Serial1.begin(9600);  
  Serial.write(embitbee.init());
  toAir("BoatWatch started."); }

void loop()
{ delay(1000);
  uint32_t msgCnt = 0;
  String strWater = "# cm water = ";
  while(true)
  { msgCnt++;
    String strNum = String(msgCnt,DEC);
    String strResult = strWater + strNum;
    toAir(strResult);
    delay(10000); } }   
#endif




/* ========================================================================================================================= */
/*  Original test code from Sodaq */
#if false
void setup() {    
   Serial.begin(9600);
   Serial1.begin(9600);  


//####### example code #############
  //write and check all init settings
 Serial.write(embitbee.init()); //if return data is 0x03 no error accured

  //write some data
 // uint8_t data[]={0x00, 0x27, 0xb4, 0x08, 0x9e, 0xfb, 0xd1, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x0b, 0x24, 0x00, 0x01, 0x24, 0xff, 0xd3, 0x01};//"Hello World";//{0x01, 0x02, 0x03, 0x04};
 uint8_t data[]="Boot maakt water";
 Serial.write(embitbee.write(data, sizeof(data)));
 // Serial.write(0xFF);
   
    
  //send and receive DEVICE_INFORMATION
 //  Serial.write(embitbee.DEVICE_INFORMATION_get());
 //  Serial.write(0xFF); 

  //example read specific data of type uint8_t
 //  Serial.write(embitbee.return_byte(embitbee.Device_information_EBI));

 //  Serial.write(0xFF);
      
   //example read specific array data of uint8_t 
   //uint8_t data2[embitbee.return_size(embitbee.Device_information_Embit_UUID)/8]; //size of array can be larger NOT smaller
   //for(int i=0; i<(embitbee.return_size(embitbee.Device_information_Embit_UUID)); i+=8){
   //  data2[i]=embitbee.return_array(embitbee.Device_information_Embit_UUID , i );
   //  Serial.write(data2[i]);
  // }

   //Serial.write(0xEE); 
   
//##################################
}

void loop() {  
   while(true){
    //some code to view send or received data
    //works with ebi-LoRa-serial-tester.exe
     while (Serial.available()) {
      char inChar = (char)Serial.read();
      Serial1.print(inChar);
     }
     while (Serial1.available()) {
      char inChar = (char)Serial1.read();
      Serial.print(inChar);
     }
   }
}
#endif


