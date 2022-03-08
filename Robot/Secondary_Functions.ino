void WiFiSetup() {
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.print("........");
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int wifi_millis = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    blinkLED1();
    if (millis() - wifi_millis > 8000) {
      Serial.print("\nRestarting ESP as WiFi not connected\n\n\n");
      buzOnce(); buzOnce(); buzOnce(); buzOnce(); buzOnce();
      ESP.restart();
    }
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
String childPath[10] = {"/AT", "/lx", "/ly", "/rx", "/ry", "/tr", "/Buzz", "/bs", "/msg", "/Firmware"};

void FirebaseInit() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);
  Firebase.reconnectWiFi(true);//  Firebase.setMaxErrorQueue(fbdo, 3);
  Serial.printf("\nFirebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  Serial.println("Connecting to Google Firebase...\n");
  while (!Firebase.ready()) {
    blinkLED1();
  }
  buzOnce();
  digitalWrite(LED_BUILTIN, LOW); ledState = true;
  Serial.println("Sending Cloud message notification...");
  if (!newFirmwareAnnounce) sendMessage("Robot is Online! ", "Robot was restarted");
  buzOnce();
  Firebase.RTDB.setIntAsync(&fbdo, "Robot/AT/Update", 0);
  Serial.println("Begin streaming");
  if (!Firebase.RTDB.beginMultiPathStream(&stream, parentPath))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
  Firebase.RTDB.setMultiPathStreamCallback(&stream, streamCallback, streamTimeoutCallback);
}
bool buzzed = false;
bool bs = 1;
void streamCallback(MultiPathStream stream)
{
  ledStateBlinkCount = 3;
  if (buzzed && bs == 1)buzStateBlinkCount = 2;
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);
  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i])) {
      switch (i) {
        case 0:
          Serial.print("AT command : ");
          Serial.println(stream.value.c_str());
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
          if (buzzed) buzStateBlinkCount = 6;
          buzzed = true;
          break;
        case 7:
          bs = stream.value.toInt();
          if (bs == 1) buzStateBlinkCount = 2;
          break;
        case 8: //sms - msg
          {
            int msg = stream.value.toInt();
            if (msg == 1) {
              Serial.println("Request to send sms");
              SendMessage();
            }
            break;
          }
        case 9: { //Firmware updates
            fv_name = stream.value.c_str();
            Firebase.RTDB.setIntAsync(&fbdo, "Robot/AT/Update", 0);
            if (newFirmwareAnnounce) {
              sendMessage("Robot: Firmware updated ", fv_name + " successfully installed");
              newFirmwareAnnounce = false;
            }
            Serial.print("Firmware Update: ");
            Serial.println(fv_name);
            if (firmwareVersion != fv_name.substring(25).toInt())
            {
              newFirmware = true;
              newFirmwareVersion = fv_name.substring(25).toInt();
              Serial.print("New Firmware Found: ");
              Serial.println(fv_name.substring(25));  //8_8_6_x
              updateFirmware(fv_name);
            }
          }
      }
    }
  }
}
void updateFirmware(String firmwareName) {
  downloading = true;
  Firebase.RTDB.setIntAsync(&fbdo, "Robot/AT/Update", 1);
  sendMessage("Robot: Downloading new firmware", fv_name);
  if (!Firebase.Storage.downloadOTA(&stream, STORAGE_BUCKET_ID, firmwareName + ".bin", fcsDownloadCallback))
    Serial.println(stream.errorReason());
}
void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("Stream timeout, resuming...\n");
  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void sendMessage(String title, String body)
{
  FCM_Legacy_HTTP_Message msg;
  msg.targets.to = "/topics/Alert";
  msg.options.time_to_live = "1000";
  msg.options.priority = "high";
  msg.payloads.notification.title = title; //"Robot is Online! ";
  msg.payloads.notification.body = body; //"Robot was restarted";
  FirebaseJson payload;    //all data key-values should be string
  payload.add("D", "robot" );
  msg.payloads.data = payload.raw();
  if (Firebase.FCM.send(&fbdo1, &msg)) {//send message to recipient
    //    Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo1).c_str());
  } else {
    Serial.println("Cloud messaging failed -> ");
    Serial.println(fbdo1.errorReason());
  }
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
  Firebase.RTDB.setJSONAsync(&fbdo, path, &json);//Firebase.setJSONAsync(fbdo, path, json);
}

