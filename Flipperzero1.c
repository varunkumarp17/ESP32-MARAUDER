#if defined(__has_include)
  #if __has_include(<Arduino.h>)
    #include <Arduino.h>
  #endif
#endif

#if defined(__has_include) && __has_include(<Adafruit_GFX.h>) && __has_include(<Adafruit_ST7735.h>)
  #include <Adafruit_GFX.h>
  #include <Adafruit_ST7735.h>
  #include <SPI.h>
#else
  #include <stdint.h>
  #include <stddef.h>
  #include <string>
  #include <chrono>
  #include <thread>

  #define INITR_BLACKTAB 0

  static inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  template<typename T>
  static inline T constrain(T amt, T low, T high) {
    return amt < low ? low : (amt > high ? high : amt);
  }

  template<typename T>
  static inline T min(T a, T b) {
    return a < b ? a : b;
  }

  class String {
  public:
    String() = default;
    String(const char* s): _data(s ? s : "") {}
    String(const std::string& s): _data(s) {}
    String(char c): _data(1, c) {}
    String(int v): _data(std::to_string(v)) {}

    String& operator+=(const String& other) {
      _data += other._data;
      return *this;
    }

    String& operator+=(char c) {
      _data.push_back(c);
      return *this;
    }

    String operator+(const String& other) const {
      return String(_data + other._data);
    }

    bool operator==(const String& other) const {
      return _data == other._data;
    }

    bool startsWith(const String& prefix) const {
      return _data.rfind(prefix._data, 0) == 0;
    }

    int indexOf(char c, int fromIndex = 0) const {
      auto pos = _data.find(c, fromIndex);
      return pos == std::string::npos ? -1 : (int)pos;
    }

    int toInt() const {
      return _data.empty() ? 0 : std::stoi(_data);
    }

    int length() const {
      return (int)_data.length();
    }

    void trim() {
      size_t start = _data.find_first_not_of(" \t\r\n");
      size_t end = _data.find_last_not_of(" \t\r\n");
      _data = (start == std::string::npos) ? std::string() : _data.substr(start, end - start + 1);
    }

    String substring(int from, int to) const {
      if (from < 0) from = 0;
      if (to < from) to = from;
      if (from >= (int)_data.length()) return String();
      return String(_data.substr(from, to - from));
    }

    String substring(int from) const {
      if (from < 0) from = 0;
      if (from >= (int)_data.length()) return String();
      return String(_data.substr(from));
    }

    operator const char*() const {
      return _data.c_str();
    }

    std::string _data;
  };

  static inline String operator+(const char* a, const String& b) {
    return String(a) + b;
  }

  static inline String operator+(const String& a, const char* b) {
    return a + String(b);
  }

  static inline String operator+(const String& a, char b) {
    return a + String(b);
  }

  class HardwareSerial {
  public:
    void begin(unsigned long) {}
    int available() { return 0; }
    char read() { return 0; }
    void print(const String&) {}
    void print(const char*) {}
    void print(char) {}
    void println(const String&) {}
    void println(const char*) {}
    void println() {}
  };

  static HardwareSerial Serial;

  static void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }

  static unsigned long millis() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    return (unsigned long)duration_cast<milliseconds>(steady_clock::now() - start).count();
  }

  class Adafruit_ST7735 {
  public:
    Adafruit_ST7735(int8_t, int8_t, int8_t, int8_t, int8_t) {}
    void initR(uint8_t) {}
    void setRotation(uint8_t) {}
    void fillScreen(uint16_t) {}
    void fillRoundRect(int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void fillCircle(int16_t, int16_t, int16_t, uint16_t) {}
    void fillTriangle(int16_t, int16_t, int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void drawPixel(int16_t, int16_t, uint16_t) {}
    void drawRect(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void fillRect(int16_t, int16_t, int16_t, int16_t, uint16_t) {}
    void drawFastHLine(int16_t, int16_t, int16_t, uint16_t) {}
    void setTextColor(uint16_t) {}
    void setTextSize(uint8_t) {}
    void setCursor(int16_t, int16_t) {}
    void print(const String&) {}
    void print(const char*) {}
    void print(char) {}
  };
#endif

#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    3
#define TFT_SCLK   4
#define TFT_MOSI   5

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// ─── Flipper Zero style colors ───────────────────────────
#define F_BG      0x0000
#define F_ORANGE  0xFD20
#define F_WHITE   0xFFFF
#define F_GRAY    0x8410
#define F_DKGRAY  0x4208
#define F_YELLOW  0xFFE0
#define F_GREEN   0x07E0
#define F_RED     0xF800
#define F_BLUE    0x07FF

// ─── Menu items ──────────────────────────────────────────
#define MAX_ITEMS 8
String menuItems[MAX_ITEMS] = {
  "Sub-GHz", "NFC", "Infrared", "iButton",
  "Bad USB", "GPIO", "Settings", "About"
};

uint16_t menuColors[MAX_ITEMS] = {
  F_ORANGE, F_BLUE, F_RED, F_GREEN,
  F_WHITE,  F_YELLOW, F_GRAY, F_ORANGE
};

int  selectedItem  = 0;
int  scrollOffset  = 0;
bool inSubMenu     = false;
bool inTextMode    = false;

// ─── Status bar data ─────────────────────────────────────
String statusLeft  = "FZ-ESP32";
String statusRight = "100%";
int    battPct     = 100;

// ─── Text mode buffer ────────────────────────────────────
String textBuffer  = "";
String textTitle   = "TERMINAL";
String serialInput = "";

unsigned long lastBlink = 0;
bool          blinkOn   = false;

// ─── Flipper dolphin ─────────────────────────────────────
void drawDolphin(int x, int y, uint16_t color) {
  tft.fillRoundRect(x,    y+4,  18, 12, 3, color);
  tft.fillCircle(x+18, y+8, 6, color);
  tft.fillCircle(x+20, y+6, 1, F_BG);
  tft.fillTriangle(x, y+4,  x, y+15, x-6, y,    color);
  tft.fillTriangle(x, y+4,  x, y+15, x-6, y+19, color);
  tft.fillTriangle(x+8, y+4, x+14, y+4, x+11, y, color);
  tft.drawPixel(x+22, y+10, F_BG);
  tft.drawPixel(x+23, y+11, F_BG);
}

// ─── Battery icon ────────────────────────────────────────
void drawBattery(int x, int y, int pct) {
  tft.drawRect(x, y, 16, 8, F_WHITE);
  tft.fillRect(x+16, y+2, 2, 4, F_WHITE);
  uint16_t c = pct > 50 ? F_GREEN :
               pct > 20 ? F_YELLOW : F_RED;
  int fill = map(pct, 0, 100, 0, 14);
  tft.fillRect(x+1, y+1, fill, 6, c);
}

// ─── Status bar ──────────────────────────────────────────
void drawStatusBar() {
  tft.fillRect(0, 0, 160, 14, F_DKGRAY);
  tft.drawFastHLine(0, 14, 160, F_ORANGE);
  tft.setTextColor(F_ORANGE);
  tft.setTextSize(1);
  tft.setCursor(3, 3);
  tft.print(statusLeft);
  drawBattery(130, 3, battPct);
  tft.setTextColor(F_WHITE);
  tft.setCursor(108, 3);
  tft.print(battPct);
  tft.print("%");
}

// ─── Main menu ───────────────────────────────────────────
void drawMainMenu() {
  tft.fillRect(0, 15, 160, 145, F_BG);
  drawDolphin(130, 18, F_ORANGE);
  tft.setTextColor(F_ORANGE);
  tft.setTextSize(1);
  tft.setCursor(4, 18);
  tft.print("[ MAIN MENU ]");
  tft.drawFastHLine(0, 28, 160, F_DKGRAY);

  int visible = min(MAX_ITEMS, 5);
  for (int i = 0; i < visible; i++) {
    int idx = i + scrollOffset;
    if (idx >= MAX_ITEMS) break;
    int y = 32 + i * 18;
    bool selected = (idx == selectedItem);
    if (selected) {
      tft.fillRect(0, y-1, 160, 16, menuColors[idx]);
      tft.setTextColor(F_BG);
    } else {
      tft.fillRect(0, y-1, 160, 16, F_BG);
      tft.setTextColor(menuColors[idx]);
    }
    tft.setTextSize(1);
    tft.setCursor(selected ? 14 : 10, y+2);
    if (selected) tft.print("> ");
    tft.print(menuItems[idx]);
    if (selected) {
      tft.fillTriangle(150, y+2, 150, y+10, 156, y+6, F_BG);
    }
  }

  if (MAX_ITEMS > 5) {
    int barH = map(5, 0, MAX_ITEMS, 0, 88);
    int barY = 32 + map(scrollOffset, 0, MAX_ITEMS-5, 0, 88-barH);
    tft.fillRect(158, 32, 2, 88, F_DKGRAY);
    tft.fillRect(158, barY, 2, barH, F_ORANGE);
  }

  tft.fillRect(0, 148, 160, 12, F_DKGRAY);
  tft.drawFastHLine(0, 148, 160, F_ORANGE);
  tft.setTextColor(F_ORANGE);
  tft.setTextSize(1);
  tft.setCursor(2, 151);
  tft.print("UP/DN=nav  OK=select");
}

// ─── Submenu screen ──────────────────────────────────────
void drawSubMenu(int idx) {
  tft.fillRect(0, 15, 160, 145, F_BG);
  tft.fillRect(0, 15, 160, 16, menuColors[idx]);
  tft.setTextColor(F_BG);
  tft.setTextSize(1);
  tft.setCursor(4, 19);
  tft.print("[ ");
  tft.print(menuItems[idx]);
  tft.print(" ]");
  tft.drawFastHLine(0, 31, 160, menuColors[idx]);
  drawDolphin(10, 40, menuColors[idx]);
  tft.setTextColor(menuColors[idx]);
  tft.setTextSize(1);
  tft.setCursor(50, 42);
  tft.print(menuItems[idx]);
  tft.setTextColor(F_GRAY);
  tft.setCursor(50, 54);
  tft.print("Module active");
  tft.setCursor(50, 66);
  tft.print("Status: READY");
  tft.drawFastHLine(0, 82, 160, F_DKGRAY);
  tft.setTextColor(F_ORANGE);
  tft.setCursor(4, 86);
  tft.print("SERIAL LOG:");
  tft.drawFastHLine(0, 96, 160, F_DKGRAY);
  tft.setTextColor(F_GREEN);
  tft.setCursor(4, 100);
  tft.print("> ");
  tft.setTextColor(F_WHITE);
  tft.print(textBuffer.substring(0, 17));
  tft.fillRect(0, 148, 160, 12, F_DKGRAY);
  tft.drawFastHLine(0, 148, 160, F_ORANGE);
  tft.setTextColor(F_ORANGE);
  tft.setTextSize(1);
  tft.setCursor(2, 151);
  tft.print("BACK=back  TYPE=input");
}

// ─── Text terminal screen ────────────────────────────────
void drawTextMode() {
  tft.fillRect(0, 15, 160, 145, F_BG);
  tft.fillRect(0, 15, 160, 14, F_DKGRAY);
  tft.drawFastHLine(0, 29, 160, F_GREEN);
  tft.setTextColor(F_GREEN);
  tft.setTextSize(1);
  tft.setCursor(4, 18);
  tft.print("// ");
  tft.print(textTitle);
  tft.print(" //");

  int maxChars = 19;
  int lines    = 7;
  int startY   = 34;

  for (int i = 0; i < lines; i++) {
    int start = i * maxChars;
    if (start >= (int)textBuffer.length()) break;
    tft.setCursor(4, startY + i * 14);
    tft.setTextColor(i == 0 ? F_ORANGE : F_WHITE);
    tft.print(textBuffer.substring(start,
              min((int)textBuffer.length(), start + maxChars)));
  }

  int cursorLine = textBuffer.length() / maxChars;
  int cursorCol  = textBuffer.length() % maxChars;
  if (blinkOn && cursorLine < lines) {
    tft.setTextColor(F_GREEN);
    tft.setCursor(4 + cursorCol * 6, startY + cursorLine * 14);
    tft.print("_");
  }

  tft.fillRect(0, 148, 160, 12, F_DKGRAY);
  tft.drawFastHLine(0, 148, 160, F_GREEN);
  tft.setTextColor(F_GREEN);
  tft.setTextSize(1);
  tft.setCursor(2, 151);
  tft.print("CLEAR=clr  BACK=menu");
}

// ─── Parse serial command ────────────────────────────────
void parseCommand(String cmd) {
  cmd.trim();
  Serial.println("CMD: [" + cmd + "]");

  if (cmd == "UP") {
    if (!inSubMenu && !inTextMode) {
      if (selectedItem > 0) selectedItem--;
      if (selectedItem < scrollOffset) scrollOffset--;
      drawStatusBar();
      drawMainMenu();
    }
    return;
  }

  if (cmd == "DOWN") {
    if (!inSubMenu && !inTextMode) {
      if (selectedItem < MAX_ITEMS - 1) selectedItem++;
      if (selectedItem >= scrollOffset + 5) scrollOffset++;
      drawStatusBar();
      drawMainMenu();
    }
    return;
  }

  if (cmd == "OK") {
    if (!inSubMenu && !inTextMode) {
      inSubMenu = true;
      drawStatusBar();
      drawSubMenu(selectedItem);
    }
    return;
  }

  if (cmd == "BACK") {
    if (inTextMode) {
      inTextMode = false;
      inSubMenu  = true;
      drawStatusBar();
      drawSubMenu(selectedItem);
    } else if (inSubMenu) {
      inSubMenu = false;
      drawStatusBar();
      drawMainMenu();
    }
    return;
  }

  if (cmd == "CLEAR") {
    textBuffer = "";
    if (inTextMode) drawTextMode();
    return;
  }

  if (cmd == "TERM") {
    inTextMode = true;
    inSubMenu  = false;
    textTitle  = "TERMINAL";
    textBuffer = "";
    drawStatusBar();
    drawTextMode();
    return;
  }

  if (cmd.startsWith("STATUS:")) {
    statusLeft = cmd.substring(7);
    drawStatusBar();
    return;
  }

  if (cmd.startsWith("BATT:")) {
    battPct = constrain(cmd.substring(5).toInt(), 0, 100);
    statusRight = String(battPct) + "%";
    drawStatusBar();
    return;
  }

  if (cmd.startsWith("MENU:")) {
    int sep = cmd.indexOf(':', 5);
    if (sep != -1) {
      int idx = cmd.substring(5, sep).toInt();
      if (idx >= 0 && idx < MAX_ITEMS) {
        menuItems[idx] = cmd.substring(sep + 1);
        if (!inSubMenu && !inTextMode) drawMainMenu();
      }
    }
    return;
  }

  // Anything else → display on screen
  if (inTextMode) {
    textBuffer += cmd + " ";
    drawTextMode();
  } else if (inSubMenu) {
    textBuffer = cmd;
    drawSubMenu(selectedItem);
  } else {
    inTextMode = true;
    textTitle  = "TERMINAL";
    textBuffer = cmd + " ";
    drawStatusBar();
    drawTextMode();
  }

  Serial.println("OK: " + cmd);
}

// ─── Setup ───────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(2000);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(F_BG);

  // Boot splash
  tft.setTextColor(F_ORANGE);
  tft.setTextSize(2);
  tft.setCursor(28, 30);
  tft.print("FLIPPER");
  tft.setCursor(42, 50);
  tft.print("ZERO");
  tft.setTextSize(1);
  tft.setTextColor(F_GRAY);
  tft.setCursor(32, 74);
  tft.print("ESP32 Edition");
  drawDolphin(70, 90, F_ORANGE);

  tft.drawRect(10, 118, 140, 6, F_DKGRAY);
  for (int i = 0; i <= 136; i += 4) {
    uint16_t c = i < 45  ? F_RED    :
                 i < 90  ? F_YELLOW :
                 i < 120 ? F_GREEN  : F_ORANGE;
    tft.fillRect(11, 119, i, 4, c);
    delay(12);
  }

  tft.setTextColor(F_ORANGE);
  tft.setCursor(36, 130);
  tft.print("READY!");
  delay(800);

  tft.fillScreen(F_BG);
  drawStatusBar();
  drawMainMenu();

  Serial.println("=== FLIPPER ESP32 READY ===");
  Serial.println("Commands: UP DOWN OK BACK TERM CLEAR");
  Serial.println("STATUS:text  BATT:val  MENU:idx:name");
}

// ─── Loop ────────────────────────────────────────────────
void loop() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    Serial.print(c);
    if (c == '\n' || c == '\r') {
      serialInput.trim();
      if (serialInput.length() > 0) {
        parseCommand(serialInput);
        serialInput = "";
      }
    } else {
      serialInput += c;
    }
  }

  if (inTextMode) {
    if (millis() - lastBlink >= 500) {
      lastBlink = millis();
      blinkOn   = !blinkOn;
      drawTextMode();
    }
  }
} 
riyakarotiya786@gm