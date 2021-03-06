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
  Serial.print("\nConnected to " + String(WIFI_SSID) + " with IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(" Strength : ");
  Serial.print(WiFi.RSSI());
  Serial.println();
}

String parentPath = "Excavator/Control/data";
String childPath[14] = {"/Buzz", "/bs", "/msg", "/AT", "/pwr", "/z1", "/z2", "/z3", "/z4", "/z5", "/z6", "/z7", "/z8", "/Firmware"};

void AllTurnOff() {
  //  Serial.println("Turnin All off PWM");
  for (int i = 0; i <= 15; i++) {
    pca9685.setPWM(i, 0, 0); // 0-7 all high - directly connected
  }
}

void FirebaseInit() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  config.fcs.download_buffer_size = 2048;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY);
  Firebase.reconnectWiFi(true);
  Serial.printf("\nFirebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  Serial.println("Connecting to Google Firebase...\n");
  while (!Firebase.ready()) {
    blinkLED1();
  }
  buzOnce();
  digitalWrite(LED_BUILTIN, LOW); ledState = true;
  Serial.println("Sending Cloud notification to user...");
  if (!newFirmwareAnnounce) {
    sendMessage("Excavator is Online! ", "Excavator was restarted");
  }
  buzOnce();
  Firebase.RTDB.setIntAsync(&fbdo, "Excavator/AT/Update", 0);
  Serial.println("Start firebase streaming");
  if (!Firebase.RTDB.beginMultiPathStream(&stream, parentPath))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
  Firebase.RTDB.setMultiPathStreamCallback(&stream, streamCallback, streamTimeoutCallback);
}

