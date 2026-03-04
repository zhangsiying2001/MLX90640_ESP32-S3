#include <Wire.h>
#include <Adafruit_MLX90640.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <TFT_eSPI.h>

// ===== ILI9341 引脚 =====
#define TFT_CS   7
#define TFT_DC   6
#define TFT_RST  8
#define TFT_MOSI 17
#define TFT_SCLK 18
#define TFT_MISO -1

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ===== MLX90640 引脚 =====
#define SDA_PIN 3
#define SCL_PIN 4
Adafruit_MLX90640 mlx;
float frame[32*24];

// ===== 缩小分辨率 =====
const int dispCols = 320;
const int dispRows = 240;
uint16_t frameBuffer[dispCols*dispRows];

// ===== 原始热像素 =====
const int pixelCols = 32;
const int pixelRows = 24;

// ===== 热成像颜色映射 =====
uint16_t colorMap(float t, float tmin, float tmax) {
  t = constrain(t, tmin, tmax);
  float f = (t - tmin) / (tmax - tmin); // 0~1

  // HSV 色相：240° (蓝) -> 0° (红)
  float H = (1.0 - f) * 240.0;  
  float S = 1.0;
  float V = 1.0;

  float C = V * S;
  float X = C * (1 - abs(fmod(H/60.0, 2) - 1));
  float m = V - C;

  float r1, g1, b1;
  if(H < 60){ r1=C; g1=X; b1=0; }
  else if(H<120){ r1=X; g1=C; b1=0; }
  else if(H<180){ r1=0; g1=C; b1=X; }
  else if(H<240){ r1=0; g1=X; b1=C; }
  else if(H<300){ r1=X; g1=0; b1=C; }
  else { r1=C; g1=0; b1=X; }

  uint8_t r = (uint8_t)((r1+m)*255);
  uint8_t g = (uint8_t)((g1+m)*255);
  uint8_t b = (uint8_t)((b1+m)*255);

  return tft.color565(r,g,b);
}


void setup() {
  Serial.begin(115200);

  // 初始化屏幕
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
  tft.begin();
  tft.setRotation(3);   // 横屏
  tft.fillScreen(ILI9341_BLACK);

  // 初始化 MLX90640
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!mlx.begin(0x33,&Wire)){
    Serial.println("未检测到 MLX90640！");
    while(1);
  }
  mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setRefreshRate(MLX90640_4_HZ);
  Serial.println("MLX90640 初始化成功");
}

void loop() {
  if(mlx.getFrame(frame) != 0){
    Serial.println("读取失败");
    return;
  }

  // 找最小最大温度
  float tmin = 1000, tmax = -1000;
  for(int i=0;i<pixelCols*pixelRows;i++){
    if(frame[i]<tmin) tmin=frame[i];
    if(frame[i]>tmax) tmax=frame[i];
  }
  if(tmax-tmin < 0.1) tmax=tmin+0.1;

  // === 缩小分辨率 + 双线性插值 ===
  for(int y=0; y<dispRows; y++){
    float fy = y*(pixelRows-1.0)/(dispRows-1);
    int y0 = (int)fy, y1 = min(y0+1,pixelRows-1);
    float dy = fy - y0;

    for(int x=0; x<dispCols; x++){
      float fx = x*(pixelCols-1.0)/(dispCols-1);
      int x0 = (int)fx, x1 = min(x0+1,pixelCols-1);
      float dx = fx - x0;

      float t00 = frame[y0*pixelCols + x0];
      float t10 = frame[y0*pixelCols + x1];
      float t01 = frame[y1*pixelCols + x0];
      float t11 = frame[y1*pixelCols + x1];

      float temp = t00*(1-dx)*(1-dy) + t10*dx*(1-dy) + t01*(1-dx)*dy + t11*dx*dy;
      frameBuffer[y*dispCols + x] = colorMap(temp,tmin,tmax);
    }
  }

  // === 一次性刷新屏幕，不逐像素 ===
//tft.pushImage(0,0,dispCols,dispRows,frameBuffer);
tft.drawRGBBitmap(0, 0, frameBuffer, dispCols, dispRows);



  delay(50); // 控制刷新
}
