#include <AnalogSmooth.h>
#define LED_PIN 13

#define X_STEP_PIN 54
#define X_DIR_PIN 55
#define X_ENABLE_PIN 38
#define X_MIN_PIN  3
#define X_MAX_PIN  2

#define e0_STEP_PIN 26
#define e0_DIR_PIN 28
#define e0_ENABLE_PIN 24

#define e1_STEP_PIN 36
#define e1_DIR_PIN 34
#define e1_ENABLE_PIN 30

int i, lastRead, force, motorSpeed;
float l_SensorValue[3];
float l_SensorValueS[3];
float l_dvdt = 0;
unsigned long lastTime[3];
unsigned long lastTimeS[3];
unsigned long speedLastTime = 0;
unsigned long dt = 100; // dt in milliseconds
float speed_dvdt = 0;
float sqweezeDvdt[3];
float pushDvdt[3];
float sqweezeForce[3];
float pushForce[3];
float sqweezeForceCal[3];
float pushForceCal[3];
int revCount;
int count;
int curentPos = 1300;
int hPos = 2600;
int mPos = 1300;
int lPos = 0;
int state = 1;
bool stop = false;
int h1State = 1;
int h2State = 1;
int h3State = 1;
float pushForceSet[3];
float sqweezeForceSet[3];
bool grasp = false;
static const uint8_t analog_pins[] = {A3, A4, A5, A9, A10, A11};

// Defaults to window size 10
AnalogSmooth as0S = AnalogSmooth();
AnalogSmooth as1S = AnalogSmooth();
AnalogSmooth as2S = AnalogSmooth();
AnalogSmooth as0P = AnalogSmooth();
AnalogSmooth as1P = AnalogSmooth();
AnalogSmooth as2P = AnalogSmooth();
// Window size can range from 1 - 100
AnalogSmooth as100 = AnalogSmooth(100);



void setup() {
  Serial.begin(115200);
  pinMode(e0_STEP_PIN, OUTPUT);
  pinMode(e0_DIR_PIN, OUTPUT);
  pinMode(e0_ENABLE_PIN, OUTPUT);

  pinMode(e1_STEP_PIN, OUTPUT);
  pinMode(e1_DIR_PIN, OUTPUT);
  pinMode(e1_ENABLE_PIN, OUTPUT);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);

  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(e0_ENABLE_PIN, LOW);
  digitalWrite(e1_ENABLE_PIN, LOW);

  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A9, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);

  digitalWrite(A3, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  digitalWrite(A9, LOW);
  digitalWrite(A10, LOW);
  digitalWrite(A11, LOW);

  unsigned long starttime = millis();
  unsigned long endtime = starttime;
  while ((endtime - starttime) <= 5000) {
    readForce();
    for (int i = 0; i < 10; i++) {
      sqweezeForceCal[0] = as0S.smooth(analogRead(A3));
      pushForceCal[0] = as0P.smooth(analogRead(A4));
      sqweezeForceCal[1] = as1S.smooth(analogRead(A5));
      pushForceCal[1] = as1P.smooth(analogRead(A9));
      sqweezeForceCal[2] = as2S.smooth(analogRead(A10));
      pushForceCal[2] = as2P.smooth(analogRead(A11));
    }
    endtime = millis();
  }
}

