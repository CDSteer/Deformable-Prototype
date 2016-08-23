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
float l_SensorValue = 0;
float l_dvdt = 0;
unsigned long lastTime = 0;
unsigned long speedLastTime = 0;
unsigned long dt = 100; // dt in milliseconds
float dvdt = 0;
float speed_dvdt = 0;
float sqweezeDvdt[3];
float pushDvdt[3];
float sqweezeForce[3];
float pushForce[3];
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

static const uint8_t analog_pins[] = {A3, A4, A5, A9, A10, A11};

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    pinMode(analog_pins[i], INPUT);
  }

  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A9, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);

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

  digitalWrite(A3, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  digitalWrite(A9, LOW);
  digitalWrite(A10, LOW);
  digitalWrite(A11, LOW);
}

void loop () {
  readSerial();
  readForce();
  //printForceValues();


    // sqweezeDvdt[0] = voltageRate(sqweezeForce[0]);

    // if (analogRead(A3) > 10){
    //   if (state != 2){
    //     // rotate(100, map(pushForce[0], 10, 300, .3, 1), e0_DIR_PIN, e0_STEP_PIN);
    //     count = count + 100;
    //     checkpos(count);
    //     readForce();
    //   }
    // } else if  (analogRead(A3) < 10) {
    //   if (state != 0){
    //     // rotate(-100, map(pushForce[0], 10, 300, .3, 1), e0_DIR_PIN, e0_STEP_PIN);
    //     if (count >= 100){
    //       count = count - 100;
    //     }
    //     checkpos(count);
    //     readForce();
    //   }
    // }

    // Serial.println(count);
    // Serial.println(state);

  // printForceValues();
  // if (pushDvdt[0]>0){
  //   Serial.println(pushDvdt[0]);
  //   readForceSet();
  //   printForceValues();
  // } else if (pushDvdt[0] == 0) {
  //   printForceValuesSet();
  // }

  pushDvdt[0] = voltageRate(pushForce[0]);
  pushDvdt[1] = voltageRate(analogRead(A9));
  pushDvdt[2] = voltageRate(pushForce[2]);

  if (pushDvdt[0]>0){
    //Serial.println(pushDvdt[0]);
    readForceSet0();
    printForceValues0();
  } else if (pushDvdt[0] == 0) {
    printForceValuesSet0();
  }

  if (pushDvdt[1]>0){
    // Serial.println(pushDvdt[1]);
    readForceSet1();
    printForceValues1();
  } else if (pushDvdt[1] == 0) {
    printForceValuesSet1();
  }

  if (pushDvdt[2]>0){
    readForceSet2();
    printForceValues2();
  } else if (pushDvdt[2] == 0) {
    printForceValuesSet2();
  }


  // Serial.println(pushDvdt[0]);
  // Serial.println(stop);
  //
  // if (pushDvdt[0] > 30 && revCount < 10){
  //   stop=false;
  //   if (revCount > 10){
  //     rotate(400, map(pushForce[0], 10, 300, .3, 1), e0_DIR_PIN, e0_STEP_PIN);
  //   } else {
  //     rotate(400, map(pushForce[0], 10, 300, .3, 1), e0_DIR_PIN, e0_STEP_PIN);
  //   }
  //   revCount++;
  // }
  // if (stop == true){
  //   if (revCount > 0 && pushDvdt[0] < 20 && pushDvdt[0] > 0) {
  //     if (revCount == 10){
  //       rotate(-400, map(pushForce[0], 10, 300, .3, 1), e0_DIR_PIN, e0_STEP_PIN);
  //     } else {
  //       rotate(-400, map(pushForce[0], 10, 300, .3, 1), e0_DIR_PIN, e0_STEP_PIN);
  //     }
  //     revCount--;
  //   }
  // }
  //
  // if (stop == false){
  //   if (pushDvdt[0] <= 0) {
  //     stop=true;
  //   }
  // }
  delay(100);

}

void readForceSet0(){
  pushForceSet[0] = analogRead(A4);
}
void readForceSet1(){
  pushForceSet[1] = analogRead(A9);
}
void readForceSet2(){
  pushForceSet[2] = analogRead(A11);
}

void readForce(){
  sqweezeForce[0] = analogRead(A3);
  pushForce[0] = analogRead(A4);
  sqweezeForce[1] = analogRead(A5);
  pushForce[1] = analogRead(A9);
  sqweezeForce[2] = analogRead(A10);
  pushForce[2] = analogRead(A11);
}

void readForceSet(){
  pushForceSet[0] = analogRead(A4);
  pushForceSet[1] = analogRead(A9);
  pushForceSet[2] = analogRead(A11);
}

void printForceValues0(){
  Serial.print("head");
  Serial.print(0);
  Serial.print(":");
  Serial.print(pushForce[0]);
  Serial.print(",");
  Serial.println(sqweezeForce[0]);
}

void printForceValues1(){
    Serial.print("head");
    Serial.print(1);
    Serial.print(":");
    Serial.print(pushForce[1]);
    Serial.print(",");
    Serial.println(sqweezeForce[1]);
}
void printForceValues2(){
    Serial.print("head");
    Serial.print(2);
    Serial.print(":");
    Serial.print(pushForce[2]);
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
    Serial.println(sqweezeForce[i]);
  }
}

void printForceValuesSet0(){
  Serial.print("head");
  Serial.print(0);
  Serial.print(":");
  Serial.print(pushForceSet[0]);
  Serial.print(",");
  Serial.println(sqweezeForce[0]);
}

void printForceValuesSet1(){
    Serial.print("head");
    Serial.print(1);
    Serial.print(":");
    Serial.print(pushForceSet[1]);
    Serial.print(",");
    Serial.println(sqweezeForce[1]);
}
void printForceValuesSet2(){
    Serial.print("head");
    Serial.print(2);
    Serial.print(":");
    Serial.print(pushForceSet[2]);
    Serial.print(",");
    Serial.println(sqweezeForce[2]);
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
      rotate(1000, 1, e0_DIR_PIN, e0_STEP_PIN);
      rotate(1000, 1, e1_DIR_PIN, e1_STEP_PIN);
      rotate(1000, 1, X_DIR_PIN, X_STEP_PIN);
    }
  }
  if (state == 2){
    if (mode == 0){
      switchMode(1);
      switchMode(0);
    }
    if (mode == 1){
      rotate(-1000, 1, e0_DIR_PIN, e0_STEP_PIN);
      rotate(-1000, 1, e1_DIR_PIN, e1_STEP_PIN);
      rotate(-1000, 1, X_DIR_PIN, X_STEP_PIN);
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

float voltageRate(int v_SensorValue){
  if (millis() - lastTime  >= dt) {  // wait for dt milliseconds
    lastTime = millis();

    Serial.print("SensorValue: ");
    Serial.println(v_SensorValue);
    Serial.print("Last voltage: ");
    Serial.println(l_SensorValue, 4);

    dvdt = (v_SensorValue - l_SensorValue) / (millis() - lastTime);
    l_SensorValue = v_SensorValue;
    return dvdt;

    Serial.print("dV/dt: ");
    Serial.println(dvdt);
  }
}

float map(float value, float istart, float istop, float ostart, float ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}
