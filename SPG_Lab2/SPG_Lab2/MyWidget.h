#pragma once
#define WIN32 // doar in Windows
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <cmath>
#include <iostream>

#define PI 3.141592f

class MyWidget : public Fl_Window {
  float XFm, XFM, YFm, YFM;
  int XPm, XPM, YPm, YPM;
  int tip_tran; // tip_tran == 0 -> scalare neuniforma, tip_tran != 0 -> scalare
                // uniforma
  float sx, sy, tx, ty;

  int width, height;

  typedef float (*MyFuncPtrType)(float);

public:
  MyWidget(int width = 512, int height = 512)
      : Fl_Window(200, 200, width, height, "SPG Lab2"), width(width),
        height(height) {}

  void init_grafic() {
    XFm = YFm = XFM = YFM = 0;
    XPm = YPm = XPM = YPM = 0;
    tip_tran = 0;
    sx = sy = tx = ty = 0;
  }
  void calctran() {
    if (XFM > XFm && YFM > YFm) {
      sx = (XPM - XPm) / (XFM - XFm);
      sy = (YPm - YPM) / (YFM - YFm);
      tx = XPm - sx * XFm;
      ty = YPM - sy * YFm;
    } else
      sx = sy = tx = ty = 0;
  }
  int XDisp(float xf) {
    // transformarea fereastra-poarta pt coordonata x
    return (int)(xf * sx + tx);
  }
  int YDisp(float yf) {
    // transformarea fereastra-poarta pt coordonata y
    return (int)(yf * sy + ty);
  }
  void cadru_poarta() {
    fl_line(XPm, YPm, XPm, YPM);
    fl_line(XPm, YPM, XPM, YPM);
    fl_line(XPM, YPM, XPM, YPm);
    fl_line(XPM, YPm, XPm, YPm);
  }
  static float f1(float x) { return tan(x); }

  static float f2(float x) { return x * sin(10 * x); }

  static float f3(float x) { return sin(10 * x); }

  static float f4(float x) { return ((x * x - 2) * (x + 3)); }

  void grafic(float xmin, float xmax, float pas, MyFuncPtrType f) {
    XFm = xmin;
    XFM = xmax;
    // se determina YFm si YFM ca fiind valoarea minima, respectiv maxima a
    // functiei f in intervalul XFm, XFM
    YFm = f(xmin);
    YFM = f(xmin);
    for (float x = xmin; x <= xmax; x += pas) {
      float y = f(x);
      if (y < YFm)
        YFm = y;
      if (y > YFM)
        YFM = y;
    }
    cadru_poarta();
    calctran();
    // trasarea axei x
    if (YFm < 0 && YFM > 0)
      fl_line(XPm, YDisp(0), XPM, YDisp(0));
    // trasarea axei y
    if (XFm < 0 && XFM > 0)
      fl_line(XDisp(0), YPm, XDisp(0), YPM);
    // trasare grafic
    for (float x = xmin; x < xmax; x += pas) {
      fl_line(XDisp(x), YDisp(f(x)), XDisp(x + pas), YDisp(f(x + pas)));
    }
  }

  void text(const char *str) { fl_draw(str, XPm, YPM + 12); }

  void draw() override {
    fl_color(FL_BLUE);

    int xmaxe, ymaxe, stg = 50, drt = 50;
    float pas = 0.1;
    init_grafic();

    xmaxe = width;
    ymaxe = height;

    XPm = stg;
    XPM = xmaxe / 2 - drt / 2;
    YPm = stg;
    YPM = ymaxe / 2 - drt / 2;

    grafic(-5, 5, pas, f1);
    text("tan(x)");

    // functia f2 ...
    XPm = xmaxe / 2 + drt / 2;
    XPM = xmaxe - drt;
    YPm = stg;
    YPM = ymaxe / 2 - drt / 2;
    grafic(-5, 5, pas, f2);
    text("x * sin(10 * x)");
    // functia f3 ...
    XPm = stg;
    XPM = xmaxe / 2 - drt / 2;
    YPm = ymaxe / 2 + drt / 2;
    YPM = ymaxe - drt;
    grafic(-5, 5, pas, f3);
    text("sin(10 * x)");
    // functia f4 ...
    XPm = xmaxe / 2 + drt / 2;
    XPM = xmaxe - drt;
    YPm = ymaxe / 2 + drt / 2;
    YPM = ymaxe - drt;
    grafic(-5, 5, pas, f4);
    text("(x * x - 2) * (x + 3)");
  }
};