void loop () {
  readSerial();
  readForce();
  // printForceValues();

  if (state == 0){
    sqweezeDvdt[0] = voltageRateSqweeze(0);
    Serial.println(sqweezeDvdt[0]);
    if  (sqweezeDvdt[0] > 15){
      sqweezeForceSet[0] = as0S.smooth(analogRead(A3))-sqweezeForceCal[0];
      sqweezeForceSet[1] = 0;
      sqweezeForceSet[2] = 0;
      printForceValuesS0();
    } else if (sqweezeDvdt[0] <= 4) {
      printForceValuesSet();
    }
    sqweezeDvdt[1] = voltageRateSqweeze(1);
    Serial.println(sqweezeDvdt[1]);
    if (sqweezeDvdt[1] > 15){
      sqweezeForceSet[1] = as1S.smooth(analogRead(A5))-sqweezeForceCal[1];
      sqweezeForceSet[0] = 0;
      sqweezeForceSet[2] = 0;
      printForceValuesS1();
    } else if (sqweezeDvdt[1] <= 4) {
      printForceValuesSet();
    }
    sqweezeDvdt[2] = voltageRateSqweeze(2);
    Serial.println(sqweezeDvdt[2]);
    if (sqweezeDvdt[2] > 15){
      sqweezeForceSet[2] = as2S.smooth(analogRead(A10))-sqweezeForceCal[2];
      sqweezeForceSet[1] = 0;
      sqweezeForceSet[0] = 0;
      printForceValuesS2();
    } else if (sqweezeDvdt[2] <= 4) {
      printForceValuesSet();
    }
  }

  if (state == 2){
    pushDvdt[0] = voltageRate(0);
    Serial.println(pushDvdt[0]);
    if (pushDvdt[0] > 15){
      readForceSet();
      printForceValues0();
    } else if (pushDvdt[0] <= 4) {
      printForceValuesSet();
    }
    pushDvdt[1] = voltageRate(1);
    Serial.println(pushDvdt[1]);
    if (pushDvdt[1] > 15){
      readForceSet();
      printForceValues1();
    } else if (pushDvdt[1] <= 4) {
      printForceValuesSet();
    }
    pushDvdt[2] = voltageRate(2);
    Serial.println(pushDvdt[2]);
    if (pushDvdt[2] > 15){
      readForceSet();
      printForceValues2();
    } else if (pushDvdt[2] <= 4) {
      printForceValuesSet();
    }
  }
  delay(100);
}

void readForce(){
  sqweezeForce[0] = as0S.smooth(analogRead(A3))-sqweezeForceCal[0];
  pushForce[0] = as0P.smooth(analogRead(A4))-pushForceCal[0];
  sqweezeForce[1] = as1S.smooth(analogRead(A5))-sqweezeForceCal[1];
  pushForce[1] = as1P.smooth(analogRead(A9))-pushForceCal[1];
  sqweezeForce[2] = as2S.smooth(analogRead(A10))-sqweezeForceCal[2];
  pushForce[2] = as2P.smooth(analogRead(A11))-pushForceCal[2];
}

void readForceSet(){
  pushForceSet[0] = as0P.smooth(analogRead(A4));
  pushForceSet[1] = as1P.smooth(analogRead(A9));
  pushForceSet[2] = as2P.smooth(analogRead(A11));
}

void readForceSetS(){
  sqweezeForce[0] = as0S.smooth(analogRead(A3))-sqweezeForceCal[0];
  sqweezeForce[1] = as0S.smooth(analogRead(A5))-sqweezeForceCal[1];
  sqweezeForce[2] = as0S.smooth(analogRead(A10))-sqweezeForceCal[2];
}

void printForceValues0(){
  Serial.print("head");
  Serial.print(0);
  Serial.print(":");
  Serial.print(pushForce[0]);
  Serial.print(",");
  Serial.println(sqweezeForceSet[0]);
}

void printForceValues1(){
    Serial.print("head");
    Serial.print(1);
    Serial.print(":");
    Serial.print(pushForce[1]);
    Serial.print(",");
    Serial.println(sqweezeForceSet[1]);
}
void printForceValues2(){
    Serial.print("head");
    Serial.print(2);
    Serial.print(":");
    Serial.print(pushForce[2]);
    Serial.print(",");
    Serial.println(sqweezeForceSet[2]);
}

void printForceValuesS0(){
  Serial.print("head");
  Serial.print(0);
  Serial.print(":");
  Serial.print(pushForceSet[0]);
  Serial.print(",");
  Serial.println(sqweezeForce[0]);
}

