/*
   AnalogMeter.cpp
   Fullscreen analog meter for M5Stack
   (c) 2021 @RR_Inyo
   Released under the MIT License
   https://opensource.org/licenses/mit-license.php
*/

#include <M5Stack.h>
#include <math.h>
#include "AnalogMeter.hpp"

AnalogMeter::AnalogMeter(float* range, int nRanges, char* unit) {
  // Set range and unit
  this->range = range;
  this->nRanges = nRanges;
  this->unit = unit;
  valHand_old = range[0]; 
}

void AnalogMeter::drawGauge() {
  // Calculate initial point
  float alpha0 = -theta0 / 2 / 180.0 * M_PI;
  int x_old = x0 + int(r0 * sin(alpha0));
  int y_old = y0 - int(r0 * cos(alpha0));

  // Draw gauge
  int x, y;
  float alpha;  
  int i = 0;
  for (int k = 0; k < N + 1; k++) {
    alpha = (theta0 * k / float(N) - theta0 / 2) / 180.0 * M_PI;
    x = x0 + int(r0 * sin(alpha));
    y = y0 - int(r0 * cos(alpha));

    if (DEBUG) {
      M5.Lcd.setCursor(0, 0, 1);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.printf("(%d, %d)\n", x, y);
      M5.Lcd.printf("%f, %f, %f\n", range[0], range[1], range[2]);
    }

    // Draw arc
    M5.Lcd.drawLine(x_old, y_old, x, y, colArc);
    x_old = x;
    y_old = y;

    // Draw gauge short lines
    if (k % (N / (nRanges - 1) / 5) == 0) {
      drawLinePolar(x0, y0, alpha, r0, 6, colGaugeSmall);
    }

    // Draw gauge lines and labels
    if (k % (N / (nRanges - 1)) == 0) {
      drawLinePolar(x0, y0, alpha, r0, 10, colGaugeLarge);

      // Print with Removed fraction if integer
      if (range[i] - floor(range[i]) > 0) {
        setCursorPolar(x0 - 12, y0 - 8, alpha, r0 + 20, 2);
        M5.Lcd.setTextColor(colLabelSmall);
        M5.Lcd.printf("%.1f", range[i]);
      } else {
        setCursorPolar(x0 - 14, y0 - 8, alpha, r0 + 25, 4);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.printf("%d", int(range[i]));
      }
      
      i++;
    }
  }
}

void AnalogMeter::update(float val) {
  // Calculate angle
  float rMin = range[0];
  float rMax = range[nRanges - 1];
  float valHand;

  // Constraint hand position
  val < rMin ? valHand = rMin : val > rMax ? valHand = rMax : valHand = val;
  
  float alpha_old = ((-0.5 + (valHand_old - rMin) / (rMax - rMin)) * theta0) / 180 * M_PI;
  float alpha = ((-0.5 + (valHand - rMin) / (rMax - rMin)) * theta0) / 180 * M_PI;

  // Erase old hand
  drawLinePolar(x0, y0, alpha_old, rHand1, rHand2, BLACK);

  // Write unit
  M5.Lcd.setCursor(144, 108, 4);
  M5.Lcd.setTextColor(colUnit);
  M5.Lcd.print(unit);

  // Draw new hand
  drawLinePolar(x0, y0, alpha, rHand1, rHand2, colHand);

  // Write digital value
  M5.Lcd.setCursor(72, 180, 7);
  M5.Lcd.setTextColor(colVal, colBG);
  M5.Lcd.printf("%2.3f", val);

  // Preserve old value
  valHand_old = valHand;
}

void AnalogMeter::drawLinePolar(int x, int y, float alpha, int r1, int r2, int col) {
  // Calculate cartesian coordinates
  int xs = int(x + r1 * sin(alpha));  
  int ys = int(y - r1 * cos(alpha));
  int xe = int(x + (r1 + r2) * sin(alpha));
  int ye = int(y - (r1 + r2) * cos(alpha));

  // Draw line
  M5.Lcd.drawLine(xs, ys, xe, ye, col);
}

void AnalogMeter::setCursorPolar(int x, int y, float alpha, int r, int font) {
  int xs, ys, xe, ye;

  // Calculate cartesian coordinates
  int xc = int(x + r * sin(alpha));  
  int yc = int(y - r * cos(alpha));

  // Draw line
  M5.Lcd.setCursor(xc, yc, font);
}
