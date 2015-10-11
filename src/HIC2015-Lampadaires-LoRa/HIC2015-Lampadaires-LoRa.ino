// Include the SX1272 and SPI library: 
#include "SX1272.h"
#include <SPI.h>

// Set to 1 to enable serial monitoring of LoRa actions, else set to 0
#define LoRaSerialDebug 1 

// LoRa Protocol Version
static const int PROTOCOL_BEGIN = 0;
static const int PROTOCOL_SIZE = 1;
uint8_t protocol_version = 0x00;

// LoRa Device Parameters
static const int ADDRESS_BEGIN = PROTOCOL_BEGIN + PROTOCOL_SIZE;
static const int ADDRESS_SIZE = 3;
static const int ADDRESS_END = ADDRESS_BEGIN+ADDRESS_SIZE;

// LoRa: UNIQUE DEVICE ADDRESS
uint8_t device_address[ADDRESS_SIZE] = { 0x00, 0x00, 0x43 };

// LoRa: Helpers
int e;
char char_msg[256];
  


void LoRaSetup() {
  // Power ON the module
  e = sx1272.ON();
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting power ON: state "));
    Serial.println(e, DEC);
  #endif
  
  // Set transmission mode and print the result
  e = sx1272.setMode(1);
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting Mode 1: state "));
    Serial.println(e, DEC);
  #endif
  
  // Set header
  e = sx1272.setHeaderON();
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting Header ON: state "));
    Serial.println(e, DEC);
  #endif
  
  // Select frequency channel
  e = sx1272.setChannel(CH_16_868);
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting Channel CH_16_868: state "));
    Serial.println(e, DEC);
  #endif
  
  // Set CRC
  e = sx1272.setCRC_ON();
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting CRC ON: state "));
    Serial.println(e, DEC);
  #endif
  
  // Select output power (Max, High or Low)
  e = sx1272.setPower('M');
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting Power M: state "));
    Serial.println(e, DEC);
  #endif
  
  // Set the node address and print the result
  e = sx1272.setNodeAddress(3);
  #if (LoRaSerialDebug == 1)
    Serial.print(F("Setting node address: state "));
    Serial.println(e, DEC);
    Serial.println(F("SX1272 for LoRa successfully configured"));
    Serial.println(F("Note: State = 0 means everythings fine!"));
    Serial.println();
  #endif
}

void setup()
{
  Serial.begin(9600);
  // Initialize LoRa Communication
  LoRaSetup();
  
}

void loop()
{
  LoRaSendMsg("A unicorn is drinking a Schnaps, and eating a delicus piece of (french) cheese.", 0);
  delay(10000);
  LoRaRecieve();
}

/**
 * Header Decoding
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
 * reviecer: {int} The address of the reciever.
 */
void LoRaSendMsg(String msg, int reciever) {
  //Message begins with protocole version
  char_msg[0] = protocol_version;
  //Message continue with global device address
  for( int i=ADDRESS_BEGIN, j=0; i<ADDRESS_END; ++i,++j ){ char_msg[i] = (char)device_address[j]; }
  for( int i=ADDRESS_END; i<256; ++i ){ char_msg[i] = 0; }
  msg.toCharArray( char_msg+ADDRESS_END, msg.length()+1 );

  e = sx1272.sendPacketTimeout(reciever, (uint8_t*)char_msg, msg.length()+ADDRESS_END );
  #if (LoRaSerialDebug == 1)
    Serial.print("Packet sent: '" + msg + "', state ");
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
  e = sx1272.receivePacketTimeout();
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


