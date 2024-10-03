#include <Arduino.h>
#include <M5AtomDisplay.h>
#include <M5Unified.h>

M5AtomDisplay display(240, 120, 120, 480, 320, 2, 2, 74250000);

String hello_str = "こんにちは、私はスタックチャンです。スーパーかわいい手乗りロボットとしてあなたのそばにいます。";




void setup() {
  display.init();
  delay(5000);
  display.setFont(&fonts::lgfxJapanGothic_40);


}

void loop() {
  display.startWrite();

  for (int i = display.width(); i > -display.textWidth(hello_str); --i)
  {
    display.drawString(hello_str, i, (display.height() - display.fontHeight()) >> 1);
  }
  display.endWrite();
  delay(20);
}