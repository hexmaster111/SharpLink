#include <SoftwareSerial.h>
#include <Arduino.h>

#define BAUDRATE0 600
#define BAUDRATE1 1200
#define BAUDRATE2 2400
#define BAUDRATE3 4800
#define BAUDRATE4 9600

#define BAUD (BAUDRATE3)

#define RX_PIN 5
#define TX_PIN 4

#define BUFFER_SIZE 1024
byte buff[BUFFER_SIZE];
SoftwareSerial SoftSerial(RX_PIN, TX_PIN, true); // RX, TX, inverse_logic = true

void setup()
{
  Serial.begin(BAUD);
  SoftSerial.begin(BAUD);
  Serial.println("Link Started");

  while (Serial.available())
  {
    Serial.read();
  }

  while (SoftSerial.available())
  {
    SoftSerial.read();
  }
}

void loop()
{
  if (Serial.available())
  { // If data comes in from serial monitor (PC), send it out to softserial port
    SoftSerial.write(Serial.read());
  }
  if (SoftSerial.available())
  { // If data comes in from softserial, send it out to serial monitor (PC)
    Serial.write(SoftSerial.read());
  }
}