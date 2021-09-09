/*
   FreqFloat_M5Stack_Meter.ino
   An mains frequency monitor based on induced potential via stray capacitance together with analog meter 
   (c) 2021 @RR_Inyo
   Released under the MIT lisence
   https://opensource.org/licenses/mit-license.php
*/

#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "myToolbox.hpp"
#include "dFilt.hpp"
#include "AnalogMeter.hpp"

/* Uncomment below for white background */
/* #define PAPER_WHITE */

// Declare NTP servers and the setting constants
const char* server1 = "ntp.nict.jp";
const char* server2 = "time.google.com";
const char* server3 = "ntp.jst.mfeed.ad.jp";
const long JST = 3600L * 9;
const int summertime = 0;

// Declare Handlers for task and timer
TaskHandle_t taskHandle;
hw_timer_t *timer = NULL;

// Declare function prototypes
void IRAM_ATTR onTimer();
void taskPeriodical(void *pvParameters);

// Declare function to report data to Google Spreadsheet and obtain HTTP handler
void reportGSS(float f);
const char* apiURL = "https://script.google.com/macros/s/XXXXXXXXXX/exec";
int httpCode;
HTTPClient http;

// Declare and initialize constants for hardware
const int ANALOG_IN = 35;

// Declare and initialize constants for digital filter
const int T_SAMPLE_US = 100;
const float ZETA = 0.707;
const float OMEGA_N = 2 * PI * 200;

// Declare and initialize constant and global variables for frequency detection
const float trig_rise = 0.03;
const int CYCLES = 50;
const int N_MAF = 2;
float freq = 50.0;
int i = 0;
float freq_buf[N_MAF];

// Define LPF instances, first- and second-order
SecondOrderLPF lpf(T_SAMPLE_US / 1e6, ZETA, OMEGA_N);

// Define analog meter handler
float range[] = {49.0, 49.5, 50.0, 50.5, 51.0};
char unit[] = "Hz";
AnalogMeter mtr(range, sizeof(range) / sizeof(range[0]), unit);

/*
   The setup function, run once
*/
void setup() {
  M5.begin();
  M5.Power.begin();
  M5.Lcd.setBrightness(80);

  // Set up the pin
  pinMode(ANALOG, INPUT);

  // Connect to WiFi
  connectWiFi();

  // Synchronize to NTP server
  configTime(JST, summertime, server1, server2, server3);

  // Creat a task to be executed periodically by signal from ISR
  xTaskCreateUniversal(
    taskPeriodical,
    "taskPeriodical",
    8192,
    NULL,
    1,
    &taskHandle,
    PRO_CPU_NUM
  );

  // Set up timer and attach ISR
  timer = timerBegin(0, 80, true); // Count up every microsecond
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, T_SAMPLE_US, true);
  timerAlarmEnable(timer);

  // Clear LCD and write fixed text
#ifdef PAPER_WHITE
  M5.Lcd.fillScreen(WHITE);
  mtr.colBG = WHITE;
  mtr.colGaugeLarge = BLACK;
  mtr.colLabelSmall = DARKGREY;
  mtr.colLabelLarge= BLACK;
  mtr.colUnit = BLACK;
  mtr.colVal = BLACK;
#else
  M5.Lcd.fillScreen(BLACK);
#endif

  // Draw meter gauge
  mtr.drawGauge();
}

/*
   The loop function, run repeadedly
*/
void loop() {
  // Change digital display color depending on frequency
  if (freq < 49.8 || freq > 50.2) {
    mtr.colVal = RED;
  } else if (freq < 49.9 || freq > 50.1) {
    mtr.colVal = YELLOW;
  } else {
#ifdef PAPER_WHITE
    mtr.colVal = BLACK;
#else
    mtr.colVal = WHITE;
#endif
  }

  // Show analog meter
  mtr.update(freq);

  // Show clock
  showClock();

  // Show IP address
  showIPAddress();

  // Show battery level
  showBatt();

  // Report to GSS every 10 second
  struct tm tm;
  if (getLocalTime(&tm)) {
    if (tm.tm_sec % 10 == 0) {
      reportGSS(freq);
    }
  }

  // Low priority period
  delay(1000);
}

// ISR, sending a signal to actual task
void IRAM_ATTR onTimer() {
  BaseType_t taskWoken;
  xTaskNotifyFromISR(taskHandle, 0, eIncrement, &taskWoken);
}

// A task periodically executed by signal from ISR
void taskPeriodical(void *pvParameters) {
  uint32_t ulNotifiedValue;

  // Declare variables for signals
  float u, y;
  static float y_old = 0;

  // Declare variables for frequency detection/calculation
  static int cycles = 0;
  static unsigned long t0;

  while (true) {
    // Wait for the signal from ISR
    xTaskNotifyWait(0, 0, &ulNotifiedValue, portMAX_DELAY);

    // A-D conversion, detect floating wire potential to sense AC-grid voltage, half-wave
    u = float(analogRead(ANALOG_IN) / 4096.0);

    // Apply second-order Butterworth
    y = lpf.apply(u);

    // Calculate frequency
    if (y_old < trig_rise && y > trig_rise) {
      if (cycles == 0) {
        t0 = micros();
      }
      if (cycles < CYCLES) {
        cycles++;
      } else {
        freq_buf[i] = 1e6 / float(micros() - t0) * CYCLES;
        cycles = 0;
        i = (i + 1) % N_MAF;
      }
    }
    y_old = y;

    // Moving average filter
    float freq_MAF = 0;
    for (int j = 0; j < N_MAF; j++) {
      freq_MAF += freq_buf[j];
    }
    freq_MAF /= N_MAF;
    freq = freq_MAF;
  }
}

// Report frequency data to Google Spreadsheet
void reportGSS(float freq)
{
  struct tm tm;
  int minute_old;
  char datebuf[32];
  char pubMessage[64];

  // Create date-time text
  if (getLocalTime(&tm)) {
    sprintf(datebuf, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  }

  // Create JSON message
  StaticJsonDocument<500> doc;
  JsonObject object = doc.to<JsonObject>();

  object["date"] = datebuf;
  object["freq"] = freq;

  serializeJson(doc, pubMessage);

  // Report to Google Spreadsheet API
  http.begin(apiURL);
  httpCode = http.POST(pubMessage);
}
