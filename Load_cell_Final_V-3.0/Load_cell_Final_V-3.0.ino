// include the library code:
#include <LiquidCrystal.h>
#include <Arduino.h>
#include <RotaryEncoder.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>



File testfile;
String fileName = "test.csv";

/*==================== EEPROM ==========================*/
int F_Min_Addr = 4;

void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}
/*======================================================*/


void print_measure();
void print_setting();

const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define PIN_IN1 A3
#define PIN_IN2 A4
#define PUSHB A5
/*============MODBUS==================*/
#define MAX485_DE      3
#define MAX485_RE_NEG  2
#define Pass_LED A1
#define Fail_LED A0
int cellStatus = 0;
volatile int result;
float temp = 0;
float Force = 0;
/*===================================*/

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

static int pos = 0;
static int newPos = 0;
int State = 0;
volatile int F_Min = readIntFromEEPROM(F_Min_Addr);

/*==================MODBUS===================*/
// instantiate ModbusMaster object
ModbusMaster node;
void preTransmission()
{ //
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}
void postTransmission()
{ //
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}
/*========================================*/


void setup()
{
  // Modbus communication runs at 19200 baud
  Serial.begin(19200);
  while (! Serial);

  // wait for SD module to start
  if (!SD.begin(10)) {
    Serial.println("No SD Module Detected");
    while (1);
  }
  delay(10);

  pinMode(PUSHB, INPUT); //Encoder button

  /*===================MODBUS==================*/
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  pinMode(Pass_LED, OUTPUT);
  pinMode(Fail_LED, OUTPUT);

  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Modbus slave ID 1
  node.begin(1, Serial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  /*===============================================*/


  /*============= LED Init================*/
  digitalWrite(Pass_LED, HIGH);
  digitalWrite(Fail_LED, LOW);
  delay(500);
  digitalWrite(Pass_LED, LOW);
  digitalWrite(Fail_LED, HIGH);
  delay(500);
  digitalWrite(Pass_LED, LOW);
  digitalWrite(Fail_LED, LOW);
  /*======================================*/


  /*=============== LCD ============================*/
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("LOAD CELL");
  delay(1000);
  if (digitalRead(PUSHB) == LOW )
  {
    State = 1;
    Serial.println(" loop State =1");
    print_setting();
    encoder.setPosition(F_Min);
    delay(1000);
  }
  else
  {
    State = 0;
    print_measure();
  }
  /*=====================================*/

}//setup



void loop()
{
  if (State == 0)
  {
    print_modbus();

  }

  if (State == 1) {
    encoder.tick();
    newPos = encoder.getPosition();
    if (pos != newPos || digitalRead(PUSHB) == LOW) {
      pos = newPos;
      States_Menu();
    }
  }
}



void States_Menu()
{
  delay(100);
  print_setting();

  if (pos >= 0)
  {
    Serial.println(pos);
    F_Min = pos;

    if (digitalRead(PUSHB) == LOW )
    {
      Serial.println("Button Pressed");
      State = 0;
      encoder.setPosition(0);

      /*============ EEPROM ===========*/
      writeIntIntoEEPROM(F_Min_Addr, F_Min);
      /*===============================*/
    }
  }

  if (pos < 0) {
    encoder.setPosition(0);
  }

}

void print_modbus()
{
  delay(100); // Very very important for proper working of modbus
  if (cellStatus == 0)
  {

    // Read 16 registers starting at 0x3100)
    result = node.readHoldingRegisters(0x07, 2);
    Force = float(node.getResponseBuffer(0x00)) / 10; // 12 =>1.2
    if (result == node.ku8MBSuccess)
    {
      print_measure();
      Serial.print("Fmax: ");
      Serial.println(Force);
      if (Force >= F_Min) // in gm
      {
        if (Force >= temp)
        {
          temp = Force;
          Serial.println("Reading");
          digitalWrite(Pass_LED, HIGH);
          digitalWrite(Fail_LED, HIGH);
        }
        else if (Force < temp)
        {
          Serial.println(" ");
          Serial.println("PASS");
          Serial.print("Fmax: ");
          Serial.println(temp);
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("PASS");
          lcd.setCursor(1, 1);
          lcd.print("Fmax: ");
          lcd.setCursor(7, 1);
          lcd.print(temp);
          lcd.setCursor(13, 1);
          lcd.print("Kg");
          cellStatus = 1;
          digitalWrite(Pass_LED, HIGH);
          digitalWrite(Fail_LED, LOW);
          SD_Results_Pass();


        }
      }

      else        // when Force < F_Min
      {
        if (Force >= temp)
        {
          temp = Force;
          Serial.println("Reading");
          digitalWrite(Pass_LED, HIGH);
          digitalWrite(Fail_LED, HIGH);
        }
        else if (Force < temp)
        {
          Serial.println(" ");
          Serial.println("Fail");
          Serial.print("Fmax: ");
          Serial.println(temp);
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("Fail");
          lcd.setCursor(1, 1);
          lcd.print("Fmax: ");
          lcd.setCursor(7, 1);
          lcd.print(temp);
          lcd.setCursor(13, 1);
          lcd.print("Kg");
          cellStatus = 1;
          digitalWrite(Pass_LED, HIGH);
          digitalWrite(Fail_LED, LOW);

          SD_Results_Fail();


        }
      }
    }

    if (result == node.ku8MBResponseTimedOut)
    {
      digitalWrite(Pass_LED, HIGH);
      digitalWrite(Fail_LED, HIGH);
      lcd.clear();
      lcd.setCursor(5, 0);     //(col, row)
      lcd.print("Meter");
      lcd.setCursor(1, 1);     //(col, row)
      lcd.print("Disconnected");
    }
  }
}

void print_measure()
{
  Serial.println("Print_Menu");
  lcd.clear();
  lcd.setCursor(1, 0);     //(col, row)
  lcd.print("Measuring Mode:");
  lcd.setCursor(1, 1);     //(col, row)
  lcd.print("F_Min: ");
  lcd.setCursor(8, 1);     //(col, row)
  lcd.print(F_Min);
  lcd.setCursor(13, 1);     //(col, row)
  lcd.print("Kg");
}

void print_setting()
{
  Serial.println("State 1");
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Setting Mode");
  lcd.setCursor(1, 1);
  lcd.print("Fmin: ");
  lcd.setCursor(8, 1);     //(col, row)
  lcd.print(F_Min);
  lcd.setCursor(13, 1);     //(col, row)
  lcd.print("Kg");
}

void SD_Results_Pass()
{
  SD.begin(10);
  // save new integer every loop and then wait 1s
  testfile = SD.open(fileName, FILE_WRITE);
  delay(100);
  if (testfile) {
    testfile.println(String(F_Min) + "," + String(temp) + "," + "PASS");
    Serial.println(String(F_Min) + "," + String(temp) + "," + "PASS");
    testfile.close();
  } else {
    Serial.println("error opening file");
  }
}

void SD_Results_Fail()
{
  SD.begin(10);
  // save new integer every loop and then wait 1s
  testfile = SD.open(fileName, FILE_WRITE);
  delay(100);
  if (testfile) {
    testfile.println(String(F_Min) + "," + String(temp) + "," + "FAIL");
    Serial.println(String(F_Min) + "," + String(temp) + "," + "FAIL");
    testfile.close();
  } else {
    Serial.println("error opening file");
  }
}
