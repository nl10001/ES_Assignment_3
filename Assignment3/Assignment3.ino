#include<stdio.h>
#include<math.h>


#define LED1 15
#define LED2 21
#define BUTTON1 22
#define BUTTON2 23
#define SQUARE_WAVE_SIG 4
#define ANALOG_SIG 2
#define MAX_RANGE 4096
#define TEST_PIN 17

#define POW_BASE10(i) pow(10, i)

//globals

//struct which holds the serial information
struct s_data {
  int digitalState;   
  int frequency;
  int filtered_analog;
};
//creating variable of type s_data
static s_data serial_info = {0, 0, 0};

//queues to communicate data
static QueueHandle_t xQueueAnalogData;
static QueueHandle_t error_data_queue;

//semaphore to protect against deadlocking
static SemaphoreHandle_t mutex;

//task handle for task 9 for suspend/resume functions
static TaskHandle_t task9;


/************ TASKS ************/


//Output digital watchdog waveform
void vTask1(void * pvParameters) {
  TickType_t xLastWakeTime;
  //
  const TickType_t xFrequency = 10 / portTICK_PERIOD_MS;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      //sigB waveform from assingment 1
      digitalWrite(LED2, HIGH);
      delayMicroseconds(50);
      digitalWrite(LED2, LOW);
      //delayuntil function which executes task at rate set in xFrequency
      vTaskDelayUntil(&xLastWakeTime, xFrequency);      
    }
}

//Monitor one digital input
void vTask2(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 200 / portTICK_PERIOD_MS;
  int button1State;
  int counter = 0;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      digitalWrite(TEST_PIN, HIGH);
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        serial_info.digitalState = digitalRead(BUTTON1);
        if((counter % 25) == 0) {
          if(serial_info.digitalState == 1) {
            vTaskResume(task9);
          }
        }
       
        xSemaphoreGive(mutex);
      }
      digitalWrite(TEST_PIN, LOW);
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
      counter++;
    }
}

//calculate frequency of square wave
void vTask3(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 1000 / portTICK_PERIOD_MS;
  int pinData;
  int wave_freq;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      //pulseIn function detects the time the square wave is high for
      pinData = pulseIn(SQUARE_WAVE_SIG, HIGH);
      //take the semaphore and check if it has been successful
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        //calculate frequency and store in struct
        serial_info.frequency = 1/(2*pinData*POW_BASE10(-6));
        //give the mutex back
        xSemaphoreGive(mutex);
      }
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

//read an analog signal
void vTask4(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 42 / portTICK_PERIOD_MS;
  int analog_data;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      //store analog data
      analog_data = analogRead(ANALOG_SIG);
      //send the analog data in the analog data queue
      xQueueSend(xQueueAnalogData, (void*) &analog_data, (TickType_t) 0);
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
      //return analog_data;
    }
}

//averaging the last 4 readings of the analog read
void vTask5(void * pvParameters) {
  TickType_t xLastWakeTime;
  int rxed_analog_data;
  int an_data;
  int analog_array[4];
  int sum;
  int counter = 0;
  int average;
  const TickType_t xFrequency = 42 / portTICK_PERIOD_MS; 
  
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
  for(;;) { // Wait for the next cycle.
    // Perform action here.
    //if the queue is not empty then store the queue value in an_data
    if(xQueueAnalogData != NULL) {
      if (xQueueReceive(xQueueAnalogData, &(rxed_analog_data), (TickType_t) 10) == pdTRUE) {
        an_data = rxed_analog_data;
      }
    }
    //switch case statement which updates the oldest element of the array
    switch(counter) {
       case 0:
         analog_array[counter] = an_data;
         break;
       case 1:
         analog_array[counter] = an_data;
         break;
       case 2:
         analog_array[counter] = an_data;
         break;
       case 3:
         analog_array[counter] = an_data;
         break;
     }
     //increment counter and reset when out of bounds
     counter++;
     if(counter == 4) {
       counter = 0;
     }
     //ensure sum is zero before using
     sum = 0;
     //iterate through the array and sum all the values
     for(int i = 0; i < 4; i++) {
       sum += analog_array[i];
     }
     //if the semaphore has been taken successfully then do the code in the statement
     if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
       //store the average analog value in the struct
       serial_info.filtered_analog = sum/4;
       //give the mutex back 
       xSemaphoreGive(mutex);
     }
     vTaskDelayUntil(&xLastWakeTime, xFrequency);
   }
}

