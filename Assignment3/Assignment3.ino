//9700us maybe +50 not sure

#include<stdio.h>

#define LED1 15
#define LED2 21
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


/*
void foo(int counter) {
  printf("foo() called  ... count = %d\n", counter);
  vTaskDelay(1000);
  unsigned int temp = uxTaskGetStackHighWaterMark(nullptr);
  printf("high stack water mark is %u\n", temp);
  foo(counter+1);
  printf("\n");
}
*/

void vTask1(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 5000;
  const TickType_t xDelay = 500;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for( ;; ) { // Wait for the next cycle.
      vTaskDelayUntil( &xLastWakeTime, xFrequency );
      // Perform action here.
      digitalWrite(LED2, HIGH);
      vTaskDelay(xDelay);
      digitalWrite(LED2, LOW);
      Serial.println("done");
    }

}



void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  pinMode(leds[0].gpio, OUTPUT);
  pinMode(leds[1].gpio, OUTPUT);
    
  xTaskCreate(&vTask1, "Task 1", 1024, NULL, 5, NULL);
  
  //vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
