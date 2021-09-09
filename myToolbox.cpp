/*
  myToolbox.cpp
  My toolbox to show variuous information on M5Stack screen
  (c) 2021 @RR_Inyo
  Released under the MIT lisence
  https://opensource.org/licenses/mit-license.php
*/

#include <M5Stack.h>
#include <WiFi.h>
#include "myToolbox.hpp"

// Declare WiFi constants
const char* ssid = "XXXXXXXXXX";
const char* password = "XXXXXXXXXX";
const int colText = DARKGREY;
const int colBG = BLACK;

// WiFi connection function
void connectWiFi(void) {
  WiFi.begin(ssid, password);
  M5.Lcd.println("Connecting to:");
  M5.Lcd.println(ssid);
  while (WiFi.status() != WL_CONNECTED); {
    delay(50);
    M5.Lcd.print('.');
  }
  M5.Lcd.print("\r\nWifi connected!\r\nIP address: ");
  M5.Lcd.println(WiFi.localIP());
  delay(100);
}

// Show digital clock on LCD
void showClock(void)
{
  struct tm tm;
  if (getLocalTime(&tm)) {
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(0, 0, 1);
    M5.Lcd.setTextColor(colText, colBG);
    M5.Lcd.printf("%d-%02d-%02d %02d:%02d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min);
  }
}

// Show IP address on LCD
void showIPAddress(void)
{
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(132, 0, 1);
  M5.Lcd.setTextColor(colText, colBG);
  M5.Lcd.print("IP:");
  M5.Lcd.print(WiFi.localIP());
}

// Show battery level on upper right corner of LCD
void showBatt(void)
{
  static bool first = true;
  static int battLevel;
  static int battLevelOld;

  // Get battery level
  battLevel = M5.Power.getBatteryLevel();

  // Do not update when battery level has not changed.
  if (battLevel == battLevelOld && !first) {
    first = false;
    return;
  }

  // Show numeric
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(270, 2, 1);
  M5.Lcd.setTextColor(colText, colBG);
  M5.Lcd.printf("%3d%%", battLevel);

  // Show blocks
  // 100%
  if (battLevel > 99) {
    M5.Lcd.fillRect(265 + 6 * 5, 2, 5, 8, colText);
  } else {
    M5.Lcd.fillRect(265 + 6 * 5, 2, 5, 8, colBG);
    M5.Lcd.drawRect(265 + 6 * 5, 2, 5, 8, colText);
  }

  // 75%
  if (battLevel > 74) {
    M5.Lcd.fillRect(265 + 6 * 6, 2, 5, 8, colText);
  } else {
    M5.Lcd.fillRect(265 + 6 * 6, 2, 5, 8, colBG);
    M5.Lcd.drawRect(265 + 6 * 6, 2, 5, 8, colText);
  }

  // 50%
  if (battLevel > 49) {
    M5.Lcd.fillRect(265 + 6 * 7, 2, 5, 8, colText);
  } else {
    M5.Lcd.fillRect(265 + 6 * 7, 2, 5, 8, colBG);
    M5.Lcd.drawRect(265 + 6 * 7, 2, 5, 8, colText);
  }

  // 25%
  if (battLevel > 24) {
    M5.Lcd.fillRect(265 + 6 * 8, 2, 5, 8, colText);
  } else {
    M5.Lcd.drawRect(265 + 6 * 8, 2, 5, 8, colText);
  }

  // Preserve bettery level
  first = false;
  battLevelOld = battLevel;
}
