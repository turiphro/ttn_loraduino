#ifndef EMBITBEE_H_
#define EMBITBEE_H_
/*
 * 
 *
 * This file is part of EMBITBee.
 */

#include <Arduino.h>
#include <stdint.h>
#include <Stream.h>

//#include <Wire.h>
//#include <iostream>



class EmbitBeeClass
{
public:
  // (16<<8|64) means ofset 16 and size 64 bits
  //Device information

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   const uint16_t Device_information_ =(0<<8|80);
    const uint16_t Device_information_EBI                          =(0<<8|10);
      const uint16_t Device_information_EBI_Protocol               =(0<<8|8);
        const uint16_t Device_information_EBI_Protocol_Family        =(0<<8|4);
        const uint16_t Device_information_EBI_Protocol_Family_Variant  =(4<<8|4);

    const uint16_t Device_information_Embit_Module               =(8<<8|8);
      const uint16_t Device_information_Embit_Module_Family              =(8<<8|4);
      const uint16_t Device_information_Embit_Module_Model               =(12<<8|2);
      const uint16_t Device_information_Embit_Module_Revision            =(14<<8|2);
  
    const uint16_t Device_information_Embit_UUID                 =(16<<8|64);
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   struct received_data{
    uint8_t *data;
    uint8_t size;  
   }received_data;

   enum Command{
    Device_information      =0x01,
     
    Device_state            =0x04,
    Reset               =0x05,
    Firmwareersion            =0x06,
    Restore_to_factory_default_settings =0x07,
    Save_settings           =0x08,
    Serial_port_configuration     =0x09,
    Output_power            =0x10,
    Operating_channel         =0x11,
    
    Physical_address          =0x20,
    Network_address           =0x21,
    Network_identifier          =0x22,
    Network_role            =0x23,
    Network_automated_settings      =0x24,
    Network_preferences         =0x25,
    Network_security          =0x26,
  
    Network_security_AppKey,
    Network_security_NwkSKey,
    Network_security_AppSKey,
   
    Network_stop            =0x30,
    Network_start           =0x31,
  
    Send_data             =0x50,
    Received_data           =0x60,
   };

   

   enum receive_error{
    receive_continue=0,     //receiving data
    
    receive_retransmission, //some error accur try to resend;
    
    receive_complete,       //no errors 
    receive_wait,           //no data received after X mseconds
    receive_wait_sequence,  //no sequence of data received after X mseconds
    receive_crc,            //wrong crc
    receive_payload,        //wrong payload
    receive_id              //wrong ID
   };
   ////////////////////////////////////////////////////////////////////////////////
   Stream *_myStream=&Serial1;

  uint32_t max_wait_sequence=300;  //wait time until data sequence is received in ms
  uint32_t max_wait_received=1000; //wait time until data is received in ms
  uint32_t max_wait_received_send=60000; //wait time until data with ACK is received in ms
  uint32_t max_retries=3;

  
  //NETWORK_PREFERENCES()/////////////////////////////////////////////////////////
  struct{
   bool Protocol     =true; //0 = LoRaEMB (private network) 1 = LoRaWAN (public network)
   bool Auto_Joining =false;//0 = Private network 1 = Public network
   bool ADR          =false; //0 = disable ADR (Adaptive Data Rate) 1 = enable ADR (Adaptive Data Rate)   
  }NETWORK_preferences;

  struct{
    const uint8_t DevEUI[8]={0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}; //always 8 bits
    const uint8_t AppEUI[8]={0x00, 0x00, 0x00, 0x00, 0x07, 0xBB, 0x77, 0x7F};
  }PHYSICAL_address;

 //PHYSICAL_ADDRESS()/////////////////////////////////////////////////////////
 struct {
  const uint8_t AppKey[16]={0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
  const uint8_t AppSKey[16]={0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
  const uint8_t NwkSKey[16]={0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
   enum{
    APPKEY=0x01,
    APPSKEY=0x11,
    NWKSKEY=0x10
   };
  }NETWORK_security;
  
 //NETWORK_IDENTIFIER()/////////////////////////////////////////////////////////

  struct{
    uint8_t ID[4]={0x00, 0x00, 0x00, 0x65};
    uint8_t size=sizeof(ID);
  }NETWORK_identifier;

    
  //NETWORK_ADDRESS()/////////////////////////////////////////////////////////
  struct{
    
   uint8_t Address[4]={0x07, 0xBB, 0x77, 0x8F};
   uint8_t size=4;
  }NETWORK_address;

  //SEND_DATA()/////////////////////////////////////////////////////////

  struct{
   const uint8_t *data;
   uint8_t size;

   bool LoraWAN      =true; //0 = LoRaEMB (private network) 1 = LoRaWAN (public network)
   bool Req_ACK      =true;//0 = Disable ACK 1 = Enable ACK
   bool ADR          =false; //0 = disable ADR (Adaptive Data Rate) 1 = enable ADR (Adaptive Data Rate)
   
   uint8_t FPort=0x06;
   int  Dst=0x7766;   //network address of the destination device for LoRaEMB
  }SEND_data;
  

  //NETWORK_ROLE()/////////////////////////////////////////////////////////
   

  struct{
     enum{
      Coordinator=0x01,
      End_Device=0x02
    };
    uint8_t role=End_Device;
    
  }NETWORK_role;



  ////////////////////////////////////////////////////////////////////////////////
  uint8_t state_init=Network_stop; 

  uint8_t init();
  uint8_t init2();
  uint8_t write(const uint8_t *data, int size);
  uint8_t get(const uint8_t state);
  
  
  void write_init();
  
  uint8_t received_payload();
  
  uint8_t receive();
  uint8_t receive_check();
  
  uint8_t receive_init_state();

   uint8_t receive_init_state2();
  uint8_t receive_write(const uint8_t  *data, int length);
  //uint8_t init_receive();

  
  void SEND_DATA(); 
  //void write_data(uint8_t type);

  void RESET();
  void NETWORK_PREFERENCES(); 
  void NETWORK_STOP();
  void NETWORK_START();
  void PHYSICAL_ADDRESS();
  void NETWORK_SECURITY(const uint8_t key);
  void NETWORK_IDENTIFIER();

  ///////////////////////////////////////////////////////////

  void NETWORK_ROLE();
  void NETWORK_ADDRESS();
  ///////////////////////////////////////////////////////////
  void    DEVICE_INFORMATION();
  uint8_t DEVICE_INFORMATION_get();
 ///////////////////////////////////////////////////////////
  uint8_t return_byte(const uint16_t type);
  uint8_t return_array(uint16_t type, uint8_t i);

  uint8_t return_size(const uint16_t type);
  uint8_t return_offset(const uint16_t type);
  //uint8_t return_byte(uint8_t offset, uint8_t size);
  ///////////////////////////////////////////////////////////
private:

  unsigned long receive_package_time;
  unsigned int SERIAL_count=0; 
  unsigned int RECEIVE_count=0;
  
  int payload_size=0xFF;
  uint8_t input[20];
  
  uint8_t crc_write   =0;
  uint8_t crc_receive =0;  

  
  void write_header(const uint8_t type);
  void write_header(const uint8_t type, int length);
  void write_body(const uint8_t data_byte);
  void write_body(const uint8_t *data_bytes, int length);
  void write_crc();

  uint8_t receive_state();
  uint8_t receive_count();
};

extern EmbitBeeClass embitbee;
#endif /* EMBITBEE_H_ */
