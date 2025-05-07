/*
 * 自动手动摘机开锁模块
 * 1.门铃触发功能暂不能用[k1,ADC]
 * 2.手动摘机开锁模式，当J1摘机开关触发时，触发K2摘机
 * 3.自动摘机开锁模式，当J2触发时，触发K2摘机，延时1秒，再触发K3开锁，延时600毫秒再触发K3一次，以防开锁失败
*/

// 引脚定义 - 使用更具描述性的名称
#define PIN_DOORBELL PA0      // 门铃触发 K1
#define PIN_PICKUP   PA1      // 摘机触发 K2
#define PIN_UNLOCK   PA4      // 开锁触发 K3
#define PIN_ADC      PB1      // ADC 采集
#define PIN_MANUAL   PB2      // 手动摘机 J1
#define PIN_AUTO     PB0      // 自动摘机 J2

// 常量定义 - 将魔术数字替换为有意义的常量
const unsigned long DEBOUNCE_DELAY = 10;       // 防抖延迟，10毫秒
const unsigned long DOORBELL_INTERVAL = 5000;  // 门铃触发最小间隔时间 (5秒)
const unsigned long UNLOCK_DELAY = 1000;       // 摘机到开锁的延迟时间 (1秒)
const unsigned long RELAY_TRIGGER_TIME = 60;   // 继电器触发持续时间 (60毫秒)
const unsigned long UNLOCK_RETRY_DELAY = 600;  // 开锁重试间隔 (600毫秒)
const unsigned long LOOP_DELAY = 50;           // 主循环延迟时间 (50毫秒)
const int ADC_THRESHOLD = 1;                   // ADC触发阈值

// 状态变量
bool isNotify = false;
unsigned long lastDoorbellTriggerTime = 30000;

// 按钮状态变量
unsigned long lastDebounceTimeManual = 0;     // J1上次防抖时间
unsigned long lastDebounceTimeAuto = 0;       // J2上次防抖时间
bool lastButtonStateManual = LOW;             // J1上次按钮状态
bool buttonStateManual = LOW;                 // J1当前按钮状态
bool lastButtonStateAuto = LOW;               // J2上次按钮状态
bool buttonStateAuto = LOW;                   // J2当前按钮状态

// 初始化函数
void setup() {
  Serial.begin(115200);
  
  // 初始化输出引脚
  pinMode(PIN_DOORBELL, OUTPUT);
  digitalWrite(PIN_DOORBELL, HIGH);
  
  pinMode(PIN_PICKUP, OUTPUT);
  digitalWrite(PIN_PICKUP, HIGH);
  
  pinMode(PIN_UNLOCK, OUTPUT);
  digitalWrite(PIN_UNLOCK, HIGH);
  
  // 初始化输入引脚
  pinMode(PIN_MANUAL, INPUT);
  pinMode(PIN_AUTO, INPUT);
  
  // 设置ADC分辨率
  analogReadResolution(12);
}

// 处理门铃功能
void handleDoorbell() {
  if (analogRead(PIN_ADC) > ADC_THRESHOLD) {
    // 检查是否已经过了最小触发间隔
    if (millis() - lastDoorbellTriggerTime >= DOORBELL_INTERVAL) {
      digitalWrite(PIN_DOORBELL, LOW);
      Serial.println("门铃触发");
      lastDoorbellTriggerTime = millis();
      delay(RELAY_TRIGGER_TIME);
      digitalWrite(PIN_DOORBELL, HIGH);
    }
  }
}

// 处理手动摘机开关
void handleManualPickup() {
  bool reading = digitalRead(PIN_MANUAL);
  
  // 检测状态变化，重置防抖计时器
  if (reading != lastButtonStateManual) {
    lastDebounceTimeManual = millis();
  }
  
  // 防抖逻辑
  if ((millis() - lastDebounceTimeManual) > DEBOUNCE_DELAY) {
    // 状态确实发生了变化
    if (reading != buttonStateManual) {
      buttonStateManual = reading;
      
      // 处理按钮按下状态
      if (buttonStateManual == HIGH) {
        digitalWrite(PIN_PICKUP, LOW);
        Serial.println("手动摘机触发");
      } else {
        digitalWrite(PIN_PICKUP, HIGH);
        Serial.println("手动摘机释放");
      }
    }
  }
  
  lastButtonStateManual = reading;
}

// 触发开锁动作
void triggerUnlock() {
  digitalWrite(PIN_UNLOCK, LOW);
  Serial.println("触发开锁");
  delay(RELAY_TRIGGER_TIME);
  digitalWrite(PIN_UNLOCK, HIGH);
}

// 处理自动摘机开锁
void handleAutoPickup() {
  bool reading = digitalRead(PIN_AUTO);
  
  // 检测状态变化，重置防抖计时器
  if (reading != lastButtonStateAuto) {
    lastDebounceTimeAuto = millis();
  }
  
  // 防抖逻辑
  if ((millis() - lastDebounceTimeAuto) > DEBOUNCE_DELAY) {
    // 状态确实发生了变化
    if (reading != buttonStateAuto) {
      buttonStateAuto = reading;
      
      // 处理按钮按下状态 (注意此引脚是低电平触发)
      if (buttonStateAuto == LOW) {
        // 触发摘机
        digitalWrite(PIN_PICKUP, LOW);
        Serial.println("自动摘机触发");
        
        // 延时后触发开锁
        delay(UNLOCK_DELAY);
        
        // 第一次开锁尝试
        triggerUnlock();
        
        // 延时后进行第二次开锁尝试
        delay(UNLOCK_RETRY_DELAY);
        triggerUnlock();
      } else {
        digitalWrite(PIN_PICKUP, HIGH);
        Serial.println("自动摘机释放");
      }
    }
  }
  
  lastButtonStateAuto = reading;
}

// 主循环
void loop() {
  // 处理门铃功能
  handleDoorbell();
  
  // 处理手动摘机
  handleManualPickup();
  
  // 处理自动摘机开锁
  handleAutoPickup();
  
  // 小延时，减少CPU占用
  delay(LOOP_DELAY);
}