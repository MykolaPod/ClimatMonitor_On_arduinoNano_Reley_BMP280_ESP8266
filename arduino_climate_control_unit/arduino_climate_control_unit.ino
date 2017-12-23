#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SoftwareSerial.h>

#define RelayPin 8
#define ErrorCodeForSensorsData -500
#define EspRxPinOnArduino 11
#define EspTxPinOnArduino 10

SoftwareSerial espSerial(EspRxPinOnArduino, EspTxPinOnArduino); // RX, TX

Adafruit_BMP280 bmp; // I2C // pin 3 - Serial clock out (SCLK) // pin 4 - Serial data out (DIN)// pin 5 - Data/Command select (D/C)// pin 6 - LCD chip select (CS)// pin 7 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 6, 7);


bool _releyState = false; //false OFF, true ON
bool _isBmpOk = false;
bool _isEspOk = false;
bool _isInitialized = false;
float _bmpPreasure = ErrorCodeForSensorsData;
float _bmpTemp = ErrorCodeForSensorsData;

const static unsigned char PROGMEM logoBmp[] =
{
  B00000000, B00011111, B11111000, B00000000, B00000111, B11111100, B00000000, B00000011, B11111111, B10000000, B00000000,
  B00000000, B01111111, B11111111, B00000000, B00111111, B11111111, B00000000, B00001111, B11111111, B11100000, B00000000,
  B00000001, B11111111, B11111111, B10000000, B11111111, B11111110, B00000000, B00111111, B11111111, B11111000, B00000000,
  B00000011, B11111111, B11111111, B11100001, B11111111, B11111000, B00000000, B11111111, B11111111, B11111100, B00000000,
  B00001111, B11111100, B00111111, B11100011, B11111111, B00110000, B00000001, B11111111, B10000011, B11111111, B00000000,
  B00001111, B11100000, B00000111, B11100111, B11110000, B00000000, B00000000, B11111100, B00000000, B01111111, B00000000,
  B00011111, B10000000, B00000001, B11001111, B11100000, B00000000, B00001110, B01110000, B00000000, B00011111, B10000000,
  B00111111, B00000000, B00000000, B10011111, B10000000, B00000000, B00111110, B01100000, B00000000, B00001111, B11000000,
  B00111110, B00000000, B00000000, B00011111, B00000000, B00000000, B01111111, B00000000, B00000000, B00000111, B11000000,
  B01111100, B00000000, B00000000, B00111111, B00000000, B00000001, B11111111, B00000000, B00000000, B00000011, B11100000,
  B01111100, B00000000, B00000000, B00111110, B00000000, B00000011, B11111111, B10000000, B00000000, B00000011, B11100000,
  B11111000, B00000000, B00010000, B00111100, B00000000, B00001111, B11111111, B10000000, B00000000, B00000001, B11110000,
  B11111000, B00000000, B01111000, B01111100, B00000000, B00111111, B11100111, B10000000, B00000000, B00000001, B11110000,
  B11111000, B00000000, B11111000, B01111100, B00000000, B01111111, B10000111, B11000000, B00000000, B00000001, B11110000,
  B11111000, B00000011, B11111100, B01111100, B00000001, B11111111, B00000111, B11000000, B00000000, B00000000, B11110000,
  B11110000, B00000111, B11111000, B01111100, B00000011, B11111100, B00000111, B11000000, B00000000, B00000000, B11110000,
  B11110000, B00011111, B11100000, B01111100, B00001111, B11111000, B00000111, B11000000, B00000000, B00000000, B11110000,
  B11111000, B01111111, B11000000, B01111100, B00011111, B11100000, B00000111, B11000000, B00000000, B00000001, B11110000,
  B11111000, B11111111, B00000000, B01111100, B01111111, B11000000, B00000111, B11000000, B00000000, B00000001, B11110000,
  B11111011, B11111110, B00000000, B01111100, B11111111, B00000000, B00000111, B10000000, B00000000, B00000001, B11110000,
  B11111111, B11111000, B00000000, B00111110, B11111100, B00000000, B00001111, B10000000, B00000000, B00000001, B11110000,
  B01111111, B11110000, B00000000, B00111110, B01111000, B00000000, B00001111, B10000000, B00000000, B00000011, B11100000,
  B01111111, B11000000, B00000000, B00111111, B01100000, B00000000, B00011111, B00000000, B00000000, B00000011, B11100000,
  B00111111, B00000000, B00000000, B00011111, B00000000, B00000000, B00111111, B00100000, B00000000, B00000111, B11000000,
  B00111111, B00000000, B00000000, B11001011, B10000000, B00000000, B01111110, B01110000, B00000000, B00001111, B11000000,
  B00011111, B11000000, B00000011, B11001100, B01000000, B00000000, B11111100, B01111000, B00000000, B00111111, B10000000,
  B00001111, B11110000, B00001111, B11100111, B11110000, B00000011, B11111100, B11111110, B00000000, B01111111, B00000000,
  B00000111, B11111111, B11111111, B11100011, B11111111, B00111111, B11111000, B01111111, B11111111, B11111110, B00000000,
  B00000011, B11111111, B11111111, B11000001, B11111111, B11111111, B11100000, B00111111, B11111111, B11111100, B00000000,
  B00000001, B11111111, B11111111, B00000000, B01111111, B11111111, B11000000, B00001111, B11111111, B11110000, B00000000,
  B00000000, B01111111, B11111110, B00000000, B00011111, B11111111, B00000000, B00000011, B11111111, B11100000, B00000000,
  B00000000, B00001111, B11110000, B00000000, B00000111, B11111000, B00000000, B00000000, B01111111, B00000000, B00000000
};


