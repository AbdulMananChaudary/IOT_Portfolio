
// Include Sensor Libaries
#include <ADSWeather.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <GP2YDustSensor.h>

//include Serial Connection Libaries
#include <Wire.h>
#include <SPI.h>



#define LED_PIN   LED_BUILTIN

// Defines the Sensor Pins
#define ANEMOMETER_PIN 4
#define VANE_PIN A2
#define RAIN_PIN 1

// Interval for calculating Sensor values in ms
#define CALC_INTERVAL 1000




//Defines BME680

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme; // I2C

unsigned long nextCalc;
unsigned long timer;


const uint8_t SHARP_LED_PIN = 7;   // Sharp Dust/particle sensor Led Pin
const uint8_t SHARP_VO_PIN = A1;    // Sharp Dust/particle analog out pin used for reading

GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1014AU0F, SHARP_LED_PIN, SHARP_VO_PIN);

ADSWeather ws1(RAIN_PIN, VANE_PIN, ANEMOMETER_PIN); //This should configure all pins correctly



void setup_GP2Y_Dustsensor() {

  dustSensor.setBaseline(0.0); // set no dust voltage according to your own experiments
  dustSensor.setCalibrationFactor(1); // calibrate against precision instrument  WTF????????????
  dustSensor.begin();
}


void setup_BME680() {
  delay(10);
  while (!Serial);
  Serial.println(F("BME680 config"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  Serial.println("BME config ready") ;
}


void setup() {
  Serial.begin(115200);
  delay(1000) ;

  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), ws1.countRain, FALLING); //ws1.countRain is the ISR for the rain gauge.
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), ws1.countAnemometer, FALLING); //ws1.countAnemometer is the ISR for the anemometer.
  nextCalc = millis() + CALC_INTERVAL;


 
  setup_BME680() ;
  setup_GP2Y_Dustsensor() ;
 
}

void loop() {

  timer = millis();

  int rainAmmount;
  long windSpeed;
  long windDirection;
  int windGust;
  float temp;
  float humid;
  float altitude ;
  float voltage = 0 ;
  float Dust_average  ;
  float Dust_Density ;
 



  ws1.update(); //Call this every cycle in your main loop to update all the sensor values

  if (timer > nextCalc)
  {

    nextCalc = timer + CALC_INTERVAL;
    rainAmmount = ws1.getRain();
    windSpeed = ws1.getWindSpeed();
    windD rection = ws1.getWindDirection();
    windGust = ws1.getWindGust();
    temp = bme.temperature - 2.0;
    humid = bme.humidity;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    Dust_average = dustSensor.getDustDensity() ;
    Dust_Density = dustSensor.getRunningAverage() ;



    //     windSpeed / 10 will give the interger component of the wind speed
    //     windSpeed % 10 will give the fractional component of the wind speed
    Serial.print("Wind speed: ");
    Serial.print(windSpeed / 10);
    Serial.print('.');
    Serial.print(windSpeed % 10);
    Serial.print(" ");

    Serial.print("Gusting at: ");
    Serial.print(windGust / 10);
    Serial.print('.');
    Serial.print(windGust % 10);
    Serial.println("");

    Serial.print("Wind Direction: ");
    Serial.print(windDirection);
    Serial.println("");

    Serial.print("Total Rain: ");
    Serial.println(rainAmmount) ;

    Serial.print(F("Temperature = "));
    Serial.print(bme.temperature);
    Serial.println(F(" *C"));

    Serial.print(F("Humidity = "));
    Serial.print(bme.humidity);
    Serial.println(F(" %"));

    Serial.print(F("Approx. Altitude = "));
    Serial.print(altitude);
    Serial.println(F(" m"));

    Serial.print(F("Battery Voltage = "));
    Serial.print(voltage);
    Serial.println(F(" V"));

    Serial.print("Dust density: ");
    Serial.print(dustSensor.getDustDensity());
    Serial.print(" ug/m3; Running average: ");
    Serial.print(dustSensor.getRunningAverage());
    Serial.println(" ug/m3");

    Serial.println("");
  }



  }
