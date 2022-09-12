# Relay Fan Output for ESPhome and Home Assistant

I just discovered [ESPHome](https://esphome.io/) and [Home
Assistant](https://www.home-assistant.io/), and they are pretty much
the most amazing thing since sliced bread.  But I needed a slightly
different method of output (I think I did anyway) that will guarantee
strict timings of relays to ensure no major switching transients with
a motor.

This is an output module for controlling relay based fans, or other
interlocking mechanisms where it's important that you exclusively only
have one output pin on at a time, including a dead time between
switches, and a minimum on time.

**Why is this needed?  Doesn't the interlock mechanism do this?**

I found that the current interlocking scheme didn't work to my
satisfaction, because if the server spams the esphome device, you
could end up with rapid switch changes in succession.  The point is to
prevent this at the microcontroller level, and as much as possible,
prevent any rapid switching on the relays (which could be bad to
disasterous for motors and other inductive loads.)

## How to use

There is a full example in `fan_test.yaml`, but here's the gist of it:


```yaml

esphome:
  includes: relayfan.h
  libraries:
    - Ticker

...


# Create a 3 speed fan
fan:
  - platform: speed
    output: relay_fan
    name: "Relay Fan 3 Speed"
    speed_count: 3
    id: attic_fan

# Create the relay fan custom output
# Make sure speed_count above is the same as n_speeds below
# The speeds array are the pin numbers for low, medium and high.
output:
  - platform: custom
    type: float
    lambda: |-
      static int speeds[] = {16, 17, 18};
      int n_speeds = 3;
      auto relay_fan = new RelayFanOutput(speeds, n_speeds, 2000, 100);
      App.register_component(relay_fan);
      return {relay_fan};

    outputs:
      id: relay_fan

```


That should pretty much do it.
