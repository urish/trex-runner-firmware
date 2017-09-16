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
#define restAngle 120
#define highAngle 30

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M1(0x30, _MOTOR_A, 1000); //Motor A
Motor M2(0x30, _MOTOR_B, 1000); //Motor B

// Should this be volatile? it's accessed from web callback
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
  do {
    delay(200);
  }
  while (!digitalRead(buttonPin));

  jumpServo.write(restAngle - 20);
  delay(200);
  jumpServo.write(restAngle);
}

void updateMotors() {
  if (speedM1 >= 0 ) {
    M1.setmotor(_CCW, speedM1);
  } else {
    M1.setmotor(_CW, -speedM1);
  }
  if (speedM2 >= 0 ) {
    M2.setmotor(_CCW, speedM2);
  } else {
    M2.setmotor(_CW, -speedM2);
  }
}

void gameSpeed(int speed) {
  speedM1 = -speed;
  speedM2 = speed;
  updateMotors();
}

void setup() {
  Serial.begin(250000);

  pinMode(buttonPin, INPUT_PULLUP);
  jumpServo.attach(D6);
  jumpServo.write(restAngle);

  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html", webIndex);
  });

  int step = 5;
  server.on("/stop", [step]() {
    gameSpeed(0);
    server.send(200, "text/html", webIndex);
  });
  server.on("/25", [step]() {
    gameSpeed(25);
    server.send(200, "text/html", webIndex);
  });
  server.on("/50", [step]() {
    gameSpeed(50);
    server.send(200, "text/html", webIndex);
  });
  server.on("/75", [step]() {
    gameSpeed(75);
    server.send(200, "text/html", webIndex);
  });
  server.on("/100", [step]() {
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
  if (!digitalRead(buttonPin)) {
    jump();
  }
  server.handleClient();
}

