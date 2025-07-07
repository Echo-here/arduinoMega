#include <Servo.h> // 서보 모터 라이브러리 포함
Servo sv;         // Servo 객체 생성

// 핀 정의
const int D0_PIN = 2;    // 조도 센서 D0 핀 (현재 서보 제어 로직에서는 사용하지 않음)
const int SERVO_PIN = 5; // 서보 모터 신호 핀

// 서보 모터의 열림/닫힘 각도 정의
const int SERVO_CLOSED_ANGLE = 0;   // 문이 닫힌 상태의 각도 (실제 디스펜서에 맞춰 조절)
const int SERVO_OPEN_ANGLE = 90;    // 문이 열린 상태의 각도 (실제 디스펜서에 맞춰 조절)

void setup() {
  // USB 시리얼 통신 시작 (Mac과의 통신용)
  // Python 스크립트의 BAUD_RATE와 동일하게 설정해야 합니다.
  Serial.begin(9600); 
  Serial.println("아두이노 메가 시작: 파우더 디스펜서 제어 대기 중");
  Serial.println("-------------------------------------------------");
  Serial.println("명령: 실수 값 (예: '1.0', '2.5', '3.0')을 받으면 해당 시간(초) 동안 열림");

  // 조도 센서 핀 설정 (현재 서보 제어 로직에서는 사용하지 않음)
  pinMode(D0_PIN, INPUT);

  // 서보 모터 설정
  sv.attach(SERVO_PIN); // 서보 모터를 핀에 연결
  sv.write(SERVO_CLOSED_ANGLE); // 초기 각도를 닫힌 상태로 설정
  delay(1000); // 서보 초기 이동 시간 대기
}

void loop() {
  // 조도 센서 상태 확인 및 USB 시리얼 모니터로 출력 (선택 사항)
  int cds_state = digitalRead(D0_PIN);
  Serial.print("조도 센서 상태: ");
  if (cds_state == HIGH) {
    Serial.println("HIGH");
  } else {
    Serial.println("LOW");
  }

  // USB 시리얼로부터 입력 확인 및 서보 제어
  if (Serial.available()) { // USB 시리얼 버퍼에 데이터가 있는지 확인
    // Python에서 보낸 첫 번째 값을 문자열로 읽고 실수로 변환
    String commandString = Serial.readStringUntil('\n'); // 개행 문자(\n)까지 문자열 읽기
    commandString.trim(); // 문자열 앞뒤 공백 제거 (Python에서 보낸 값에 공백이 있을 수 있으므로)

    Serial.print("USB 시리얼 수신 명령 (문자열): '");
    Serial.print(commandString);
    Serial.print("' -> ");

    float floatValue = commandString.toFloat(); // 수신된 문자열을 실수(float)로 변환
    int openDurationMs = 0; // 서보를 열어둘 시간 (밀리초)

    // 변환된 실수 값으로 서보 열림 시간 결정
    if (floatValue > 0) { // 양수 값인 경우에만 처리
        // 실수 값을 초 단위로 간주하고 밀리초로 변환 (예: 1.5 -> 1500ms)
        openDurationMs = (int)(floatValue * 1000); 
        Serial.print("서보 ");
        Serial.print(floatValue);
        Serial.println("초 동안 열림 (실수 값 기반)");
    } else {
        Serial.println("유효하지 않은 값 또는 0 이하의 값 수신");
    }

    // 유효한 열림 시간이 설정되었을 경우에만 서보 제어
    if (openDurationMs > 0) {
      sv.write(SERVO_OPEN_ANGLE);  // 서보 문을 열림 각도로 이동
      delay(openDurationMs);       // 지정된 시간 동안 유지
      sv.write(SERVO_CLOSED_ANGLE); // 서보 문을 닫힘 각도로 이동
      Serial.println("서보 닫힘");
    }
    
    // USB 시리얼 버퍼에 남은 데이터를 모두 비워줍니다.
    while (Serial.available()) {
      Serial.read();
    }
  }

  delay(200); // 루프 주기 조절
}