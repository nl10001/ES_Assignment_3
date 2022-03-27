//9700us maybe +50 not sure

#include<stdio.h>

#define LED1 15
#define LED2 21
#define BUTTON1 22
#define BUTTON2 23
#define NUM_TIMERS 7

struct s_led {
  byte          gpio;    // LED GPIO number
  byte          state;  // LED state
  unsigned      napms;  // Delay to use (ms)
  TaskHandle_t  taskh;  // Task handle
};

static s_led leds[2] = {
  { LED1, 0, 1000, 0 },
  { LED2, 0, 500, 0 }
};

int button1State = 0;

//Output digital watchdog waveform
void vTask1(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 9750;
  const TickType_t xDelay = 50;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      digitalWrite(leds[1].gpio, HIGH);
      vTaskDelay(xDelay);
      digitalWrite(leds[1].gpio, LOW);
      //Serial.println("done");
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

//Monitor one digital input
void vTask2(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 200;
  
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      button1State = digitalRead(BUTTON1);
      //Serial.println(button1State);
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(leds[0].gpio, OUTPUT);
  pinMode(leds[1].gpio, OUTPUT);
  
  pinMode(BUTTON1, INPUT);
    
  xTaskCreate(vTask1, "Task 1", 1024, NULL, 0, NULL);
  xTaskCreate(vTask2, "Task 2", 1024, NULL, 1, NULL);
  
  //vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