void printForceValuesS1(){
    Serial.print("head");
    Serial.print(1);
    Serial.print(":");
    Serial.print(pushForceSet[1]);
    Serial.print(",");
    Serial.println(sqweezeForce[1]);
}
void printForceValuesS2(){
    Serial.print("head");
    Serial.print(2);
    Serial.print(":");
    Serial.print(pushForceSet[2]);
    Serial.print(",");
    Serial.println(sqweezeForce[2]);
}

void printForceValuesSet(){
  for (i = 0; i<3; i++){
    Serial.print("head");
    Serial.print(i);
    Serial.print(":");
    Serial.print(pushForceSet[i]);
    Serial.print(",");
    Serial.println(sqweezeForceSet[i]);
  }
}

void printForceValuesSet0(){
  Serial.print("head");
  Serial.print(0);
  Serial.print(":");
  Serial.print(pushForceSet[0]);
  Serial.print(",");
  Serial.println(sqweezeForceSet[0]);
}

void printForceValuesSet1(){
    Serial.print("head");
    Serial.print(1);
    Serial.print(":");
    Serial.print(pushForceSet[1]);
    Serial.print(",");
    Serial.println(sqweezeForceSet[1]);
}
void printForceValuesSet2(){
    Serial.print("head");
    Serial.print(2);
    Serial.print(":");
    Serial.print(pushForceSet[2]);
    Serial.print(",");
    Serial.println(sqweezeForceSet[2]);
}

void printForceValuesSetS0(){
  Serial.print("head");
  Serial.print(0);
  Serial.print(":");
  Serial.print(pushForce[0]);
  Serial.print(",");
  Serial.println(sqweezeForceSet[0]);
}

void printForceValuesSetS1(){
    Serial.print("head");
    Serial.print(1);
    Serial.print(":");
    Serial.print(pushForce[1]);
    Serial.print(",");
    Serial.println(sqweezeForceSet[1]);
}
void printForceValuesSetS2(){
    Serial.print("head");
    Serial.print(2);
    Serial.print(":");
    Serial.print(pushForce[2]);
    Serial.print(",");
    Serial.println(sqweezeForceSet[2]);
}

void printForceValues(){
  for (i = 0; i<3; i++){
    Serial.print("head");
    Serial.print(i);
    Serial.print(":");
    Serial.print(pushForce[i]);
    Serial.print(",");
    Serial.println(sqweezeForce[i]);
  }
}

void switchMode(int mode){
  if (state == 0){
    if (mode == 1){
      rotate(2200, 1, e0_DIR_PIN, e0_STEP_PIN);
      rotate(2200, 1, e1_DIR_PIN, e1_STEP_PIN);
      rotate(2200, 1, X_DIR_PIN, X_STEP_PIN);
    }
    if (mode == 2){
      switchMode(1);
      switchMode(2);
    }
  }
  if (state == 1){
    if (mode == 0){
      rotate(-2200, 1, e0_DIR_PIN, e0_STEP_PIN);
      rotate(-2200, 1, e1_DIR_PIN, e1_STEP_PIN);
      rotate(-2200, 1, X_DIR_PIN, X_STEP_PIN);
    }
    if (mode == 2){
      rotate(700, 1, e0_DIR_PIN, e0_STEP_PIN);
      rotate(700, 1, e1_DIR_PIN, e1_STEP_PIN);
      rotate(900, 1, X_DIR_PIN, X_STEP_PIN);
    }
  }
  if (state == 2){
    if (mode == 0){
      switchMode(1);
      switchMode(0);
    }
    if (mode == 1){
      rotate(-700, 1, e0_DIR_PIN, e0_STEP_PIN);
      rotate(-700, 1, e1_DIR_PIN, e1_STEP_PIN);
      rotate(-900, 1, X_DIR_PIN, X_STEP_PIN);
    }
  }
  state = mode;
  //setCurentPos(mode);
}
void setCurentPos(int mode){
  switch (mode) {
    case 0:
      curentPos = 2600;
      break;
    case 1:
      curentPos = 1300;
      break;
    case 2:
      curentPos = 0;
      break;
  }
}
void checkpos(int pos){
  switch (pos) {
    case 2600:
      switchMode(0);
      break;
    case 1300:
      switchMode(1);
      break;
    case 0:
      switchMode(2);
      break;
  }
}