//do nothing tasks to emulate intensive function
void vTask6(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      __asm__ __volatile__ ("nop");
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

//determine error code value
void vTask7(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 333 / portTICK_PERIOD_MS;
  int error_code;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      //take semaphore
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        //determine error code value based off filtered analog value
        if(serial_info.filtered_analog > (MAX_RANGE/2)) {
          error_code = 1;
        }
        //otherwise stay LOW
        else {
          error_code = 0;
        }
        //give the semaphore back
        xSemaphoreGive(mutex);
        //send the error code value in the error data queue
        xQueueSend(error_data_queue, (void*) &error_code, (TickType_t) 0);
      }
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

//set the led to whatever the error code value is
void vTask8(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 333 / portTICK_PERIOD_MS;
  int rxed_error_code;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      //if the queue is not empty then set the led equal the the value in the queue
      if(error_data_queue != NULL) {
        if(xQueueReceive(error_data_queue, &(rxed_error_code), (TickType_t) 0) == pdTRUE) {
          digitalWrite(LED1, rxed_error_code);          
        }
      }   
      vTaskDelayUntil(&xLastWakeTime, xFrequency); 
    }
}

//print the values of the struct to the serial port when button 1 is pressed
void vTask9(void * pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 5000 / portTICK_PERIOD_MS;
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
    for(;;) { // Wait for the next cycle.
      // Perform action here.
      //take the semaphore
      if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        if(serial_info.digitalState == 1) {
          
          //print the state of button 1
          Serial.print(serial_info.digitalState);
          Serial.print(", ");
      
          //print the frequency of the 3.3v square wave signal
          Serial.print(serial_info.frequency); 
          Serial.print(", ");
          
          //print the filtered analog input
          Serial.println(serial_info.filtered_analog);          
        }
        //give the semaphore back
        xSemaphoreGive(mutex);
      }
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
      //suspend task 9
      vTaskSuspend(NULL);
      //Serial.println(uxTaskGetStackHighWaterMark(NULL));
    }
}

/************ CREATION OF TASKS ************/

void setup() {
  // put your setup code here, to run once:
  //enable the serial port and wait 2 seconds for initialisation
  Serial.begin(115200);
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  //pin outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(TEST_PIN, OUTPUT);

  //pin inputs
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(SQUARE_WAVE_SIG, INPUT);
  pinMode(ANALOG_SIG, INPUT);

  //create the analog queue
  xQueueAnalogData = xQueueCreate(1, sizeof(int));
  //check if the queue has actually been created otherwise throw error print
  if(xQueueAnalogData == NULL) {
    for(;;) {
      Serial.println("analog queue fail");
    }
  }

  //create the error data queue
  error_data_queue = xQueueCreate(1, sizeof(int));
  //check if the queue has actually been created otherwise throw error print
  if(error_data_queue == NULL) {
    for(;;) {
      Serial.println("error_data queue fail");
    }
  }

  //create the mutex
  mutex = xSemaphoreCreateMutex();
  //check if the semaphore has actually been created otherwise throw error print
  if(mutex == NULL) {
    for(;;) {
      Serial.println("semaphore fail");
    }
  }

  //create the tasks with the stack sizes checked using stack highwatermark
  xTaskCreate(vTask1, "Task 1", 550, NULL, 5, NULL);
  xTaskCreate(vTask2, "Task 2", 550, NULL, 3, NULL);
  xTaskCreate(vTask3, "Task 3", 800, NULL, 4, NULL);  
  xTaskCreate(vTask4, "Task 4", 800, NULL, 2, NULL);
  xTaskCreate(vTask5, "Task 5", 800, NULL, 1, NULL);
  xTaskCreate(vTask6, "Task 6", 550, NULL, 1, NULL);
  xTaskCreate(vTask7, "Task 7", 550, NULL, 2, NULL);
  xTaskCreate(vTask8, "Task 8", 550, NULL, 1, NULL);
  xTaskCreate(vTask9, "Task 9", 550, NULL, 1, &task9);
  //suspend task 9 on creation
  vTaskSuspend(task9);

}

void loop() {
  // put your main code here, to run repeatedly:
    
}
