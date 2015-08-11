#include "Arduino.h"
#include "EmbitBee.h"

using namespace std;

void setup() {    
   Serial.begin(9600);
   Serial1.begin(9600);  


//####### example code #############
  //write and check all init settings
 Serial.write(embitbee.init()); //if return data is 0x03 no error accured

  //write some data
 // uint8_t data[]={0x00, 0x27, 0xb4, 0x08, 0x9e, 0xfb, 0xd1, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x0b, 0x24, 0x00, 0x01, 0x24, 0xff, 0xd3, 0x01};//"Hello World";//{0x01, 0x02, 0x03, 0x04};
 uint8_t data[]={0xD1, 0xD2, 0xD3};
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



