#include "WEMOS_Motor.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>

const char* ssid = "SSID";
const char* password = "Password";
int pwm;
Servo jumpServo;
ESP8266WebServer server(80);

#define buttonPin D5
#define servoPin D6
#define lowAngle 100
#define highAngle 20

#define jumpSound 1
#define scoreSound 2

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M1(0x30, _MOTOR_A, 1000); //Motor A
Motor M2(0x30, _MOTOR_B, 1000); //Motor B

// Should this be volatile? it's accessed from web callback
boolean connected = false;
boolean introSoundPlayed = false;
volatile boolean playing = false;
volatile int speedM1 = -50;
volatile int speedM2 = -50;
const char* webIndex = "<html><head>"
                       "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                       "</head>"
                       "<p><a href='/stop'>Stop</a></p>"
                       "<p><a href='/25'>Slow</a></p>"
                       "<p><a href='/50'>Medium</a></p>"
                       "<p><a href='/75'>Fast</a></p>"
                       "<p><a href='/100'>Fastest</a></p>"
                       "<p><a href='/jump'>jump</a></p>"
                       "</html>"
                       ;

void playerCommand(uint8_t cmd, uint8_t arg1, uint8_t arg2) {
  uint8_t data[10] = {0x7e, 0xff, 0x6, cmd, 0, arg1, arg2, 0, 0, 0xef};
  int16_t checksum = 0 - data[1] - data[2] - data[3] - data[4] - data[5] - data[6];
  data[7] = (checksum >> 8) & 0xff;
  data[8] = checksum & 0xff;
  Serial.write(data, sizeof(data));
}

void playerCommand(uint8_t cmd, uint16_t arg) {
  playerCommand(cmd, arg >> 8, arg & 0xff);
}

void playSound(uint16_t fileNum, uint8_t volume) {
  if (volume > 0) {
    playerCommand(0x6, 0, volume);
    delay(20);
  }
  playerCommand(0x12, fileNum);
}

void jump() {
  jumpServo.attach(servoPin);
  jumpServo.write(highAngle);
  delay(200);
  playSound(jumpSound, 30);
  while (!digitalRead(buttonPin)) {
    delay(20);
  }
  jumpServo.detach();
}

void updateMotors() {
  if (!playing) {
    M1.setmotor(_CCW, 0);
    M2.setmotor(_CW, 0);
    return;
  }
  if (speedM1 >= 0 ) {
    M1.setmotor(_CCW, speedM1);
  } else {
    M1.setmotor(_CW, -speedM1);
  }
  /*  if (speedM2 >= 0 ) {
      M2.setmotor(_CCW, speedM2);
    } else {
      M2.setmotor(_CW, -speedM2);
    }*/
}

void gameSpeed(int speed) {
  speedM1 = -speed;
  speedM2 = speed;
  updateMotors();
}

void setup() {
  Serial.begin(9600);
  Serial.println("READY!");

  pinMode(buttonPin, INPUT_PULLUP);

  jumpServo.attach(D6);
  jumpServo.write(lowAngle);
  delay(100);
  jumpServo.detach();

  WiFi.begin(ssid, password);
  server.on("/", []() {
    server.send(200, "text/html", webIndex);
  });

  int step = 5;
  server.on("/stop", [step]() {
    playing = false;
    updateMotors();
    server.send(200, "text/html", webIndex);
  });
  server.on("/25", [step]() {
    playing = true;
    gameSpeed(25);
    server.send(200, "text/html", webIndex);
  });
  server.on("/50", [step]() {
    playing = true;
    gameSpeed(50);
    server.send(200, "text/html", webIndex);
  });
  server.on("/75", [step]() {
    playing = true;
    gameSpeed(75);
    server.send(200, "text/html", webIndex);
  });
  server.on("/100", [step]() {
    playing = true;
    gameSpeed(100);
    server.send(200, "text/html", webIndex);
  });
  server.on("/jump", []() {
    jump();
    server.send(200, "text/html", webIndex);
  });
  server.begin();
  updateMotors();
}


void loop() {
  updateMotors();

  if (millis() > 2000 && !playing && !introSoundPlayed) {
    playSound(scoreSound, 16);
    introSoundPlayed = true;
  }

  if (!connected && WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");
    Serial.println(WiFi.localIP());
    connected = true;
  }

  if (!digitalRead(buttonPin)) {
    if (!playing) {
      playing = true;
      delay(500);
    } else {
      jump();
    }
  }

  server.handleClient();
}

