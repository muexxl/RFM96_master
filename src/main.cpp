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
#include <RHSPIDriver.h>

#include <mxsupport.h>

#define FREQUENCY 868.0
#define SERIAL_SPEED 115200
#define SIZE 32
#define POWER 20
#define SPREADINGFACTOR 6 //6..12

#define REG_Detection_Optimize 0x31
#define REG_Detect_Threshold 0x37

// Singleton instance of the radio driver
RH_RF95 rf95(10, 2);

//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
//RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

int led = 9;
uint8_t data[SIZE];
bool reportingDue{false};
uint8_t *pCounter = reinterpret_cast<uint8_t *>(&data);

void init_data();
void update_data();
void test();

void set_SF6();
void set_implicit_Header();
void set_detection_optimize();
void set_detection_threshold();
void print_register(uint8_t reg);

void setup()
{

  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
  //  pinMode(4, OUTPUT);
  //  digitalWrite(4, HIGH);

  pinMode(led, OUTPUT);
  Serial.begin(SERIAL_SPEED);
  while (!Serial)
    ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");
  else
  {
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

  rf95.setModemConfig(rf95.Bw500Cr45Sf128);
  //rf95.setSpreadingFactor(SPREADINGFACTOR);
  // implicit header mode
  //rf95.spiWrite(0x1d, ((rf95.spiRead(0x1d) & 0xfe )| 0x01));

  rf95.setFrequency(FREQUENCY);
  rf95.setTxPower(POWER, false);
  init_data();
  Serial.print(F("BW500 configuration\n"));
  print_register(RH_RF95_REG_1D_MODEM_CONFIG1);
  print_register(RH_RF95_REG_1E_MODEM_CONFIG2);
  print_register(RH_RF95_REG_26_MODEM_CONFIG3);
  print_register(RH_RF95_REG_31_DETECT_OPTIMIZE);
  print_register(RH_RF95_REG_37_DETECTION_THRESHOLD);

  Serial.print(F("Changing to SF6\n"));
  set_SF6();
  print_register(RH_RF95_REG_1D_MODEM_CONFIG1);
  print_register(RH_RF95_REG_1E_MODEM_CONFIG2);
  print_register(RH_RF95_REG_26_MODEM_CONFIG3);
  print_register(RH_RF95_REG_31_DETECT_OPTIMIZE);
  print_register(RH_RF95_REG_37_DETECTION_THRESHOLD);

  Serial.print(F("Setting implicit Header\n"));
  set_implicit_Header();
  print_register(RH_RF95_REG_1D_MODEM_CONFIG1);
  print_register(RH_RF95_REG_1E_MODEM_CONFIG2);
  print_register(RH_RF95_REG_26_MODEM_CONFIG3);
  print_register(RH_RF95_REG_31_DETECT_OPTIMIZE);
  print_register(RH_RF95_REG_37_DETECTION_THRESHOLD);

  Serial.print(F("Setting detection optimize\n"));
  set_detection_optimize();
  print_register(RH_RF95_REG_1D_MODEM_CONFIG1);
  print_register(RH_RF95_REG_1E_MODEM_CONFIG2);
  print_register(RH_RF95_REG_26_MODEM_CONFIG3);
  print_register(RH_RF95_REG_31_DETECT_OPTIMIZE);
  print_register(RH_RF95_REG_37_DETECTION_THRESHOLD);

  Serial.print(F("Setting detection Threshold\n"));
  set_detection_threshold();
  print_register(RH_RF95_REG_1D_MODEM_CONFIG1);
  print_register(RH_RF95_REG_1E_MODEM_CONFIG2);
  print_register(RH_RF95_REG_26_MODEM_CONFIG3);
  print_register(RH_RF95_REG_31_DETECT_OPTIMIZE);
  print_register(RH_RF95_REG_37_DETECTION_THRESHOLD);



}


void set_SF6(){
  rf95.setSpreadingFactor(6);
}

void set_implicit_Header(){
  rf95.spiWrite(RH_RF95_REG_1D_MODEM_CONFIG1, ((rf95.spiRead(RH_RF95_REG_1D_MODEM_CONFIG1) & 0xfe )| 0x01));
}

void set_detection_optimize(){
  rf95.spiWrite(REG_Detection_Optimize, ((rf95.spiRead(REG_Detection_Optimize) & 0xf8 )| 0x05)); // 0x05 = 0b101
}

void set_detection_threshold(){
  rf95.spiWrite(REG_Detect_Threshold, 0x0C); // 0x0C for SF6 , 0x0A for SF7..12
}

void print_register(uint8_t reg){
  Serial.print("Register 0x");
  Serial.print(reg, HEX);
  Serial.print("\t 0x");
  Serial.print(rf95.spiRead(reg), HEX);
  Serial.print("\n");
}


void test(){

  //rf95.printRegisters();
  
  byte modem_config1 = rf95.spiRead(RH_RF95_REG_1D_MODEM_CONFIG1);
  byte modem_config2 = rf95.spiRead(RH_RF95_REG_1E_MODEM_CONFIG2);
  byte modem_config3 = rf95.spiRead(RH_RF95_REG_26_MODEM_CONFIG3);
  
  Serial.print(modem_config1, HEX);
  Serial.print(F(" "));
  Serial.print(modem_config2, HEX);
  Serial.print(F(" "));
  Serial.print(modem_config3, HEX);
  
  Serial.print("\nModem 1:");
  printObject(&modem_config1, 1);

  Serial.print("\nModem 2:");
  printObject(&modem_config2, 1);

  Serial.print("\nModem 3:");
  printObject(&modem_config3, 1);
  
  delay(1000);
}
void init_data()
{
  for (uint8_t i = 0; i < SIZE; i++)
  {
    data[i] = i + 0x28;
  }
  *pCounter = 0;
  printObject(&data, SIZE);
}

void update_data()
{
  ++*pCounter;
}

void loop()
{
  rf95.send(data, SIZE);
  update_data();
  rf95.waitPacketSent();
}

ISR(TIMER1_COMPA_vect)
{
  //Serial.println("This is TIMER1 ISR A\n");
  reportingDue = true;
}
