/*
 * 
 *
 * This file is part of EmbitBee.
 */
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The MIT License (MIT)
//
//Copyright (c) 2015 
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <Stream.h>
//#include <string.h>// mem copy
#include <avr/wdt.h>
#include <stdlib.h>
#include "EmbitBee.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
//new function in progress to generate point to point connection
uint8_t EmbitBeeClass::init2(){  
  const uint8_t array[]={0x00, 0x01};
  memcpy(NETWORK_address.Address, array, sizeof(array)); 
 uint8_t temp=0;
 state_init=Network_stop;
 write_init();
 do{
  temp=receive_check();
  if(temp==receive_complete){
    if(receive_init_state2()==receive_continue){
      
      write_init();temp=receive_continue;
    }
  }
 }while(!temp);
 return temp; //send first package of init();
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t EmbitBeeClass::init(){
 uint8_t temp=0;
 state_init=Reset;
 write_init();
 do{
  temp=receive_check();
  if(temp==receive_complete){
    if(receive_init_state()==receive_continue){
      write_init();
      temp=receive_continue;
    }
  }
  //edit after x times wrong receive commands it will just continue with the rest of the commands
  else if(temp!=receive_continue){
    write_init();
    temp=receive_init_state();
    
  }
 }while(!temp);
 return temp; //send first package of init();
}

uint8_t EmbitBeeClass::write(const uint8_t *data, int size){
  uint8_t temp=0;
  SEND_data.data=data;
  SEND_data.size=size;
  state_init=Send_data;
  write_init();
  do{
    temp=receive_check();  
  }while(!temp);
  return temp;
}


uint8_t EmbitBeeClass::get(uint8_t state){
  uint8_t temp=0;
  state_init=state;
  write_init();
  do{
    temp=receive_check();  
  }while(!temp);
  received_data.data=input+3;
  received_data.size=payload_size;
  return temp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//return specific data

uint8_t EmbitBeeClass::return_byte(uint16_t type){
  uint8_t size=type&0xFF;
  uint8_t offset=type>>8;
  uint8_t temp=(received_data.data[offset/8]<<offset%8);
  temp=temp>>(8-size);
  return temp;
}


uint8_t EmbitBeeClass::return_array(uint16_t type, uint8_t i){
  //uint8_t size=type&0xFF;
  uint8_t offset=type>>8;
  uint8_t temp=(received_data.data[(offset+i)/8]); //there can be assumed there is no ofset less than a byte
  return temp;
}


uint8_t EmbitBeeClass::return_size(uint16_t type){
  return type&0xFF;
}
uint8_t EmbitBeeClass::return_offset(uint16_t type){
  return type>>8;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t EmbitBeeClass::DEVICE_INFORMATION_get(){
  DEVICE_INFORMATION();
  uint8_t temp=get(Device_information);
  return temp;
}

void EmbitBeeClass::DEVICE_INFORMATION(){
  write_header(Device_information);
}



 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EmbitBeeClass::NETWORK_ROLE(){
  write_header(Network_role, 1);
  write_body(&NETWORK_role.role, 1);
  write_crc();
}

void EmbitBeeClass::NETWORK_ADDRESS(){
  write_header(Network_address, NETWORK_address.size);
  write_body(NETWORK_address.Address, NETWORK_address.size);
  write_crc();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void EmbitBeeClass::RESET(){
  write_header(Reset); 
}


void EmbitBeeClass::NETWORK_PREFERENCES(){
  uint8_t NetPREF[]={(NETWORK_preferences.Protocol<<7|NETWORK_preferences.Auto_Joining<<6|NETWORK_preferences.ADR<<5)};
  write_header(Network_preferences, sizeof(NetPREF));
  write_body(NetPREF, sizeof(NetPREF));
  write_crc();   
}


void EmbitBeeClass::NETWORK_STOP(){
  write_header(Network_stop); 
}

void EmbitBeeClass::NETWORK_START(){
  write_header(Network_start);
}

void EmbitBeeClass::PHYSICAL_ADDRESS(){
  write_header(Physical_address, sizeof(PHYSICAL_address.AppEUI)+sizeof(PHYSICAL_address.DevEUI));  
  write_body(PHYSICAL_address.AppEUI, sizeof(PHYSICAL_address.AppEUI));
  write_body(PHYSICAL_address.DevEUI, sizeof(PHYSICAL_address.DevEUI)); 
  write_crc();
}

void EmbitBeeClass::NETWORK_IDENTIFIER(){
  write_header(Network_identifier,  NETWORK_identifier.size); 
  write_body(NETWORK_identifier.ID, NETWORK_identifier.size);
  write_crc();
}

void EmbitBeeClass::NETWORK_SECURITY(uint8_t key){
   switch(key){
      case 0x01:
        write_header(Network_security, sizeof(NETWORK_security.AppKey)+1);//extra byte because APPKEY is written
        write_body(NETWORK_security.APPKEY);//AppKey
        write_body(NETWORK_security.AppKey, sizeof(NETWORK_security.AppKey));
      break;
      case 0x011:
        write_header(Network_security, sizeof(NETWORK_security.AppSKey)+1);
        write_body(NETWORK_security.APPSKEY);//AppSKey
        write_body(NETWORK_security.AppSKey, sizeof(NETWORK_security.AppSKey));
      break;
      case 0x10:
        write_header(Network_security, sizeof(NETWORK_security.NwkSKey)+1);
        write_body(NETWORK_security.NWKSKEY);//AppSKey
        write_body(NETWORK_security.NwkSKey, sizeof(NETWORK_security.NwkSKey));
      break;  
   }
   write_crc();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void EmbitBeeClass::SEND_DATA(){//write_data(){
  //Serial.write(0xEE);
  //Serial.write(SEND_data.size);
  
  //Serial.write(0xEE);
  
  if(SEND_data.LoraWAN){
    write_header(Send_data, SEND_data.size+3);
    write_body(SEND_data.LoraWAN<<3|SEND_data.Req_ACK<<2|SEND_data.ADR<<1);
    write_body(0x00);
    write_body(SEND_data.FPort);
    write_body(SEND_data.data, SEND_data.size);
  }
  else{
    write_header(Send_data, SEND_data.size+4);
    write_body(SEND_data.LoraWAN<<3);
    write_body(0x00);
    write_body(SEND_data.Dst>>8);
    write_body(SEND_data.Dst&0xFF);
    write_body(SEND_data.data, SEND_data.size);
  }
  write_crc();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t EmbitBeeClass::receive_init_state(){
    switch(state_init){
      case Reset:
          state_init=Device_state;
          return receive_continue;      
      break;
      case Device_state:
          state_init=Network_stop;
          return receive_continue;    
      break;
      case Network_stop:
          state_init=Network_preferences;
          return receive_continue;
      break;
     case Network_preferences:
          state_init=Physical_address;
          return receive_continue;
     break; 
      case Physical_address:
          state_init=Network_address;
          return receive_continue;         
     break;
     case Network_address:
          state_init=Network_identifier;
          return receive_continue;
     break;
     case Network_identifier:
          state_init=Network_security_AppKey;
          return receive_continue;
     break;
     case Network_security_AppKey:
          state_init=Network_security_NwkSKey;
          return receive_continue;

     break;
     case Network_security_NwkSKey :
          state_init=Network_security_AppSKey;
          return receive_continue;

     break;
     case Network_security_AppSKey :
          state_init=Network_start;
          return receive_continue;

     break;
     case Network_start :
       return receive_complete; //complete
     break; 
  
  }
  return receive_id; //wrong id
}



uint8_t EmbitBeeClass::receive_init_state2(){
    switch(state_init){
      case Network_stop:
          state_init=Network_address;
          return receive_continue;

      break;
     case Network_address:
          state_init=Network_identifier;
          return receive_continue;
          
     break;
     
     case Network_identifier:
          state_init=Network_preferences;
          return receive_continue;

     break;
      //////???
      case Network_role:
          state_init=Network_preferences;
          return receive_continue;

     break;
     
     case Network_preferences:
          state_init=Network_security_AppKey;
          return receive_continue;

     break;
     
     case Network_start :
       return receive_complete; //complete
     break; 
 
  }
  return receive_id; //wrong id
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t EmbitBeeClass::receive_check(){
  uint8_t state=receive();
  if(state==receive_complete){
    uint8_t state2=receive_state();
    if(state2==receive_complete){//all data is reiceived ok;
      RECEIVE_count=0;
      return receive_complete;//no error
    }     
    else{//correct data not received 
      if(receive_count()){//retransmission
        write_init();
        return receive_continue; 
      }
      else{//wrong payload or id
        return state2;//Wrong payload or ID
      }
    }
  }
  else if(state==receive_retransmission){//send the data again
    write_init();
    //Serial.write(0x11);
    return receive_continue;
  }
  return state;
}


void EmbitBeeClass::write_init(){
    switch(state_init){
      case Device_state:
      break;
      case Reset:
        RESET();
      break;
      case Network_stop:
        NETWORK_STOP();
      break;
      case Physical_address:    
        PHYSICAL_ADDRESS();
     break;
     case Network_identifier:
        NETWORK_IDENTIFIER();
     break;
     case Network_preferences:
        NETWORK_PREFERENCES();
     break;
     case Network_security_AppKey:
        NETWORK_SECURITY(NETWORK_security.APPKEY);
     break;
     case Network_security_NwkSKey :
        NETWORK_SECURITY(NETWORK_security.NWKSKEY);
     break;
     case Network_security_AppSKey :
        NETWORK_SECURITY(NETWORK_security.APPSKEY);
     break;
     case Network_start : 
        NETWORK_START();
     break;
     case Send_data :
        SEND_DATA();
     break;
     case Device_information :
        DEVICE_INFORMATION();
     break;
     case Network_role :
        NETWORK_ROLE();
     break;
     case Network_address :
        NETWORK_ADDRESS();
     break;
  }
}



uint8_t EmbitBeeClass::receive_state(){
    switch(state_init){
      case Device_state:
        if((payload_size==5)&&(input[2]==uint8_t(state_init|0x80))){ //some payload of init checking
          return receive_complete;
        }
      break;        
      case Reset:
        if((payload_size==5)&&(input[2]==uint8_t(state_init|0x80))){ //some payload of init checking
          return receive_complete;
        }
      break;      
      case Network_stop:
        if((payload_size==5)&&(input[2]==uint8_t(state_init|0x80))){ //some payload of init checking
          return receive_complete;
        }
      break;
      case Physical_address:
        if((payload_size==5)&&(input[2]==uint8_t(state_init|0x80))){
          return receive_complete;
        }            
     break;
     case Network_identifier:
        if((payload_size==5)&&(input[2]==uint8_t(state_init|0x80))){
          return receive_complete;
        }
     break;
     case Network_preferences:
        if((payload_size==5)&&(input[2]==uint8_t(state_init|0x80))){
          return receive_complete;
        }
     break;
     case Network_security_AppKey:
        if((payload_size==5)&&(input[2]==uint8_t(Network_security|0x80))){
          return receive_complete;
        }
     break;
     case Network_security_NwkSKey :
        if((payload_size==5)&&(input[2]==uint8_t(Network_security|0x80))){
          return receive_complete;
        }
     break;
     case Network_security_AppSKey :
        if((payload_size==5)&&(input[2]==uint8_t(Network_security|0x80))){
          return receive_complete;
        }
     break;
     case Network_start :
       return receive_complete; //complete
     break; 

     case Send_data :
        if((payload_size==8)&&(input[2]==uint8_t(Send_data|0x80))){
           return receive_complete;
        }
     break;

    ////////////////////////////////
     case Device_information :
        if(payload_size==((Device_information_&0xFF)/8+4)&&(input[2]==uint8_t(Device_information|0x80))){
           return receive_complete;
        }
     break;

     case Network_role :
        if((payload_size==5)&&(input[2]==uint8_t(state_init|0x80))){
            return receive_complete;
        }
     break;

     case Network_address :
        if((payload_size==5)&&(input[2]==uint8_t(Network_address|0x80))){
            return receive_complete;
        }
     break;
  }
  return receive_id; //wrong id
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void EmbitBeeClass::write_header(uint8_t type){
  write_header(type, NULL);
  write_crc();
}

void EmbitBeeClass::write_header(const uint8_t type, int length){
  crc_write=length+type+4;
  _myStream->write(length+4&0xFF00); 
  _myStream->write(length+4&0x00FF);
  _myStream->write(type);
  //Serial.write(type); 
}

void EmbitBeeClass::write_body(const uint8_t data_byte){
  _myStream->write(data_byte);
  //Serial.write(test2);
  crc_write+=data_byte; 
}

void EmbitBeeClass::write_body(const uint8_t *data_bytes, int length)
{
  for(int i=0; i<length; i++){
    _myStream->write(data_bytes[i]);
    crc_write+=data_bytes[i];
  }
}

void EmbitBeeClass::write_crc(){
  _myStream->write(crc_write);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t EmbitBeeClass::receive_count(){
  if(RECEIVE_count>=max_retries){//stop exc
     RECEIVE_count=0;
     return false;  //Wrong payload or ID
   }
   else{//try retransmission
     RECEIVE_count++;
     return true; 
   } 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t EmbitBeeClass::receive(){
  uint8_t state=received_payload();

  //no errors received
  if(state==receive_complete){
    return receive_complete;
  }
  
  //not yet complete package received
  else{
    if(state==receive_continue){//wait for data received
      return receive_continue;
    }
    else if(state==receive_wait_sequence){//data stall after x ms try receiving next package
      return receive_continue;
    }
    else if((state==receive_wait)){//no data reived after x ms
      if(receive_count()){//retransmission
        return receive_retransmission; 
      }
      else{//stop exc
        return receive_wait;//Waited to long for a byte
      } 
    }
    else{//return other error message
      return state;
    }
  }
  return state;
}


uint8_t EmbitBeeClass::received_payload(){
  // maximum allowed sequence to wait
  //static 
  static uint8_t  reset_time=1;

  //read serial data
  //Serial.write(0xBB);
  if (_myStream->available()){ 
    //Serial.write(0xAA);
    do { //reapeat if serial stays available
        reset_time=1;
        //static  unsigned long receive_package_time=millis(); //
        //after first byte it has to send afte x msec
        if(SERIAL_count==0){
          receive_package_time=millis();
        }
        else{
          if(millis()-receive_package_time>max_wait_sequence){
            crc_receive=0;
            SERIAL_count=0;
            reset_time=1;
            //Serial.write(0x99);
            return receive_wait_sequence;
          }
          else{
            receive_package_time=millis();
          }
        }

        uint8_t inchar =_myStream->read(); 
        input[SERIAL_count]=inchar;
        Serial.write(inchar);
        
        SERIAL_count++;
        
        //read payload size
        if(SERIAL_count==2){ 
          payload_size=(input[0]<<8)|(input[1]);
        }
        
        //check payload is correct
        if (SERIAL_count==payload_size){ 
          
          //package correctly
          if(crc_receive==inchar){
             //Serial.write(0x11);
             crc_receive=0;
             SERIAL_count=0;
             return receive_complete;
          }

          //crc incorrect
          else {
            //Serial.write(0x22);
            crc_receive=0;
            SERIAL_count=0;
            //Serial.write(0x88);
            return receive_crc;//crc error
          }
        }
   
        //new data is receiving thus add crc
        else{
          crc_receive+=inchar;  
        }
      }while(_myStream->available());    
    }
  
  else{
      //check waiting to long
      if(reset_time==1){
          receive_package_time=millis();
          reset_time=0;
       }
      else if(millis()-receive_package_time>max_wait_received){
        //send error messag
          if(state_init==Send_data){//waiting on ack can be verry long  && SEND_data.LoraWAN==true
               if(millis()-receive_package_time>max_wait_received_send){
                  crc_receive=0;
                  SERIAL_count=0;
                  reset_time=1;
                  return receive_wait;//waiting to long to get a byte
               }
            
          }
          else{
            crc_receive=0;
            SERIAL_count=0;
            reset_time=1;
            return receive_wait;//waiting to long to get a byte
          }
          //Serial.write(0x77);
          
      } 
  }
  return receive_continue;//continue
}


EmbitBeeClass embitbee;