void gpsRequest() {
  Serial7600.println("AT+CGNSSINFO");
}

float BVSlope = 3200 / 8.24;
void CheckVoltage() {
  //  battVoltage.reading(analogRead(battVoltagePin));
  int bv = analogRead(battVoltagePin);
  float voltagema = 0;
  if (bv >= 500) {
    battVoltage.reading(bv);
  } else {
    buzStateBlinkCount = 2;
    ReportVoltage();
  }
}
void ReportVoltage() {
  int bv = analogRead(battVoltagePin);
  float voltagema = 0;
  if (bv >= 500) {
    voltagema = battVoltage.reading(bv) / BVSlope;
  } else {
    buzStateBlinkCount = 2;
  }
  FirebaseJson json;
  json.add("V", voltagema );
  json.add("V_adc", voltagema * BVSlope);
  json.add("i_adc", bv );
  json.add("i_adc_V", bv / BVSlope );
  json.add("D", date2 );
  json.add("T", time1 );
  Firebase.RTDB.setJSONAsync(&fbdo, "Robot/BatteryVoltage", &json);
}

void BlinkLED() {
  if (ledStateBlinkCount > 0) {
    if (millis() - ledPrevMillis > ledBlinkTime || ledPrevMillis == 0 ) {
      ledPrevMillis = millis();
      if (ledState) {
        digitalWrite(LED_BUILTIN, HIGH);
      } else {
        digitalWrite(LED_BUILTIN, LOW);
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
  delay(buzBlinkTime);
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
  if (tr != tri && millis() - gripperP > gripperS ) {
    if (tr > tri) {
      tri++;
    } else {
      tri--;
    }
    pca9685.setPWM(gripper, 0, tri);
    gripperP = millis();
  }
  if ( rx != rxi && millis() - gripperRP > gripperRS ) {
    if (rx > rxi) {
      rxi++;
    } else {
      rxi--;
    }
    pca9685.setPWM(gripperR, 0, rxi);
    gripperRP = millis();
  }
  if ( ry != ryi && millis() - arm2P > arm2S ) {
    if (ry > ryi) {
      ryi++;
    } else {
      ryi--;
    }
    pca9685.setPWM(arm2, 0, ryi);
    arm2P = millis();
  }
  if (ly != lyi &&  millis() - arm1P > arm1S ) {
    if (ly > lyi) {
      lyi++;
    } else {
      lyi--;
    }
    pca9685.setPWM(arm1, 0, lyi);
    arm1P = millis();
  }
  if (lx != lxi && millis() - baseRP > baseRS ) {
    if (lx > lxi) {
      lxi++;
    } else {
      lxi--;
    }
    pca9685.setPWM(baseR, 0, lxi);
    baseRP = millis();
  }
}
//The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info)
{
  if (info.status == fb_esp_fcs_download_status_init)
  {
    Serial.printf("Downloading firmware %s (%d)\n", info.remoteFileName.c_str(), info.fileSize);
    sendMessage("Robot: New firmware " + fv_name, "Downloading new firmware....");
  }
  else if (info.status == fb_esp_fcs_download_status_download)
  {
    ledStateBlinkCount = 10;
    if (buzzed && bs == 1) buzStateBlinkCount = 2;
    Serial.printf("Downloaded %d%s\n", (int)info.progress, "%");
  }
  else if (info.status == fb_esp_fcs_download_status_complete)
  {
    sendMessage("Robot: New firmware " + fv_name , "Downloaded and restarting....");
    EEPROM.write(2, newFirmwareVersion);
    EEPROM.commit();
    Serial.println("Update firmware completed.");
    Serial.println("\nESP32 Restarting in 3 seconds...\n\n");
    delay(3000);
    ESP.restart();
  }
  else if (info.status == fb_esp_fcs_download_status_error)
  {
    Serial.println("Download " + fv_name + " failed " + info.errorMsg.c_str());
    sendMessage("Robot: Download firmware failed !", fv_name + " : " + info.errorMsg.c_str());
    Serial.println("\nESP32 Restarting in 2 seconds...\n\n");
    delay(2000);
    ESP.restart();
  }
}
