#include <WiFi.h>
#include <analogWrite.h>
#define currentFirmware 1.1
#include "HardwareSerial.h"
#include "time.h"
#include <Firebase_ESP_Client.h>
#include "credentials.h"
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <movingAvg.h>                  // https://github.com/JChristensen/movingAvg
movingAvg battVoltage(20);

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
Adafruit_PWMServoDriver  pca9685 = Adafruit_PWMServoDriver(0x40);

#define I2C_SDA 19// purple //yellow
#define I2C_SCL 18// blue //green//black
#define LED_BUILTIN  5
#define battVoltagePin  35
#define ATrx  34 // green//grey
#define ATtx  25 // yellow 
#define excavator  26 // white 
#define buzzer  27 //orange

const char* ntpServer = "pool.ntp.org"; const long  gmtOffset_sec = 0; const int   daylightOffset_sec = 3600;
char date1 [8]; char time1 [10];
int date2 = 0;

bool ledState = true;
double ledStateBlinkCount = 3;
int ledBlinkTime = 40; //in milliseconds
int ledPrevMillis = 0;
bool ATbusy = false;
bool buzState = false;
double buzStateBlinkCount = 1;
int buzBlinkTime = 20; //in milliseconds
int buzPrevMillis = 0;
int buz_f = 180;
int pwm_on = 4095;
int on_time = 700;
int on_time_long = 1200;

int z1_millis = 0;
int z2_millis = 0;
int z3_millis = 0;
int z4_millis = 0;
int z5_millis = 0;
int z6_millis = 0;
int z7_millis = 0;
int z8_millis = 0;
bool z1s = false;
bool z2s = false;
bool z3s = false;
bool z4s = false;
bool z5s = false;
bool z6s = false;
bool z7s = false;
bool z8s = false;
int i1 = 0;
int i2 = 0;
int i3 = 0;
int i4 = 0;
int i5 = 0;
int i6 = 0;
int i7 = 0;
int i8 = 0;
FirebaseData stream, streamAT, fbdo, fbdo1;
FirebaseAuth auth; FirebaseConfig config;
bool taskCompleted = false;

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
TaskScheduler bvCheckTask(25, CheckVoltage);
TaskScheduler batteryVoltageTask(1000, ReportVoltage);
TaskScheduler gpsUpdateTask(4000, gpsRequest);
void logMemory() {
  log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
}
void setup()
{
  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());

  logMemory();
  byte* psdRamBuffer = (byte*)ps_malloc(500000);
  logMemory();
  free(psdRamBuffer);
  logMemory();


  Serial.begin(115200);
  Serial7600.begin(115200, SERIAL_8N1, ATrx, ATtx); // 34-RX 25-TX of ESP-Wrover
  battVoltage.begin();
  Wire.begin(I2C_SDA, I2C_SCL);// Initialize I2C connection
  pca9685.begin();  // Initialize PCA9685
  pca9685.setPWMFreq(2);  // This is the maximum PWM frequency
  AllTurnOff();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buzzer, OUTPUT); digitalWrite(buzzer, LOW); //OFF
  pinMode(excavator, OUTPUT); digitalWrite(excavator, LOW); //ON
  pinMode(battVoltagePin, INPUT);
  WiFiSetup();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  FirebaseInit();
}

void loop()
{
  CheckATSerial();
  BlinkLED();
  checkDriveDelay();
  PlayBuzzer();
  bvCheckTask.run();
  checkDriveDelay();
  batteryVoltageTask.run();
  checkDriveDelay();
  localTimeTask.run();
  if (!ATbusy) gpsUpdateTask.run();
  checkDriveDelay();
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
            if (message_pos > 50 ) writeFirebase(message, "Excavator/Control/GNSS");
            else Serial.println("Empty message received");
          }
          else if (strncmp(message, "+CGPSINFO", 9) == 0) {
            if (message_pos > 50 ) writeFirebase(message, "Excavator/Control/GPS");
          }
          else {
            writeFirebase(message, "Excavator/AT");
            Serial.println(message);
            if (strncmp(message, "PB DONE", 7) == 0) {
              Serial.print("SMS check passed  -> Sending SMS to Dushyant");
              ATbusy = true;
              SendMessage();
              ATbusy = false;
            }

          }
        }
      }
      message_pos = 0;
    }
  }
}
