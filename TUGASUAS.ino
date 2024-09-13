#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <TM1637Display.h>

// Pin Definitions
const int buzzerPin = 13;
const int buttonPin = 11;
const int tempPin = A0; // Pin analog untuk LM35
const int CLK = 2; // CLK pin of TM1637 time segment
const int DIO = 3; // DIO pin of TM1637 time segment

// LCD and RTC
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the I2C address if needed
RTC_DS3231 rtc;

// TM1637 Display Time Segment
TM1637Display display(CLK, DIO);

// Variables
float temperature;
int tempThreshold = 40; // Batas suhu untuk alarm (40°C)
bool alarmActive = false;
bool showTemperature = true;
unsigned long lastButtonPress = 0;

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.backlight();

  // Initialize RTC
  if (!rtc.begin()) {
    lcd.print("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    lcd.print("RTC lost power");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize Buzzer
  pinMode(buzzerPin, OUTPUT);

  // Initialize Button
  pinMode(buttonPin, INPUT);

  // Initialize TM1637 Display
  display.setBrightness(0x0f); // mengatur brightness (0x00 ke 0x0f)
}

void loop() {
  // Baca suhu dari LM35 sensor suhu
  int analogValue = analogRead(tempPin);
  temperature = (analogValue * 5.0 * 100.0) / 1024.0;

  // Check if reading was successful
  if (isnan(temperature)) {
    Serial.println("Failed to read temperature!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error reading");
    lcd.setCursor(0, 1);
    lcd.print("temp sensor");
    delay(2000);
    return;
  }

  // Read time
  DateTime now = rtc.now();

  // Display time on TM1637
  int displayTime = (now.hour() * 100) + now.minute();
  display.showNumberDecEx(displayTime, 0b01000000, true); // Display HH:MM with colon

  // Display time and temperature on LCD
  if (showTemperature) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C");
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(now.day(), DEC);
    lcd.print('/');
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.year(), DEC);
    lcd.setCursor(0, 1);
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    lcd.print(now.second(), DEC);
  }

  // Check temperature threshold
  if (temperature > tempThreshold && !alarmActive) {
    alarmActive = true;
    digitalWrite(buzzerPin, HIGH);
  } else if (temperature <= tempThreshold && alarmActive) {
    alarmActive = false;
    digitalWrite(buzzerPin, LOW);
  }

  // Handle button press to switch display mode
  if (digitalRead(buttonPin) == HIGH) {
    if (millis() - lastButtonPress > 500) {
      showTemperature = !showTemperature;
      lastButtonPress = millis();
    }
  }

  delay(1000);
}
