//blink LED at D13

void setup() {

  PORTC.DIRSET = PIN7_bm; //D13 as output
  PORTC.OUTSET = PIN7_bm; //turn on LED
  delay(2000);
  PORTC.OUTCLR = PIN7_bm; //turn off LED
       
}

void loop(){
        PORTC.OUTSET = PIN7_bm; //turn on LED
        delay(1000);
        PORTC.OUTCLR = PIN7_bm; //turn off LED
        delay(1000);
  }
