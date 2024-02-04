
#include <BlynkSimpleStream.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).

#define BLYNK_TEMPLATE_ID "TMPLvKzTF_nx"
#define BLYNK_DEVICE_NAME "Proteus"
#define BLYNK_AUTH_TOKEN "y8LwpjuH1TOMHw0x8oEw-ILfjK-TdPw8"
char auth[] = "y8LwpjuH1TOMHw0x8oEw-ILfjK-TdPw8";




void setup()
{
 
  // Blynk will work through Serial
  // Do not read or write this serial manually in your sketch
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
}

void loop()
{
  Blynk.run();
}