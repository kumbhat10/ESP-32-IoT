//#define WIFI_SSID "4207 Hyperoptic Fibre 2.4Ghz"
//#define WIFI_PASSWORD "SA6cL5AEDYmz"
#define WIFI_SSID "Dushyant's Galaxy S21 Ultra 5G"
#define WIFI_PASSWORD "qwerty12"
//#define WIFI_SSID "Pixel6"
//#define WIFI_PASSWORD "qwerty12"
void WiFiSetup() {
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.print("........");
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    blinkLED1();
  }
  buzOnce();
  digitalWrite(LED_BUILTIN, LOW); ledState = true;
  Serial.println();
  Serial.print("Connected to " + String(WIFI_SSID) + " with IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(" Strength : ");
  Serial.print(WiFi.RSSI());
  Serial.println();
}

String parentPath = "Robot/Control/data";
String childPath[8] = {"/AT", "/lx", "/ly", "/rx", "/ry", "/tr", "/Buzz", "/bs"};

void FirebaseInit() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);//  Firebase.setMaxErrorQueue(fbdo, 3);
  Serial.println("Connecting to Firebase..."); //while(Firebase
  Serial.println();
  while (!Firebase.ready()) {
    blinkLED1();
  }
  buzOnce();
  digitalWrite(LED_BUILTIN, LOW); ledState = true;
  Serial.println("Sending Cloud message notification...");
  sendMessage();
  buzOnce();
  Serial.println("Begin streaming");
  //  if (!Firebase.beginStream(stream, "/Control/data"))
  //    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
  //  Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);
  if (!Firebase.beginMultiPathStream(stream, parentPath))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
  Firebase.setMultiPathStreamCallback(stream, streamCallback, streamTimeoutCallback);
}
bool buzzed = false;
bool bs = 1;
void streamCallback(MultiPathStreamData stream)
{
  ledStateBlinkCount = 3;
  if (buzzed)buzStateBlinkCount = 2;
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);
  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i])) {
      switch (i) {
        case 0:
          //          Serial.println(stream.value.c_str());
          Serial7600.println(stream.value.c_str());
          break;
        case 1:
          lx = stream.value.toInt();
          break;
        case 2:
          ly = stream.value.toInt();
          break;
        case 3:
          rx = stream.value.toInt();
          break;
        case 4:
          ry = stream.value.toInt();
          break;
        case 5:
          tr = stream.value.toInt();
          break;
        case 6:
          if (buzzed)buzStateBlinkCount = 6;
          buzzed = true;
          break;
        case 7:
          bs = stream.value.toInt();
          break;
      }
    }
  }
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("Stream timeout, resuming...\n");
  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void sendMessage()
{
  fbdo1.fcm.begin(FIREBASE_FCM_SERVER_KEY);    
  fbdo1.fcm.setTopic("Alert");    
  fbdo1.fcm.setPriority("high");    
  fbdo1.fcm.setTimeToLive(1000);
  fbdo1.fcm.setNotifyMessage("IoT Robot is Online! ", "Robot was restarted");
  FirebaseJson json1;
  json1.add("D", "robot" );
  fbdo1.fcm.setDataMessage(json1);
  Serial.printf("Send cloud message... %s\n", 
  Firebase.sendTopic(fbdo1) ? "Successful" : 
  fbdo1.errorReason().c_str());
}

void getLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  strftime (date1, 9, "%Y%m%d", &timeinfo);
  strftime (time1, 9, "%H:%M:%S", &timeinfo);
  sscanf(date1, "%d", &date2);
}

void writeFirebase(String message, String path) {
  FirebaseJson json;
  json.add("M", message );
  json.add("D", date2 );
  json.add("T", time1 );
  Firebase.setJSONAsync(fbdo, path, json);
}

void gpsRequest() {
  Serial7600.println("AT+CGNSSINFO");
  //  Serial7600.println("AT+CGPSINFO");
}

float BVSlope = 3200 / 8.24;
void CheckVoltage() {
  //  Serial.print("Reporting Battery voltage  ");
  battVoltage.reading(analogRead(battVoltagePin));
}
void ReportVoltage() {
  int bv = analogRead(battVoltagePin);
  float voltagema = 0;
  if (bv >= 100) {
    voltagema = battVoltage.reading(bv) / BVSlope;
  } else {
    buzStateBlinkCount = 2;
  }
  FirebaseJson json;
  json.add("V", voltagema );
  json.add("V_adc", voltagema *BVSlope);
  json.add("i_adc", bv );
  json.add("i_adc_V", bv/ BVSlope );
  json.add("D", date2 );
  json.add("T", time1 );
  Firebase.setJSONAsync(fbdo, "Robot/BatteryVoltage", json);
}

void BlinkLED() {
  if (ledStateBlinkCount > 0) {
    if (millis() - ledPrevMillis > ledBlinkTime || ledPrevMillis == 0 ) {
      ledPrevMillis = millis();
      if (ledState) {
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(buzzer, LOW);//on
      } else {
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(buzzer, HIGH);//on
      }
      ledState = !ledState;
      ledStateBlinkCount = ledStateBlinkCount - 0.5;
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW); ledState = true;
  }
}
void buzOnce() {
  digitalWrite(buzzer, HIGH);//on
  delay(70);
  digitalWrite(buzzer, LOW);//on
}
void PlayBuzzer() {
  if (buzStateBlinkCount > 0 & bs == 1) {
    if (millis() - buzPrevMillis > buzBlinkTime  ) {
      buzPrevMillis = millis();
      if (buzState) {
        digitalWrite(buzzer, LOW);//off
      } else {
        digitalWrite(buzzer, HIGH);//on
      }
      buzState = !buzState;
      buzStateBlinkCount = buzStateBlinkCount - 1;
    }
  } else {
    digitalWrite(buzzer, LOW);//off
    buzState = false;
  }

}

void blinkLED1() {
  if (ledState) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  ledState = !ledState;
  delay(ledBlinkTime);
}

void DriveServo() {
  if (millis() - gripperP > gripperS && tr != tri) {
    if (tr > tri) {
      tri++;
    } else {
      tri--;
    }
    pca9685.setPWM(gripper, 0, tri);
    gripperP = millis();
  }
  if (millis() - gripperRP > gripperRS && rx != rxi) {
    if (rx > rxi) {
      rxi++;
    } else {
      rxi--;
    }
    pca9685.setPWM(gripperR, 0, rxi);
    gripperRP = millis();
  }
  if (millis() - arm2P > arm2S && ry != ryi) {
    if (ry > ryi) {
      ryi++;
    } else {
      ryi--;
    }
    pca9685.setPWM(arm2, 0, ryi);
    arm2P = millis();
  }
  if (millis() - arm1P > arm1S && ly != lyi) {
    if (ly > lyi) {
      lyi++;
    } else {
      lyi--;
    }
    pca9685.setPWM(arm1, 0, lyi);
    arm1P = millis();
  }
  if (millis() - baseRP > baseRS && lx != lxi) {
    if (lx > lxi) {
      lxi++;
    } else {
      lxi--;
    }
    pca9685.setPWM(baseR, 0, lxi);
    baseRP = millis();
  }
}
