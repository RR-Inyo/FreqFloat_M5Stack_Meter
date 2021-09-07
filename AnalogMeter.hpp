/*
   AnalogMeter.hpp
   Fullscreen analog meter for M5Stack
   (c) 2021 @RR_Inyo
   Released under the MIT License
   https://opensource.org/licenses/mit-license.php
*/

#ifndef _ABALOGMETER_
#define _ANALOGMETER_

class AnalogMeter {
  public:
    AnalogMeter(float *range, int numRanges, char *unit);
    void drawGauge();
    void update(float val);

    // Color settings
    int colBG = BLACK;
    int colArc = LIGHTGREY;
    int colGaugeSmall = DARKGREY;
    int colGaugeLarge = WHITE;
    int colLabelSmall = LIGHTGREY;
    int colLabelLarge = LIGHTGREY;
    int colHand = RED;
    int colUnit = WHITE;
    int colVal = WHITE;

  private:
    // For debugging
    const bool DEBUG = false;

    // Constants for drawing gauge
    const int x0 = 159;
    const int y0 = 319;
    const int r0 = 270;
    const float theta0 = 56.0;
    const int N = 100;

    // Constants for drawing hand
    const int rHand1 = 165;
    const int rHand2 = 103;

    // Variables for data range
    float *range;
    int nRanges;
    char *unit;
    float valHand_old;

    // Functions for drawing
    void drawLinePolar(int x, int y, float alpha, int r1, int r2, int col);
    void setCursorPolar(int x, int y, float alpha, int r, int font);
    void drawNeedle(float val, int col);
};

#endif
