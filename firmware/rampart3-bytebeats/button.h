#pragma once

/* 
 *  from pikocore 

https://github.com/schollz/pikocore/tree/main/doth
  
 
 MIT License

Copyright (c) 2023 Zack Scholl

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

class Button {
  uint8_t gpio;
  bool val[2];
  bool rising;
  bool falling;
  bool changed;
  uint16_t debounce;
  uint16_t debounce_time;

 public:
  void Init(uint8_t gpio_, uint16_t debounce_time_) {
    gpio = gpio_;
//    gpio_init(gpio);
//    gpio_set_dir(gpio, GPIO_IN);
//    gpio_pull_up(gpio);
    pinMode(gpio, INPUT_PULLUP);
    //_state = digitalRead(_pin);
    debounce_time = debounce_time_;
    val[1] = (bool)(1 - digitalRead(gpio));
  }

  bool On() { return val[0]; }

  void Set(bool v) {
    val[0] = v;
    rising = val[0] > val[1];
    falling = val[0] < val[1];
    changed = rising || falling;
    if (changed) debounce = debounce_time;
    val[1] = val[0];
  }

  void Read() {
    if (debounce == 0) {
      Set((bool)(1 - digitalRead(gpio)));
    } else {
      changed = false;
      rising = false;
      falling = false;
      debounce--;
    }
  }
  bool Changed(bool reset) {
    // can only be read once
    if (changed && reset) {
      changed = false;
      return true;
    }
    return changed;
  }
  bool ChangedHigh(bool reset) {
    // can only be read once
    if (changed && reset) {
      changed = false;
      return true && val[1];
    }
    return changed && val[1];
  }
  bool Rising() { return rising; }
  bool Falling() { return falling; }
};
