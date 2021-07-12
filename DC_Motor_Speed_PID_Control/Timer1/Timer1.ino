/* Example code with timer intyerrutp that will create an interruption each 
 *  1000ms using timer1 and prescalar of 256.
Calculations (for 500ms): 
  System clock 16 Mhz and Prescalar 256;
  Timer 1 speed = 16Mhz/256 = 62.5 Khz    
  Pulse time = 1/62.5 Khz =  16us  
  Count up to = 1000ms / 16us = 62500 (so this is the value the OCR register should have)*/  
bool LED_STATE = true;

void setup() {
  pinMode(13, OUTPUT);        //Set the pin to be OUTPUT
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
  sei();                     //Enable back the interrupts
}

void loop() {
  // put your main code here, to run repeatedly:
}

//With the settings above, this IRS will trigger each 1000ms.
ISR(TIMER1_COMPA_vect){
  TCNT1  = 0;                  //First, set the timer back to 0 so it resets for next interrupt
  LED_STATE = !LED_STATE;      //Invert LED state
  digitalWrite(13,LED_STATE);  //Write new state to the LED on pin D5
}
