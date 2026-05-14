#pragma once
#include <Screen.h>

// Field-by-field clock/date editor.
//
// Navigation:
//   NEXT (tap or held for accel) : increment current field value
//   OK                           : confirm field, advance to next
//                                  on final field (Year) saves and calls ui.goBack()
//
// Fields: Hour → Minute → Day → Month → Year
class ClockSetScreen : public Screen {
public:
    void onEnter(UINav& ui) override;
    void update(UINav& ui) override;
    void render(Adafruit_GFX& display) override;
    void onNext(UINav& ui) override;
    void onOk(UINav& ui) override;

    uint16_t refreshMs() const override { return 100; }

private:
    enum Field : uint8_t { HOUR, MINUTE, DAY, MONTH, YEAR };

    Field   _field  = HOUR;
    int16_t _hour   = 0;
    int16_t _minute = 0;
    int16_t _day    = 1;
    int16_t _month  = 1;
    int16_t _year   = 2024;

    void    loadCurrentTime();
    void    saveAndExit(UINav& ui);
    void    increment();

    int16_t     value()    const;
    void        setValue(int16_t v);
    int16_t     minVal()   const;
    int16_t     maxVal()   const;
    const char* fieldName() const;
};
