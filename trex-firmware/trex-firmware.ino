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
#define restAngle 150
#define highAngle 20

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M1(0x30, _MOTOR_A, 1000); //Motor A
Motor M2(0x30, _MOTOR_B, 1000); //Motor B

// Should this be volatile? it's accessed from web callback
boolean connected = false;
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

void jump() {
  jumpServo.write(highAngle);
  Serial.println("JUMP!");
  do {
    delay(200);
  }
  while (!digitalRead(buttonPin));

  jumpServo.write(restAngle - 20);
  delay(200);
  jumpServo.write(restAngle);
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
  Serial.begin(115200);
  Serial.println("READY");

  pinMode(buttonPin, INPUT_PULLUP);
  jumpServo.attach(D6);
  jumpServo.write(restAngle);

  WiFi.begin(ssid, password);
  Serial.println("");
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

