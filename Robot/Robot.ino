#include <WiFi.h>

#include "HardwareSerial.h"
#include "time.h"
#include <Firebase_ESP_Client.h>
#include "credentials.h"
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <EEPROM.h>
#include <movingAvg.h>
movingAvg battVoltage(20);

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver  pca9685 = Adafruit_PWMServoDriver(0x40);

#define I2C_SDA 19//13 // green //15 and more importantly 2 and 12 are strapping pins
#define I2C_SCL 18//14 //yellow //blue vcc orange ground
#define baseR  13  //lx - base rotate
#define arm1  5 // ly - main arm 
#define arm2  8 ///ry - second arm
#define gripperR  11  //rx  - gripper rotate
#define gripper  0 // tr  - gripper  45-103
#define LED_BUILTIN  5
#define buzzer  21
#define battVoltagePin  35//34
#define ATrx  34
#define ATtx  25

const char* ntpServer = "pool.ntp.org"; const long  gmtOffset_sec = 0; const int   daylightOffset_sec = 3600;
char date1 [8]; char time1 [10];
int date2 = 0;

bool ledState = true;
volatile double ledStateBlinkCount = 3;
int ledBlinkTime = 40; //in milliseconds
int ledPrevMillis = 0;

bool buzState = false;
volatile double buzStateBlinkCount = 1;
int buzBlinkTime = 20; //in milliseconds
int buzPrevMillis = 0;

//FirebaseJsonData lx, ly, rx, ry, tr;
int lxi = 285;
int lyi = 375;
int rxi = 300;
int ryi = 470;
int tri = 270;

volatile int lx = 288;
volatile int ly = 376;
volatile int rx = 303;
volatile int ry = 470;
volatile int tr = 270;

int baseRS = 1 ; //lx - base rotate
int arm1S = 3 ;// ly - main arm
int arm2S = 1; ///ry - second arm
int gripperRS = 0;  //rx  - gripper rotate
int gripperS = 0; // tr - gripper  45-103

unsigned long baseRP = 0;  //lx - base rotate
unsigned long arm1P = 0; // ly - main arm
unsigned long arm2P = 0; ///ry - second arm
unsigned long gripperRP = 0;  //rx  - gripper rotateunsigned long gripperP  0; // tr - gripper  45-103
unsigned long gripperP = 0;  //tr  - gripper

FirebaseData stream, streamAT, fbdo, fbdo1;
FirebaseAuth auth; FirebaseConfig config;

volatile bool ATbusy = false;
volatile bool newFirmware = false;
volatile bool newFirmwareAnnounce = false;
volatile int firmwareVersion = 0; //EEPROM.read(1); address 1
volatile int newFirmwareVersion = 0; //EEPROM.read(2); address 2
String fv_name = "";

HardwareSerial Serial7600(1);

class TaskScheduler {
    unsigned long timeDelay;
    void (*function)();
    unsigned long prevMillis = 0;
  public:
    TaskScheduler(unsigned long timeDelay, void (*func)() ) {
      this->timeDelay = timeDelay;
      this->function = func;
    }
    void run() {
      if (millis() - prevMillis > timeDelay) {
        (*function)();
        prevMillis = millis();
      }
    }
};

void demo() {
  //  Serial.println("Demo  ");
  //  Serial.println(char(26)); //for SUB printu
  //  Serial7600.println("AT");
}
TaskScheduler localTimeTask(1000, getLocalTime);
TaskScheduler bvCheckTask(50, CheckVoltage);
TaskScheduler batteryVoltageTask(2000, ReportVoltage);
TaskScheduler gpsUpdateTask(4000, gpsRequest);

void setup()
{
  Serial.begin(115200);
  Serial.println();  Serial.println();  Serial.println();
  EEPROM.begin(512);
  // Only to be used for New ESP32 for first time - then it automatically updates
  //  EEPROM.write(1, 1); //remove it - set newFirmwareVersion to 1
  //  EEPROM.write(2, 1); //remove it - set newFirmwareVersion to 1
  //  EEPROM.commit(); //remove it

  firmwareVersion = EEPROM.read(1);
  newFirmwareVersion = EEPROM.read(2);
  if (newFirmwareVersion != firmwareVersion) {
    Serial.print("Previous Firmware Version : "); Serial.println(firmwareVersion);
    Serial.print("New Firmware Version : "); Serial.println(newFirmwareVersion);
    firmwareVersion = EEPROM.read(2);
    EEPROM.write(1, newFirmwareVersion);
    EEPROM.commit();
    newFirmwareAnnounce = true;
    Serial.println();
    Serial.println("Firmware update was successfull : ");
  }
  Serial.print("Current firmware version is : ");  Serial.println(EEPROM.read(1));  Serial.println();

  Serial7600.begin(115200, SERIAL_8N1, ATrx, ATtx); // 34-RX 25-TX of ESP-Wrover
  battVoltage.begin();
  Wire.begin(I2C_SDA, I2C_SCL);// Initialize I2C connection
  pca9685.begin();  // Initialize PCA9685
  pca9685.setPWMFreq(50);  // This is the maximum PWM frequency
  //  pca9685.setPWM(gripper, 0, tri);
  //  pca9685.setPWM(gripperR, 0, rxi);
  //  pca9685.setPWM(arm2, 0, ryi);
  //  pca9685.setPWM(arm1, 0, lyi);
  //  pca9685.setPWM(baseR, 0, lxi);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buzzer, OUTPUT); digitalWrite(buzzer, LOW); //off
  pinMode(battVoltagePin, INPUT);
  WiFiSetup();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  FirebaseInit();
}

void loop()
{
  CheckATSerial();
  BlinkLED();
  DriveServo();
  PlayBuzzer();
  if (!newFirmware) bvCheckTask.run();
  if (!newFirmware) batteryVoltageTask.run();
  if (!newFirmware) localTimeTask.run();
  if (!ATbusy && !newFirmware) gpsUpdateTask.run();
  PlayBuzzer();
}

const unsigned int MAX_MESSAGE_LENGTH = 100;
void CheckATSerial() {
  while (Serial.available() > 0) {
    Serial7600.write(Serial.read());
  }
  while (Serial7600.available() > 0) {
    char inByte = Serial7600.read();
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;
    if ( inByte != '\n' && inByte != '\r')   //Check not terminating character)
    { message[message_pos] = inByte;
      message_pos++;
    } else // Full message received...
    {
      message[message_pos] = '\0';
      if (strncmp(message, "OK", 2) != 0) {
        if (message_pos > 2) {
          if (strncmp(message, "+CGNSSINFO", 10) == 0) {
            if (message_pos > 50 ) writeFirebase(message, "Robot/Control/GNSS");
            else Serial.println("Empty message received");
          }
          else if (strncmp(message, "+CGPSINFO", 9) == 0) {
            if (message_pos > 50 ) writeFirebase(message, "Robot/Control/GPS");
          }
          else {
            writeFirebase(message, "Robot/AT");
            Serial.println(message);
            if (strncmp(message, "MISSED_CALL", 11) == 0) {
              Serial.println("Missed call now ");
              Serial.println(message);
            }
            else if (strncmp(message, "PB DONE", 7) == 0) {
              Serial.print("SMS check passed  -> Sending SMS to Dushyant");
              ATbusy = true;
//              SendMessage();
              ATbusy = false;
            }
          }
        }
      }
      message_pos = 0;
    }
  }
}
