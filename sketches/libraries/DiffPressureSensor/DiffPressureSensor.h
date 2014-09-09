/*
 * DiffPressureSensor
 * written for MP3V5010DP differential pressure sensor
 * for use with Apitronics Bee
 * utilizing XMEGA ADC Examples by Scott_Schmit
 *
 * Created: 9/8/2014
 *  Author: Colin Dignam
 *
 * MP3V5010 pressure sensor:
 *  Nominal Transfer Value: Vout = VS x (0.09 x P + 0.08)
      ± (Pressure Error x Temp. Factor x 0.09 x VS)
    VS = 3.0 V ± 0.30 Vdc
 * Temp. Factor = 1 for 0 to 85 degC
 */

#ifndef DiffPressureSensor_H
#define DiffPressureSensor_H

#define DEBUG


#include <Sensorhub.h>

#include <avr/io.h>
// These 2 files need to be included in order to read
// the production calibration values from EEPROM
#include <avr/pgmspace.h>
#include <stddef.h>

#define MP3V5010DP_UUID 0x0019
#define MP3V5010DP_LENGTH_OF_DATA 2
#define MP3V5010DP_SCALE 1
#define MP3V5010DP_SHIFT 0

 #define MP3_ADC_ERR_SHIFT (-165)
 #define MP3_ADC_GAIN 2.06


//using Sensorhub.h library for sensor definition
class DiffPressureSensor: public Sensor
{
      public:
        DiffPressureSensor(uint8_t analogPin):Sensor(MP3V5010DP_UUID, MP3V5010DP_LENGTH_OF_DATA, MP3V5010DP_SCALE, MP3V5010DP_SHIFT, false, 1)
        {
          //We are only using Port A pins 1 through 6
          if (analogPin > 6)
          {
            _analogPin = 6;
          }
          else if (analogPin <= 0)
          {
            _analogPin = 1;
          }
          else
          {
            _analogPin = analogPin; 
          }
        }

        void init(void)
        {
          #ifndef ADCA_init
          #define ADCA_init true
          // ADC Clock was disabled initially in sysclk_init()
          // Must re-activate the ADC clock before configuring its registers (we're using ADCA)
          PR.PRPA &= ~0x02; // Clear ADC bit in Power Reduction Port B Register
          // Calibration values are stored at production time
          // Load stored bytes into the calibration registers
          // First NVM read is junk and must be thrown away
          ADCA.CALL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0) );
          ADCA.CALH = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1) );
          ADCA.CALL = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0) );
          ADCA.CALH = ReadCalibrationByte( offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1) );
          #endif
        }

        void getData()
        {                  
          float sample = readSensorADC();
          uint16_t tmp = (sample + MP3V5010DP_SHIFT) * MP3V5010DP_SCALE;
          data[1]=tmp>>8;
          data[0]=tmp;
        }  

        float readSensorADC()
        {
          PORTA.DIRCLR = (1<<_analogPin); //set analog pin to input
          ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;    // 12 bit conversion
          // Set Vref to External reference from AREF pin on PORT A
          ADCA.REFCTRL = ADC_REFSEL1_bm; //(1<<5)
          // Set ADC clock to 125kHz:  CPU_per/64    =>    8MHz/64 = 125kHz
          ADCA.PRESCALER = ADC_PRESCALER2_bm;
          //We're going to use CH1 for Pressure Sensors (channel 0 is used by Onboard Temp sensing)
          ADCA.CH1.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;  // single ended
          //Here we select the pin associated with Port A -> PA1 = 1
          //if _analogPin is out or range 1 to 6 -> default to PA1
          switch (_analogPin)
          {
            case 1:
              ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;
              break;
            case 2:
              ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;
              break;
            case 3:
              ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc;
              break;
            case 4:
              ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc;
              break;
            case 5:
              ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc;
              break;
            case 6:
              ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc;
              break;
            default:
              ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;
              break;
          }
          int16_t sample = 0;    //be careful we don't overflow sample below - max ADC is 2^12 = 4096 each
          //delay(100);
          for(int i=0; i<8; i++)
          {
              ADCA.CH1.CTRL |= ADC_CH_START_bm;
              while(!ADCA.CH1.INTFLAGS);
              delay(5);
              sample+=ADCA.CH1.RES;
            }
          sample >>= 3;
          float resultADC = ((float)(sample + MP3_ADC_ERR_SHIFT)/4096)*MP3_ADC_GAIN; //convert to mV
          #ifdef DEBUG

          Serial.print("P Sensor ");
          Serial.print(_analogPin);
          Serial.print(": ");
          Serial.print(sample);
          Serial.print("  ");
          Serial.print(resultADC);
          Serial.println("V");
          #endif
          return resultADC;
        }

        String getName(){
          return "Diff Pressure Sensor";
        }
        String getUnits(){
          return " mV";
        }

      private:
        unsigned int _result;
        uint8_t _analogPin;
        
        uint8_t ReadCalibrationByte(uint8_t index) {
          uint8_t result;
          NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
          result = pgm_read_byte(index);
          NVM_CMD = NVM_CMD_NO_OPERATION_gc;
          return(result);
        }

          
};

#endif  /* END DiffPressureSensor_H */