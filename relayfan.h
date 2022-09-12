#include "esphome.h"
#include "math.h"
#include <Ticker.h>

using namespace esphome;

typedef struct {
    int next;
    int current;
    int is_off;
    int64_t interlocked_until;
    int64_t interlock_time_us;
    int n_speeds;
    int *speeds;
} FanSettings_t;

class RelayFanOutput : public Component, public FloatOutput {
private:
    FanSettings_t settings;
    Ticker periodicTicker;
    int check_interval_ms;

    static void handle_timer(FanSettings_t *settings) {
        // state machine is stepped at each call:
        int64_t now = esp_timer_get_time();
        if (now < settings->interlocked_until) {
            // nothing to do.  buh bye
        } else {
            if (settings->next != settings->current) {
                // if not equal... turn everything off and set the interlock.
                for (int i = 0; i < settings->n_speeds; i++)
                    digitalWrite(settings->speeds[i], LOW);
                settings->is_off = 1;
                ESP_LOGD("fan", "turned all off");
                settings->interlocked_until = now + settings->interlock_time_us;
                settings->current = settings->next;
            } else {
                if (settings->is_off) {
                    settings->is_off = 0;
                    settings->current = settings->next;
                    digitalWrite(settings->current, HIGH);
                    ESP_LOGD("fan", "turned on pin %d", settings->current);
                }
                settings->interlocked_until = now + settings->interlock_time_us;
            }
        }
    }
private:
    RelayFanOutput(void) {};
    
public:
    RelayFanOutput(int *speed_pins, int n_speeds,
                           int interlock_time_ms=2000,
                           int check_interval_ms=500) {
        settings.speeds = speed_pins;
        settings.n_speeds = n_speeds;
        settings.interlock_time_us = interlock_time_ms*1000;
        settings.current = 0;
        settings.next = 0;
        settings.is_off = 1;
        settings.n_speeds = n_speeds;
        settings.speeds = speed_pins;
        this->check_interval_ms = check_interval_ms;
    }

    void setup() override {
        // This will be called by App.setup()
        int64_t now = esp_timer_get_time();
        for (int i = 0; i < settings.n_speeds; i++) {
            int pin = settings.speeds[i];
            pinMode(settings.speeds[i], OUTPUT);
            digitalWrite(pin, LOW);
        }
        settings.interlocked_until = now + settings.interlock_time_us;
        periodicTicker.attach_ms(check_interval_ms, handle_timer, &settings);
    }
  
  void write_state(float state) override {
      // state is the amount this output should be on, from 0.0 to 1.0
      // we need to convert it to an integer first
      int value = round(state * settings.n_speeds);
      if (value < 0 || value > settings.n_speeds) {
          ESP_LOGD("fan", "INVALID state value: %f", state);
          return;
      }
      int pin = -1; // all off.
      if (value > 0)
          pin = settings.speeds[value-1];
      ESP_LOGD("fan", "state = %f, value = %d, pin = %d", state, value, pin);
      settings.next = pin;
  }
};
