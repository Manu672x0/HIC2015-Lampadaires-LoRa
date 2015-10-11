/* 
  HIC2015-Lampadaires-LoRa based on sx1270 and Stratagem/Wavebricks. 
  Copyright (C) 2015 Manuel Rauscher
    
  This program is free software: you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
  GNU General Public License for more details. 
  
  You should have received a copy of the GNU General Public License 
  along with this program.  If not, see <http://www.gnu.org/licenses/>. 

*/

// Include the SX1272 and SPI library: 
#include "SX1272.h"
#include <SPI.h>

// LoRa Protocol Version
static const int PROTOCOL_BEGIN = 0;
static const int PROTOCOL_SIZE = 1;
uint8_t protocol_version = 0x00;

// LoRa Device Parameters
static const int ADDRESS_BEGIN = PROTOCOL_BEGIN + PROTOCOL_SIZE;
static const int ADDRESS_SIZE = 3;
static const int ADDRESS_END = ADDRESS_BEGIN+ADDRESS_SIZE;

/*
 * ~~~~ LORA CONFIGURATION ~~~~
 */

// Set to 1 to enable serial monitoring of LoRa actions, else set to 0
#define LoRaSerialDebug 1
// Set the power mode of LoRa (Please check the limitation defined by LAW) M:max/H:high/L:low
static const char LoRaPoweringMode = 'L';
// LoRa: UNIQUE DEVICE ADDRESS (hex).(hex).(hex) --> The last will be the device address
uint8_t device_address[ADDRESS_SIZE] = { 0x00, 0x00, 0x2a };

/*
 * ~~~~ END LORA CONFIGURATION ~~~~
 */
 
// LoRa: Helpers
int e;
char char_msg[256];

void LoRaSetup() {
  // Power UP the module.
  e = sx1272.ON();
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting power ON: state "));
    Serial.println(e, DEC);
  #endif
  
  // Set transmission mode and print the result.
  e = sx1272.setMode(1);
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting Mode 1: state "));
    Serial.println(e, DEC);
  #endif
  
  // Set header.
  e = sx1272.setHeaderON();
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting Header ON: state "));
    Serial.println(e, DEC);
  #endif
  
  // Select frequency channel.
  e = sx1272.setChannel(CH_16_868);
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting Channel CH_16_868: state "));
    Serial.println(e, DEC);
  #endif
  
  // Set CRC.
  e = sx1272.setCRC_ON();
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting CRC ON: state "));
    Serial.println(e, DEC);
  #endif
  
  // Select output power (Max, High or Low).
  e = sx1272.setPower(LoRaPoweringMode);
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting Power M: state "));
    Serial.println(e, DEC);
  #endif
  
  // Set the node address and print the result.
  e = sx1272.setNodeAddress(device_address[ADDRESS_SIZE-1]);
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting node address: state "));
    Serial.println(e, DEC);
    Serial.println(F("SX1272 for LoRa successfully configured!"));
    Serial.println(F("Note: State = 0 means everything is fine!"));
    Serial.print("My Address: " );
    for(int iad=0; iad < ADDRESS_SIZE; iad++) {
      Serial.print(device_address[iad]);
      Serial.print(" ");
    }
    Serial.println();
    Serial.println();
  #endif
}

void setup()
{
  Serial.begin(9600);
  // Initialize LoRa Communication.
  LoRaSetup();
  
}

void loop()
{
  LoRaSendMsg("A unicorn is drinking a Schnaps, and eating a delicus piece of (french) cheese.", 43);
  delay(10000);
  //LoRaRecieve();
}

/**
 * LoRa Header Decoding
 */
bool decode_header( 
    const uint8_t version_number,
    char * const frame_raw,
    const size_t length_raw,
    char* & frame,
    size_t& length,
    String& device_id )
{
  if( 0 == version_number )
  {
    frame = frame_raw+4;
    length = length_raw - 4;
    uint8_t d0 = (uint8_t)frame_raw[1];
    uint8_t d1 = (uint8_t)frame_raw[2];
    uint8_t d2 = (uint8_t)frame_raw[3];

    device_id = String( d0 ) + "." + String( d1 ) + "." + String( d2 ); 
    return true;
  }
  return false;
}

/**
 * Sends a message to a defined address
 * Prameters:
 * msg: {String} The message to send.
 * recieverAddress: {int} The address of the reciever. (0 for broadcast)
 */
void LoRaSendMsg(String msg, int recieverAddress) {
  //Message begins with protocole version
  char_msg[0] = protocol_version;
  //Message continue with global device address
  for( int i=ADDRESS_BEGIN, j=0; i<ADDRESS_END; ++i,++j ){ char_msg[i] = (char)device_address[j]; }
  for( int i=ADDRESS_END; i<256; ++i ){ char_msg[i] = 0; }
  msg.toCharArray( char_msg+ADDRESS_END, msg.length()+1 );

  e = sx1272.sendPacketTimeoutACKRetries(recieverAddress, (uint8_t*)char_msg, msg.length()+ADDRESS_END );
  #if (LoRaSerialDebug == 1)
    Serial.print("Packet sent: '" + msg + "', To: '" + recieverAddress + "', State ");
    Serial.println(e, DEC);
  #endif
}

/**
 * Listen for incoming messages.
 */
void LoRaRecieve() {
    // Receive message
  #if (LoRaSerialDebug == 1)
    Serial.println("Listening...");
  #endif
  e = sx1272.receivePacketTimeoutACK();
  if( 0 == e )
  {
    #if (LoRaSerialDebug == 1)
      Serial.println("=======================================");
      Serial.println("");
      Serial.println("");
      Serial.println( "Recieving new message..." );
    #endif
    using namespace std;
    // PAYLOAD
    char *frame_raw = (char *)sx1272.packet_received.data;
    size_t length_raw = (size_t)sx1272._payloadlength;

    // Version 
    uint8_t version_number = frame_raw[0];
    char *frame = frame_raw;
    size_t length = length_raw;

    String device_id;
    decode_header( 
        version_number, 
        frame_raw, length_raw, 
        frame, length,
        device_id );
        
    #if (LoRaSerialDebug == 1)
      Serial.println("Sender Device Id: " + String(device_id) );
      Serial.println("Content: ");
    #endif
    
    for( int c=0; c<length; ++c )
    {
      #if (LoRaSerialDebug == 1)
        Serial.print( (char)frame[c] );
      #endif
    }
    // TODO: Add a callback/Event call with the recieved message
    #if (LoRaSerialDebug == 1)
      Serial.println("");
      Serial.println("");
      Serial.println("=======================================");
      Serial.println("");
    #endif
  }
}