bool buzzed = false;
bool bs = 1;
void streamCallback(MultiPathStream stream)
{
  ledStateBlinkCount = 2;
  if (buzzed && bs == 1)buzStateBlinkCount = 2;
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);
  for (size_t i = 0; i < numChild; i++)
  { // String childPath[8] = {"/Buzz", "/bs", "/sms", "/AT", "/pwr", "/z1", "/z2", "/z3", "/z4", "/z5", "/z6", "/z7", "/z8"};
    if (stream.get(childPath[i])) {
      switch (i) {
        case 0: //buzz - play buzzer
          if (buzzed)buzStateBlinkCount = 6;
          buzzed = true;
          break;
        case 1: //bs = buzzer status
          bs = stream.value.toInt();
          if (bs == 1) buzStateBlinkCount = 2;
          Serial.print("BS :- ");
          Serial.println(bs);
          break;
        case 2: //sms
          {
            int msg = stream.value.toInt();
            if (msg == 1) {
              Serial.println("Request to send sms");
              SendMessage();
            }
            break;
          }
        case 3: { //AT
            String c = stream.value.c_str();
            Serial.print("AT command : ");
            Serial.println(c);
            ATbusy = true;
            Serial7600.println(c);
            ATbusy = false;
            break;
          }
        case 4: //pwr
          if (stream.value.toInt() == 1) {
            AllTurnOff();
            digitalWrite(excavator, HIGH); //ON
          }
          else {
            digitalWrite(excavator, LOW); //OFF
          }
          break;
        case 5: { //z1
            Serial.print("Z1 ");
            i1 = stream.value.toInt();
            Serial.println(i1);
            if (i1 != 0) {
              z1_millis = millis();
            }
            z1s = true;
            drive(1, i1);
            break;
          }
        case 6: { //z2
            Serial.print("Z2 ");
            i2 = stream.value.toInt();
            Serial.println(i2);
            if (i2 != 0) {
              z2_millis = millis();
            }
            z2s = true;
            drive(2, i2);
            break;
          }
        case 7: { //z3
            Serial.print("Z3 ");
            i3 = stream.value.toInt();
            Serial.println(i3);
            if (i3 != 0) {
              z3_millis = millis();

            }
            z3s = true;
            drive(3, i3);
            break;
          }
        case 8: { //z4
            Serial.print("Z4 ");
            i4 = stream.value.toInt();
            Serial.println(i4);
            if (i4 != 0) {
              z4_millis = millis();
            }
            z4s = true;
            drive(4, i4);
            break;
          }
        case 9: { //z5
            Serial.print("Z5 ");
            i5 = stream.value.toInt();
            Serial.println(i5);
            if (i5 != 0) {
              z5_millis = millis();
            }
            z5s = true;
            drive(5, i5);
            break;
          }
        case 10: { //z6
            Serial.print("Z6 ");
            i6 = stream.value.toInt();
            Serial.println(i6);
            if (i6 != 0) {
              z6_millis = millis();
            }
            z6s = true;
            drive(6, i6);
            break;
          }
        case 11: { //z7
            Serial.print("Z7 ");
            i7 = stream.value.toInt();
            Serial.println(i7);
            if (i7 != 0) {
              z7_millis = millis();
            }
            z7s = true;
            drive(7, i7);
            break;
          }
        case 12: { //z8
            Serial.print("Z8 ");
            i8 = stream.value.toInt();
            Serial.println(i8);
            if (i8 != 0) {
              z8_millis = millis();
            }
            z8s = true;
            drive(8, i8);
            break;
          }
        case 13: { //Firmware updates
            fv_name = stream.value.c_str();
            if (newFirmwareAnnounce) {
              sendMessage("Excavator: Firmware updated ", fv_name + " successfully installed");
              newFirmwareAnnounce = false;
            }
            Firebase.RTDB.setIntAsync(&fbdo, "Excavator/AT/Update", 0);            Serial.print("Firmware Update: ");
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
  Firebase.RTDB.setIntAsync(&fbdo, "Excavator/AT/Update", 1);
  sendMessage("Excavator: Downloading new firmware", fv_name);
  if (!Firebase.Storage.downloadOTA(&stream, STORAGE_BUCKET_ID, firmwareName + ".bin", fcsDownloadCallback))
    Serial.println(stream.errorReason());
}

void drive(int channel, int i) {
  if (i == 0) {
    pca9685.setPWM(channel - 1, 0, 0);
    pca9685.setPWM(channel + 7, 0, 0);
  } else if (i > 0) {
    pca9685.setPWM(channel + 7, 0, 0);
    pca9685.setPWM(channel - 1, 0, pwm_on);
  } else if (i < 0) {
    pca9685.setPWM(channel - 1, 0, 0);
    pca9685.setPWM(channel + 7, 0, pwm_on);
  }
}

void checkDriveDelay() {
  if (z1s && (millis() - z1_millis > on_time)) {//z1
    drive(1, 0);
    z1s = false;
  }
  if (z2s && (millis() - z2_millis > on_time)) {//z2
    drive(2, 0);
    z2s = false;
  }
  if (z3s &&  (millis() - z3_millis > on_time)) {//z3
    drive(3, 0);
    z3s = false;
  }
  if ( z4s && (millis() - z4_millis > on_time)) {//z4
    z4s = false;
    drive(4, 0);
  }
  if (z5s && (millis() - z5_millis > on_time_long)) {//z5
    z5s = false;
    drive(5, 0);
  }
  if ( z6s && (millis() - z6_millis > on_time)) {//z6
    z6s = false;
    drive(6, 0);
  }
  if (z7s && (millis() - z7_millis > on_time)) {//z7
    z7s = false;
    drive(7, 0);
  }
  if ( z8s && (millis() - z8_millis > on_time)) {//z8
    z8s = false;
    drive(8, 0);
  }
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
  msg.payloads.notification.title = title; //"Excavator is Online! ";
  msg.payloads.notification.body = body; //"Excavator was restarted";
  FirebaseJson payload;    //all data key-values should be string
  payload.add("D", "excavator" );
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
  Firebase.RTDB.setJSONAsync(&fbdo, path, &json);
}

void gpsRequest() {
  Serial7600.println("AT+CGNSSINFO");
}

float BVSlope = 3315 / 8.37;
void CheckVoltage() {
  //  battVoltage.reading(analogRead(battVoltagePin));
  int bv = analogRead(battVoltagePin);
  float voltagema = 0;
  if (bv >= 1000) {
    battVoltage.reading(bv);
  } else {
    buzStateBlinkCount = 2;
    ReportVoltage();
  }
}
void ReportVoltage() {
  int bv = analogRead(battVoltagePin);
  float voltagema = 0;
  if (bv >= 1000) {
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
  Firebase.RTDB.setJSONAsync(&fbdo, "Excavator/BatteryVoltage", &json);
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
  digitalWrite(buzzer, LOW);//off
}
void PlayBuzzer() {
  if (buzStateBlinkCount > 0 && bs == 1) {
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
//The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info)
{
  if (info.status == fb_esp_fcs_download_status_init)
  {
    Serial.printf("Downloading firmware %s (%d)\n", info.remoteFileName.c_str(), info.fileSize);
    sendMessage("Excavator: New firmware " + fv_name, "Downloading new firmware....");
  }
  else if (info.status == fb_esp_fcs_download_status_download)
  {
    ledStateBlinkCount = 10;
    if (buzzed && bs == 1) buzStateBlinkCount = 2;
    Serial.printf("Downloaded %d%s\n", (int)info.progress, "%");
  }
  else if (info.status == fb_esp_fcs_download_status_complete)
  {
    sendMessage("Excavator: New firmware " + fv_name , "Downloaded and restarting....");
    EEPROM.write(2, newFirmwareVersion);
    EEPROM.commit();
    Serial.println("Update firmware completed.");
    Serial.println("\nESP32 Restarting in 3 seconds...\n\n");
    Firebase.RTDB.setIntAsync(&fbdo, "Excavator/AT/Update", 2);
    delay(3000);
    ESP.restart();
  }
  else if (info.status == fb_esp_fcs_download_status_error)
  {
    Serial.println("Download " + fv_name + " failed " + info.errorMsg.c_str());
    sendMessage("Excavator: Download firmware failed !", fv_name + " : " + info.errorMsg.c_str());
    Serial.println("\nESP32 Restarting in 2 seconds...\n\n");
    delay(2000);
    ESP.restart();
  }
}
