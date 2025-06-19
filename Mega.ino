#include <Servo.h>

const int ldrPin = 2;        // 디지털 조도 센서 DO → D2 핀
const int servoPin = 5;      // 서보 PWM 신호 → D5
bool lightDetected = false;

Servo myservo;

void setup() {
  Serial.begin(9600);        // 시리얼 시작 필수!
  pinMode(ldrPin, INPUT);
  myservo.attach(servoPin);
  myservo.write(0);          // 서보 시작 위치
}

void loop() {
  int light = digitalRead(ldrPin);  // 0 or 1 출력

  Serial.print("조도 센서 상태: "); Serial.println(light);  // 상태 확인용

  // 빛 감지 조건 (필요시 반대로 바꿔야 함)
  if (light == LOW) {  // 보통 LOW가 빛 감지인 경우가 많음
    if (!lightDetected) {
      Serial.println("빛 감지됨");
      lightDetected = true;
    }
  } else {
    lightDetected = false;
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    Serial.print("받은 명령: "); Serial.println(cmd);

    if (lightDetected) {
      if (cmd == "open") {
        myservo.write(90);
        Serial.println("서보: 열림");
      } else if (cmd == "close") {
        myservo.write(0);
        Serial.println("서보: 닫힘");
      } else {
        Serial.println("알 수 없는 명령");
      }
    } else {
      Serial.println("⚠ 조도 조건 미달 - 명령 무시됨");
    }
  }

  delay(200);
}
