#include <DallasTemperature.h>
#include <OneWire.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <DS3232RTC.h>
#include <SPI.h>
#include <SD.h>

#define DHTTYPE DHT22
#define DHTPIN A0
#define rain A1
#define relay 9

OneWire oneWire(2);
DallasTemperature sensors(&oneWire);

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

File myFile;
long prev_min = 0;
String t = "temp.txt", h = "humid.txt", s = "state.txt";

void setup() {

  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);

  lcd.begin(16, 2);
  start_sd(10);
  day_state();

  dht.begin();
  sensors.begin();
  sensors.setResolution(10);

  setSyncProvider(RTC.get);

  Alarm.alarmRepeat(06,00,0, day_on);
  Alarm.alarmRepeat(00,00,0, day_off);

}

void loop() {

  sensors.requestTemperatures();

  if (minute() != prev_min) {
    prev_min = minute();

    save_temp();
    save_humid();
  }

  lcd.setCursor(4,0);
  lcd.print(time());
  lcd.setCursor(0,1);
  lcd.print(dht.readTemperature());
  lcd.setCursor(11,1);
  lcd.print(sensors.getTempCByIndex(1));

  Alarm.delay(500);
}

void save_temp(){

  myFile = SD.open(t, FILE_WRITE);
  if (myFile) {
    myFile.println(time() + " " + date() + "," + temp());
    myFile.close();
  }
}

void save_humid(){

  myFile = SD.open(h, FILE_WRITE);
  if (myFile) {
    myFile.println(time() + " " + date() + "," + humid());
    myFile.close();
  }
}

int get_state(){

  myFile = SD.open(s);
  char state = myFile.read();
  myFile.close();

  return state;
}

void set_state(char state){

  File st = SD.open(s, O_TRUNC | O_WRITE);
  if (st) {
    st.print(state);
    st.close();
  }
}

void day_state(){

  get_state() == 49 ? digitalWrite(relay, LOW) : digitalWrite(relay, HIGH);
}

void day_on(){

  digitalWrite(relay, LOW);
  set_state('1');
}

void day_off(){

  digitalWrite(relay, HIGH);
  set_state('0');
}

String time(){

  char time[10];
  sprintf(time, "%02d:%02d:%02d", hour(), minute(), second());
  return String(time);
}

String date(){

  char date[12];
  sprintf(date, "%02d-%s-%d", day(), monthShortStr(month()), year());
  return String(date);
}

String temp(){

  char temp[75];
  char dt[5];

  dtostrf(dht.readTemperature(), 2, 2, dt);
  sprintf(temp, "air_temp:%s,earth_temp1:%d,earth_temp2:%d,rtc_temp:%d",\
  dt, sensors.getTempCByIndex(0), sensors.getTempCByIndex(1), RTC.temperature() / 4.);

  return temp;
}

String humid(){

  char humid[22];
  char dh[5];

  dtostrf(dht.readHumidity(), 2, 2, dh);
  sprintf(humid, "air_hum:%s,rain:%d", dh, map(analogRead(rain), 0, 1023, 100, 0));

  return humid;
}

void start_sd(int pin){

  !SD.begin(pin) ? lcd.print("SD failed!") : lcd.print("SD present.");
  lcd.clear();
}
