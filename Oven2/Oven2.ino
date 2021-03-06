#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>

#define DHTPIN 2   // Датчик DHT подключается к пину 2
#define DHTTYPE    DHT11

#define gaz 9      // Реле подачи газа
#define iskra 8    // Реле поджига

// Дисплей подключается по i2c
// SDA - A4
// SCL - A5

LiquidCrystal_I2C _lcd1(0x27, 16, 2); // Подключаем LCD дисплей
DHT_Unified dht(DHTPIN, DHTTYPE);
sensors_event_t event;

byte dhtT, dhtTold, dhtH, dhtHold, kT, kTold, state, oldstate;
int BlinkTime = 2000, Step = 0;
unsigned long TimeSensors = 1000, TBlink = 2000, TimeGaz = 5000;

// Подключение датчика термопары
int thermoDO = 4;  // MISO
int thermoCS = 5;  // CS
int thermoCLK = 6; // SCLK
bool LED = false , NewDisplay = false, Gaz = false;
byte DDD, HHH, MMM, SSS;

// Инициализация термопары
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // Иногда будем мигать светодиодом
  Wire.begin();                 // Инициализация i2c
  delay(10);                    // Подумаем о вечном 10ms
  _lcd1.init();                 // Инициализация LCD дисплея
  _lcd1.backlight();            // Видимо включаем подсветку
  _lcd1.setCursor(9, 1);
  _lcd1.print("Hello !");

  dht.begin();


  // Выходы
  digitalWrite(gaz, HIGH);  // Сразу выключаем
  pinMode(gaz, OUTPUT);     // Реле подачи газа
  digitalWrite(gaz, HIGH);  // На всякий случай, из-за странного модуля реле

  digitalWrite(iskra, HIGH);   // Сразу выключаем
  pinMode(iskra, OUTPUT);      // Реле поджига
  digitalWrite(iskra, HIGH);   // Сразу выключаем

  Serial.begin(9600);
} // setup

void loop() {

  GetSensors();
  if (NewDisplay) {
    DisplayT();
    DisplayH();
    DisplayTk();
    Monitor();
    NewDisplay = false;
  }
  Gorelka();
  Blink();

} // loop

void GetSensors() {
  if (TimeSensors < millis()) {
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) dhtT = 255;
    else dhtT = round(event.temperature);

    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) dhtH = 255;
    else dhtH = round(event.relative_humidity);

    kT = thermocouple.readCelsius();
    TimeSensors += 1000;
    NewDisplay = true;
  }
} // GetSensors

void Gorelka() {
  if (TimeGaz < millis()) {
    _lcd1.setCursor(9, 1);
    switch (Step) {
      case 0:
        if (dhtT <= 30 && !Gaz ) {   // Если температура меньше или равна 20 градусам
          digitalWrite(gaz, LOW);    // Включаем подачу газа
          digitalWrite(iskra, LOW);  // Включаем поджиг
          TimeGaz = millis() + 3000; // Таймер на выключение поджига 3 секунды
          _lcd1.print("Start ~");
          state = 10;
          BlinkTime = 250;
          Step = 10;
        }
        else if (dhtT >= 34 && Gaz) {
          digitalWrite(gaz, HIGH);    // Выключаем подачу газа
          digitalWrite(iskra, HIGH);  // Выключаем поджиг (на всякий случай)
          TimeGaz = millis() + 60000; // Повторный пуск не раньше чем через минуту
          Gaz = false;                // Газ выключен
          _lcd1.print("Gaz off");
          state = 0;
          BlinkTime = 2000;
        }
        else {
          TimeGaz = millis() + 1000;// Проверяем температуру раз в секунду
        }
        break;

      case 10:
        digitalWrite(iskra, HIGH);    // Выключаем поджиг
        TimeGaz = millis() + 8000;    // Ждём ещё 8 секунд
        Step = 20;
        _lcd1.print("Start  ");
        state = 20;
        BlinkTime = 500;
        break;

      case 20:
        if (kT < 50) {
          digitalWrite(gaz, HIGH);    // Выключаем подачу газа
          TimeGaz = millis() + 180000;// Попытка включить через 3 минуты
          _lcd1.print("Restart");
          state = 30;
          BlinkTime = 2000;
        }
        else {
          Gaz = true;              // Подожгли успешно
          _lcd1.print("Gaz on ");
          state = 40;
          BlinkTime = 1000;
        }
        Step = 0;
        break;

      default:
        Step = 0;
        TimeGaz = millis() + 500;
        break;
    }
  }
}

void DisplayT() {
  _lcd1.setCursor(0, 0);
  _lcd1.print("T=");
  if (dhtT == 255) {
    _lcd1.print("err ");
  }
  else {
    _lcd1.print(dhtT);
    _lcd1.print(char(0xDF));
    _lcd1.print("C ");
  }
} //DisplayT

void DisplayH() {
  _lcd1.setCursor(11, 0);
  _lcd1.print("H=");
  if (dhtH == 255) {
    _lcd1.print("err");
  }
  else {
    _lcd1.print(dhtH);
    _lcd1.print("%");
  }
} //DisplayH

void DisplayTk() {
  _lcd1.setCursor(0, 1);
  _lcd1.print("Tk=");
  _lcd1.print(kT);
  _lcd1.print(char(0xDF));
  _lcd1.print("C ");
} // DisplayTk


void Blink() {
  if (TBlink < millis()) {
    digitalWrite(LED_BUILTIN, LED);
    LED = !LED;
    TBlink += BlinkTime;
  }
}

void Monitor() {
  unsigned long UpTime;
  if ((state != oldstate) || (dhtT != dhtTold) || (dhtH != dhtHold) || (kT != kTold)) {
    UpTime = millis() / 1000;
    SSS = UpTime % 60;
    UpTime = (UpTime - SSS) / 60;
    MMM = UpTime % 60;
    UpTime = ( UpTime - MMM ) / 60;
    HHH = UpTime % 24;
    DDD = (UpTime - HHH) / 24;
    Serial.print(F("up "));
    Serial.print(DDD);
    Serial.print(F(" days, "));
    Serial.print(HHH);
    Serial.print(F(":"));
    Serial.print(MMM);
    Serial.print(F(":"));
    Serial.println(SSS);

    if ((dhtT != dhtTold) || (dhtH != dhtHold)) {
      Serial.print(F("Температура: "));
      Serial.print(dhtT);
      Serial.print(F("°C, Влажность: "));
      Serial.print(dhtH);
      Serial.println(F("%"));
      dhtTold = dhtT;
      dhtHold = dhtH;
    }
    if (kT != kTold) {
      Serial.print(F("Термопара: "));
      Serial.print(kT);
      Serial.println(F("°C"));
      kTold = kT;
    }
    if (state != oldstate) {
//      Serial.print(F("Статус : "));

      switch (state) {
        case 0:
          Serial.println(F("Ожидание"));
          break;

        case 10:
          Serial.println(F("Подача газа. Зажигание"));
          break;

        case 20:
          Serial.println(F("Зажигание выключено"));
          break;

        case 30:
          Serial.println(F("Температура термопары низкая. Повторная попытка через 3 минуты"));
          break;

        case 40:
          Serial.println(F("Успешный запуск"));
          break;
      }
      oldstate = state;
    }
    Serial.println();
  }
}