void readSerial(){
  if (Serial.available()) {
    char ch = Serial.read();
    switch(ch) {
      case 'h':
        switchMode(0);
        break;
      case 'm':
        switchMode(1);
        break;
      case 'l':
        switchMode(2);
        break;
      case 's':
        rotate(-100, 1, e0_DIR_PIN, e0_STEP_PIN);
        state = 1;
        break;
      case 'a':
        rotate(100, 1, e0_DIR_PIN, e0_STEP_PIN);
        state = 1;
        break;
      case 'q':
        rotate(-100, 1, e1_DIR_PIN, e1_STEP_PIN);
        state = 1;
        break;
      case 'w':
        rotate(100, 1, e1_DIR_PIN, e1_STEP_PIN);
        state = 1;
        break;
      case 'z':
        rotate(-100, 1, X_DIR_PIN, X_STEP_PIN);
        state = 1;
        break;
      case 'x':
        rotate(100, 1, X_DIR_PIN, X_STEP_PIN);
        state = 1;
        break;
    }
  }
}

void rotate(int steps, float speed, int drivePin, int stepPin){
  //rotate a specific number of microsteps (8 microsteps per step) - (negitive for reverse movement)
  //speed is any number from .01 -> 1 with 1 being fastest - Slower is stronger
  int dir = (steps > 0)? HIGH:LOW;
  steps = abs(steps);
  digitalWrite(drivePin, dir);
  float usDelay = (1/speed) * 70;

  for(int i=0; i < steps; i++){
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(usDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(usDelay);
  }
}

void rotateDeg(float deg, float speed, int drivePin, int stepPin){
  //rotate a specific number of degrees (negitive for reverse movement)
  //speed is any number from .01 -> 1 with 1 being fastest - Slower is stronger
  int dir = (deg > 0)? HIGH:LOW;
  digitalWrite(drivePin,dir);
  int steps = abs(deg)*(1/0.225);
  float usDelay = (1/speed) * 70;

  for(int i=0; i < steps; i++){
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(usDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(usDelay);
  }
}

float voltageRateSqweeze(int v_SensorValue){
  float dvdt = 0;
  if (millis() - lastTimeS[v_SensorValue]  >= dt) {  // wait for dt milliseconds
    lastTimeS[v_SensorValue] = millis();

    Serial.print("SensorValue: ");
    Serial.println(sqweezeForce[v_SensorValue]);
    Serial.print("Last voltage: ");
    Serial.println(l_SensorValueS[v_SensorValue], 4);

    dvdt = (sqweezeForce[v_SensorValue] - l_SensorValueS[v_SensorValue]) / (millis() - sqweezeForce[v_SensorValue]);
    l_SensorValueS[v_SensorValue] = sqweezeForce[v_SensorValue];
    // if (dvdt > 1){
    //   Serial.print("dV/dt: ");
    //   Serial.println(dvdt*100000);
    // }
    return dvdt*100000;
  }
}

float voltageRate(int v_SensorValue){
  float dvdt = 0;
  if (millis() - lastTime[v_SensorValue]  >= dt) {  // wait for dt milliseconds
    lastTime[v_SensorValue] = millis();
    Serial.print("SV: ");
    Serial.println(pushForce[v_SensorValue]);
    Serial.print("LV: ");
    Serial.println(l_SensorValue[v_SensorValue], 4);

    dvdt = (pushForce[v_SensorValue] - l_SensorValue[v_SensorValue]) / (millis() - pushForce[v_SensorValue]);
    l_SensorValue[v_SensorValue] = pushForce[v_SensorValue];
    // if (dvdt > 1){
    //   Serial.print("dV/dt: ");
    //   Serial.println(dvdt*100000);
    // }
    return dvdt*100000;
  }
}

float map(float value, float istart, float istop, float ostart, float ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}
