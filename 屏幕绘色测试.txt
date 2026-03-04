#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// ======= 屏幕引脚定义（根据你的接法改）=======
#define TFT_CS   7
#define TFT_DC   6
#define TFT_RST  8  // 如果屏幕没接RST脚，可写 -1

#define TFT_MOSI 17
#define TFT_SCLK 18
#define TFT_MISO -1  // 不用读取可以写 -1

// ======= 初始化 TFT 屏幕 =======
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ========== 程序开始 ==========
void setup() {
  Serial.begin(115200);
  Serial.println("开始初始化屏幕...");

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
  tft.begin();
  tft.setRotation(3); // 横屏模式（可改成0,1,2,3试）

  Serial.println("屏幕初始化完成，开始绘制颜色渐变...");

  // 屏幕分辨率 240x320
  int width = tft.width();
  int height = tft.height();

  for (int x = 0; x < width; x++) {
    // 根据X位置生成RGB分量
    uint8_t r = map(x, 0, width, 0, 255);
    uint8_t g = 255 - abs(120 - x % 240);
    uint8_t b = 255 - r;

    uint16_t color = tft.color565(r, g, b);
    tft.drawFastVLine(x, 0, height, color);
  }

  Serial.println("颜色绘制完成！");
}

void loop() {
  // 不重复绘制
}
