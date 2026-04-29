// ============================================================
//  LINE FOLLOWER - FAR RIGHT SENSOR OVERSHOOT FIXED
//  BLACK = 0 (line), WHITE = 1 (surface)
// ============================================================

#define S1 A0   // Far Left
#define S2 A1   // Mid Left
#define S3 A2   // Center
#define S4 A3   // Mid Right
#define S5 A4   // Far Right

#define ENA 10
#define IN1  4
#define IN2  5
#define ENB  9
#define IN3  2
#define IN4  3

// ============================================================
//  PID
// ============================================================
float Kp = 18.0;
float Ki =  0.0;
float Kd = 12.0;

const int BASE_SPEED  = 65;
const int MAX_SPEED   = 120;
const int MIN_SPEED   = -120;

float lastError = 0;
float integral  = 0;
int   lastDir   = 1;

// ============================================================
void setup() {
  pinMode(S1, INPUT); pinMode(S2, INPUT); pinMode(S3, INPUT);
  pinMode(S4, INPUT); pinMode(S5, INPUT);
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  Serial.begin(9600);
}

// ============================================================
void loop() {
  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  // ── ALL BLACK = STOP ──────────────────────────────────────
  if (s1==0 && s2==0 && s3==0 && s4==0 && s5==0) {
    stopMotors();
    integral = 0; lastError = 0;
    return;
  }

  // ── ALL WHITE = lost line, spin to find it ────────────────
  if (s1==1 && s2==1 && s3==1 && s4==1 && s5==1) {
    integral = 0;
    if (lastDir < 0) moveMotors(-70, 70);
    else             moveMotors( 70,-70);
    return;
  }

  // ============================================================
  //  FAR RIGHT SENSOR FIX — THE MAIN FIX
  //
  //  Problem was: S5 black → correction too big → overshoot
  //  Fix:         S5 black → STOP left wheel completely
  //                        → RIGHT wheel slow reverse
  //                        → Robot gently pivots right
  //                        → Waits until S3 or S4 sees line
  //                        → Then goes back to normal PID
  // ============================================================

  // FAR RIGHT only — hardest right turn needed
  if (s5==0 && s4==1 && s3==1 && s2==1 && s1==1) {
    lastDir = 1;
    integral = 0;
    // Left wheel forward slow, Right wheel reverse slow = gentle right pivot
    moveMotors(60, -50);
    Serial.println("FAR RIGHT PIVOT");
    return;
  }

  // FAR RIGHT + MID RIGHT — still needs strong right turn
  if (s5==0 && s4==0 && s3==1 && s2==1 && s1==1) {
    lastDir = 1;
    integral = 0;
    moveMotors(70, -40);
    Serial.println("MID+FAR RIGHT PIVOT");
    return;
  }

  // FAR LEFT only — hardest left turn needed
  if (s1==0 && s2==1 && s3==1 && s4==1 && s5==1) {
    lastDir = -1;
    integral = 0;
    // Right wheel forward slow, Left wheel reverse slow = gentle left pivot
    moveMotors(-55, 70);
    Serial.println("FAR LEFT PIVOT");
    return;
  }

  // FAR LEFT + MID LEFT — still needs strong left turn
  if (s1==0 && s2==0 && s3==1 && s4==1 && s5==1) {
    lastDir = -1;
    integral = 0;
    moveMotors(-45, 75);
    Serial.println("MID+FAR LEFT PIVOT");
    return;
  }

  // ============================================================
  //  NORMAL PID — center sensors handle smooth turns
  // ============================================================
  float num = 0, den = 0;
  float w[5] = {-2.0, -1.0, 0.0, 1.0, 2.0};
  int   sv[5] = {s1, s2, s3, s4, s5};

  for (int i = 0; i < 5; i++) {
    if (sv[i] == 0) {
      num += w[i];
      den += 1.0;
    }
  }

  float error = (den > 0) ? (num / den) : lastError;

  if (error < -0.1) lastDir = -1;
  if (error >  0.1) lastDir =  1;

  integral += error;
  integral  = constrain(integral, -30, 30);

  float derivative = error - lastError;
  float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
  lastError = error;

  int leftSpeed  = BASE_SPEED + (int)correction;
  int rightSpeed = BASE_SPEED - (int)correction;

  leftSpeed  = constrain(leftSpeed,  MIN_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, MIN_SPEED, MAX_SPEED);

  moveMotors(leftSpeed, rightSpeed);

  Serial.print("E:"); Serial.print(error);
  Serial.print(" C:"); Serial.print((int)correction);
  Serial.print(" L:"); Serial.print(leftSpeed);
  Serial.print(" R:"); Serial.println(rightSpeed);
}

// ============================================================
void moveMotors(int L, int R) {
  if (L >= 0) { analogWrite(ENA,  L); digitalWrite(IN1,HIGH); digitalWrite(IN2,LOW);  }
  else        { analogWrite(ENA, -L); digitalWrite(IN1,LOW);  digitalWrite(IN2,HIGH); }
  if (R >= 0) { analogWrite(ENB,  R); digitalWrite(IN3,HIGH); digitalWrite(IN4,LOW);  }
  else        { analogWrite(ENB, -R); digitalWrite(IN3,LOW);  digitalWrite(IN4,HIGH); }
}

void stopMotors() {
  analogWrite(ENA,0); digitalWrite(IN1,LOW); digitalWrite(IN2,LOW);
  analogWrite(ENB,0); digitalWrite(IN3,LOW); digitalWrite(IN4,LOW);
}