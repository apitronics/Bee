#include <Clock.h>
#include <Onboard.h>
#include <XBeePlus.h>
#include <Wire.h>  //we'll be depending on the core's Wire library
#include <Sensorhub.h>
#include <Bee.h>

#include <OneWire.h>
#include <DallasTemperature.h>

String beeAddress = "\"s8\"";

String APN = "M2M.T-Mobile.com";

String SERVER= "54.68.20.10";

#include <WeatherPlug.h>

#define EC5_HUMIDITY_UUID 0x17
#define EC5_LENGTH_OF_DATA 2
#define EC5_TEMP_SCALE 10
#define EC5_TEMP_SHIFT 0

class EC5_SoilHumidity: public Sensor
{
   public:
      EC5_SoilHumidity(uint8_t channel, uint8_t samplePeriod=1):Sensor(EC5_HUMIDITY_UUID, EC5_LENGTH_OF_DATA, EC5_TEMP_SCALE, EC5_TEMP_SHIFT, false,samplePeriod){
        _channel = channel;
      };
        String getName() { return "EC-5 Soil Humidity";}
        String getUnits() {return "mV"; }
        void init() {weatherPlug.init();}
        void getData() { 
          uint16_t sample = weatherPlug._getADC(_channel)/2.0*1.0071108127079073;
          data[1] = sample >> 8;
          data[0] = sample;
        }
        uint16_t _channel;
};

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
EC5_SoilHumidity ec_red(0);
EC5_SoilHumidity ec_green(1);
EC5_SoilHumidity ec_black(2);


#define NUM_SENSORS 12
Sensor * sensor[] = {&batteryGauge, &onboardTemp, &BMP_temp,&BMP_press, &windDir, &sht_temp, &windSpeed, &rainfall, &sht_rh, &ec_red, &ec_green,&ec_black};
Sensorhub sensorhub(sensor,NUM_SENSORS);

#define GSM Serial1
#define GSM_POWER 11
#define VREG_EN 6
#define GSM_RESET 10

#define DEBUG
#define XBEE
#define GSM_ENABLE

#define MINS_BETWEEN_SAMPLES 0
#define SECS_BETWEEN_SAMPLES 30

#define MINS_BETWEEN_LOGS 15
#define MINS_BETWEEN_UPLOADS 180

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


void GSMout(char * output, uint8_t length){
  for(int i=0; i<length; i++){
    GSM.write(output[i]);
    //Serial.write(output[i]);
  }
}

void GSMout(String output){
  //Serial.println(output);
  for(int i=0; i<output.length(); i++){
    GSM.write(output[i]);
  }
}

void GSMout2(char * output, uint8_t length){
  for(int i=0; i<length; i++){
    GSM.write(output[i]);
    Serial.write(output[i]);
    delay(1);
  }
}

void GSMout2(String output){
  for(int i=0; i<output.length(); i++){
    #ifdef GSM_ENABLE
    GSM.write(output[i]);
    #endif
    Serial.write(output[i]);
    delay(1);
  }
}


bool tryCommand(String command, String expected, uint8_t length, bool verbose=false, uint16_t delayMS=2500){
    GSMout(command);
    answer = getData(delayMS);
    
    Serial.println(answer);
    
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
        //Serial.println(answer);
        Serial.flush();
        return false;
      }
          
    }
    return true; 
}

bool trySGACT(){
  //second parameter is retires but we need to wait long enough to get it
  char command[] = "AT#SGACT=1,1\r";
  GSMout(&command[0],14);
  answer = getData(3500);
  Serial.println(answer);
  if(answer.endsWith("#OK") || answer.endsWith("0ATOK") || answer.endsWith("0OK")) return false;
  else if (answer.endsWith("OK")) return true;
  else return false;
}


#define DATA_SIZE 25
#define MAX_SAMPLES 48

uint8_t data_buffer[MAX_SAMPLES][DATA_SIZE];

#define LEN_OF_TIMESTAMP 19
char timeStamps[MAX_SAMPLES][LEN_OF_TIMESTAMP];

uint8_t sampleCount=0;

void initBuffer(){
  for(int i=0;i<MAX_SAMPLES;i++){
    String defaultTime = "00:00:00, 00/00/00";
    defaultTime.toCharArray(timeStamps[i],LEN_OF_TIMESTAMP);
    for(int j=0; j<DATA_SIZE;j++) data_buffer[i][j]=0;
  }
}

void postHeader(String length, String path="/"){
        Serial.println();
        char header1[] = "POST ";
        GSMout2(&header1[0],5);    
        GSMout2(path);
        char header1b[] = " HTTP/1.1\r\nUser-Agent: Bee\r\nHost: ";
        
        GSMout2(&header1b[0],34);
        GSMout2(SERVER);  
  
        
        GSMout2("\r\nContent-Type: Application/json\r\n"); // media type
	GSMout2("Connection: close\r\n");
	GSMout2("Content-Length: ");
	GSMout2(length);

        char header3[]="\r\n\r\n";
	GSMout2(&header3[0], 4);

}

