/**
     DVS event camera latency test using 2 leds that alternate as quickly as they are detected to blink by algorithm.

    Author: Tobi Delbruck, Feb 2021
*/
#define VERSION  "DVSLatencyMeasurement dated 21.2.2021"
#define BAUDRATE 115200 // serial port baud rate, host must set same speed
// NOTES
// 1. when using Chinese Arduino Nano with CH340 USB serial, use Processor/AtMega328P (old bootloader)
// 2. Using Serial Monitor to test, set "No line ending", otherwise line ending will set LED off immediately

const int LED1 = 2, LED2 = 3;
const unsigned long FLASH_DELAY_US = 5000; // flash time (maximum) of LED
unsigned long lastToggleUs = 0;

int led = 0; // 1, 2 to flash 1st or 2nd led
bool on = true;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  Serial.begin(BAUDRATE);
  Serial.println("DVSLatencyMeasurement ready to serve");
  Serial.println(VERSION);
}

void loop() {
  int ser = Serial.read();
  if (ser != -1)  {
    led = ser - '0'; // convert the character '1'-'9' to decimal 1-9
    on = 1;
  }
   if (led == 1) {
    digitalWrite(LED1, on);
    digitalWrite(LED2, 0);
  } else if (led == 2) {
    digitalWrite(LED2, on);
    digitalWrite(LED1, 0);
  } else if (led == 3) {
    digitalWrite(LED2, on);
    digitalWrite(LED1, on);
  } else if(ser=='p'){
    // echo back, latency test 
    Serial.print('p');
  }else {
    digitalWrite(LED1, 0);
    digitalWrite(LED2, 0);
  }
  unsigned long now = micros();
  if (now - lastToggleUs > FLASH_DELAY_US) {
    on = 0;
    lastToggleUs = now;
  }
}
