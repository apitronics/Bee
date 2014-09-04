//Use PortD pin 0 to turn LED on / off by pull down -- WiredAND setup
//Sep 2


void setup(){
  PORTD.PIN0CTRL = 0b00101111; //OPC = 101 -> WiredAND / ISC = 111 -> INPUT_DISABLE
  PORTD.DIR = (1<<0);              //pin 0 is set to output
  PORTD.OUT &= ~(0x01);              //off   
  delay(1000);
  
  
  
  
}

void loop(){
  PORTD.OUT |= (0x01);   //on
    delay(6000);
  PORTD.OUT &= ~(0x01); 
    delay(6000);    
}