void setup() {
  Serial.begin(115200);
  Serial.println("initialize");
  initialize();
  displayModuleSatuses();
}

void loop() {
  if (espSerial.available() > 0) {
    espSerial.read();
  }

  _bmpTemp = getBmpTemp();
  _bmpPreasure = getBmpPreasure();
  _releyState = getReleyState();

  modeController();
  displayParameters(_bmpTemp, _bmpPreasure, _releyState);
  delay(2000);
}

void initialize () {
  initDisplay();
  initRelay();
  initBmp();
  initEsp();

  delay(1000);
  _isInitialized = true;
}

void initDisplay() {
  display.begin();              // Инициализация дисплея
  display.setContrast(20);      // Устанавливаем контраст
  display.clearDisplay();
  display.setTextColor(BLACK);  // Устанавливаем цвет текста
  display.setTextSize(1);       // Устанавливаем размер текста
  display.clearDisplay();       // Очищаем дисплей
  display.display();
  delay(1000);
  display.drawBitmap(0, 0, logoBmp, 84, 32, BLACK); // x, y, logo, w, h, color
  display.setCursor(0, 33);
  display.print("creative media");
  display.setCursor(15, 40);
  display.print("solutions");
  display.display();
}
void initRelay() {
  pinMode(RelayPin, OUTPUT);
  turnReleyOff();
}
void initBmp() {
  _isBmpOk = bmp.begin();
}
void initEsp() {
  espSerial.begin(115200); //default baud rate for current firmware
  delay(500);
  Serial.println("change baud rate");
  espSerial.println("AT+UART_CUR=9600,8,1,0,0"); //software serial does not work on baud >38400 stable. Setting Esp baud to 9600 for correct work
  delay(500);
  espSerial.begin(9600);
  delay(500);
  bool isEspOk = getEspState();
  if(isEspOk){
    
  }
  else {
    
  }
}

void displayModuleSatuses() {
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  display.print("BMP280: ");
  if (!bmp.begin()) {
    display.println("FAIL");
  }
  else {
    display.println("OK");
  }
  
  display.print("Mode: ");
  if (getReleyState()) {
    display.println("ON");
  } else {
    display.println("OFF");
  }  

  display.print("WIFI: ");
  if (!getEspState()) {
    display.println("FAIL");
  }
  else {
    display.println("OK");
  }
  
  display.display();
  delay(1000);
}

void displayParameters (float bmpTemp, float bmpPreasure, bool releyState) {
  if (!_isInitialized) {
    return;
  }
  else {
    display.clearDisplay();
    display.display();
    display.setCursor(0, 0);
    display.print("Here:");
    display.print(bmpTemp);
    display.println(" C");
    display.print("Here:");
    display.print(bmpPreasure);
    display.println(" mA");
    display.drawLine(0, 16, display.width(), 16, BLACK);
    display.setCursor(0, 18);
    display.print("There:");
    display.print(bmpTemp);
    display.println(" C");
    display.print("There:");
    display.print(bmpTemp);
    display.println(" %");
    display.drawLine(0, 34, display.width(), 34, BLACK);
    display.setCursor(0, 36);
    display.print("Mode: ");
    if (getReleyState()) {
      display.println("ON");
    } else {
      display.println("OFF");
    }
    display.display();
  }
}

void modeController() {
  if (_bmpTemp == ErrorCodeForSensorsData) {
    return;
  }

  if (_bmpTemp > 28.00) {
    turnReleyOn();
  }
  if (_bmpTemp < 27.00) {
    turnReleyOff();
  }

}

void turnReleyOn () {
  digitalWrite(RelayPin, LOW);
  _releyState = true;
}

void turnReleyOff () {
  digitalWrite(RelayPin, HIGH);
  _releyState = false;
}

float getBmpTemp() {
  if (_isBmpOk) {
    return bmp.readTemperature();
  }
  else {
    return ErrorCodeForSensorsData;
  }
}

float getBmpPreasure() {
  if (_isBmpOk) {
    return (bmp.readPressure() / 101325) * 1000;
  }
  else {
    return ErrorCodeForSensorsData;
  }
}

bool getReleyState() {
  return _releyState;
}

bool getEspState () {
  byte attemptsCounter = 0;
  byte maxAttemptsBeforeResetEsp = 2;
  Serial.println("start getesp state");
  if (!espSerial) {
    _isEspOk = false;
    Serial.println("esp serial is false");
  }
  else {
    while (attemptsCounter < maxAttemptsBeforeResetEsp) {
      attemptsCounter++;
      Serial.println("sending at");
      espSerial.println("AT");
      delay(500);
      if (espSerial.available()) {
        if (espSerial.find("OK")) {
          Serial.println("OK received");
          _isEspOk = true;
          break;
        }
      }
      _isEspOk = false;
    }
  }
  return _isEspOk;
}
