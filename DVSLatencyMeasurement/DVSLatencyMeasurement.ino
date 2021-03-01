/**
     DVS event camera latency test using LED detected to flash by PC algorithm.

    Author: Tobi Delbruck, Feb 2021
*/
#define VERSION  "DVSLatencyMeasurement dated 21.2.2021"
#define BAUDRATE 2000000 // 115200 // serial port baud rate, host must set same speed
// NOTES
// 1. when using Chinese Arduino Nano with CH340 USB serial, use Processor/AtMega328P (old bootloader)
// 2. Using Serial Monitor to test, set "No line ending", otherwise line ending will set LED off immediately

const int LED = 2;
const unsigned long FLASH_DELAY_US = 10000; // flash time (maximum) of LED
const unsigned long FLASH_INTERVAL_US = 50000; // flash interval for master mode
unsigned long timeLedSwitchedOnUs = 0;

bool on = true;
// master mode, we generate the LED flash, and wait for PC message that it saw it via DVS.
// then we print the latency in us back to PC.
bool master = false;
boolean flash = false;

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
}

void loop() {
  int ser = Serial.read();
  if (ser == 'm') { // m for master, pc told us to light up
    // turn on LEDs and wait for 'd' message that PC detected LED
    master = true;
    flash = false;
    delayMicroseconds(random(FLASH_DELAY_US, FLASH_INTERVAL_US)); // delay before turning on LED
    digitalWrite(LED, 1);
    timeLedSwitchedOnUs = micros();
  }  else if (ser == 'd') { // PC detected the LED via DVS
    digitalWrite(LED, 0);
    long deltaTimeUs = micros() - timeLedSwitchedOnUs;
    //    Serial.println(deltaTimeUs);
    Serial.write((byte*)&deltaTimeUs, sizeof(long)); // sent in little endian binary
  } else if (ser == '1') {
    digitalWrite(LED, 1);
    timeLedSwitchedOnUs = micros();
    master = false;
    flash = false;
  } else if (ser == '0') { 
    digitalWrite(LED, 0);
    master = false;
    flash = false;
  }else if (ser == 'f') { // flash LED, for setting ROI on host
    digitalWrite(LED, 1);
    timeLedSwitchedOnUs = micros();
    master = false;
    flash = true;
  } else if (ser == 'p') {
    // echo back, latency test
    Serial.print('p');
    master = false;
    flash = false;
  }
  unsigned long now = micros();
  if (now - timeLedSwitchedOnUs > FLASH_DELAY_US) {
    if (!flash)
      digitalWrite(LED, 0);
    else {
      on = !on;
      timeLedSwitchedOnUs = now;
      digitalWrite(LED, on);
    }
  }
}
