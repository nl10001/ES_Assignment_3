void foo(int counter) {
  printf("foo() called  ... count = %d\n", counter);
  vTaskDelay(1000);
  unsigned int temp = uxTaskGetStackHighWaterMark(nullptr);
  printf("high stack water mark is %u\n", temp);
  foo(counter+1);
  printf("\n");
}

void task(void *ignore) {
  foo(1);  
}

void setup() {
  // put your setup code here, to run once:
  xTaskCreate(&task, "task", 4096, NULL, 5, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
