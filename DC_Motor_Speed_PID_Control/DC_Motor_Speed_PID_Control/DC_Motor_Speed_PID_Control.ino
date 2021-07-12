
const byte interruptPin = 2;
volatile byte state = LOW;
unsigned int rpm;
volatile byte rpmcount;

void setup() {
  Serial.begin(9600);

  //====================== Interrupt =================================
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), rpmCal, FALLING);
  //==================================================================

  //================================ Timer1 =============================
  cli();                      //stop interrupts for till we make the settings
  /*1. First we reset the control register to amke sure we start with everything disabled.*/
  TCCR1A = 0;                 // Reset entire TCCR1A to 0
  TCCR1B = 0;                 // Reset entire TCCR1B to 0

  /*2. We set the prescalar to the desired value by changing the CS10 CS12 and CS12 bits. */
  TCCR1B |= B00000100;        //Set CS12 to 1 so we get prescalar 256

  /*3. We enable compare match mode on register A*/
  TIMSK1 |= B00000010;        //Set OCIE1A to 1 so we enable compare match A

  /*4. Set the value of register A to 62500*/
  OCR1A = 62500;             //Finally we set compare register A to this value for 1-Sec Delay
  sei();
  //==========================================================================



}

void loop() {
  
}

void rpmCal() {
  //Each rotation, this interrupt function is run twice, so take that into consideration for
  //calculating RPM
  //Update count
  rpmcount++;
}

//============================ Timer1 ======================================
//With the settings above, this IRS will trigger each 1000ms.
ISR(TIMER1_COMPA_vect) {
  TCNT1  = 0;                  //First, set the timer back to 0 so it resets for next interrupt


  
Serial.print("RPM: ");
Serial.println((rpmcount*60)/7);
rpmcount=0;
  //detachInterrupt(0);
}
//==========================================================================
