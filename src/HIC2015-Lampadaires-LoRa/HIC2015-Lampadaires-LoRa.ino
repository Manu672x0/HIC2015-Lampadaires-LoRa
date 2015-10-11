// Include the SX1272 and SPI library: 
#include "SX1272.h"
#include <SPI.h>

// define a protocol version
static const int PROTOCOL_BEGIN = 0;
static const int PROTOCOL_SIZE = 1;
uint8_t protocol_version = 0x00;

// define a global device number
static const int ADDRESS_BEGIN = PROTOCOL_BEGIN + PROTOCOL_SIZE;
static const int ADDRESS_SIZE = 3;
static const int ADDRESS_END = ADDRESS_BEGIN+ADDRESS_SIZE;
uint8_t device_address[ADDRESS_SIZE] = { 0x00, 0x00, 0x43 };

int e;

// define the destination address to send packets
int rx_address = 42;

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

void setup()
{
  Serial.begin(9600);
  Serial.println("Temperature and humidity data sent via LoRa.");

  
  // Power ON the module
  e = sx1272.ON();
  Serial.print(F("Setting power ON: state "));
  Serial.println(e, DEC);
  
  // Set transmission mode and print the result
  e = sx1272.setMode(1);
  Serial.print(F("Setting Mode 1: state "));
  Serial.println(e, DEC);
  
  // Set header
  e = sx1272.setHeaderON();
  Serial.print(F("Setting Header ON: state "));
  Serial.println(e, DEC);
  
  // Select frequency channel
  e = sx1272.setChannel(CH_16_868);
  Serial.print(F("Setting Channel CH_16_868: state "));
  Serial.println(e, DEC);
  
  // Set CRC
  e = sx1272.setCRC_ON();
  Serial.print(F("Setting CRC ON: state "));
  Serial.println(e, DEC);
  
  // Select output power (Max, High or Low)
  e = sx1272.setPower('M');
  Serial.print(F("Setting Power M: state "));
  Serial.println(e, DEC);
  
  // Set the node address and print the result
  e = sx1272.setNodeAddress(3);
  Serial.print(F("Setting node address: state "));
  Serial.println(e, DEC);
  
  // Print a success message
  Serial.println(F("SX1272 successfully configured"));
  Serial.println();
}

char   char_msg[256];  

void loop()
{
  sendMsg();
  delay(10000);
}

void sendMsg() {
  //Begin print
  Serial.println();
  //End print
  
  String msg = "Rubarbe" + String('#');
  
  
  //Message begins with protocole version
  char_msg[0] = protocol_version;
  //Message continue with global device address
  for( int i=ADDRESS_BEGIN, j=0; i<ADDRESS_END; ++i,++j ){ char_msg[i] = (char)device_address[j]; }
  for( int i=ADDRESS_END; i<256; ++i ){ char_msg[i] = 0; }
  msg.toCharArray( char_msg+ADDRESS_END, msg.length()+1 );

  e = sx1272.sendPacketTimeout(rx_address, (uint8_t*)char_msg, msg.length()+ADDRESS_END );
  //e = sx1272.sendPacketTimeout(rx_address, "Test" );
  Serial.print(F("Packet sent, state "));
  Serial.println(e, DEC);
  //delay( 600000 );//10min
  delay( 5000 );//30s
  //delay( 60000 );//1min
}

void Rx() {
    // Receive message
  Serial.println("Listen");
  e = sx1272.receivePacketTimeout();
  if( 0 == e )
  {
    Serial.println( "We got a message !" );
    using namespace std;
    //PAYLOAD
    char *frame_raw = (char *)sx1272.packet_received.data;
    size_t length_raw = (size_t)sx1272._payloadlength;

    //Version 
    uint8_t version_number = frame_raw[0];
    char *frame = frame_raw;
    size_t length = length_raw;

    String device_id;

    Serial.println( device_id );

    decode_header( 
        version_number, 
        frame_raw, length_raw, 
        frame, length,
        device_id );

    Serial.println( "Message:");
    for( int c=0; c<length; ++c )
    {
      Serial.print( (char)frame[c] );
    }
    Serial.println("");
    Serial.println("");
    Serial.println("=======================================");
    Serial.println("");
  }
}


