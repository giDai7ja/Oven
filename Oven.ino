#include <max6675.h>

#define MinTemp 30  // Минимальная (начальная) температура (°C)
#define MaxTemp 40 // Максимальная температура (°C)
#define StepTemp 30 // Шаг изменения температуры (°C)
#define StepTime 1 // Время одного шага (min)
#define Heater 9 // Выход управления нагревателем 

// Подключение датчика термопары
int thermoDO = 4;  // MISO
int thermoCS = 5;  // CS
int thermoCLK = 6; // SCLK

// Рабочие переменные
unsigned long TimeGetTemp = 0;
unsigned long TimeS = 0;
int Temperature = MinTemp;
int TemperatureOld = 0;
int BlinkTime = 1000;
bool OFF = false;
bool Heat = false;
bool HeatOld = false;
bool LED = false;
double T, Told, TBlink = 2000;

// Инициализация термопары
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Инициализация программы
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(Heater, OUTPUT);
  digitalWrite(Heater, LOW);
  Serial.begin(9600); // Для вывода отладочной информации в последовательный порт
  Serial.println("Termostat");
  delay(1000); // Задержка для стабилизации термопары
}

// Главный цикл программы, выполняется бесконечно
void loop() {
  if ( !OFF ) {
    SetTemp();
    Thermostat();
  }
  Blink();
  delay(100);
}

// Подпрограмма установки температуры термостата
void SetTemp() {
  TimeS = millis() / 1000;
  Temperature = MinTemp + (TimeS / (StepTime * 60) * StepTemp); // Вычисляем температуру для текущего времени

  if (Temperature > MaxTemp) { // Проверяем на максимум
    Temperature = MinTemp;
    OFF = true; // Признак окончания цикла
    Serial.print("Время (s) : ");
    Serial.println(TimeS);
    Serial.println("ОСТАНОВКА");
    BlinkTime = 500;
  }
  else {
    if (Temperature != TemperatureOld) {
      Serial.print("Время (s) : ");
      Serial.print(TimeS);
      Serial.print(",  Цель (°C) : ");
      Serial.println(Temperature);
      TemperatureOld = Temperature;
    }
  }
}

// Подпрограмма термостата
void Thermostat() {
  if ( !OFF ) {
    T = thermocouple.readCelsius();
    if (T < (Temperature - 1)) Heat = true;
    if (T >= Temperature) Heat = false;
  }
  else {
    Heat = false;
  }

  digitalWrite(Heater, Heat);

  if ( T != Told ) {
    Serial.print("Время (s) : ");
    Serial.print(TimeS);
    Serial.print(", Температура : ");
    Serial.println(T);
    Told = T;
  }
  if (Heat != HeatOld ) {
    Serial.print("Время (s) : ");
    Serial.print(TimeS);
    Serial.print(",  Нагрев : ");
    if ( Heat ) Serial.println("ВКЛ");
    else Serial.println("ВЫКЛ");
    HeatOld = Heat;
  }
}

void Blink() {
  if (TBlink < millis()) {
    digitalWrite(LED_BUILTIN, LED);
    LED = !LED;
    TBlink += BlinkTime;
  }
}
