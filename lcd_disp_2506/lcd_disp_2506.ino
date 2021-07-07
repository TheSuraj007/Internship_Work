// include the library code:
#include <LiquidCrystal.h>
#include <Arduino.h>
#include <RotaryEncoder.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define PIN_IN1 9
#define PIN_IN2 10
#define PUSHB 13
#define LED1 8
#define LED2 7
#define LED3 6

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::TWO03);

static int pos = -1;
static int newPos = 0;
int columnsLCD = 16;
int Dis_Pos = 0;
int State = 6;
int incomingdata = 0;
bool motor_bldc = true;
//int DURATION_IN_MILLIS = 1000; //for button press
//int buttonState = 0;  //button state for modes

volatile int Motor, I_Min, I_Max = 0;
float Volt = 0;

String MenuLine[4] = {" Motor", " Set min Current", " Set max Current", " Set Voltage"};
//                      0             1                     2                     3
int MenuItems = 4;


void setup()
{
  lcd.clear();
  Serial.begin(115200);
  // Serial1.begin(9600);
  while (! Serial);
  Serial.println("SimplePollRotator example for the RotaryEncoder library.");

  pinMode(PUSHB, INPUT); //Encoder button
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  digitalWrite (LED1, LOW);
  digitalWrite (LED2, LOW);
  digitalWrite (LED3, LOW);
  
  lcd.begin(16, 4);
  //lcd.print("Hello World!");
  delay(1000);
  if (digitalRead(PUSHB) == LOW ) {
      amain();
      State = 0;
      
    }
    else 
    {
      State = 6;
    }
}
void loop()
{
  //buttonState = 0;  //no button
  //  initial();
  encoder.tick();
  newPos = encoder.getPosition();
  if (pos != newPos || digitalRead(PUSHB) == LOW) {
    pos = newPos;
    amain();

  }
}

void print_menu()
{
  lcd.clear();
  lcd.setCursor(1, 0);     //(col, row)
  lcd.print("Motor:");
  lcd.setCursor(1, 1);     //(col, row)
  lcd.print("I_Min:");
  lcd.setCursor(1, 2);     //(col, row)
  lcd.print("I_Max:");
  lcd.setCursor(1, 3);     //(col, row)
  lcd.print("Volt :");
  lcd.setCursor(8, 0);     //(col, row)
  if (Motor == 1)
  {
    lcd.print("BLDC");
  }
  else
  {
    lcd.print("DC  ");
  }
  lcd.setCursor(8, 1);     //(col, row)
  lcd.print(I_Min);
  lcd.setCursor(8, 2);     //(col, row)
  lcd.print(I_Max);
  lcd.setCursor(9, 3);     //(col, row)
  lcd.print(Volt);
}

