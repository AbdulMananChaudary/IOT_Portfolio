#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ADSWeather.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <GP2YDustSensor.h>
#include <Wire.h>
#include <SPI.h>

const String MQTT_HOST("530447c8a7f441a084ed99f501538261.s1.eu.hivemq.cloud");
const int MQTT_PORT = 8883;
const String MQTT_CLIENT_ID("test_hive_client_1");
const String MQTT_USER("test");
const String MQTT_PWD("test");

String wifiSid = "wifi63";
String wifiPwd = "M489YP69664MV4";

// The CA certificate used to sign the HiveMQ server certificate
static const char caCert[] PROGMEM = R"EOF(
  // ... (same as before)
)EOF";

X509List caCertList(caCert);

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

#define MQTT_TIME_UPDATE_PERIOD 5000
unsigned long nextTimeUpdate = 0;

#define LED_PIN LED_BUILTIN

#define ANEMOMETER_PIN 4
#define VANE_PIN A2
#define RAIN_PIN 1
#define CALC_INTERVAL 1000

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME680 bme; // I2C

unsigned long nextCalc;
unsigned long timer;

const uint8_t SHARP_LED_PIN = 7;
const uint8_t SHARP_VO_PIN = A1;
GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1014AU0F, SHARP_LED_PIN, SHARP_VO_PIN);

ADSWeather ws1(RAIN_PIN, VANE_PIN, ANEMOMETER_PIN);

void mqttCallback(const char *topic, byte *payload, unsigned int length)
{
  char value[length + 1];
  memcpy(value, payload, length);
  value[length] = '\0';

  Serial.print("MQTT update: ");
  Serial.print(topic);
  Serial.print(":");
  Serial.println(value);
}

void connectToWifi()
{
  WiFi.mode(WIFI_STA);

  WiFi.disconnect();
  WiFi.begin(wifiSid.c_str(), wifiPwd.c_str());

  Serial.println("");
  Serial.print("Connecting to " + wifiSid + " - ");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("");
  Serial.print("Connected, IP Address = ");
  Serial.println(WiFi.localIP());
}

void checkWifi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectToWifi();
  }
}

void checkMQTT()
{
  if (!mqttClient.connected())
  {
    String details = MQTT_HOST + "/";
    details += MQTT_PORT;

    Serial.println("Connecting to MQTT server: " + details);

    mqttClient.setServer(MQTT_HOST.c_str(), MQTT_PORT);

    while (!mqttClient.connected())
    {
      Serial.println(".");

      timeClient.update();
      time_t now = (time_t)timeClient.getEpochTime();
      wifiClient.setX509Time(now);

      int ret = mqttClient.connect(MQTT_CLIENT_ID.c_str(), MQTT_USER.c_str(), MQTT_PWD.c_str());

      Serial.print("MQTT Connect returned: ");
      Serial.println(ret);

      if (!mqttClient.connected())
        delay(5000);
    }

    Serial.println("Connected to MQTT");

    mqttClient.setCallback(mqttCallback);
    mqttClient.subscribe("test/value1");
  }
}

void setup_GP2Y_Dustsensor()
{
  dustSensor.setBaseline(0.0);
  dustSensor.setCalibrationFactor(1);
  dustSensor.begin();
}

void setup_BME680()
{
  delay(10);
  while (!Serial)
    ;
  Serial.println(F("BME680 config"));

  if (!bme.begin())
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1)
      ;
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);
  Serial.println("BME config ready");
}

void setup()
{
  Serial.begin(9600);
  delay(1000);

  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), ws1.countRain, FALLING);
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), ws1.countAnemometer, FALLING);
  nextCalc = millis() + CALC_INTERVAL;

  setup_BME680();
  setup_GP2Y_Dustsensor();

  Serial.println("Starting");

  wifiClient.setTrustAnchors(&caCertList);

  connectToWifi();
  checkMQTT();

  nextTimeUpdate = millis() + MQTT_TIME_UPDATE_PERIOD;
}

void loop()
{
  checkWifi();
  checkMQTT();
  mqttClient.loop();

  timer = millis();

  int rainAmmount;
  long windSpeed;
  long windDirection;
  int windGust;
  float temp;
  float humid;
  float altitude;
  float voltage = 0;
  float Dust_average;
  float Dust_Density;

  ws1.update();

  if (timer > nextCalc)
  {
    nextCalc = timer + CALC_INTERVAL;
    rainAmmount = ws1.getRain();
    windSpeed = ws1.getWindSpeed();
    windDirection = ws1.getWindDirection();
    windGust = ws1.getWindGust();
    temp = bme.temperature - 2.0;
    humid = bme.humidity;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    Dust_average = dustSensor.getDustDensity();
    Dust_Density = dustSensor.getRunningAverage();

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
    Serial.println(rainAmmount);

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
  
  if (millis() > nextTimeUpdate)
  {
    timeClient.update();
    const char *payload = timeClient.getFormattedTime().c_str();
    mqttClient.publish("test/current-time", payload, true);
    nextTimeUpdate = millis() + MQTT_TIME_UPDATE_PERIOD;
  }

  delay(50);
}
