#include <Servo.h>
Servo sv;

const int D0 = 2;
const int svp = 5;

void setup() {
  Serial.begin(9600);
  pinMode(D0, INPUT);

  sv.attach(svp);
  sv.write(0);
  delay(1000);
  }

void loop() {
  int cds_state = digitalRead(D0);

  //상태 확인(잘나옴)
  if (cds_state == HIGH) {
    Serial.println("HIGH");
  } else {
    Serial.println("LOW");
  }

//서브모터 UART 제어 확인(잘됨)
if (Serial.available()) {
  char receivedChar = Serial.read();

  if (receivedChar == '1') {
    sv.write(90); //시리얼에 1 넣으면 90도로 회전
  } else {
    sv.write(0); //다른값 들어오면 0도로 복귀
  }
  while(Serial.available()) {
    Serial.read();
  }
}
  delay(200);

  //모터 확인(잘됨)
  // sv.write(180);
  // delay(1000);
  // sv.write(0);
}
