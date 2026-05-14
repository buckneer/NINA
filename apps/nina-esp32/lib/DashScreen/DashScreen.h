#pragma once
#include <Screen.h>
#include <DashWidget.h>
#include <DashState.h>

// Main dashboard screen.
//
// Layout (128x64):
//   y=0  ┌─────────────────────┐
//        │  top widget (~42px) │  e.g. ClockWidget
//   y=42 ├─────────────────────┤
//        │  Trip: xxx km       │
//        ├─────────────────────┤  divider
//        │  Odo:  xxx km       │
//   y=64 └─────────────────────┘
//
// The bottom strip is always rendered by this class.
// The top zone is delegated to the active DashWidget.
class DashScreen : public Screen {
public:
    void setWidget(DashWidget* widget) { _topWidget = widget; }
    void setState(const DashState& s)  { _state = s; }

    void render(Adafruit_GFX& display) override;
    uint16_t refreshMs() const override { return 500; }

private:
    DashWidget* _topWidget = nullptr;
    DashState   _state;

    void renderBottomStrip(Adafruit_GFX& display);
};
