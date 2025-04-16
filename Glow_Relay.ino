#include <TFT_eSPI.h>
#include <Arduino.h>

// Wio Terminal引脚定义（基于官方Wiki）
TFT_eSPI tft = TFT_eSPI();

const int SCREEN_W = 320;
const int SCREEN_H = 240;
const int LIGHT_SIZE = 12;
const int TORCH_SIZE = 8;
const int TRACK_Y[] = {50, 120, 190};

int lightTrack = 1;
int torchX = 320;
int torchTrack = 0;
float torchSpeed = 3.0;
int score = 0;
int torchCount = 0;
bool gameOver = false;
int colorIndex = 0;
int frameCount = 0;
uint16_t colors[] = {TFT_RED, 0xFD20, TFT_YELLOW, TFT_GREEN, 0x07FF, TFT_BLUE, TFT_PURPLE};
uint16_t darkColors[] = {0x7800, 0x7A00, 0x7BC0, 0x03E0, 0x051F, 0x001F, 0x780F};

// 绘制五角星
void drawStar(int x, int y, int r, uint16_t color) {
  float angle = PI / 2;
  int vertices[10];
  for (int i = 0; i < 10; i += 2) {
    vertices[i] = x + r * cos(angle);
    vertices[i + 1] = y - r * sin(angle);
    angle += PI / 5;
    vertices[i + 2] = x + r * 0.5 * cos(angle);
    vertices[i + 3] = y - r * 0.5 * sin(angle);
    angle += PI / 5;
  }
  tft.fillTriangle(vertices[0], vertices[1], vertices[4], vertices[5], vertices[8], vertices[9], color);
  tft.fillTriangle(vertices[2], vertices[3], vertices[6], vertices[7], vertices[8], vertices[9], color);
  tft.fillTriangle(vertices[2], vertices[3], vertices[4], vertices[5], vertices[0], vertices[1], color);
}

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(3);
  
  // 星空渐变背景
  for (int y = 0; y < SCREEN_H / 2; y++) {
    tft.drawFastHLine(0, y, SCREEN_W, tft.color565(0, 0, 16 * y / SCREEN_H));
  }
  for (int y = SCREEN_H / 2; y < SCREEN_H; y++) {
    tft.drawFastHLine(0, y, SCREEN_W, TFT_BLACK);
  }
  // 随机星星
  for (int i = 0; i < 8; i++) {
    tft.drawPixel(random(SCREEN_W), random(SCREEN_H), TFT_WHITE);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  pinMode(WIO_BUZZER, OUTPUT);
  digitalWrite(WIO_BUZZER, LOW);

  // 细线跑道（淡紫）
  for (int i = 0; i < 3; i++) {
    tft.drawFastHLine(0, TRACK_Y[i], SCREEN_W, 0xC81F);
  }
}

void loop() {
  frameCount++;
  
  if (gameOver) {
    // 渐变红屏
    for (int y = 0; y < SCREEN_H; y++) {
      tft.drawFastHLine(0, y, SCREEN_W, tft.color565(128 - y / 4, 0, 0));
    }
    // 金边文字
    tft.setTextColor(0xFFE0);
    tft.setCursor(51, 101); tft.print("Game Over!");
    tft.setCursor(51, 131); tft.print("Score: "); tft.print(score);
    tft.setCursor(51, 161); tft.print("Press C to Restart");
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(50, 100); tft.print("Game Over!");
    tft.setCursor(50, 130); tft.print("Score: "); tft.print(score);
    tft.setCursor(50, 160); tft.print("Press C to Restart");
    
    if (digitalRead(WIO_KEY_C) == LOW) {
      score = 0;
      torchCount = 0;
      lightTrack = 1;
      torchX = 320;
      torchTrack = random(3);
      torchSpeed = 3.0;
      colorIndex = 0;
      gameOver = false;
      // 重画背景
      for (int y = 0; y < SCREEN_H / 2; y++) {
        tft.drawFastHLine(0, y, SCREEN_W, tft.color565(0, 0, 16 * y / SCREEN_H));
      }
      for (int y = SCREEN_H / 2; y < SCREEN_H; y++) {
        tft.drawFastHLine(0, y, SCREEN_W, TFT_BLACK);
      }
      for (int i = 0; i < 8; i++) {
        tft.drawPixel(random(SCREEN_W), random(SCREEN_H), TFT_WHITE);
      }
      // 重画跑道
      for (int i = 0; i < 3; i++) {
        tft.drawFastHLine(0, TRACK_Y[i], SCREEN_W, 0xC81F);
      }
      delay(200);
    }
    delay(100);
    return;
  }

  // 检查按键
  bool keyPressed = false;
  if (digitalRead(WIO_KEY_A) == LOW && lightTrack != 0) {
    lightTrack = 0;
    keyPressed = true;
    Serial.println("Key A pressed");
  } else if (digitalRead(WIO_KEY_B) == LOW && lightTrack != 1) {
    lightTrack = 1;
    keyPressed = true;
    Serial.println("Key B pressed");
  } else if (digitalRead(WIO_KEY_C) == LOW && lightTrack != 2) {
    lightTrack = 2;
    keyPressed = true;
    Serial.println("Key C pressed");
  }
  if (keyPressed) {
    delay(100);
  }

  // 更新火炬
  tft.fillCircle(torchX, TRACK_Y[torchTrack], TORCH_SIZE + 2, TFT_BLACK);
  torchX -= (int)torchSpeed;
  if (torchX < 50 - TORCH_SIZE) {
    if (lightTrack == torchTrack) {
      score++;
      torchCount++;
      colorIndex = (colorIndex + 1) % 7;
      if (torchCount % 2 == 0) {
        torchSpeed += 0.5;
        if (torchSpeed > 8.0) torchSpeed = 8.0;
      }
      tone(WIO_BUZZER, 1000, 50);
      torchX = 320;
      torchTrack = random(3);
    } else {
      gameOver = true;
      tone(WIO_BUZZER, 500, 200);
    }
  }

  // 绘制光点（渐变星光）
  tft.fillCircle(50, TRACK_Y[lightTrack], LIGHT_SIZE + 2, TFT_BLACK);
  tft.fillCircle(50, TRACK_Y[lightTrack], 8, colors[colorIndex]);
  tft.fillCircle(50, TRACK_Y[lightTrack], LIGHT_SIZE, darkColors[colorIndex]);

  // 绘制火炬（五角星+光晕）
  if (frameCount % 5 < 3) {
    tft.fillCircle(torchX, TRACK_Y[torchTrack], TORCH_SIZE + 2, tft.color565(255, 255, 128));
    drawStar(torchX, TRACK_Y[torchTrack], TORCH_SIZE, TFT_WHITE);
  } else {
    tft.fillCircle(torchX, TRACK_Y[torchTrack], TORCH_SIZE + 2, tft.color565(200, 200, 100));
    drawStar(torchX, TRACK_Y[torchTrack], TORCH_SIZE, 0xC618);
  }

  // 分数
  tft.fillRect(0, 0, 100, 20, TFT_BLACK);
  tft.setCursor(0, 0);
  tft.print("Score: ");
  tft.print(score);

  delay(30);
}
