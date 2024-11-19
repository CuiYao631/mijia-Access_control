/*
 * 自动手动摘机开锁模块
 * 1.门铃触发功能暂不能用[k1,ADC]
 * 2.手动摘机开锁模式，当J1摘机开关触发时，触发K2摘机
 * 3.自动摘机开锁模式，当J2触发时，触发K2摘机，延时1秒，再触发K3开锁，延时600毫秒再触发K3一次，以防开锁失败
*/

bool isNotify = false;
unsigned long lastTriggerTime = 30000;

// 门铃触发
#define K1 PA0
// 摘机触发
#define K2 PA1
// 开锁触发
#define K3 PA4
// ADC 采集
#define ADC PB1
// 手动摘机
#define J1 PB2
// 自动摘机
#define J2 PB0

unsigned long debounceDelay = 10;    // 防抖延迟，10毫秒
unsigned long lastDebounceTime1 = 0; // J1上次防抖时间
unsigned long lastDebounceTime2 = 0; // J2上次防抖时间
bool lastButtonState1 = LOW;         // J1上次按钮状态
bool buttonState1 = LOW;             // J1当前按钮状态
bool lastButtonState2 = LOW;         // J2上次按钮状态
bool buttonState2 = LOW;             // J2当前按钮状态

void setup() {
  Serial.begin(115200);
  pinMode(K1, OUTPUT);
  digitalWrite(K1, HIGH);
  analogReadResolution(12);

  pinMode(K2, OUTPUT);
  digitalWrite(K2, HIGH);

  pinMode(K3, OUTPUT);
  digitalWrite(K3, HIGH);

  pinMode(J1, INPUT);
  pinMode(J2, INPUT);
}

void loop() {
  if (analogRead(ADC) > 1) {
    if (millis() - lastTriggerTime >= 5000) {
      digitalWrite(K1, LOW);
      Serial.println("notify");
      lastTriggerTime = millis();
      delay(60);
    }
  } else {
    digitalWrite(K1, HIGH);
  }

  // 检测 J1 的数字电平并进行防抖处理
  bool reading1 = digitalRead(J1);
  if (reading1 != lastButtonState1) {
    lastDebounceTime1 = millis();
  }
  if ((millis() - lastDebounceTime1) > debounceDelay) {
    if (reading1 != buttonState1) {
      buttonState1 = reading1;
      if (buttonState1 == HIGH) {
        digitalWrite(K2, LOW);
        Serial.println("J1 LOW");
      } else {
        digitalWrite(K2, HIGH);
        Serial.println("J1 HIGH");
      }
    }
  }
  lastButtonState1 = reading1;

  // 检测 J2 的数字电平并进行防抖处理
  bool reading2 = digitalRead(J2);
  if (reading2 != lastButtonState2) {
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (reading2 != buttonState2) {
      buttonState2 = reading2;
      if (buttonState2 == LOW) {
        // 操作 K2
        digitalWrite(K2, LOW);
        Serial.println("J2 LOW, K2 triggered");
        delay(1000); // 延迟 1000 毫秒

        // 第一次操作 K3
        digitalWrite(K3, LOW);
        Serial.println("K3 LOW (1)");
        delay(60); // 触发时间
        digitalWrite(K3, HIGH);
        Serial.println("K3 HIGH (1)");

        delay(600); // K3 两次触发之间的间隔

        // 第二次操作 K3
        digitalWrite(K3, LOW);
        Serial.println("K3 LOW (2)");
        delay(60); // 触发时间
        digitalWrite(K3, HIGH);
        Serial.println("K3 HIGH (2)");
      } else {
        digitalWrite(K2, HIGH);
        Serial.println("J2 HIGH");
      }
    }
  }
  lastButtonState2 = reading2;

  delay(50);
}