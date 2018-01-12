#include "InternetButton.h"     /* clock_t, clock, CLOCKS_PER_SEC */

InternetButton b = InternetButton();

const String version = "1.0.3";
// int led = 0;
// int colorRed, colorGreen, colorBlue;
String status = "initialized"; //initialized, worktime, restime, worktimecomplete, resttimecomplete
const int workTimeTotal = 1*60;
const int restTimeTotal = 1*60;
const int workIncrementSeconds = workTimeTotal / 11;
const int restIncrementSeconds = restTimeTotal / 11;
int timerStart; //millis of timer start
int lastLEDOn; //last led turned on
int initBrightness = 40;
int brightness = initBrightness;
long msElapsed;
bool breathingIn = true;

void setup() {
  // Tell b to get everything ready to go
  b.begin();
  b.allLedsOff();
  b.setBrightness(brightness);
  Serial.begin(9600);
  Particle.variable("led", lastLEDOn);
  Particle.variable("msElapsed", msElapsed);
  Particle.variable("status", status);
}

void reinit() {
  b.allLedsOff();
  if (brightness != 40) {
    brightness = initBrightness;
    b.setBrightness(initBrightness);
  }
  status == "initialized";
  lastLEDOn = 0;
}

void getToWorkOrRest(String newstatus) {
  reinit();
  timerStart = millis();
  b.allLedsOff();
  lastLEDOn = 0;
  status = newstatus;
  Particle.publish(status);
}

void loop(){
  msElapsed = (millis() - timerStart)/1000;

  if(b.buttonOn(1)) {
    //Let's do this!
    if (status == "initialized") {
      //start working
      getToWorkOrRest("worktime");
    }
  }

  if (b.buttonOn(2)) {
    //I was distracted or I am tired of resting
    if (status == "worktime") {
      long timeBeforeInterrupt = (millis() - timerStart);
      Particle.publish("interrupted", String(timeBeforeInterrupt));
      reinit();
    } else if (status == "resting") {
      getToWorkOrRest("worktime");
    }
  }

  if (b.buttonOn(3)) {
    //section completed - switching modes
    if (status == "worktimecomplete") {
      getToWorkOrRest("resttime");
    } else if (status == "restingcomplete"){
      getToWorkOrRest("worktime");
    }
  }

  if (b.buttonOn(4)) {
    //reset
    reinit();
    Particle.publish("reset");
  }

  if (status == "worktime") {
    Serial.println(String(msElapsed) + " of " + (lastLEDOn * workIncrementSeconds));
    if (msElapsed > workTimeTotal) {
      status = "worktimecomplete";
      Particle.publish("work time completed!");
    }
    else if (msElapsed > (lastLEDOn * workIncrementSeconds)) {
      if (lastLEDOn < 12) {
        lastLEDOn++;
        Particle.publish("led lit - " + String(lastLEDOn));
      }
      b.ledOn(lastLEDOn, 255, 0, 0);
    }
  } else if (status == "resttime") {
    Serial.println(String(msElapsed) + " of " + (lastLEDOn * restIncrementSeconds));
    if (msElapsed > workTimeTotal) {
      status = "resttimecomplete";
      Particle.publish("rest time completed!");
    }
    else if (msElapsed > (lastLEDOn * restIncrementSeconds)) {
      if (lastLEDOn < 12) {
        lastLEDOn++;
        Particle.publish("led lit - " + String(lastLEDOn));
      }
      b.ledOn(lastLEDOn, 0, 255, 0);
    }
  } else if (status == "worktimecomplete" || status == "resttimecomplete") {
    Serial.println(brightness);
    //waiting for mode change - leds should be breathing
    if (brightness >= 100 || brightness <= 0) {
      breathingIn = !breathingIn;
    }
    if (breathingIn) {
      brightness = brightness + 1;
    } else {
      brightness = brightness - 1;
    }
    b.setBrightness(brightness);
    b.allLedsOn(128, 128, 128);
  }

  delay(100);
}
