
#include <Sensorhub.h>
#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Bee.h>

String beeAddress = "\"s8\"";
String APN = "internetd.gdsp";

#include <WeatherPlug.h>
#define NUM_SAMPLES 32
DateTime date = DateTime(__DATE__, __TIME__);

OnboardTemperature onboardTemp;
BatteryGauge batteryGauge;
BMP_Temperature BMP_temp;
BMP_Pressure BMP_press;
WindDirection windDir;
WindSpeed windSpeed;
Rainfall rainfall;
SHT2x_temp sht_temp;
SHT2x_RH sht_rh;


#define NUM_SENSORS 9
Sensor * sensor[] = {&onboardTemp, &batteryGauge, &BMP_temp,&BMP_press, &windDir, &windSpeed, &rainfall, &sht_temp, &sht_rh};
//#define NUM_SENSORS 2
//Sensor * sensor[] = {&onboardTemp, &batteryGauge};
Sensorhub sensorhub(sensor,NUM_SENSORS);

const uint8_t poob = 5;

uint8_t data[poob];


#define GSM Serial1
#define GSM_POWER 11
#define VREG_EN 6
#define GSM_RESET 10

// Specify data and clock connections and instantiate SHT1x object
#define SHT_DATA 2
#define SHT_CLOCK 3
#define VOLTAGE 3.3

#define DEBUG
#define XBEE
#define GSM_ENABLE

#define SECS_BETWEEN_SAMPLES 15
#define MINS_BETWEEN_LOGS 5
#define MINS_BETWEEN_UPLOADS 15

bool firstRun;
String answer;

String getData(uint16_t waitTime=5000){
  uint32_t start = millis();
  String output="";
  while(abs(millis()-start)<waitTime){
   while(GSM.available()){
     int incoming = GSM.read();
     //throw out carriage returns and stuff
     if(incoming!=10 && incoming!=13) output+=char(incoming);
   }
  }
  //if(output=="") Serial.println("no data!");
  return output;
}

String getData2(uint16_t waitTime=5000){
  uint32_t start = millis();
  String output="";
  while(abs(millis()-start)<waitTime){
   while(GSM.available()){
     int incoming = GSM.read();
     output+=char(incoming);
   }
  }
  return output;
}


void GSMout(String output){
  for(int i=0; i<output.length(); i++){
    GSM.write(output[i]);
    delay(1);
  }
}

void GSMout2(String output){
  for(int i=0; i<output.length(); i++){
    GSM.write(output[i]);
    Serial.write(output[i]);
    delay(1);
  }
}


