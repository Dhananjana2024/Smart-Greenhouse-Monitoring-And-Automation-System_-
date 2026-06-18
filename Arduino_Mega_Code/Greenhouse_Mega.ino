#include <Keypad.h>
#include <Servo.h>
#include <DHT.h>

// ================= PINS =================
#define SERVO_PIN 9
#define SOIL_PIN A0

#define DHTPIN1 31
#define DHTPIN2 33

#define PIR1 34
#define PIR2 35

#define LED_LIGHT 36
#define BUZZER 37

#define RELAY_PUMP 30
#define RELAY_FAN 32

#define DHTTYPE DHT11

Servo door;

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

// ================= KEYPAD =================
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {22,23,24,25};
byte colPins[COLS] = {26,27,28,29};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ================= PASSWORD =================
String password = "1234";
String input = "";

// ================= TIMERS =================
unsigned long lastSensorTime = 0;
unsigned long lastMotionTime = 0;

// ================= MANUAL FLAGS =================
bool manualFan = false;
bool manualPump = false;
bool manualLight = false;

// ================= SETUP =================
void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);   // ESP8266

  door.attach(SERVO_PIN);
  door.write(0);

  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);

  pinMode(PIR1, INPUT);
  pinMode(PIR2, INPUT);

  pinMode(LED_LIGHT, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RELAY_PUMP, HIGH);
  digitalWrite(RELAY_FAN, HIGH);
  digitalWrite(LED_LIGHT, LOW);
  digitalWrite(BUZZER, LOW);

  dht1.begin();
  dht2.begin();

  Serial.println("GREENHOUSE READY");
}

// ================= DOOR LOCK =================
void checkDoor() {

  char key = keypad.getKey();

  if (key) {

    Serial.print("KEY: ");
    Serial.println(key);

    if (key == '#') {

      if (input == password) {

        Serial.println("DOOR OPEN");

        door.write(90);
        delay(5000);
        door.write(0);

        Serial.println("DOOR CLOSE");
      }
      else {

        Serial.println("WRONG PASSWORD");

        for (int i = 0; i < 5; i++) {
          digitalWrite(BUZZER, HIGH);
          delay(200);
          digitalWrite(BUZZER, LOW);
          delay(200);
        }
      }

      input = "";
    }

    else if (key == '*') {
      input = "";
    }

    else {
      input += key;
    }
  }
}

// ================= RECEIVE COMMANDS =================
void readESPCommands() {

  if (Serial1.available()) {

    String cmd = Serial1.readStringUntil('\n');
    cmd.trim();

    Serial.print("RECEIVED: ");
    Serial.println(cmd);

    if (cmd == "FAN_ON") {

      manualFan = true;
      digitalWrite(RELAY_FAN, LOW);

      Serial.println("FAN TURNED ON");
    }

    else if (cmd == "FAN_OFF") {

      manualFan = false;
      digitalWrite(RELAY_FAN, HIGH);

      Serial.println("FAN TURNED OFF");
    }

    else if (cmd == "PUMP_ON") {

      manualPump = true;
      digitalWrite(RELAY_PUMP, LOW);

      Serial.println("PUMP TURNED ON");
    }

    else if (cmd == "PUMP_OFF") {

      manualPump = false;
      digitalWrite(RELAY_PUMP, HIGH);

      Serial.println("PUMP TURNED OFF");
    }

    else if (cmd == "LIGHT_ON") {

      manualLight = true;
      digitalWrite(LED_LIGHT, HIGH);

      Serial.println("LIGHT TURNED ON");
    }

    else if (cmd == "LIGHT_OFF") {

      manualLight = false;
      digitalWrite(LED_LIGHT, LOW);

      Serial.println("LIGHT TURNED OFF");
    }
  }
}

// ================= SENSOR SYSTEM =================
void checkSensors() {

  int soil = analogRead(SOIL_PIN);

  float t1 = dht1.readTemperature();
  float t2 = dht2.readTemperature();

  int motion = (digitalRead(PIR1) || digitalRead(PIR2)) ? 1 : 0;

  if (isnan(t1)) t1 = 0;
  if (isnan(t2)) t2 = 0;

  // ================= AUTO PUMP =================
  if (!manualPump) {

    if (soil > 700) {
      digitalWrite(RELAY_PUMP, LOW);
    }
    else {
      digitalWrite(RELAY_PUMP, HIGH);
    }
  }

  // ================= AUTO FAN =================
  if (!manualFan) {

    if (t1 > 32 || t2 > 32) {
      digitalWrite(RELAY_FAN, LOW);
    }
    else {
      digitalWrite(RELAY_FAN, HIGH);
    }
  }

  // ================= AUTO LIGHT =================
  if (!manualLight) {

    if (motion == 1) {

      digitalWrite(LED_LIGHT, HIGH);
      lastMotionTime = millis();
    }

    if (millis() - lastMotionTime > 30000) {

      digitalWrite(LED_LIGHT, LOW);
    }
  }

  // ================= DEBUG =================
  Serial.print("T1=");
  Serial.print(t1);

  Serial.print(" T2=");
  Serial.print(t2);

  Serial.print(" Soil=");
  Serial.print(soil);

  Serial.print(" Motion=");
  Serial.println(motion);

  // ================= SEND TO ESP =================
  Serial1.print(t1);
  Serial1.print(",");

  Serial1.print(t2);
  Serial1.print(",");

  Serial1.print(soil);
  Serial1.print(",");

  Serial1.println(motion);
}

// ================= LOOP =================
void loop() {

  checkDoor();

  readESPCommands();

  if (millis() - lastSensorTime >= 1000) {

    lastSensorTime = millis();

    checkSensors();
  }
}