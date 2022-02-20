#include <WiFi.h>
#define FirmwareVersion 1.1
#include "HardwareSerial.h"
#include "time.h"
#include <Firebase_ESP_Client.h>
#include "credentials.h"
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <EEPROM.h>
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
volatile double ledStateBlinkCount = 3;
int ledBlinkTime = 40; //in milliseconds
volatile int ledPrevMillis = 0;
volatile bool ATbusy = false;
bool buzState = false;
double buzStateBlinkCount = 1;
int buzBlinkTime = 20; //in milliseconds
volatile int buzPrevMillis = 0;
int buz_f = 180;
int pwm_on = 4095;
int on_time = 700;
int on_time_long = 1200;

volatile int z1_millis = 0;
volatile int z2_millis = 0;
volatile int z3_millis = 0;
volatile int z4_millis = 0;
volatile int z5_millis = 0;
volatile int z6_millis = 0;
volatile int z7_millis = 0;
volatile int z8_millis = 0;
volatile bool z1s = false;
volatile bool z2s = false;
volatile bool z3s = false;
volatile bool z4s = false;
volatile bool z5s = false;
volatile bool z6s = false;
volatile bool z7s = false;
volatile bool z8s = false;
volatile int i1 = 0;
volatile int i2 = 0;
volatile int i3 = 0;
volatile int i4 = 0;
volatile int i5 = 0;
volatile int i6 = 0;
volatile int i7 = 0;
volatile int i8 = 0;
FirebaseData stream, fbdo, fbdo1;
FirebaseAuth auth; FirebaseConfig config;
bool taskCompleted = false;

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

void logMemory() {
  log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
}

TaskScheduler localTimeTask(1000, getLocalTime);
TaskScheduler bvCheckTask(25, CheckVoltage);
TaskScheduler batteryVoltageTask(1000, ReportVoltage);
TaskScheduler gpsUpdateTask(4000, gpsRequest);

void setup()
{
  Serial.begin(115200);
  Serial.println();  Serial.println();  Serial.println();
  EEPROM.begin(512);

//  EEPROM.write(1, 1); //remove it - set newFirmwareVersion to 1
//  EEPROM.write(2, 1); //remove it - set newFirmwareVersion to 1
//  EEPROM.commit(); //remove it
  
  firmwareVersion = EEPROM.read(1); Serial.print("old ver e1 "); Serial.println(EEPROM.read(1));
  newFirmwareVersion = EEPROM.read(2); Serial.print("old ver e1 "); Serial.println(EEPROM.read(2));
  if (newFirmwareVersion != firmwareVersion) {
    firmwareVersion = EEPROM.read(2);
    EEPROM.write(1, newFirmwareVersion);
    EEPROM.commit();
    newFirmwareAnnounce = true;
    Serial.println("Firmware update successfull : ");
  }
  Serial.print("Current firmware version is : ");  Serial.println(EEPROM.read(1));  Serial.println();


  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());

  logMemory();
  byte* psdRamBuffer = (byte*)ps_malloc(500000);
  logMemory();
  free(psdRamBuffer);
  logMemory();

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
  if(!newFirmware) bvCheckTask.run();
  checkDriveDelay();
  if(!newFirmware) batteryVoltageTask.run();
  checkDriveDelay();
  if(!newFirmware) localTimeTask.run();
  if (!ATbusy && !newFirmware) gpsUpdateTask.run();
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
