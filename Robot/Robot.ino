#include <WiFi.h>

#include "HardwareSerial.h"
#include "time.h"
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"//Provide the token generation process info.
#include "addons/RTDBHelper.h"//Provide the RTDB payload printing info and other helper functions.
#include <movingAvg.h>                  // https://github.com/JChristensen/movingAvg
movingAvg battVoltage(10);

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

#define API_KEY "AIzaSyCA2zQP6hrOXAAsspTExUeQKsl4hhsJqOI" //"AIzaSyCIOWckVbYTrLsujeseL8eB-lVVD4O43nE"
#define DATABASE_URL "https://ttl-iot-default-rtdb.europe-west1.firebasedatabase.app" //"fun-iot-default-rtdb.europe-west1.firebasedatabase.app"
#define USER_EMAIL "kumbhat10@gmail.com"
#define USER_PASSWORD "Kanakrajj10"
#define FIREBASE_FCM_SERVER_KEY "AAAAzkYiwb0:APA91bEwuO6l8QJ-QE-FNMPcwmJwec-qaf1LXlNVoQ35xUUn3AJsBzykKg6NxI57xO5sKxYDQAmEOTqsFjC6ZZhA0lgTuEeScL9al4soFSu_4FQLBYfASLCdn6ICLZsO94aWsXGnVRFe"

const char* ntpServer = "pool.ntp.org"; const long  gmtOffset_sec = 0; const int   daylightOffset_sec = 3600;
char date1 [8]; char time1 [10];
int date2 = 0;

bool ledState = true;
double ledStateBlinkCount = 3;
int ledBlinkTime = 40; //in milliseconds
int ledPrevMillis = 0;

bool buzState = false;
double buzStateBlinkCount = 1;
int buzBlinkTime = 20; //in milliseconds
int buzPrevMillis = 0;


//FirebaseJsonData lx, ly, rx, ry, tr;
int lxi = 285;
int lyi = 375;
int rxi = 300;
int ryi = 470;
int tri = 270;

int lx = 288;
int ly = 376;
int rx = 303;
int ry = 470;
int tr = 270;

int baseRS = 3 ; //lx - base rotate
int arm1S = 6 ;// ly - main arm
int arm2S = 5; ///ry - second arm
int gripperRS = 2;  //rx  - gripper rotate
int gripperS = 0; // tr - gripper  45-103

unsigned long baseRP = 0;  //lx - base rotate
unsigned long arm1P = 0; // ly - main arm
unsigned long arm2P = 0; ///ry - second arm
unsigned long gripperRP = 0;  //rx  - gripper rotateunsigned long gripperP  0; // tr - gripper  45-103
unsigned long gripperP = 0;  //tr  - gripper

FirebaseData stream, streamAT, fbdo, fbdo1;
FirebaseAuth auth; FirebaseConfig config;

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
TaskScheduler bvCheckTask(20, CheckVoltage);
TaskScheduler batteryVoltageTask(1000, ReportVoltage);
TaskScheduler gpsUpdateTask(5000, gpsRequest);

void setup()
{
  Serial.begin(115200);
  Serial7600.begin(115200, SERIAL_8N1, ATrx, ATtx); // 34-RX 25-TX of ESP-Wrover
  battVoltage.begin();
  Wire.begin(I2C_SDA, I2C_SCL);// Initialize I2C connection
  pca9685.begin();  // Initialize PCA9685
  pca9685.setPWMFreq(50);  // This is the maximum PWM frequency
  pca9685.setPWM(gripper, 0, tri);
  pca9685.setPWM(gripperR, 0, rxi);
  pca9685.setPWM(arm2, 0, ryi);
  pca9685.setPWM(arm1, 0, lyi);
  pca9685.setPWM(baseR, 0, lxi);
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
  bvCheckTask.run();
  batteryVoltageTask.run();
  localTimeTask.run();
  gpsUpdateTask.run();
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
      if (message_pos > 2) {
        //        Serial.print("Message pos :  ");
        //        Serial.println(message_pos);
        //        Serial.println(message);
        if (message_pos > 70) writeFirebase(message, "Robot/Control/GNSS") ;// Firebase.setStringAsync(fbdo, "/Control/GNSS", message);
        else if (message_pos > 50) writeFirebase(message, "Robot/Control/GPS"); //Firebase.setStringAsync(fbdo, "/Control/GPS", message);
      }
      message_pos = 0;
    }

    //    Serial.print(char(Serial7600.read()));
  }
}
