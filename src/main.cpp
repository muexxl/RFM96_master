// rf95_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing server
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf95_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with
// the RFM95W, Adafruit Feather M0 with RFM95

#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <mxsupport.h>

#define FREQUENCY 868.0
#define SERIAL_SPEED 115200
#define SIZE 251
#define POWER 2 
// Singleton instance of the radio driver
RH_RF95 rf95(10,2);
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
//RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95 

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

int led = 9;
uint8_t data[SIZE];
bool reportingDue{false};
uint8_t * pCounter = reinterpret_cast<uint8_t*>(&data);
  

void init_data();
void update_data();

void setup() 
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);

  pinMode(led, OUTPUT);     
  Serial.begin(SERIAL_SPEED);
  while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");
  else {
    Serial.println("init successful");
  }
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // You can change the modulation parameters with eg
  // rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);

// The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 2 to 20 dBm:
//  driver.setTxPower(20, false);
  // If you are using Modtronix inAir4 or inAir9, or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for 0 to 15 dBm and with useRFO true. 
  // Failure to do that will result in extremely low transmit powers.
//  driver.setTxPower(14, true);

  rf95.setFrequency(FREQUENCY);
  rf95.setTxPower(POWER, false);
  init_data();
}

void init_data(){
  for (uint8_t i = 0; i < SIZE; i++)
  {
    data[i]=i+0x28;
  }
  *pCounter=0;
 printObject(&data, SIZE);
}

void update_data() {
  ++*pCounter;
}

void loop()
{ 
    rf95.send(data, SIZE);
    update_data();
    printObject(data, SIZE);
    rf95.waitPacketSent();
}

ISR(TIMER1_COMPA_vect)
{
    //Serial.println("This is TIMER1 ISR A\n");
    reportingDue = true;
}
