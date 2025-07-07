import paho.mqtt.client as mqtt
import serial
import time
import sys
import json

# --- 시리얼 포트 설정 ---
SERIAL_PORT = "/dev/cu.usbserial-1120"  # 아두이노 포트로 변경 (사용자 설정)
BAUD_RATE = 9600  # 아두이노와 동일한 보드레이트 설정

# --- MQTT 설정 ---
BROKER_ADDRESS = "10.150.2.255"
BROKER_PORT = 1883

# 모든 MQTT 통신에 사용할 단일 토픽
MQTT_COMMON_TOPIC = "stock/topic"

# --- 시리얼 연결 초기화 ---
ser = None 
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # 아두이노 초기화 대기
    print(f"Arduino에 시리얼 연결 성공: {SERIAL_PORT}")
except serial.SerialException as e:
    print(f"시리얼 연결 실패: {e}")
    sys.exit(1)

# --- MQTT 콜백 함수 ---
def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("MQTT 브로커에 성공적으로 연결되었습니다!")
        client.subscribe(MQTT_COMMON_TOPIC) 
        print(f"토픽 구독 시작: {MQTT_COMMON_TOPIC}")
    else:
        print(f"MQTT 브로커 연결 실패, 에러 코드: {reason_code}")
        if ser and ser.is_open:
            ser.close()
        sys.exit(1)

def on_message(client, userdata, msg):
    raw_payload = msg.payload.decode().strip()
    print(f"\n--- MQTT 메시지 수신 ---")
    print(f"토픽: {msg.topic}")
    print(f"원본 메시지: '{raw_payload}'") 
    print(f"---------------------------\n")

    try:
        data = json.loads(raw_payload) 

        if "sugar" in data: 
            print("JSON 데이터 파싱 완료 (디스펜서 명령):")
            for key, value in data.items():
                print(f"  {key}: {value}")
            print("---------------------------\n")

            value_to_send = str(data["sugar"]) 
            try:
                ser.write(value_to_send.encode())  
                print(f"아두이노로 전송 완료 ('sugar' 값): '{value_to_send}'")
            except serial.SerialException as e:
                print(f"아두이노로 전송 실패: {e}")
        elif "light_sensor" in data: 
            print("JSON 데이터 파싱 완료 (조도 센서 값 - 자체 발행):")
            print(f"  light_sensor: {data['light_sensor']}")
            print("---------------------------\n")
        else:
            print("오류: 알 수 없는 JSON 형식 또는 필수 키('sugar', 'light_sensor')가 없습니다.")

    except json.JSONDecodeError as e:
        print(f"오류: JSON 파싱 실패 - {e}")
        print(f"수신된 메시지가 올바른 JSON 형식이 아닙니다: '{raw_payload}'")
    except Exception as e:
        print(f"알 수 없는 오류 발생: {e}")

# --- MQTT 클라이언트 설정 ---
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2) 
client.on_connect = on_connect
client.on_message = on_message

# --- 브로커 연결 시도 ---
try:
    print(f"MQTT 브로커 '{BROKER_ADDRESS}:{BROKER_PORT}'에 연결 시도 중...")
    client.connect(BROKER_ADDRESS, BROKER_PORT, 60)
except Exception as e:
    print(f"MQTT 연결 실패: {e}")
    if ser and ser.is_open:
        ser.close()
    sys.exit(1)

# --- 네트워크 루프 시작 및 프로그램 유지 ---
client.loop_start() 

print("MQTT 메시지 수신/발신 대기 중입니다. 프로그램을 종료하려면 Ctrl+C를 누르세요.")

try:
    while True:
        if ser.in_waiting > 0:
            arduino_data = ser.readline().decode('utf-8').strip()
            if arduino_data:
                print(f"아두이노로부터 시리얼 수신: {arduino_data}")
                
                # 조도 센서 값 추출 및 JSON 형식으로 MQTT 발행
                try:
                    light_value = None # 발행할 조도 값 (초기화)

                    # 'CDS: [값]' 형태 처리 (아날로그 값)
                    if arduino_data.startswith("CDS: "):
                        light_value_str = arduino_data.replace("CDS: ", "")
                        light_value = int(light_value_str) 
                        print(f"파싱된 조도 아날로그 값: {light_value}")
                    # '조도 센서 상태: HIGH/LOW' 형태 처리 (디지털 상태)
                    elif arduino_data.startswith("조도 센서 상태: "):
                        light_state_str = arduino_data.replace("조도 센서 상태: ", "")
                        # HIGH/LOW를 1/0 또는 문자열 그대로 발행할 수 있습니다.
                        # 여기서는 "HIGH" 또는 "LOW" 문자열 그대로 발행합니다.
                        light_value = light_state_str 
                        print(f"파싱된 조도 디지털 상태: {light_value}")
                    
                    if light_value is not None: # 유효한 조도 값이 파싱되었다면 발행
                        # JSON 객체 생성
                        light_sensor_json = {"light_sensor": light_value}
                        
                        # JSON 문자열로 변환하여 발행
                        client.publish(MQTT_COMMON_TOPIC, json.dumps(light_sensor_json)) 
                        print(f"MQTT 발행 완료 (조도 센서): 토픽='{MQTT_COMMON_TOPIC}', 값='{json.dumps(light_sensor_json)}'")
                    else:
                        print(f"오류: 알 수 없는 아두이노 시리얼 데이터 형식 (발행하지 않음): {arduino_data}")

                except ValueError:
                    print(f"오류: 조도 센서 값이 숫자로 변환할 수 없습니다: '{arduino_data}'")
                except Exception as e:
                    print(f"조도 센서 값 처리 중 오류 발생: {e}")
        time.sleep(0.1) 
except KeyboardInterrupt:
    print("\n프로그램 종료 요청 감지. 연결을 해제합니다...")
finally:
    if client:
        client.loop_stop() 
        client.disconnect() 
        print("MQTT 클라이언트 연결 해제 완료.")
    if ser and ser.is_open:
        ser.close() 
        print("아두이노 시리얼 포트 닫기 완료.")
    print("프로그램이 종료되었습니다.")