// IR Sensor Pins
#define S1 A0
#define S2 A1
#define S3 A2
#define S4 A3
#define S5 A4

// Motor Pins
#define ENA 10
#define IN1 4
#define IN2 5
#define ENB 9
#define IN3 2
#define IN4 3

int baseSpeed = 80;
int maxSpeed = 100;

void setup() {
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  // Print sensor values
  Serial.print(s1); Serial.print(" ");
  Serial.print(s2); Serial.print(" ");
  Serial.print(s3); Serial.print(" ");
  Serial.print(s4); Serial.print(" ");
  Serial.println(s5);

  // -------- STRICT CONDITIONS --------

  // ALL WHITE → STOP
  if (s1==1 && s2==1 && s3==1 && s4==1 && s5==1) {
    moveMotors(0, 0);
    return;
  }

  // ALL BLACK → STOP
  if (s1==0 && s2==0 && s3==0 && s4==0 && s5==0) {
    moveMotors(0, 0);
    return;
  }

  // -------- PID LOGIC --------
  // Assuming BLACK = 0

  int error = 0;

  if (s1 == 0) error -= 4;
  if (s2 == 0) error -= 2;
  if (s3 == 0) error += 0;
  if (s4 == 0) error += 2;
  if (s5 == 0) error += 4;

  int correction = error * 40;

  int leftSpeed = baseSpeed - correction;
  int rightSpeed = baseSpeed + correction;

  // Limit speed
  leftSpeed = constrain(leftSpeed, 0, maxSpeed);
  rightSpeed = constrain(rightSpeed, 0, maxSpeed);

  moveMotors(leftSpeed, rightSpeed);
}

// Motor Control Function
void moveMotors(int leftSpeed, int rightSpeed) {
  // Left Motor
  analogWrite(ENA, leftSpeed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Right Motor
  analogWrite(ENB, rightSpeed);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}