bool postFooter(){
        GSMout2("\r\n\r\n");
        GSMout2("\r\n");
 
        
        delay(500);
        String tmp="";
        #ifdef GSM_ENABLE
        
        uint8_t count=0;
        while(tmp==""){
          Serial.print(".");
          GSMout("\r\n\r\n");
          tmp = getData2(5000);
          if(count++==15){
            GSMout2("+++\r");
            Serial.println(getData2(3000));
            return false;
          } 
          
        }
        Serial.println(tmp);
        
        String datetime="";
        for (int i=105; i<102+25;i++) datetime+=tmp[i];

        clock.getDate();
        
        if(( atoi(&datetime[17])!=clock.hour || atoi(&datetime[20])!=clock.minute) && datetime[19]==':' ){
          Serial.print("Current: ");
          Serial.print(clock.hour);
          Serial.print(":");
          Serial.println(clock.minute);
          clock.setDate(datetime);
          Serial.print("adjusting time: ");
          Serial.println(datetime);
        }
        
        Serial.println();
        GSMout("+++\r");
        /*
        GSMout("AT#SH=2\r");
        Serial.println(getData2(1000));
        GSMout("AT#SGACT=1,0\r");
        Serial.println(getData2(1000));
        */
        delay(3000);
        return datetime[19]==':';
        #else
          return true;
        #endif
        
        
        
}

bool postData(){
        String post = "{\"address\":" + beeAddress + ",\"data\":{";
        //post+="}}";
        
        uint8_t dataPointLength = 20 + DATA_SIZE*2 + 4 -1 ;

        //String length = String(post.length() + dataPointLength*sampleCount + 1);
        String length = String(4+post.length() + dataPointLength*sampleCount);
        postHeader(length);
        GSMout2(post);   
     
        //print out datapoints
        for(int i=0;i<sampleCount;i++){
          char quote[2] = "\"";
          GSMout2(&quote[0],1);
          GSMout2(&timeStamps[i][0],LEN_OF_TIMESTAMP-1);

          //GSMout2(timeStamps[i]);
          GSMout2(&quote[0],1);
          GSMout2(":");
          GSMout2(&quote[0],1);
          for(int j=0; j<DATA_SIZE;j++){
                  String output = String(data_buffer[i][j],HEX);
                  if(output.length()<2 && j!=6){
                      output='0'+output;
                  }
                  GSMout2(output);                  
          }
          GSMout2(&quote[0],1);
          if(i!=sampleCount-1) GSMout2("\,");
        }
        
        GSMout2("}}");
        return postFooter();
}

