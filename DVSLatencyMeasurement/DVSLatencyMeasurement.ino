/**
     DVS event camera latency test using LED detected to flash by PC algorithm.

    Author: Tobi Delbruck, Feb 2021
*/
#define VERSION  "DVSLatencyMeasurement dated 21.2.2021"
#define BAUDRATE 2000000 // 9600 // 2000000 // 115200 // serial port baud rate, host must set same speed
// NOTES
// 1. when using Chinese Arduino Nano with CH340 USB serial, use Processor/AtMega328P (old bootloader)
// 2. Using Serial Monitor to test, set correct baud rate, "No line ending", otherwise line ending will set LED off immediately

const byte STATE_IDLE = 0,
           STATE_MASTER_DELAYING = 1,
           STATE_MASTER_WAITING = 2,
           STATE_SINGLE_FLASH_ON = 3,
           STATE_FLASHING_ON = 4,
           STATE_FLASHING_OFF = 5,
           STATE_LED_ON = 6;

byte state;
int ser;
const int LED = LED_BUILTIN;
const unsigned long FLASH_LENGTH_US = 1000; // flash ON/OFF cycle time of LED
const unsigned long FLASH_INTERVAL_US = 50000; // flash interval
unsigned long timeStateChangedUs = 0;

bool on = true;
// master mode, we generate the LED flash, and wait for PC message that it saw it via DVS.
// then we print the latency in us back to PC.

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(BAUDRATE);
  Serial.println("DVSLatencyMeasurement ready to serve");
  Serial.println(VERSION);
  randomSeed(analogRead(0));
  digitalWrite(LED, 1);
  delay(100);
  digitalWrite(LED, 0); // flash once on boot
  state = STATE_IDLE;
  ser = -1;
}

void loop() {
  unsigned long now = micros();
  ser=-1;
  while (Serial.available() > 0) {
    ser = Serial.read(); // drain the port, get last char to avoid 1-ahead problem
  }
  switch (ser) {
    case -1:
      break;
    case '1': {
        // momentarily blink (turn on once) LED
        digitalWrite(LED, 1);
        state = STATE_SINGLE_FLASH_ON;
        timeStateChangedUs = now;
      }
      break;
    case 'd': {
        // PC detected the LED via DVS
        digitalWrite(LED, 0);
        long deltaTimeUs = now - timeStateChangedUs;
        timeStateChangedUs = now;
        state = STATE_MASTER_DELAYING;
        //    Serial.println(deltaTimeUs);
        Serial.write((byte*)&deltaTimeUs, sizeof(long)); // sent in little endian binary
      }
      break;
    case 'p': {
        // echo back, latency test
        Serial.print('p');
        state = STATE_IDLE;
      }
      break;
    case 'm': {
        // flash LED,  wait for 'd' message that PC detected LED
        state = STATE_MASTER_DELAYING;
        timeStateChangedUs = now;
      }
      break;
    case '0': {
        state = STATE_IDLE;
      }
      break;
    case 'f': {
        // flash LED continuously, for setting ROI on host
        timeStateChangedUs = now;
        state = STATE_FLASHING_ON;
      }
      break;
    case '2': {
        // turn on LED, no flashing
        digitalWrite(LED, 1);
        state = STATE_LED_ON;
        timeStateChangedUs = now;
      }
      break;
    default: {
        Serial.println("?");
      }
  }

  switch (state) {
    case STATE_IDLE:
      digitalWrite(LED, 0);
      break;
    case STATE_MASTER_DELAYING:
      if (now - timeStateChangedUs > FLASH_INTERVAL_US) {
        digitalWrite(LED, 1);
        timeStateChangedUs = now;
        state = STATE_MASTER_WAITING;
      } else {
        digitalWrite(LED, 0); // LED off now, turn on after delay
      }
      break;
    case STATE_MASTER_WAITING:
      break;
    case STATE_SINGLE_FLASH_ON:
      if (now - timeStateChangedUs > FLASH_LENGTH_US) {
        digitalWrite(LED, 0);
        state = STATE_IDLE;
      }
      break;
    case STATE_FLASHING_ON:
      if (now - timeStateChangedUs > FLASH_LENGTH_US) {
        digitalWrite(LED, 0);
        state = STATE_FLASHING_OFF;
        timeStateChangedUs = now;
      }
      break;
    case STATE_FLASHING_OFF:
      if (now - timeStateChangedUs > FLASH_LENGTH_US) {
        digitalWrite(LED, 1);
        state = STATE_FLASHING_ON;
        timeStateChangedUs = now;
      }
      break;
  }
}
