#define BLYNK_TEMPLATE_ID "TMPL62O09iZld"
#define BLYNK_TEMPLATE_NAME "Greenhouse System 02v"
#define BLYNK_AUTH_TOKEN "OqeyMaPL2VRWv5SLSPpimU5gv_BlDJhT"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>

// ================= WIFI =================
char ssid[] = "Dhananjana iPhone 12";
char pass[] = "buddhi2222";

// ================= SERIAL TO MEGA =================
// ESP D6 = RX, D5 = TX
SoftwareSerial MegaSerial(D6, D5);

BlynkTimer timer;

// ================= SENSOR VALUES =================
float t1 = 0;
float t2 = 0;
int soil = 0;
int motion = 0;

String incomingData = "";

// ================= READ DATA FROM MEGA =================
void readMegaData() {

  while (MegaSerial.available()) {

    char c = MegaSerial.read();

    if (c == '\n') {

      incomingData.trim();

      Serial.print("RAW: ");
      Serial.println(incomingData);

      int c1 = incomingData.indexOf(',');
      int c2 = incomingData.indexOf(',', c1 + 1);
      int c3 = incomingData.indexOf(',', c2 + 1);

      if (c1 > 0 && c2 > 0 && c3 > 0) {

        t1 = incomingData.substring(0, c1).toFloat();
        t2 = incomingData.substring(c1 + 1, c2).toFloat();
        soil = incomingData.substring(c2 + 1, c3).toInt();
        motion = incomingData.substring(c3 + 1).toInt();

        Blynk.virtualWrite(V0, t1);
        Blynk.virtualWrite(V1, t2);
        Blynk.virtualWrite(V2, soil);
        Blynk.virtualWrite(V3, motion);

        Serial.println("Data sent to Blynk");
      }

      incomingData = "";
    }
    else {
      incomingData += c;
    }
  }
}

// ================= FAN =================
BLYNK_WRITE(V4) {

  int state = param.asInt();

  if (state) {
    MegaSerial.println("FAN_ON");
    Serial.println("SEND -> FAN_ON");
  } else {
    MegaSerial.println("FAN_OFF");
    Serial.println("SEND -> FAN_OFF");
  }
}

// ================= PUMP =================
BLYNK_WRITE(V5) {

  int state = param.asInt();

  if (state) {
    MegaSerial.println("PUMP_ON");
    Serial.println("SEND -> PUMP_ON");
  } else {
    MegaSerial.println("PUMP_OFF");
    Serial.println("SEND -> PUMP_OFF");
  }
}

// ================= LIGHT =================
BLYNK_WRITE(V6) {

  int state = param.asInt();

  if (state) {
    MegaSerial.println("LIGHT_ON");
    Serial.println("SEND -> LIGHT_ON");
  } else {
    MegaSerial.println("LIGHT_OFF");
    Serial.println("SEND -> LIGHT_OFF");
  }
}

// ================= BLYNK CONNECTED =================
BLYNK_CONNECTED() {

  Serial.println("Blynk Connected");

  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
}

// ================= WIFI CHECK =================
void checkConnection() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi Lost");

    WiFi.begin(ssid, pass);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 10000) {

      delay(500);
      Serial.print(".");
    }

    Serial.println();
  }
}

// ================= SETUP =================
void setup() {

  Serial.begin(9600);

  MegaSerial.begin(9600);

  Serial.println();
  Serial.println("ESP8266 STARTING");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, readMegaData);
  timer.setInterval(10000L, checkConnection);

  Serial.println("ESP READY");
}

// ================= LOOP =================
void loop() {

  Blynk.run();
  timer.run();
}