void amain() {
  //Serial.print(State);
  if (State == 0) {
    
    Dis_Pos = abs(pos) % 1;
    Serial.print("Dis_Pos= ");
    Serial.println(Dis_Pos);
    lcd.clear();
    //lcd.setCursor(0, Dis_Pos);     //(col, row)
   // lcd.print(">");
    lcd.setCursor(2, 0);     //(col, row)
    lcd.print("Setting mode");
    delay(1000);
      State = 1;
  }

  if (State == 1) {
    print_menu();
    lcd.setCursor(8, 1);     //(col, row)
    lcd.print(EEPROM.read(2));
    lcd.setCursor(8, 2);     //(col, row)
    lcd.print(EEPROM.read(3));
    lcd.setCursor(8, 3);     //(col, row)
    lcd.print(EEPROM.read(4)/10.0);
    delay(200);
    Dis_Pos = abs(pos) % MenuItems;
    // Serial.println(Dis_Pos);
    lcd.setCursor(0, Dis_Pos);     //(col, row)
    lcd.print(">");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    if (Dis_Pos == 0 && digitalRead(PUSHB) == LOW  )
    {
      pos = 0;
      State = 2;
    }
    else if (Dis_Pos == 1 && digitalRead(PUSHB) == LOW )
    {
      State = 3;
      encoder.setPosition(I_Min);
      pos = I_Min;
    }
    else if (Dis_Pos == 2 && digitalRead(PUSHB) == LOW )
    {
      State = 4;
      encoder.setPosition(I_Max);
      pos = I_Max;
    }
    else if (Dis_Pos == 3 && digitalRead(PUSHB) == LOW)
    {
      State = 5;
      encoder.setPosition(Volt);
      pos = Volt;
    }
  }
  if (State == 2) {
    delay(100);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    Motor = abs(pos) % 2;
    lcd.setCursor(8, 0);     //(col, row)
    if (Motor == 1)
    {
      bool motor_bldc = true;
      EEPROM.update(1, motor_bldc);
      lcd.print("BLDC");
    }
    else
    {
      bool motor_bldc = false;
      EEPROM.update(1, motor_bldc);
      lcd.print("DC  ");

    }
    if (digitalRead(PUSHB) == LOW)
    {
      State = 1;
      //Write value to EEPROM
      Serial.print("the value of eeprom is:");
      Serial.println(EEPROM.read(1));
    }
  }
  if (State == 3) {
    delay(100);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    lcd.setCursor(8, 1);     //(col, row)
    if (pos >= 0 && pos <= I_Max)
    {
      lcd.setCursor(8, 1);     //(col, row)
      lcd.print(pos);
      I_Min = pos;

      if (digitalRead(PUSHB) == LOW)
      {
        I_Min = pos;
        State = 1;
        encoder.setPosition(0);
        //Write value to EEPROM
        EEPROM.update(2, I_Min);
        Serial.print("the value of I_Min is:");
        Serial.println(EEPROM.read(2));
      }
    }
    if (pos < 0) {
      encoder.setPosition(0);
    }

    if (pos > 30) {
      encoder.setPosition(30);
    }
  }
  if (State == 4) {
    delay(100);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    lcd.setCursor(8, 2);     //(col, row)
    if (pos >= I_Min && pos <= 30)
    {
      lcd.setCursor(8, 2);     //(col, row)
      lcd.print(pos);
      I_Max = pos;
      if (digitalRead(PUSHB) == LOW)
      {
        I_Max = pos;
        State = 1;
        encoder.setPosition(0);
        //Write value to EEPROM
        EEPROM.update(3, I_Max);
        Serial.print("the value of I_Max is:");
        Serial.println(EEPROM.read(3));
      }
    }
    if (pos < 0) {
      encoder.setPosition(0);
    }

    if (pos > 30) {
      encoder.setPosition(30);
    }
  }
  
  if (State == 5) {
    delay(100);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    lcd.setCursor(8, 1);     //(col, row)
    if (pos >= 0 && pos <= 255)
    {
      lcd.setCursor(8, 3);     //(col, row)
      Volt = float(pos) / 10;
      lcd.print(Volt);
      Serial.println(Volt);
      if (digitalRead(PUSHB) == LOW)
      {
        Volt = float(pos) / 10;
        Serial.println(Volt);
        State = 0;
        encoder.setPosition(0);
        //Write value to EEPROM
        EEPROM.update(4, Volt * 10);
        Serial.print("the value of Voltage is:");
        Serial.println(EEPROM.read(4));
      }
    }
    if (pos < 0) {
      encoder.setPosition(0);
    }
    if (pos > 255) {
      encoder.setPosition(255);
    }
  }


  if (State == 6) {
    //if (digitalRead(PUSHB) == LOW)
    //{
     // State = 6;
    //}
    lcd.clear();
    lcd.print("Measure Values  ");
    delay(1000);
    lcd.clear();
    lcd.setCursor(1, 0);     //(col, row)
    lcd.print("Motor:");
    lcd.setCursor(1, 1);     //(col, row)
    lcd.print("I_Min:");
    lcd.setCursor(1, 2);     //(col, row)
    lcd.print("I_Max:");
    lcd.setCursor(1, 3);     //(col, row)
    lcd.print("Volt :");
    lcd.setCursor(8, 0);     //(col, row)
    if(EEPROM.read(1)== 1){
      lcd.print("BLDC");
      }
    else{
      lcd.print("DC");
      }
    
    lcd.setCursor(8, 1);     //(col, row)
    lcd.print(EEPROM.read(2));
    lcd.setCursor(8, 2);     //(col, row)
    lcd.print(EEPROM.read(3));
    lcd.setCursor(8, 3);     //(col, row)
    lcd.print(EEPROM.read(4)/10.0);
  }
  //selectmotor();
}

/*void selectmotor(){
  if (motor_bldc){
    lcd.print("measure for BLDC motor");
  }
  else
  {
    lcd.print("measure for DC");
  }
  }*/



// The End
