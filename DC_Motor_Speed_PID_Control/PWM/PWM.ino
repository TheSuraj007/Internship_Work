//Initializing LED Pin
int led_pin = 6;
void setup() {
  //Declaring LED pin as output
  pinMode(led_pin, OUTPUT);
}
void loop() {
  //Fading the LED
  for(int i=0; i<100; i++){
    analogWrite(led_pin, i);
    delay(20);
  }
  for(int i=100; i>0; i--){
    analogWrite(led_pin, i);
    delay(20);
  }
}
