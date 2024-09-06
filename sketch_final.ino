#include <Servo.h>  // 서보모터 제어를 위한 라이브러리

// Define pins for stepper motor 1 (기존 스탭 모터)
const int stepperPin1 = 7;   // IN1 on the ULN2003
const int stepperPin2 = 11;  // IN2 on the ULN2003
const int stepperPin3 = 12;  // IN3 on the ULN2003
const int stepperPin4 = 13;  // IN4 on the ULN2003
const int keypadPin = A0;    // Analog pin for keypad

// Define pins for linear motor
const int linearMotorPin1 = 9;   // N1 on the L298N
const int linearMotorPin2 = 8;  // N2 on the L298N
const int linearMotorEnable = 10; // Enable pin on the L298N

// Define pins for stepper motor 2 (새로운 스탭 모터)
const int stepper2Pin1 = 2;
const int stepper2Pin2 = 3;
const int stepper2Pin3 = 4;
const int stepper2Pin4 = 5;

// Define pin for servo motor
const int servoPin = 6;

// Define analog values for each button
const int SW1_VALUE = 0;     // A구역 (0~90도)
const int SW2_VALUE = 140;   // B구역 (90~180도)
const int SW3_VALUE = 320;   // D구역 (270~360도)
const int SW4_VALUE = 500;   // C구역 (180~270도)

// Define the step sequence for the 28BYJ-48 stepper motor
const int steps[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

// Define the number of steps per revolution for 28BYJ-48
const int stepsPerRevolution = 4096;  // 4096 steps for 360 degrees
const int stepsPerDegree = stepsPerRevolution / 360;  // Steps per degree

int currentStep = 0;  // Track the current step
int currentAngle = 45;  // Start at 45 degrees

Servo servoMotor;  // 서보모터 객체 생성

void setup() {
  // Set stepper motor pins as outputs
  pinMode(stepperPin1, OUTPUT);
  pinMode(stepperPin2, OUTPUT);
  pinMode(stepperPin3, OUTPUT);
  pinMode(stepperPin4, OUTPUT);

  // Set linear motor pins as outputs
  pinMode(linearMotorPin1, OUTPUT);
  pinMode(linearMotorPin2, OUTPUT);
  pinMode(linearMotorEnable, OUTPUT);

  // Set stepper motor 2 pins as outputs
  pinMode(stepper2Pin1, OUTPUT);
  pinMode(stepper2Pin2, OUTPUT);
  pinMode(stepper2Pin3, OUTPUT);
  pinMode(stepper2Pin4, OUTPUT);

  // Attach servo motor to its pin
  servoMotor.attach(servoPin);

  // Initialize serial communication for debugging
  Serial.begin(9600);
}

void loop() {
  int keypadValue = analogRead(keypadPin);

  // Print the keypad value for debugging
  Serial.println(keypadValue);

  // Check if SW1 is pressed (A구역)
  if (keypadValue > SW1_VALUE - 10 && keypadValue < SW1_VALUE + 10) {
    rotateToAngle(45); // Rotate to 45 degrees (A구역)
  }
  // Check if SW2 is pressed (B구역)
  else if (keypadValue > SW2_VALUE - 10 && keypadValue < SW2_VALUE + 10) {
    rotateToAngle(135); // Rotate to 135 degrees (B구역)
  }
  // Check if SW3 is pressed (D구역)
  else if (keypadValue > SW3_VALUE - 10 && keypadValue < SW3_VALUE + 10) {
    rotateToAngle(315); // Rotate to 315 degrees (D구역)
  }
  // Check if SW4 is pressed (C구역)
  else if (keypadValue > SW4_VALUE - 10 && keypadValue < SW4_VALUE + 10) {
    rotateToAngle(225); // Rotate to 225 degrees (C구역)
  }

  // Small delay to avoid bouncing issues
  delay(50);
}

void rotateToAngle(int targetDegree) {
  // Calculate the angle to move
  int stepsToMove = (targetDegree - currentAngle) * stepsPerDegree;

  if (stepsToMove > 0) {
    // Rotate forward
    for (int i = 0; i < stepsToMove; i++) {
      currentStep = (currentStep + 1) % stepsPerRevolution;
      setStepperStep(currentStep % 8);
      delay(2);
    }
  } else {
    // Rotate backward
    stepsToMove = -stepsToMove;
    for (int i = 0; i < stepsToMove; i++) {
      currentStep = (currentStep - 1 + stepsPerRevolution) % stepsPerRevolution;
      setStepperStep(currentStep % 8);
      delay(2);
    }
  }

  // Update current angle to the target degree
  currentAngle = targetDegree; // Update the angle to the new position
  
  // Delay 2 seconds before additional motor actions
  delay(2000);

  // Perform additional motor actions
  moveLinearMotor();
  rotateStepperMotor2();
  rotateServoMotor();
  
  // 추가 동작: KGU-3429 모터 정방향 회전
  moveLinearMotorReverse();
  rotateStepperMotor2Reverse();
  rotateServoMotorReverse();
}

// 스텝 모터 단계를 설정하는 함수
void setStepperStep(int stepIndex) {
  digitalWrite(stepperPin1, steps[stepIndex][0]);
  digitalWrite(stepperPin2, steps[stepIndex][1]);
  digitalWrite(stepperPin3, steps[stepIndex][2]);
  digitalWrite(stepperPin4, steps[stepIndex][3]);
}

// 리니어 모터를 정방향으로 움직이는 함수
void moveLinearMotor() {
  digitalWrite(linearMotorPin1, HIGH);
  digitalWrite(linearMotorPin2, LOW);
  digitalWrite(linearMotorEnable, HIGH);
  delay(3000); // 리니어 모터의 동작 시간 설정
  digitalWrite(linearMotorEnable, LOW);
}

// KGU-3429 모터를 역방향으로 움직이는 함수
void moveLinearMotorReverse() {
  digitalWrite(linearMotorPin1, LOW);
  digitalWrite(linearMotorPin2, HIGH);
  digitalWrite(linearMotorEnable, HIGH);
  delay(3000); // 리니어 모터의 역방향 동작 시간 설정
  digitalWrite(linearMotorEnable, LOW);
}

// 두 번째 스텝 모터를 정방향으로 회전하는 함수
void rotateStepperMotor2() {
  int stepsToMove = 90 * stepsPerDegree;  // 90 degrees

  for (int i = 0; i < stepsToMove; i++) {
    setStepper2Step(i % 8);
    delay(2);
  }
}

// 두 번째 스텝 모터를 역방향으로 회전하는 함수
void rotateStepperMotor2Reverse() {
  int stepsToMove = 90 * stepsPerDegree;  // 90 degrees

  for (int i = stepsToMove; i > 0; i--) {
    setStepper2Step((i - 1) % 8);
    delay(2);
  }
}

// 두 번째 스텝 모터의 단계를 설정하는 함수
void setStepper2Step(int stepIndex) {
  digitalWrite(stepper2Pin1, steps[stepIndex][0]);
  digitalWrite(stepper2Pin2, steps[stepIndex][1]);
  digitalWrite(stepper2Pin3, steps[stepIndex][2]);
  digitalWrite(stepper2Pin4, steps[stepIndex][3]);
}

// 서보 모터를 180도 회전시키는 함수
void rotateServoMotor() {
  servoMotor.write(180); // 서보모터 움직이는 각도 조절
  delay(1000);  // Allow time for the servo to move
}

// 서보 모터를 90도 역방향으로 회전시키는 함수
void rotateServoMotorReverse() {
  servoMotor.write(90); // 서보모터를 90도로 회전
  delay(1000);  // Allow time for the servo to move
}