void postIDs(){	
        String post = "{ \"address\": " + beeAddress + ", \"sensors\": \[";
        
        String length = String(post.length() + NUM_SENSORS*9 +1);
        postHeader(length, "/egg/new");
        
        GSMout2(post);
        
        for(int i=0;i<NUM_SENSORS;i++){
          GSMout2("\"");
          String out = "0x";
          if(sensorhub.ids[i*2]<10) out+="0";
          out+=String(sensorhub.ids[i*2],HEX);
          if(sensorhub.ids[i*2+1]<10) out+="0";
          out+=String(sensorhub.ids[i*2+1],HEX);
          GSMout2(out);
          GSMout2("\"");
          if(i!=NUM_SENSORS-1) GSMout2(",");
        }
        GSMout2("]}");
	postFooter();
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
    delay(2500);
    answer = getData(5000);
    Serial.println("attempting connection");
    
    //make sure the initial command went out properly
    Serial.println(answer);


    for(int i=0; i<command.length()-1; i++){      
      if(command[i]!=answer[i])  {
        return false;
      }      
    }
    
    //if a really quick connection or failure happened, we already caught CONNECT or ERROR
    if(answer.endsWith("CONNECT")){
      return true;
    }
    else if(answer.endsWith("ERROR")){
      return false;
    }
    uint16_t count = 0;
    
    
    while(count++<10){
      Serial.println("listening...");
      answer = getData(2500);
      while(GSM.available()){
        answer+=char(GSM.read());
        Serial.println(answer);
        if(answer=="CONNECT"){
          return true;    
        }
        else if(answer=="ERROR"){
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
    Serial.println();
    
    //GSMout("AT#SH=2\r");
    //Serial.println(getData2(1000));
    return false;
}


bool moduleReady(){
  bool moduleStatus = tryCommand("AT\r","ATOK", 4);
  return moduleStatus;
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


void setup(){
  initBuffer();
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
  clock.setAlarm1Delta(MINS_BETWEEN_SAMPLES, SECS_BETWEEN_SAMPLES);
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
  
  //post IDs to server
  //if(GSM_init()){
  //  connectTo(SERVER,"125");
  //  postIDs();
  //}
  
  #endif


}

unsigned long awake=0;

bool GSM_init(){
  getTelitReady();
  Serial.print("Attempting SGACT");
  uint32_t GSM_attempts = 0;
        while(!trySGACT()){
          
          //LED FEEDBACK
          pinMode(A1,OUTPUT);
          if(GSM_attempts%2==0)  digitalWrite(A1,HIGH); 
          else                   digitalWrite(A1,LOW); 
          
          Serial.print(".");
          GSM_attempts++;
         
          GSMout("AT#SGACT=1,0\r");
          getData(2500);
          GSMout("\r");
          getData(500);
          if(GSM_attempts>45){
            Serial.println();
            return false;
          }
        }  
  Serial.println();
  return true;
}

bool connectTo(String server, String port){
  Serial.println("SGACT OK");
  //try to connect 3 times
  for(int i=0; i<3; i++){
    if(tryConnect("AT#SD=1,0,"+port+",\""+server+"\",0,0\r")){
      Serial.println("CONNECT");
      return true;
    }
  }
  return false;
}

uint8_t lastLogMin = 61;

void loop(){
  clock.getDate();
  
  Serial.print(".");
  digitalWrite(5,HIGH);
  sensorhub.sample(false);  
  bool buttonPressed =  !clock.triggeredByA2() && !clock.triggeredByA1() ;


  //if A1 woke us up and its log time OR if its the first run OR if the button has been pushed
  if( ( clock.triggeredByA1() && (clock.minute%MINS_BETWEEN_LOGS==0 && lastLogMin!=clock.minute) ) || firstRun ||  buttonPressed){
    clock.timestamp().toCharArray(timeStamps[sampleCount],LEN_OF_TIMESTAMP);
    Serial.println();
    Serial.println(timeStamps[sampleCount]);
    sensorhub.log(true);
    for(int i=0;i<sensorhub.getDataSize();i++){
      data_buffer[sampleCount][i]=sensorhub.data[i];
    }
    sampleCount++;
    lastLogMin=clock.minute;
  }
  
  bool enoughPower= true;
  //if we have another power and either A2 woke us up, it's the first run, or the button has been pressed
  if ( enoughPower && (clock.triggeredByA2() || firstRun || buttonPressed) ){    
        #ifdef GSM_ENABLE
        delay(100);
        uint32_t timeGSM;
        if(!firstRun) wakeTelit();                  
        
        if(!GSM_init()){
          Serial.println("Giving up on SGACT");
          digitalWrite(A1,LOW);
          //try again in 15 minutes
          clock.setAlarm2Delta(MINS_BETWEEN_LOGS);
          clock.setAlarm1Delta(MINS_BETWEEN_SAMPLES,SECS_BETWEEN_SAMPLES);
          sleepTelit();
        }
        //otherwise we have a good SGACT connection!
        else  {
          if(connectTo(SERVER, "126")){
            //if succesful 
            if(postData()){
              sampleCount=0;           //we can set sampleCount back to 0
              for(int i=0;i<10;i++){   //do a light show to celebrate
                digitalWrite(A1,HIGH);
                delay(100);
                digitalWrite(A1,LOW);
                delay(100);
              }
              sleepTelit();
              clock.setAlarm2Delta(MINS_BETWEEN_UPLOADS);
              clock.setAlarm1Delta(MINS_BETWEEN_SAMPLES,SECS_BETWEEN_SAMPLES);
            }
            else{  //if failed
              digitalWrite(A1,LOW);
              clock.setAlarm2Delta(MINS_BETWEEN_LOGS);  //we'll try again next time we log
              clock.setAlarm1Delta(MINS_BETWEEN_SAMPLES,SECS_BETWEEN_SAMPLES);
              sleepTelit();
            }
            
            
          }
          else{
              Serial.println("failed AT#SD");
              Serial.println(answer);
              digitalWrite(A1,LOW);
              clock.setAlarm2Delta(MINS_BETWEEN_LOGS);
              clock.setAlarm1Delta(MINS_BETWEEN_SAMPLES,SECS_BETWEEN_SAMPLES);
              sleepTelit();
          }
        }
      #else
      postData();
      clock.setAlarm2Delta(MINS_BETWEEN_UPLOADS);
      #endif
  }
  //////////////////////////
  //GO TO SLEEP
  //////////////////////////

  digitalWrite(5,LOW);
  disableI2C();
  #ifdef DEBUG
  delay(100);
  #endif
    
  clock.setAlarm1Delta(MINS_BETWEEN_SAMPLES,SECS_BETWEEN_SAMPLES);
  digitalWrite(VREG_EN,LOW);
  weatherPlug.sleep();
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
        Serial.print("getting module ready");
        while(!ready){
          Serial.print(".");
          ready = moduleReady();
          if (++attempts%8==0) wakeTelit();            
        }
        Serial.println();
}