bool tryCommand(String command, String expected, uint8_t length, bool verbose=false, uint16_t delayMS=2500){
    GSMout(command);
    answer = getData(delayMS);
    for(int i=0; i<length; i++){
      if(verbose){
      Serial.print("[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(int(answer[i]));
      Serial.print(", ");
      Serial.println(int(expected[i]));
      }  
      
      if(expected[i]!=answer[i])  {
        Serial.flush();
        return false;
      }
          
    }
    return true; 
}

bool trySGACT(){
  String command = "AT#SGACT=1,1\r";
  String expected ="AT#SGACT=1,1#SGACT";
  uint8_t length = 38;
  bool verbose = false;
  uint16_t delayMS = 5000;
  
  GSMout(command);
  answer = getData(delayMS);
  
  for(int i=1; i<length; i++){
    if(verbose){
      Serial.print(i);
      Serial.print(": ");
      Serial.print(answer[i]);
      Serial.println(answer[i+1]);
    }


    if(answer[i]=='O' & answer[i+1]=='K')  {
      return true;
    }
  }
  return false; 
}

void postHTTP(String reqStr){	
        String length = String(reqStr.length());	//Convert strlength into ascii for contentLengh
        GSMout2("POST ");
	GSMout2("/");
	GSMout2(" HTTP/1.1");
	GSMout2("\r\n");
        GSMout2("User-Agent: Bee");
        GSMout2("\r\n");
	GSMout2("HOST:ds.apitronics.com\r\n");
	GSMout2("Content-Type: Application/json\r\n"); // media type
	GSMout2("Connection:keep-alive\r\n");
	GSMout2("Content-Length: ");
	GSMout2(length);
	GSMout2("\r\n\r\n");
	GSMout2(reqStr);
	GSMout2("\r\n\r\n");
        GSMout2("\r\n");
        delay(500);
        answer="";
        while(answer==""){
          answer = getData2(5000);
        }
        Serial.println(answer);
        String datetime = "";
        for (int i=105; i<102+25;i++) datetime+=answer[i];
        clock.getDate();
        if(atoi(&datetime[17])!=clock.hour || atoi(&datetime[20])!=clock.minute){
          Serial.print("Current: ");
          Serial.print(clock.hour);
          Serial.print(":");
          Serial.println(clock.minute);
          clock.setDate(datetime);
          Serial.println("adjusting time");
        }
        
        Serial.println();
        GSMout("+++\r");
        Serial.println(getData2(1000));
        delay(3000);
        Serial.println("Tried to finish sending");   
}



void configure(){
        GSMout("AT&K0\r");		//set flow control off
        Serial.println(getData(4000));
  
        bool ready=false;
        uint16_t attempts = 0;
        
        String command =  "AT+CGDCONT=1,\"IP\",\""+ APN +"\",\"0.0.0.0\"";          
        while(!ready){
          Serial.println("Attempting CGD Configuration...");
          ready = tryCommand(command+"\r", command+"OK", command.length()+4);
          if (++attempts%8==0){
            wakeTelit(); 
            GSMout("AT&K0\r");		//set flow control off
            Serial.println(getData(4000));
          }            
          
          
        }        
        Serial.print("CGD OK:");
        Serial.println(answer);
        
        while(!tryCommand("AT#SCFG=1,1,300,90,600,50\r","AT#SCFG=1,1,300,90,600,50OK", 31)){
          #ifdef DEBUG
          Serial.println("failed socket configuration");
          Serial.println(answer);
          #endif
          
        }
        Serial.println("Socket configuration OK");
}


bool tryConnect(String command, uint32_t delayMS = 2500){
    GSMout(command);
    answer = getData(5000);
    Serial.println("attempting connection");
    
    //make sure the initial command went out properly
    Serial.println(answer);
    Serial.println(answer.length());
    for(int i=0; i<35; i++){      
      if(command[i]!=answer[i])  {
        return false;
      }      
    }
    if(answer.endsWith("CONNECT")) return true;
    uint16_t count = 0;
    
    while(count++<15){
      Serial.println("listening...");
      delay(1000);
      answer = "";
      while(GSM.available()){
        answer+=char(GSM.read());
        
        Serial.println(answer);
        if(answer=="\r\nCONNECT\r\n" ){
          return true;    
        }
        else if(answer=="\r\nERROR\r\n"){
          return false;
        }
        else{
          for(int i=0; i<answer.length();i++){
            Serial.print("[");
            Serial.print(i);
            Serial.print("]: ");
            Serial.print(int(answer[i]));
            Serial.print(' ');
            Serial.println(answer[i]);
          }
        } 
      }
    }
    return false;
}


bool moduleReady(){
  bool moduleStatus = tryCommand("AT\r","ATOK", 4);
  Serial.println(answer);
  return moduleStatus;
  
  //  #ifdef DEBUG
  //  Serial.println("Module not yet ready");
  //  Serial.println(answer);
  //  #endif
  //  return false;
 // }
  //#ifdef DEBUG
  //Serial.println("Module ready");
  //#endif
  //return true;
}

void wakeTelit(){
  pinMode(VREG_EN,OUTPUT);
  digitalWrite(VREG_EN,HIGH);
  
  
  pinMode(GSM_RESET,OUTPUT);
  digitalWrite(GSM_RESET,LOW);
  
  pinMode(GSM_POWER,OUTPUT);
  digitalWrite(GSM_POWER,HIGH);
  
  
  delay(3500);
  
  
  pinMode(GSM_POWER,OUTPUT);
  digitalWrite(GSM_POWER,LOW);

}

void resetTelit(){
  
  pinMode(GSM_RESET,OUTPUT);
  digitalWrite(GSM_RESET,HIGH);

  delay(800);
  
  pinMode(GSM_RESET,LOW);
  digitalWrite(GSM_RESET,INPUT);
  
  delay(3000);

}

void sleepTelit(){
    //put module to sleep
  pinMode(GSM_POWER,OUTPUT);
  digitalWrite(GSM_POWER,HIGH);
  

  
  pinMode(GSM_POWER,OUTPUT);
  digitalWrite(GSM_POWER,LOW);
  
    #ifdef DEBUG
    Serial.println("Put module to sleep");
    #endif
    
    pinMode(VREG_EN,OUTPUT);
    digitalWrite(VREG_EN,LOW);
    #ifdef DEBUG
    Serial.println("Disabling VREG");
    #endif
    
}
//GSM BS END

float resultADC;

void setup(){
  //select GPSM module
  pinMode(9,OUTPUT);
  digitalWrite(9,LOW);
  
  //Expansion LED is set as output
  pinMode(A1,OUTPUT);
  digitalWrite(A1,LOW);
  
  GSM.begin(9600);

  
  #ifdef DEBUG
  Serial.begin(57600);
  delay(1000);
  Serial.println("*****RESET   RESET   RESET*****");
  #endif
  
  
  pinMode(6,OUTPUT);
  digitalWrite(6,HIGH);
  
  //TURN THINGS ON!
  pinMode(5,OUTPUT);
  digitalWrite(5,LOW);
  
  sensorhub.init();

  
  clock.begin(date);
  clock.setDate(date);
  configureSleep();
  clock.enableAlarm1();
  clock.enableAlarm2();
  
  //disable SD-SPI
  PORTE.DIR |= 0b11100000;
  PORTE.OUTCLR |= 0b11100000;
  
  while(!I2C_READY());
  
  clock.enableAlarm1();
  clock.enableAlarm2();
  clock.setAlarm1Delta(0,SECS_BETWEEN_SAMPLES);
  clock.setAlarm2Delta(MINS_BETWEEN_UPLOADS);
  
  //everything from here out will be data dumps
  firstRun=true;
  

  
  #ifdef GSM_ENABLE
  
  wakeTelit();

  
  #ifdef DEBUG
  Serial.println("BEGIN");
  #endif
  pinMode(A0, INPUT);

  #ifdef DEBUG
  Serial.println(getData());
  #else
  getData();
  #endif
  
  configure();
  Serial.println("Done configuring");
  //sleepTelit();
  #endif

}





unsigned long awake=0;

String packet = "";

String sensorReading;

void loop(){
  
  digitalWrite(5,HIGH);
 
  sensorhub.sample(true);

  
  bool buttonPressed =  !clock.triggeredByA2() && !clock.triggeredByA1() ;
  

  
  //if A1 woke us up and its log time OR if its the first run OR if the button has been pushed
  if( ( clock.triggeredByA1() && (clock.minute%MINS_BETWEEN_LOGS==0) ) || firstRun ||  buttonPressed){
    clock.print();
    sensorhub.log(true);
  }
  
  bool enoughPower= true;
  //if we have another power and either A2 woke us up, it's the first run, or the button has been pressed
  if ( enoughPower && (clock.triggeredByA2() || firstRun || buttonPressed) ){    
        #ifdef GSM_ENABLE
        delay(100);
        uint32_t timeGSM;
        if(!firstRun) wakeTelit();          
        getTelitReady();
        uint32_t GSM_attempts = 0;
        bool failed_GSM = false;
        while(!trySGACT()){
          
          //LED FEEDBACK
          pinMode(A1,OUTPUT);
          if(GSM_attempts%2==0)  digitalWrite(A1,HIGH); 
          else                   digitalWrite(A1,LOW); 
          
          
          Serial.print("failed AT#SGACT: ");
          Serial.println(GSM_attempts++);
          Serial.println(answer);
          GSMout("AT#SGACT=1,0\r");
          Serial.println(getData(2500));
          GSMout("\r");
          Serial.println(getData(500));
          if(GSM_attempts>45){
            failed_GSM = true;
            break;
          }
        }
        
        if(failed_GSM){
          Serial.println("Giving up on SGACT");
          digitalWrite(A1,LOW);
          //try again in 15 minutes
          packet+=", ";
          clock.setAlarm2Delta(MINS_BETWEEN_UPLOADS);
          delay(100);
          sleepTelit();
        }
        //otherwise we have a good SGACT connection!
        else  {
          Serial.println("SGACT OK");
          //try to connect 3 times
          for(int i=0; i<3; i++){
            if(tryConnect("AT#SD=1,0,126,\"ds.apitronics.com\",0,0\r")){
              Serial.println("CONNECT");
              String post = "{" + packet + "}";
              post = "{ \"address\": " + beeAddress + ", \"data\":" + post + "}";
              postHTTP(post);
              packet="";
              sleepTelit();
              clock.setAlarm2Delta(MINS_BETWEEN_UPLOADS);
              for(int i=0;i<10;i++){
                  digitalWrite(A1,HIGH);
                  delay(100);
                  digitalWrite(A1,LOW);
                  delay(100);
              }
              break;
            }
            //if connection fails, we'll try again in 15 minutes
            if(i==2){
              Serial.println("failed AT#SD");
              Serial.println(answer);
              digitalWrite(A1,LOW);
              packet+=", ";
              clock.setAlarm2Delta(MINS_BETWEEN_UPLOADS);
              delay(100);
              sleepTelit();
            }
          }
        }

      #else
      Serial.println("packet that would be transmitted: ");
      Serial.println(packet);
      clock.setAlarm2Delta(MINS_BETWEEN_UPLOADS);
      packet="";
      #endif
  }
  //////////////////////////
  //GO TO SLEEP
  //////////////////////////
    
  
    
    
  digitalWrite(5,LOW);
  disableI2C();
  Serial.println();
  #ifdef DEBUG
  delay(100);
  #endif
    
  clock.setAlarm1Delta(0,SECS_BETWEEN_SAMPLES);
  digitalWrite(VREG_EN,LOW);
  sleep();
  delay(500);
  firstRun=false;
}


//if we re enable I2C via the library, we'll waste a lot of memory!
void enableI2C(){
  unsigned long twiSpeed=0; 
  unsigned long twiBaudrate=0;
  
  PORTC.PIN0CTRL = 0x38;    
  PORTC.PIN1CTRL = 0x38;
  
  TWIC.MASTER.CTRLA = TWI_MASTER_INTLVL_LO_gc 
	| TWI_MASTER_RIEN_bm 
	| TWI_MASTER_WIEN_bm 
	| TWI_MASTER_ENABLE_bm;
  twiSpeed=400000UL;
  twiBaudrate=(F_CPU/(twiSpeed<<2))-5UL;
  TWIC.MASTER.BAUD=(uint8_t)twiBaudrate;
  TWIC.MASTER.STATUS=TWI_MASTER_BUSSTATE_IDLE_gc;  
}


void disableI2C(){
  TWIC.MASTER.CTRLB = 0;
  //TWIC.MASTER.BAUD = 0;
  TWIC.MASTER.CTRLA = 0;
  //TWIC.MASTER.STATUS = 0;
}



void softwareReset(){
  CPU_CCP=CCP_IOREG_gc;
  RST.CTRL=RST_SWRST_bm; 
}

void getTelitReady(){
         bool ready=false;
        uint16_t attempts = 0;
        while(!ready){
          Serial.print("in ready loop: ");
          Serial.println(attempts);
          ready = moduleReady();
          if (++attempts%8==0) wakeTelit();            
        } 
}

