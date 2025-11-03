#include <Arduino.h>
#include <DHT.h>

// Hardware pins
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define TRIG_PIN 3
#define ECHO_PIN 4
#define LED1_PIN 9      // Ultrasonic task LED
#define LED2_PIN 10     // Blink task LED
#define BUILTIN_LED 13  // DHT11 task LED

// Task Control Block structure
typedef struct {
  void (*taskFunc)();       // Task function pointer
  unsigned long period;     // Task execution period (ms)
  unsigned long lastRun;    // Last execution timestamp
  uint8_t priority;         // 0 = highest priority
  volatile bool ready;      // Is task ready to run?
  const char *name;         // Task name
} TCB;

// Task definitions
#define MAX_TASKS 3
TCB tasks[MAX_TASKS];
volatile int currentTask = 0;

// Forward declarations
void taskUltrasonicLED();
void taskDHT11();
void taskBlinkLED();

// Timer interrupt flag
volatile bool scheduleFlag = false;

// Function to set up Timer1
void setupTimer1() {
  // Disable interrupts during setup
  noInterrupts();
  
  // Clear Timer1 registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  // Set compare match register for 1ms (1kHz)
  // Arduino runs at 16MHz
  // Prescaler = 64
  // Timer frequency = 16MHz / 64 = 250kHz
  // For 1ms: 250000 / 1000 = 250
  OCR1A = 249;  // (250 - 1)
  
  // Turn on CTC mode (Clear Timer on Compare)
  TCCR1B |= (1 << WGM12);
  
  // Set prescaler to 64
  TCCR1B |= (1 << CS11) | (1 << CS10);
  
  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  // Enable interrupts
  interrupts();
}

// ============================================
// TIMER1 ISR: Runs every 1ms
// ============================================
ISR(TIMER1_COMPA_vect) {
  // Set flag to trigger scheduler
  scheduleFlag = true;
}

// ============================================
// PREEMPTIVE SCHEDULER
// ============================================
void scheduler() {
  unsigned long now = millis();
  int nextTask = -1;
  uint8_t highestPriority = 255;
  
  // Check which tasks are ready to run
  for (int i = 0; i < MAX_TASKS; i++) {
    // Task is ready if period has elapsed
    if (tasks[i].ready && (now - tasks[i].lastRun >= tasks[i].period)) {
      // Select highest priority task
      if (tasks[i].priority < highestPriority) {
        highestPriority = tasks[i].priority;
        nextTask = i;
      }
    }
  }
  
  // Execute the selected task
  if (nextTask != -1) {
    currentTask = nextTask;
    
    // Update last run time
    tasks[nextTask].lastRun = now;
    
    // Execute task
    Serial.print("[");
    Serial.print(now);
    Serial.print("ms] Running: ");
    Serial.println(tasks[nextTask].name);
    
    tasks[nextTask].taskFunc();
  }
}

// ============================================
// SETUP: Initialize hardware and tasks
// ============================================
void setup() {
  Serial.begin(9600);
  
  // Initialize pins
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  dht.begin();
  
  Serial.println("=== Preemptive RTOS Started ===");
  Serial.println("Task priorities:");
  Serial.println("0: Ultrasonic+LED (200ms) - HIGHEST");
  Serial.println("1: DHT11 (2000ms) - MEDIUM");
  Serial.println("2: Blink LED (1000ms) - LOWEST\n");
  
  // Initialize Task Control Blocks
  // Task 0: Ultrasonic + LED (Highest priority)
  tasks[0].taskFunc = taskUltrasonicLED;
  tasks[0].period = 200;
  tasks[0].lastRun = 0;
  tasks[0].priority = 0;
  tasks[0].ready = true;
  tasks[0].name = "Ultrasonic+LED";
  
  // Task 1: DHT11 sensor
  tasks[1].taskFunc = taskDHT11;
  tasks[1].period = 2000;
  tasks[1].lastRun = 0;
  tasks[1].priority = 1;
  tasks[1].ready = true;
  tasks[1].name = "DHT11";
  
  // Task 2: Blink LED (Lowest priority)
  tasks[2].taskFunc = taskBlinkLED;
  tasks[2].period = 1000;
  tasks[2].lastRun = 0;
  tasks[2].priority = 2;
  tasks[2].ready = true;
  tasks[2].name = "BlinkLED";
  
  // Setup Timer1 for preemptive scheduling
  setupTimer1();
  
  Serial.println("System initialized. Starting tasks...\n");
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
  // Check if scheduler should run (triggered by timer)
  if (scheduleFlag) {
    scheduleFlag = false;
    scheduler();
  }
}

// ============================================
// TASK 1: ULTRASONIC SENSOR + LED CONTROL
// Priority: 0 (HIGHEST)
// Period: 200ms
// ============================================
long readUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  long distanceCm = duration / 58;
  return distanceCm;
}

void taskUltrasonicLED() {
  long distance = readUltrasonicDistance();
  
  if (distance > 0 && distance < 20) {
    digitalWrite(LED1_PIN, HIGH);
    Serial.print("  OBJECT DETECTED! Distance: ");
  } else {
    digitalWrite(LED1_PIN, LOW);
    Serial.print("  Clear. Distance: ");
  }
  
  Serial.print(distance);
  Serial.println(" cm");
}

// ============================================
// TASK 2: DHT11 TEMPERATURE & HUMIDITY
// Priority: 1 (MEDIUM)
// Period: 2000ms
// ============================================
void taskDHT11() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println("  DHT11 read failed!");
  } else {
    Serial.print("  Temp: ");
    Serial.print(t);
    Serial.print("°C | Humidity: ");
    Serial.print(h);
    Serial.println("%");
    
    // Toggle built-in LED as heartbeat
    digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
  }
}

// ============================================
// TASK 3: SIMPLE LED BLINK
// Priority: 2 (LOWEST)
// Period: 1000ms
// ============================================
void taskBlinkLED() {
  digitalWrite(LED2_PIN, !digitalRead(LED2_PIN));
  Serial.print("  LED2 toggled to: ");
  Serial.println(digitalRead(LED2_PIN) ? "ON" : "OFF");
}
