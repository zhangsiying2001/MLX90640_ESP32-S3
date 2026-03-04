#include <Wire.h>
#include <Adafruit_MLX90640.h>

#define SDA_PIN 3
#define SCL_PIN 4

Adafruit_MLX90640 mlx;

float frame[32*24]; // 32*24 = 768

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(100);

  if (!mlx.begin()) {
    Serial.println("未检测到 MLX90640，请检查接线！");
    while (1) delay(10);
  }
  Serial.println("MLX90640 初始化成功");

  mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setResolution(MLX90640_ADC_18BIT);
  //mlx.setRefreshRate(MLX90640_8_HZ); // 1~64Hz 已测试 无信号
  mlx.setRefreshRate(MLX90640_1_HZ); // 1~64Hz


}

void loop() {
  for(int i=0; i<5; i++){
    if(mlx.getFrame(frame) == 0){
        Serial.println("读取成功");
        break;
    } else {
        Serial.println("读取失败");
        delay(100);
    }
}

  // 简单打印温度
  for (int i=0; i<32; i+=8) { // 每行取8个点打印
    for (int j=0; j<24; j+=8) {
      Serial.print(frame[i*24 + j], 1);
      Serial.print("\t");
    }
    Serial.println();
  }
  Serial.println("----");
  delay(500);
}
