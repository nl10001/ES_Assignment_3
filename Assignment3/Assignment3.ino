//9700us maybe +50 not sure

#include<stdio.h>
#include<math.h>

#define LED1 15
#define LED2 21
#define BUTTON1 22
#define BUTTON2 23
#define SQUARE_WAVE_SIG 4
#define ANALOG_SIG 0
#define MAX_RANGE 4096
#define NUM_TIMERS 7

#define POW_BASE10(i) pow(10, i)

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

struct s_data {
  int digitalState;
  int frequency;
  int filtered_analog;
};

//globals
static s_data serial_info = {0, 0, 0};
static QueueHandle_t xQueueAnalogData;
static SemaphoreHandle_t mutex;

int button1State = 0;
int pinData = 0;
int wave_freq = 0;
int analog_data = 0;


/************ TASKS ************/

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
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        serial_info.digitalState = digitalRead(BUTTON1);
        
        xSemaphoreGive(mutex);
      }
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vTask3(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 1000;
  
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      //pinData = pulseIn(SQUARE_WAVE_SIG, LOW);
      pinData = 2689;
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        serial_info.frequency = 1/(2*pinData*POW_BASE10(-6));
         //Serial.println(wave_freq);
         
        xSemaphoreGive(mutex);
      }
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vTask4(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 42;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      //analog_data = analogRead(ANALOG_SIG);
      analog_data = 4096;
      xQueueSend(xQueueAnalogData, (void*) &analog_data, (TickType_t) 0);
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
      //return analog_data;
    }
}

void vTask5(void * pvParameters) {
  TickType_t xLastWakeTime;
  int rxed_analog_data;
  int analog_array[4];
  int sum;
  int counter = 0;
  int average;
  const TickType_t xFrequency = 42; 
  if( xQueueAnalogData != NULL) {
    if (xQueueReceive(xQueueAnalogData, &(rxed_analog_data), (TickType_t) 0) == pdTRUE) {
   
    }
  }
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      switch(counter) {
        case 0:
          analog_array[counter] = rxed_analog_data;
          break;
        case 1:
          analog_array[counter] = rxed_analog_data;
          break;
        case 2:
          analog_array[counter] = rxed_analog_data;
          break;
        case 3:
          analog_array[counter] = rxed_analog_data;
          break;
      }
      counter++;
      if(counter == 4) {
        counter = 0;
      }
      sum = 0;
      for(int i = 0; i < 4; i++) {
        sum += analog_array[i];
        //Serial.println(sum);
      }
      //Serial.println(sum);
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        serial_info.filtered_analog = sum/4;
        //Serial.println(average);
        
        xSemaphoreGive(mutex);
      }
      
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vTask6(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 100;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      __asm__ __volatile__ ("nop");
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vTask7(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 333;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        if (serial_info.filtered_analog > (MAX_RANGE/2)) {
          error_code = 1;
        }
        //otherwise stay LOW
        else {
          error_code = 0;
        }
        xSemaphoreGive(mutex);
      }
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vTask9(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 5000;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        //print the state of button 1
        Serial.print(serial_info.digitalState);
        Serial.print(", ");
    
        //print the frequency of the 3.3v square wave signal
        Serial.print(serial_info.frequency); 
        Serial.print(", ");
        
        //print the filtered analog input
        Serial.println(serial_info.filtered_analog);

        xSemaphoreGive(mutex);
      }
       vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/************ CREATION OF TASKS ************/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  
  pinMode(leds[0].gpio, OUTPUT);
  pinMode(leds[1].gpio, OUTPUT);
  
  pinMode(BUTTON1, INPUT);

  xQueueAnalogData = xQueueCreate(1, sizeof(int));
  if(xQueueAnalogData == NULL) {
    for(;;);
  }

  mutex = xSemaphoreCreateMutex();
  if(mutex == NULL) {
    for(;;);
  }
  
  xTaskCreate(vTask1, "Task 1", 1024, NULL, 2, NULL);
  xTaskCreate(vTask2, "Task 2", 1024, NULL, 1, NULL);
  xTaskCreate(vTask3, "Task 3", 1024, NULL, 1, NULL);
  xTaskCreate(vTask4, "Task 4", 4096, NULL, 1, NULL);
  xTaskCreate(vTask5, "Task 5", 4096, NULL, 1, NULL);
  xTaskCreate(vTask6, "Task 6", 1024, NULL, 1, NULL);
  xTaskCreate(vTask7, "Task 7", 1024, NULL, 1, NULL);
  xTaskCreate(vTask9, "Task 9", 4096, NULL, 1, NULL);

  //xSemaphoreTake(mutex, portMAX_DELAY);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
