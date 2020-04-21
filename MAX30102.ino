
#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"
#include "spo2_algorithm.h"


MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

int8_t idx =0;
void setup()
{
Serial.begin(115200);
Serial.println("Initializing...");

// Initialize sensor
if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD,MAX30105_ADDRESS)) //Use default I2C port, 400kHz speed
{
  Serial.println("MAX30105 was not found. Please check wiring/power. ");
}
Serial.printf("PartID = %d \n",particleSensor.readPartID());

Serial.println("Place your index finger on the sensor with steady pressure.");

particleSensor.setup(); //Configure sensor with default settings
particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
particleSensor.enableDIETEMPRDY();
}

void loop()
{
   long irValue = particleSensor.getIR();
      idx++ ;

   /*
   redBuffer[idx] = particleSensor.getRed();
   irBuffer[idx] =irValue ; 
   
   if (idx > 100)
   { 
      maxim_heart_rate_and_oxygen_saturation(irBuffer, 100,
                                         redBuffer, &spo2, &validSPO2,
                                         &heartRate, &validHeartRate);
      idx = 0 ; 
   }
   */
  float temperatureC = particleSensor.readTemperature(); 


   if (checkForBeat(irValue) == true)
   {
      //We sensed a beat!
      long delta = millis() - lastBeat;
     lastBeat = millis();

     beatsPerMinute = 60 / (delta / 1000.0);

     if (beatsPerMinute < 255 && beatsPerMinute > 20)
     {
       rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
       rateSpot %= RATE_SIZE; //Wrap variable

       //Take average of readings
       beatAvg = 0;
       for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
       beatAvg /= RATE_SIZE;
    }
}

//Serial.print("IR=");
//Serial.print(irValue);
//Serial.print(", BPM=");
//Serial.print(beatsPerMinute);
if (idx == 100)
{
  Serial.printf("Avg BPM:%d\n",beatAvg);
  Serial.printf("Temp: %f\n",temperatureC);
  idx = 0 ;
}
/*
if (validSPO2)
{
  Serial.printf("SPO2 =%d \n",spo2);
}
*/
if (irValue < 50000)
{
  Serial.print(" No finger?");

  Serial.println();
}
}
