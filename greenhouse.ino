#include <GyverButton.h>
#include <EEPROM.h>
#include <DHT.h>
#define DHTTYPE DHT11 //DHT22, DHT11, DHT21
#define DHTPIN 2 //pin -data input

#define W1o   4 //relay output to open  windows
#define W1c   3 //relay output to close windows

#define W2o   8 //relay output to open  windows
#define W2c   7 //relay output to close windows


#define GTIME 10000 // time to open velvs

#define W1time 5000 //time  to close or open windows 1

#define W2time 5000 //time  to close or open windows 2

#define W1_OPEN  5 //limit switch to open window 1
#define W1_CLOSE 6 //limit switch to close window 1

#define W2_OPEN  10  //limit switch to open window 2
#define W2_CLOSE 11   //limit switch to close window 2

#define SENSOR1 A2
#define SENSOR2 A3

#define S1 12
#define S2 13


#define TIME_DOOR_OC  5000 //time to close or open door

#define TEMP_MODE 1 //1 - t++, 2 - t--

#define PHOTORESISTOR A1  //photoresistor pin
#define PHmode 2   //1 - if >, 2 - if<

#define STIME 60000 //time to setings

//Дефайны
#define SENSOR  1  //1 - lcd 16/2, 2 - 128/64 
#define DEBUG    //0-9 - on,  " " - off
#define BTN   9 //button input 
#define POT   A0  //potentiometr input pin

#define MINUSr 1 //1 - gnd driving/0 - 5v driving


//ПреПроцессор
#ifdef DEBUG
#define debug_print(x) Serial.println(x)
#else debug_print(x)

#endif
#define ASCII_CONVERT '0'
#if (SENSOR == 2)
#include <TroykaOLED.h>
TroykaOLED oled(0x3C);

#elif (SENSOR == 1)
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C   lcd(0x27, 16, 2);
#endif

GButton btn(BTN);
DHT dht(DHTPIN, DHTTYPE);
//Переменные
bool d_flag = 0;
bool dr = 0;
byte SHmode = 0;

bool day = 0; //1 - day / 0 - night
bool door = 1;  //1 - open
bool w1 = 1;  //0 - close
bool w2 = 1; //o - close

bool w1e  = 1; //1-enabled
bool w2e  = 1;//1-enabled
bool de = 1; //1-enabled

bool t5 = 1;

//Chars
byte cels[] {
  B10110,
  B01001,
  B01000,
  B01000,
  B01000,
  B01001,
  B00110,
  B00000
};
byte persent[] {
  B11100,
  B11001,
  B10010,
  B00100,
  B01001,
  B10011,
  B00111,
  B00000
};
byte load25[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B10000,
  B01000,
  B00100
};
byte load50[] = {
  B00000,
  B00000,
  B00000,
  B00100,
  B01000,
  B10000,
  B01000,
  B00100
};
byte load75[] = {
  B00000,
  B00000,
  B00000,
  B00100,
  B01010,
  B10001,
  B01000,
  B00100
};
byte load100[] = {
  B00000,
  B00000,
  B00000,
  B00100,
  B01010,
  B10001,
  B01010,
  B00100
};
byte up_pointer[] = {
  B00100,
  B01110,
  B10101,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100
};
byte down_pointer[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B10101,
  B01110,
  B00100
};
int td = TIME_DOOR_OC;
byte temp = eeprom_read_byte(0);  //eeprom read temp_setting value for index 0
byte PHval = eeprom_read_byte(1); //eeprom read photoresistor value for index 1
byte w_temp = eeprom_read_byte(2); //eeprom read water temp for index 2
byte hud = eeprom_read_byte(3); //eeprom read humidity for ants
void setup() {
  Serial.begin(19200);
  btn.setTimeout(5000);

  //Debug

  //Temp-sensor
  dht.begin();

  //Pins
  pinMode(W1o, OUTPUT);
  pinMode(W1c, OUTPUT);

  pinMode(W2o, OUTPUT);
  pinMode(W2c, OUTPUT);

  pinMode(W1_OPEN, INPUT_PULLUP);
  pinMode(W1_CLOSE, INPUT_PULLUP);

  pinMode(W2_OPEN, INPUT_PULLUP);
  pinMode(W2_CLOSE, INPUT_PULLUP);

  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);

  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

#if(MINUSr == 1)
  dr = 0;
  digitalWrite(W1o, HIGH);
  digitalWrite(W1c, HIGH);
  digitalWrite(W2o, HIGH);
  digitalWrite(W2c, HIGH);
  digitalWrite(S1, HIGH);
  digitalWrite(S2, HIGH);
#else
  dr = 1;
  digitalWrite(W1o, LOW);
  digitalWrite(W1c, LOW);
  digitalWrite(W2o, LOW);
  digitalWrite(W2c, LOW);
  digitalWrite(S1, LOW);
  digitalWrite(S2, LOW);
#endif
#if(SENSOR == 1)
  lcd.init();
#endif
  //lcd create chars
  lcd.createChar(3, cels);  //celsium symbol
  lcd.createChar(4, persent); //persent symbol
  lcd.createChar(5, load25);
  lcd.createChar(6, load50);
  lcd.createChar(7, load75);
  lcd.createChar(8, load100);
  lcd.createChar(9, up_pointer);
  lcd.createChar(10, down_pointer);
  //Display two
#if (SENSOR == 2)
  oled.begin();
  oled.setCoding(Encoding::UTF8);
  oled.setFont(alphabet12x16);
  oled.print("Работает", OLED_CENTER, 20);
  //Display one
#elif (SENSOR == 1)
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Click: diagnost.");
  lcd.setCursor(0, 1);
  lcd.print("2-click: setting");
  unsigned long t = millis();
  while (millis() - t < 5000) {
    btn.tick();
    if (btn.isSingle()) {
      diagnostick(dr);
      d_flag = 1;
    }
    if (btn.isDouble()) {
      setting(1);
      d_flag = 1;
    }
  }
  if (millis() - t >= 5000 && d_flag == 0) {
    lcd.clear();
    lcd.noBacklight();
  }
#endif
  start();
  byte hum = eeprom_read_byte(3);
  if (w_temp > 9) {
    bool f = 0;
    if (hum > 9) {
      f = 1;
    }
    int mass[] = {6, 1, w_temp, f, hum};
    sends(6, mass);
  } else {
    bool f = 0;
    if (hum > 9) {
      f = 1;
    }
    int mass[] = {6, 0, w_temp, f, hum};
    sends(6, mass);
  }
}

byte wtemp = 0;

bool m = 0;
bool ff1 = 1;

char w1wo = 0;
char w2wo = 0;
char ddo = 0;
char w1wc = 0;
char w2wc = 0;
char ddc = 0;
uint32_t tt2 = millis();
uint32_t t3 = millis();
uint32_t t4 = millis();

uint32_t s1t = millis();
uint32_t s2t = millis();

int sensor1 = analogRead(SENSOR1);
int sensor2 = analogRead(SENSOR2);


bool hlev = 0;
bool llev = 0;

bool s1 = 1;
bool s2 = 1;

bool water = 1;
byte buffer[8];
//=========LOOP=====================================================================================================================
void loop() {
  if (parsing()) {
    switch (buffer[0]) {
      case 1:
        if (buffer[1] == 1) {
          wtemp = (String(buffer[2]) + String(buffer[3])).toInt();
          hlev = buffer[4];
          llev = buffer[5];
        } else {
          wtemp = buffer[2];
          hlev = buffer[3];
          llev = buffer[4];
        }
        break;
      case 3:
        water = buffer[1];
        break;
      case 4:
        if (buffer[1] == 1) {
          Serial.print("$");
          Serial.print(9);
           Serial.print(1);
          Serial.println(";");
          lcd.clear();
          uint32_t ttr = millis();
          uint32_t timen = millis();
          bool secpar = buffer[3];
          byte data = 0;
          byte stat = 0;
          byte box = buffer[2];
          while (millis() - ttr < 10000) {

            lcd.setCursor(0, 0);
            lcd.print("Box");
            lcd.print(box);
            lcd.print(": ");
            lcd.print(data);
            lcd.setCursor(7, 1);
            if (millis() - timen > 200) {
              timen = millis();
              if (stat < 3) {
                stat++;
              } else {
                stat = 0;
              }
            }
            switch (stat) {
              case 0:
                lcd.write(5);
                break;
              case 1:
                lcd.write(6);
                break;
              case 2:
                lcd.write(7);
                break;
              case 3:
                lcd.write(8);
                break;
            }
            if (parsing()) {
              if (buffer[0] == 4 && buffer[1] == 0) {
                break;
              }
              if (buffer[0] == 5) {
                secpar = buffer[1];
                if (secpar) {
                  data = (String(buffer[2]) + String(buffer[3])).toInt();
                } else {
                  data = buffer[2];
                }
              }
            }
          }
        }

        break;

    }
  }
  if (millis() - t4 > 3000) {
    if (dop() == 1) {
      door = 1;
    } else {
      door = 0;
    }
    t4 = millis();
  }
  if (t5 == 0) {
    btn.setTimeout(5000);
  }
  btn.tick();
  nulltopin(dr);
  int ph = analogRead(PHOTORESISTOR);
  ph = map(ph, 0, 1023, 0, 99);
  if (btn.isSingle()) {
    lcd.clear();
    if (SHmode == 3) {
      SHmode = 0;
    }
    else {
      SHmode++;
    }
  }
  if (btn.isDouble()) {
    lcd.clear();
    diagnostick(dr);
    lcd.clear();
  }
  if (btn.isHold() && m == 0) {
    lcd.clear();
    lcd.backlight();
    uint32_t t1 = millis();
    while (millis() - t1 < 1500) {
      lcd.setCursor(0, 0);
      lcd.print("Settings mode");
    }
    setting(1);
  }
  if (m == 1) {
    uint32_t t1 = millis();
    while (millis() - t1 < 1000) {}
    m = 0;
  }

  if (SHmode == 0) {
    if (ff1 == 0) {
      Serial.print("$");
      Serial.print(3);
      Serial.print(0);
      Serial.println(";");
      ff1 = 1;
    }
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Temp:");
    lcd.setCursor(5, 0);
    lcd.print(dht_temp());
    lcd.write(3);
    lcd.print(" PH:");
    lcd.print(ph);
    lcd.setCursor(0, 1);
    lcd.print("ST:");
    lcd.setCursor(3, 1);
    lcd.print(temp);
    lcd.write(3);
    lcd.print(" Sph:");
    lcd.print(PHval);
  } if (SHmode == 1) {
    if (ff1 == 0) {
      Serial.print("$");
      Serial.print(3);
      Serial.print(0);
      Serial.println(";");
      ff1 = 1;
    }
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Day:");
    lcd.setCursor(5, 0);
    lcd.print(day);
    lcd.setCursor(6, 0);
    lcd.print(" Win 1:");
    lcd.setCursor(13, 0);
    lcd.print(w1);
    lcd.setCursor(0, 1);
    lcd.print("Door:");
    lcd.setCursor(6, 1);
    lcd.print(door);
    lcd.setCursor(7, 1);
    lcd.print(" Win 2: ");
    lcd.print(w2);
  }
  if (SHmode == 2) {
    if (ff1 == 1) {
      Serial.print("$");
      Serial.print(3);
      Serial.print(1);
      Serial.println(";");
      ff1 = 0;
    }
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("wTemp:");
    lcd.setCursor(7, 0);
    lcd.print(wtemp);
    lcd.write(3);
    lcd.setCursor(0, 1);
    lcd.print("Hlev:");
    lcd.setCursor(6, 1);
    lcd.print(hlev);
    lcd.setCursor(8, 1);
    lcd.print("Llev:");
    lcd.setCursor(14, 1);
    lcd.print(llev);
  }
  if (SHmode == 3) {
    sensor1 = analogRead(SENSOR1);
    sensor2 = analogRead(SENSOR2);
    sensor1 = map(sensor1, 0, 1023, 0, 99);
    sensor2 = map(sensor2, 0, 1023, 0, 99);
    if (ff1 == 0) {
      Serial.print("$");
      Serial.print(3);
      Serial.print(0);
      Serial.println(";");
      ff1 = 1;
    }
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("sHum:");
    lcd.setCursor(6, 0);
    lcd.print(hud);
    lcd.setCursor(0, 1);
    lcd.print("s1:");
    lcd.setCursor(4, 1);
    lcd.print(sensor1);
    lcd.setCursor(7, 1);
    lcd.print("s2:");
    lcd.setCursor(11, 1);
    lcd.print(sensor2);

  }
  if (millis() - tt2 > 60000) {
    w1wo = 0;
    w2wo = 0;
    ddo = 0;
    w1wc = 0;
    w2wc = 0;
    ddc = 0;
    tt2 = millis();
  }
  if (w1wo >= 2) {
    windowO_close(dr);
    w1wo = 0;
    w1e = 0;
    eeprom_write_byte(5, 1);
  }
  if (w2wo >= 2) {
    windowT_close(dr);
    w2wo = 0;
    w2e = 0;
    eeprom_write_byte(6, 1);
  }
  if (w1wc >= 2) {
    windowO_close(dr);
    w1wc = 0;
    w1e = 0;
    eeprom_write_byte(7, 1);
  }
  if (w2wc >= 2) {
    windowT_close(dr);
    w2wc = 0;
    w2e = 0;
    eeprom_write_byte(8, 1);
  }
  if (ddo >= 2) {
    door_close(dr);
    ddo = 0;
    de = 0;
    eeprom_write_byte(9, 1);
  }
  if (ddc >= 2) {
    door_close(dr);
    ddc = 0;
    de = 0;
    eeprom_write_byte(10, 1);
  }
  day_fase(PHval);
  temp_work(dr, temp);
  if (millis() - t3 > 2500) {
    lcd.clear();
    t3 = millis();
  }

  if (s1 == 0 && millis() - s1t > GTIME * 3) {
    s1 = 1;
  }
  if (s2 == 0 && millis() - s2t > GTIME * 3) {
    s2 = 1;
  }
//  if (water) {
//    sensors();
//  }
}
//================LOOP=========================================================================================================

void diagnostick(bool dr) {
  bool t = 0;
  lcd.backlight();
  lcd.clear();
  uint32_t t1 = millis();
  uint32_t t2 = millis();
  if (eeprom_read_byte(5)) {
    lcd.setCursor(0, 0);
    lcd.print("W1o");
    t = 1;
  }
  if (eeprom_read_byte(6)) {
    lcd.setCursor(5, 0);
    lcd.print("W2o");
    t = 1;
  }
  if (eeprom_read_byte(7)) {
    lcd.setCursor(10, 0);
    lcd.print("W1c");
    t = 1;
  }
  if (eeprom_read_byte(8)) {
    lcd.setCursor(0, 1);
    lcd.print("W2c");
    t = 1;
  }
  if (eeprom_read_byte(9)) {
    lcd.setCursor(5, 0);
    lcd.print("Do");
    t = 1;
  }
  if (eeprom_read_byte(10)) {
    lcd.setCursor(10, 0);
    lcd.print("Dc");
    t = 1;
  }
  while (millis() - t1 < 600000) {
    if (t == 1) {
      btn.tick();
      if (btn.isSingle()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Diagnostick mode");
        byte i = 5;
        while (i <= 10) {
          eeprom_write_byte(i, 0);
          i++;
        }
        w1wo = 0;
        w1wc = 0;
        w2wc = 0;
        w2wo = 0;
        ddo = 0;
        ddc = 0;
        w1e = 1;
        w2e = 1;
        de = 1;
        t = 0;
        lcd.clear();
      }
    } else {
      btn.tick();
      String menu[5] = {"WINDOW1", "WINDOW2", "DOOR",  "DAY", "TEMP"};
      String menu_d[5] = {"OPEN/CLOSE", "OPEN/CLOSE", "OPEN/CLOSE",  "DAY/PHOTO", "TEMP/STEMP"};
      byte ml = 5; //length of array
      int an = analogRead(POT);
      an = map(an, 0, 1023, 0, ml - 1); //index of array from potentiometr value

      lcd.setCursor(0, 0);
      lcd.write(9);
      lcd.setCursor(0, 1);
      lcd.write(10);

      lcd.setCursor(5, 0);
      lcd.print(menu[an]);
      lcd.setCursor(3, 1);
      lcd.print(menu_d[an]);

      if (btn.isSingle()) {
        diag(an, dr);
      }
      if (btn.isDouble()) {
        break;
      }
      if (millis() - t2 > 600) {
        lcd.clear();
        t2 = millis();
      }
    }
  }
}
void diag(unsigned int an, bool dr) {
  int ph;
  PHval = eeprom_read_byte(1);
  temp = eeprom_read_byte(0);
  bool swo = dop();
  bool swc = doc();
  btn.setTimeout(300);
  t5 = 0;
  bool oc = 0;
  bool blik = 0;
  uint32_t t1 = millis();
  uint32_t t2 = millis();
  uint32_t t3 = millis();
  lcd.clear();
  while (millis() - t1 < 120000) {
    ph = analogRead(PHOTORESISTOR);
    ph = map(ph, 0, 1023, 0, 99);
    btn.tick();
    if (btn.isDouble()) {
      break;
    }
    switch (an) {
      case 0:  //Window 1
        btn.tick();
        if (btn.isSingle()) {
          oc = !oc;
        }
        if (btn.isHold()) {
          if (dr == 1) {
            if (oc == 1) {
              digitalWrite(W1c, HIGH);
              digitalWrite(W1o, LOW);
            } else {
              digitalWrite(W1o, HIGH);
              digitalWrite(W1c, LOW);
            }
          }
          if (dr == 0) {
            if (oc == 1) {
              digitalWrite(W1c, LOW);
              digitalWrite(W1o, HIGH);
            } else {
              digitalWrite(W1o, LOW);
              digitalWrite(W1c, HIGH);
            }
          }
        } else {
          nulltopin(dr);
        }
        lcd.setCursor(0, 0);
        lcd.print("Window1");
        lcd.setCursor(8, 0);
        if (blik == 0) {
          if (oc == 0) {
            lcd.print("to open");
          } else {
            lcd.print("to close");
          }
        } else {
          lcd.setCursor(8, 0);
          lcd.print("        ");
        }
        lcd.setCursor(1, 1);
        lcd.print("SWo:");
        lcd.setCursor(5, 1);
        lcd.print(!digitalRead(W1_OPEN));
        lcd.setCursor(10, 1);
        lcd.print("SWc:");
        lcd.setCursor(14, 1);
        lcd.print(!digitalRead(W1_CLOSE));

        if (millis() - t2 > 500) {
          blik = !blik;
          t2 = millis();
        }
        break;

      case 1:  //Window 2
        btn.tick();
        if (btn.isSingle()) {
          oc = !oc;
        }
        if (btn.isHold()) {
          if (dr == 1) {
            if (oc == 1) {
              digitalWrite(W2c, HIGH);
              digitalWrite(W2o, LOW);
            } else {
              digitalWrite(W2o, HIGH);
              digitalWrite(W2c, LOW);
            }
          }
          if (dr == 0) {
            if (oc == 1) {
              digitalWrite(W2c, LOW);
              digitalWrite(W2o, HIGH);
            } else {
              digitalWrite(W2o, LOW);
              digitalWrite(W2c, HIGH);
            }
          }
        } else {
          nulltopin(dr);
        }
        lcd.setCursor(0, 0);
        lcd.print("Window2");
        lcd.setCursor(8, 0);
        if (blik == 0) {
          if (oc == 0) {
            lcd.print("to open");
          } else {
            lcd.print("to close");
          }
        } else {
          lcd.setCursor(8, 0);
          lcd.print("        ");
        }
        lcd.setCursor(1, 1);
        lcd.print("SWo:");
        lcd.setCursor(5, 1);
        lcd.print(!digitalRead(W2_OPEN));
        lcd.setCursor(10, 1);
        lcd.print("SWc:");
        lcd.setCursor(14, 1);
        lcd.print(!digitalRead(W2_CLOSE));

        if (millis() - t2 > 500) {
          blik = !blik;
          t2 = millis();
        }
        break;


      case 2:  //Door
        btn.tick();
        if (btn.isSingle()) {
          oc = !oc;
        }
        if (btn.isHold()) {
          if (oc == 1) {
            Serial.print("$");
            Serial.print(4);
            Serial.print(0);
            Serial.print(dr);
            Serial.println(";");

            Serial.print("$");
            Serial.print(4);
            Serial.print(1);
            Serial.print(!dr);
            Serial.println(";");
          } else {
            Serial.print("$");
            Serial.print(4);
            Serial.print(1);
            Serial.print(dr);
            Serial.println(";");

            Serial.print("$");
            Serial.print(4);
            Serial.print(0);
            Serial.print(!dr);
            Serial.println(";");
          }
        } else if (btn.isRelease()) {
          Serial.print("$");
          Serial.print(4);
          Serial.print(0);
          Serial.print(!dr);
          Serial.println(";");

          Serial.print("$");
          Serial.print(4);
          Serial.print(1);
          Serial.print(!dr);
          Serial.println(";");
        }
        lcd.setCursor(0, 0);
        lcd.print("Door");
        lcd.setCursor(8, 0);
        if (blik == 0) {
          if (oc == 0) {
            lcd.print("to open");
          } else {
            lcd.print("to close");
          }
        } else {
          lcd.setCursor(8, 0);
          lcd.print("        ");
        }
        lcd.setCursor(1, 1);
        lcd.print("SWo:");
        lcd.setCursor(5, 1);
        lcd.print(swo);
        lcd.setCursor(10, 1);
        lcd.print("SWc:");
        lcd.setCursor(14, 1);
        lcd.print(swc);

        if (millis() - t2 > 500) {
          blik = !blik;
          t2 = millis();
        }
        if (millis() - t3 > 1500) {
          swo = dop();
          swc = doc();
          t3 = millis();
        }
        break;
      case 4:
        day_fase(PHval);
        lcd.setCursor(5, 0);
        lcd.print("Day:");
        lcd.setCursor(10, 0);
        lcd.print(day);
        lcd.setCursor(0, 1);
        lcd.print("PH:");
        lcd.setCursor(4, 1);
        lcd.print(ph);
        lcd.setCursor(8, 1);
        lcd.print("Sph:");
        lcd.setCursor(13, 1);
        lcd.print(PHval);


        break;
      case 5:
        int tempn = dht.readTemperature();

        lcd.setCursor(5, 0);
        lcd.print("Temp:");
        lcd.setCursor(11, 0);
        lcd.print(tempn);
        lcd.setCursor(4, 1);
        lcd.print("sTemp:");
        lcd.setCursor(11, 1);
        lcd.print(temp);


        break;



    }
  }
}
void setting(byte tst) {
  btn.setTimeout(800);
  t5 = 0;
  //flags
  unsigned long t1 = millis();
  unsigned long t2 = millis();
  int pt = analogRead(POT);
  bool f1 = 1;    //value has/hasn`t on screen
  bool f2 = 0; //flag to read/read not value from analog
  byte txt = tst;  //which data is changing
  //lcd default set
  lcd.clear();
  lcd.backlight();
  while (millis() - t1 < STIME) {
    //eeprom
    byte temp = eeprom_read_byte(0);  //eeprom read temp_setting value for index 0
    byte PHval = eeprom_read_byte(1); //eeprom read photoresistor value for index 1
    byte w_temp = eeprom_read_byte(2);
    byte hum = eeprom_read_byte(3);
    //analog
    //don`t touch analod checking
    if (pt - 5 < analogRead(POT) && analogRead(POT) < pt + 5) {
      btn.tick();
      if (btn.isSingle()) {
        if (txt == 1) {
          txt++;
        }
        else if (txt == 2) {
          txt = 3;
        }
        else if (txt == 3) {
          txt = 4;
        }
        else {
          txt = 1;
        }
      }
      if (btn.isDouble()) {
        if (txt == 1) {
          txt = txt + 2;
        }
        else if (txt == 2) {
          txt = 4;
        }
        else if (txt == 4) {
          txt = 2;
        }
        else if (txt == 3) {
          txt = 1;
        }
      }
      if (btn.isHold()) {
        lcd.clear();
        uint32_t t1 = millis();
        while (millis() - t1 < 1500) {
          lcd.setCursor(0, 0);
          lcd.print("Turning..");
          lcd.setCursor(5, 1);
          lcd.print("work mode");
        }
        update_params();
        lcd.clear();
        break;
      }
      if (f1 == 1) {
        lcd.setCursor(0, 0);
        lcd.print("Temp:");
        lcd.print(temp);
        lcd.write(3);   //celsium
        lcd.print(" Aqu:");
        lcd.print(w_temp);
        lcd.write(3); //celsium
        lcd.setCursor(0, 1);
        lcd.print("PH:");
        lcd.print(PHval);
        lcd.write(4);
        lcd.setCursor(7, 1);
        lcd.print("HUM: ");
        lcd.print(hum);
        lcd.setCursor(15, 1);
        lcd.write(4); //celsium
      }
      else {
        switch (txt) {
          case 1:
            lcd.setCursor(0, 0);
            lcd.print("Temp:  ");
            lcd.write(3);
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH:");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(7, 1);
            lcd.print("HUM: ");
            lcd.print(hum);
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
          case 2:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:  ");
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH:");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(7, 1);
            lcd.print("HUM: ");
            lcd.print(hum);
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
          case 3:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH:  ");
            lcd.write(4);
            lcd.setCursor(7, 1);
            lcd.print("HUM: ");
            lcd.print(hum);
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
          case 4:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH:");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(7, 1);
            lcd.print("HUM:   ");
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
        }
      }


      if (millis() - t2 >= 500) {
        t2 = millis();
        f1 = !f1;
      }
    } else {
      f2 = 1;
    }
    //----------------------------------------------------------------------------------------------------------------------
    while (f2 == 1 && millis() - t1 < STIME) {
      int ph = analogRead(PHOTORESISTOR);
      int set = analogRead(POT);
      set = map(set, 0, 1023, 0, 99);
      ph  = map(ph, 0, 1023, 0, 99);
      btn.tick();
      if (btn.isSingle()) {
        if (txt == 1) {
          txt++;
          eeprom_write_byte(0, set);
          setting(2);
        }
        else if (txt == 2) {
          txt = 3;
          eeprom_write_byte(2, set);
          setting(3);
        }
        else if (txt == 3) {
          txt = 4;
          eeprom_write_byte(1, set);
          setting(4);
        }
        else {
          txt = 1;
          eeprom_write_byte(3, set);
          setting(1);
        }
      }
      if (btn.isDouble()) {
        if (txt == 1) {
          txt = txt + 2;
          eeprom_write_byte(0, set);
          setting(3);
        }
        else if (txt == 2) {
          txt = 4;
          eeprom_write_byte(2, set);
          setting(4);
        }
        else if (txt == 3) {
          txt = 1;
          eeprom_write_byte(1, set);
          setting(2);
        } else {
          txt = 2;
          eeprom_write_byte(3, set);
          setting(2);
        }
      }
      if (f1 == 1) {
        switch (txt) {
          case 1:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(set);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH: ");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(8, 1);
            lcd.print("HUM: ");
            lcd.print(hum);
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium

            break;
          case 2:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:");
            lcd.print(set);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH: ");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(8, 1);
            lcd.print("HUM: ");
            lcd.print(hum);
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
          case 3:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH:");
            lcd.print(set);
            lcd.write(4);
            lcd.print(" n:");
            lcd.print(ph);
            break;
          case 4:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH: ");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(8, 1);
            lcd.print("HUM: ");
            lcd.print(set);
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
        }
      }
      else {
        switch (txt) {
          case 1:
            lcd.setCursor(0, 0);
            lcd.print("Temp:  ");
            lcd.write(3);
            lcd.setCursor(8, 0);
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH: ");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(8, 1);
            lcd.print("HUM:   ");
            lcd.print(hum);
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
          case 2:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:  ");
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH: ");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(8, 1);
            lcd.print("HUM:   ");
            lcd.print(hum);
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
          case 3:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH:  ");
            lcd.write(4);
            lcd.print(" n:");
            lcd.print(ph);
            lcd.setCursor(11, 1);
            lcd.print("      ");
            break;
          case 4:
            lcd.setCursor(0, 0);
            lcd.print("Temp:");
            lcd.print(temp);
            lcd.write(3);   //celsium
            lcd.print(" Aqu:");
            lcd.print(w_temp);
            lcd.write(3); //celsium
            lcd.setCursor(0, 1);
            lcd.print("PH: ");
            lcd.print(PHval);
            lcd.write(4);
            lcd.setCursor(8, 1);
            lcd.print("HUM:   ");
            lcd.setCursor(15, 1);
            lcd.write(4); //celsium
            break;
        }
      }
      if (millis() - t2 >= 500) {
        t2 = millis();
        f1 = !f1;
      }
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button: ");
  lcd.setCursor(15, 0);
  lcd.print("3");
  uint32_t t3 = millis();
  while (millis() - t3 < 3100) {
    btn.tick();
    if (btn.isSingle()) {
      setting(1);
    }
    if (millis() - t3 == 2000) {
      lcd.setCursor(15, 0);
      lcd.print("1");
    }
    if (millis() - t3 == 1000) {
      lcd.setCursor(15, 0);
      lcd.print("2");
    }
    if (millis() - t3 == 3000) {
      lcd.setCursor(15, 0);
      lcd.print("0");
    }
  }
  lcd.clear();
  lcd.noBacklight();
  update_params();
}
//-g-g-gg-g-g-g-g-g-g-g-g-g--g-g-g-g-g-g--g-g-g-g-g-g-g--g-g-g-g-g-g--g-g-g-g--g-gg-g-g-g-
int dht_temp() {
  return dht.readTemperature();
}
int dht_hum() {
  return dht.readHumidity();
}

void windowO_open(bool dr) {
  if (w1e == 1) {
    w1wo++;
    lcd.clear();
    unsigned long t1 = millis();
    unsigned long timen = millis();
    byte stat = 0;
    while (millis() - timen < W1time && digitalRead(W1_OPEN) == 1) {
      lcd.setCursor(0, 0);
      lcd.print("1 window opening");
      lcd.setCursor(7, 1);
      if (millis() - t1 > 200) {
        t1 = millis();
        if (stat < 3) {
          stat++;
        } else {
          stat = 0;
        }
      }
      switch (stat) {
        case 0:
          lcd.write(5);
          break;
        case 1:
          lcd.write(6);
          break;
        case 2:
          lcd.write(7);
          break;
        case 3:
          lcd.write(8);
          break;
      }
      if (dr == 1) {
        if (millis() - timen < W1time && digitalRead(W1_OPEN) != 0) {
          digitalWrite(W1o, HIGH);
        }
        if (digitalRead(W1_OPEN) == 0 || millis() - timen >= W1time) {
          digitalWrite(W1o, LOW);
          w1 = 1;
        }
      }
      if (dr == 0) {
        if (millis() - timen < W1time && digitalRead(W1_OPEN) != 0) {
          digitalWrite(W1o, LOW);
        }
        if (digitalRead(W1_OPEN) == 0 || millis() - timen >= W1time) {
          digitalWrite(W1o, HIGH);
          w1 = 1;
        }
      }
    }
    lcd.clear();
  }
}
void windowT_open(bool dr) {
  if (w2e == 1) {
    w2wo++;
    lcd.clear();
    unsigned long t1 = millis();
    unsigned long timen = millis();
    byte stat = 0;
    while (millis() - timen < W2time && digitalRead(W2_OPEN) == 1) {
      lcd.setCursor(0, 0);
      lcd.print("2 window opening");
      lcd.setCursor(7, 1);
      if (millis() - t1 > 200) {
        t1 = millis();
        if (stat < 3) {
          stat++;
        } else {
          stat = 0;
        }
      }
      switch (stat) {
        case 0:
          lcd.write(5);
          break;
        case 1:
          lcd.write(6);
          break;
        case 2:
          lcd.write(7);
          break;
        case 3:
          lcd.write(8);
          break;
      }
      if (dr == 1) {
        if (millis() - timen < W2time && digitalRead(W2_OPEN) != 0) {
          digitalWrite(W2o, HIGH);
        }
        if (digitalRead(W2_OPEN) == 0 || millis() - timen >= W2time) {
          digitalWrite(W2o, LOW);
          w2 = 1;
        }
      }
      if (dr == 0) {
        if (millis() - timen < W2time && digitalRead(W2_OPEN) != 0) {
          digitalWrite(W2o, LOW);
        }
        if (digitalRead(W2_OPEN) == 0 || millis() - timen >= W2time) {
          digitalWrite(W2o, HIGH);
          w2 = 1;
        }
      }
    }
    lcd.clear();
  }
}

void door_close(bool dr) {
  if (de == 1) {

    Serial.print("$");
    Serial.print(2);
    Serial.print(dr);
    if (td / 1000 < 9) {
      Serial.print(0);
      Serial.print((td / 1000) );
    } else {
      Serial.print(1);
      Serial.print((td / 1000)   / 10);
      Serial.print((td / 1000)  % 10);
    }
    Serial.println(";");
    ddc++;
    lcd.clear();
    unsigned long t1 = millis();
    unsigned long timen = millis();
    byte stat = 0;
    while (millis() - timen < TIME_DOOR_OC) {
      lcd.setCursor(0, 0);
      lcd.print("Door closing");
      lcd.setCursor(7, 1);
      if (millis() - t1 > 200) {
        t1 = millis();
        if (stat < 3) {
          stat++;
        } else {
          stat = 0;
        }
      }
      switch (stat) {
        case 0:
          lcd.write(5);
          break;
        case 1:
          lcd.write(6);
          break;
        case 2:
          lcd.write(7);
          break;
        case 3:
          lcd.write(8);
          break;
      }
    }
  }
}
void windowO_close(bool dr) {
  if (w1e == 1) {
    w1wc++;
    lcd.clear();
    unsigned long timen = millis();
    unsigned long t1 = millis();
    byte stat = 0;
    while (digitalRead(W1_CLOSE) == 1 && millis() - timen < W1time) {
      lcd.setCursor(0, 0);
      lcd.print("1 window closing");
      lcd.setCursor(7, 1);
      if (millis() - t1 > 200) {
        t1 = millis();
        if (stat < 3) {
          stat++;
        } else {
          stat = 0;
        }
      }
      switch (stat) {
        case 0:
          lcd.write(5);
          break;
        case 1:
          lcd.write(6);
          break;
        case 2:
          lcd.write(7);
          break;
        case 3:
          lcd.write(8);
          break;
      }
      if (dr == 1) {
        if (millis() - timen < W1time && digitalRead(W1_CLOSE) != 0) {
          digitalWrite(W1c, HIGH);
        }
        if (digitalRead(W1_CLOSE) == 0 || millis() - timen >= W1time) {
          digitalWrite(W1c, LOW);
          w1 = 0;
        }
      }
      if (dr == 0) {
        if (millis() - timen < W1time && digitalRead(W1_CLOSE) != 0) {
          digitalWrite(W1c, LOW);
        }
        if (digitalRead(W1_CLOSE) == 0 || millis() - timen >= W1time) {
          digitalWrite(W1c, HIGH);
          w1 = 0;

        }
      }
    }
    lcd.clear();
  }
}
void windowT_close(bool dr) {
  if (w2e == 1) {
    w2wc++;
    debug_print("w2c");
    lcd.clear();
    unsigned long timen = millis();
    unsigned long t1 = millis();
    byte stat = 0;
    while (digitalRead(W2_CLOSE) == 1 && millis() - timen < W2time) {
      lcd.setCursor(0, 0);
      lcd.print("2 window closing");
      lcd.setCursor(7, 1);
      if (millis() - t1 > 200) {
        t1 = millis();
        if (stat < 3) {
          stat++;
        } else {
          stat = 0;
        }
      }
      switch (stat) {
        case 0:
          lcd.write(5);
          break;
        case 1:
          lcd.write(6);
          break;
        case 2:
          lcd.write(7);
          break;
        case 3:
          lcd.write(8);
          break;
      }
      if (dr == 1) {
        if (millis() - timen < W2time && digitalRead(W2_CLOSE) != 0) {
          digitalWrite(W2c, HIGH);
        }
        if (digitalRead(W2_CLOSE) == 0 || millis() - timen >= W1time) {
          digitalWrite(W2c, LOW);
          w2 = 0;
        }
      }
      if (dr == 0) {
        if (millis() - timen < W2time && digitalRead(W2_CLOSE) != 0) {
          digitalWrite(W2c, LOW);
        }
        if (digitalRead(W2_CLOSE) == 0 || millis() - timen >= W2time) {
          digitalWrite(W2c, HIGH);
          w2 = 0;

        }
      }
    }
    lcd.clear();
  }
}
void door_open(bool dr) {
  if (de == 1) {
    Serial.print("$");
    Serial.print(1);
    Serial.print(dr);
    if (td / 1000 < 9) {
      Serial.print(0);
      Serial.print(td / 1000);
    } else {
      Serial.print(1);
      Serial.print((td / 1000)  / 10);
      Serial.print((td / 1000)  % 10);
    }
    Serial.println(";");
    ddo++;
    lcd.clear();
    unsigned long t1 = millis();
    unsigned long timen = millis();
    byte stat = 0;
    while (millis() - timen < TIME_DOOR_OC) {
      lcd.setCursor(0, 0);
      lcd.print("Door opening");
      lcd.setCursor(7, 1);
      if (millis() - t1 > 200) {
        t1 = millis();
        if (stat < 3) {
          stat++;
        } else {
          stat = 0;
        }
      }
      switch (stat) {
        case 0:
          lcd.write(5);
          break;
        case 1:
          lcd.write(6);
          break;
        case 2:
          lcd.write(7);
          break;
        case 3:
          lcd.write(8);
          break;
      }
    }
    lcd.clear();

  }
}
void work_temp(int setting_temp, bool dr) {

  int temp = dht_temp();
#if (TEMP_MODE == 1)
  if (temp >= setting_temp + 10) {
    if (temp == setting_temp || temp > setting_temp) {
      if (digitalRead(W1_CLOSE) == 0 || w1 == 0) {
        windowO_open(dr);
      }
      if (digitalRead(W2_CLOSE) == 0 || w2 == 0) {
        windowT_open(dr);
      }
      if (door != 1) {
        door_open(dr);
      }
    } else {
      if (door != 0) {
        door_close(dr);
      }
      if (digitalRead(W1_OPEN) == 0 || w1 == 1) {
        windowO_close(dr);
      }
      if (digitalRead(W2_OPEN) == 0 || w2 == 1) {
        windowT_close(dr);
      }
    }
  }
  if (temp >= setting_temp + 3) {
    if (temp == setting_temp || temp > setting_temp) {
      if (digitalRead(W1_CLOSE) == 0 || w1 == 0) {
        windowO_open(dr);
      }
      if (digitalRead(W2_CLOSE) == 0 || w2 == 0) {
        windowT_open(dr);
      }
    } else {
      if (digitalRead(W1_OPEN) == 0 || w1 == 1) {
        windowO_close(dr);
      }
      if (digitalRead(W2_OPEN) == 0 || w2 == 1) {
        windowT_close(dr);
      }
    }
  }
#elif (TEMP_MODE == 2)
  if (temp >= setting_temp + 10) {
    if (temp == setting_temp || temp > setting_temp - 3) {
      if (digitalRead(W1_CLOSE) == 0 || w1 == 0) {
        windowO_open(dr);
      }
      if (digitalRead(W2_CLOSE) == 0 || w2 == 0) {
        windowT_open(dr);
      }
      if (door != 1) {
        door_open(dr);
      }
    } else {
      if (door != 0) {
        door_close(dr);
      }
      if (digitalRead(W1_OPEN) == 0 || w1 == 1) {
        windowO_close(dr);
      }
      if (digitalRead(W2_OPEN) == 0 || w2 == 1) {
        windowT_close(dr);
      }
    }
  }
  if (temp >= setting_temp) {
    if (temp == setting_temp || temp > setting_temp - 3) {
      if (digitalRead(W1_CLOSE) == 0 || w1 == 0) {
        windowO_open(dr);
      }
      if (digitalRead(W2_CLOSE) == 0 || w2 == 0) {
        windowT_open(dr);
      }
    } else {
      if (door != 0) {
        door_close(dr);
      }
      if (digitalRead(W1_OPEN) == 0 || w1 == 1) {
        windowO_close(dr);
      }
      if (digitalRead(W1_OPEN) == 0 || w1 == 1) {
        windowO_close(dr);
      }
    }
  }
#endif
  if (temp < setting_temp && setting_temp - temp >= 5) {
    if (digitalRead(W1_OPEN) == 0 || w1 == 1) {
      windowO_close(dr);
    }
    if (digitalRead(W2_OPEN) == 0 || w2 == 1) {
      windowT_close(dr);
    }
    if (door != 0) {
      door_close(dr);
    }
  }
}

void day_fase(int ph) {
  int val = analogRead(PHOTORESISTOR);
  val = map(val, 0, 1023, 0, 99);
#if(PHmode == 1)
  if (val < ph) {
    day = 1;
  }
  else {
    day = 0;
  }
#elif(PHmode == 2)
  if (val > ph) {
    day = 1;
  }
  else {
    day = 0;
  }
#endif
}

void temp_work(bool dr, int temp) {
  if (day == 0) {
    if (digitalRead(W1_OPEN) == 0 || w1 == 1) {
      windowO_close(dr);
    }
    if (door != 0 ) {
      door_close(dr);
    }
    night_t(dr);
  }
  if (day == 1) {
    work_temp(temp, dr);
  }
}
void nulltopin(bool dr) {
  if (dr == 1) {
    digitalWrite(W1o, LOW);
    digitalWrite(W1c, LOW);
    digitalWrite(W2o, LOW);
    digitalWrite(W2c, LOW);
    digitalWrite(S1, LOW);
    digitalWrite(S2, LOW);
  }
  if (dr == 0) {
    digitalWrite(W1o, HIGH);
    digitalWrite(W1c, HIGH);
    digitalWrite(W2o, HIGH);
    digitalWrite(W2c, HIGH);
    digitalWrite(S1, HIGH);
    digitalWrite(S2, HIGH);
  }
}

void start() {
  lcd.backlight();
  if (digitalRead(W1_OPEN) == 0) {
    w1 = 1;
  }
  else if (digitalRead(W1_CLOSE) == 0) {
    w1 = 0;
  }
  else {
    windowO_close(dr);
  }
  if (digitalRead(W2_OPEN) == 0) {
    w2 = 1;
  }
  else if (digitalRead(W2_CLOSE) == 0) {
    w2 = 0;
  }
  else {
    windowT_close(dr);
  }
  if (dop() == 1) {
    door = 1;
  }
  else if (doc() == 1) {
    door = 0;
  }
  else {
    door_close(dr);
  }
}
void night_t(bool dr) {
  if (dht_temp() - temp > 5) {
    if (digitalRead(W2_CLOSE) == 0 || w2 == 0) {
      windowT_open(dr);
    }
  } else {
    if (digitalRead(W2_OPEN) == 0 || w2 == 1) {
      windowT_close(dr);
    }
  }
}


void update_params() {
  temp = eeprom_read_byte(0);  //eeprom read temp_setting value for index 0
  PHval = eeprom_read_byte(1); //eeprom read photoresistor value for index 1
  w_temp = eeprom_read_byte(2); //eeprom read water temp for index 2
  hud = eeprom_read_byte(3);
  byte hum = eeprom_read_byte(3);
  if (w_temp > 9) {
    bool f = 0;
    if (hum > 9) {
      f = 1;
    }
    int mass[] = {6, 1, w_temp, f, hum};
    sends(6, mass);
  } else {
    bool f = 0;
    if (hum > 9) {
      f = 1;
    }
    int mass[] = {6, 0, w_temp, f, hum};
    sends(6, mass);
  }
}
void water_temp() {

}
int parsing() {
  static bool parseStart = false;
  static byte counter = 0;
  if (Serial.available()) {
    char in = Serial.read();
    if (in == '\n' || in == '\r') return 0; // игнорируем перевод строки
    if (in == ';') {        // завершение пакета
      parseStart = false;
      return counter;
    }
    if (in == '$') {        // начало пакета
      parseStart = true;
      counter = 0;
      return 0;
    }
    if (parseStart) {       // чтение пакета
      // - '0' это перевод в число (если отправитель print)
      buffer[counter] = in - ASCII_CONVERT;
      counter++;
    }
  }
  return 0;
}
bool dop() {
  uint32_t tt = millis();
  Serial.print("$");
  Serial.print(5);
  Serial.print(1);
  Serial.println(";");
  while (millis() - tt < 150) {
    if (parsing()) {
      switch (buffer[0]) {
        case 2:
          if (buffer[1] == 1) {
            return buffer[2];
          } else {
            return 0;
          }
          break;

      }

    }
  }
}
bool doc() {
  uint32_t tt = millis();
  Serial.print("$");
  Serial.print(5);
  Serial.print(0);
  Serial.println(";");
  while (millis() - tt < 150) {
    if (parsing()) {
      switch (buffer[0]) {
        case 2:
          if (buffer[1] == 0) {
            return buffer[2];
          } else {
            return 0;
          }
          break;

      }


    }
  }
}
void sensors() {
  sensor1 = analogRead(SENSOR1);
  sensor1 = map(sensor1, 0, 1023, 0, 99);
  sensor2 = analogRead(SENSOR2);
  sensor2 = map(sensor2, 0, 1023, 0, 99);
  if (sensor1 < hud && s1 == 1) {
    uint32_t t = millis();
    lcd.clear();
    unsigned long timen = millis();
    byte stat = 0;
    lcd.clear();
    while (millis() - t < GTIME  && sensor1 < hud) {
      sensor1 = analogRead(SENSOR1);
      sensor1 = map(sensor1, 0, 1023, 0, 99);
      lcd.setCursor(0, 0);
      lcd.print("Box1: ");
      lcd.print(sensor1);
      lcd.setCursor(7, 1);
      if (millis() - timen > 200) {
        timen = millis();
        if (stat < 3) {
          stat++;
        } else {
          stat = 0;
        }
      }
      switch (stat) {
        case 0:
          lcd.write(5);
          break;
        case 1:
          lcd.write(6);
          break;
        case 2:
          lcd.write(7);
          break;
        case 3:
          lcd.write(8);
          break;
      }
      digitalWrite(S1, dr);
    }
    nulltopin(dr);
    s1 = 0;
    s1t = millis();
  }
  if (sensor2 < hud && s2 == 1) {
    lcd.clear();
    unsigned long timen = millis();
    byte stat = 0;
    uint32_t t = millis();
    while (millis() - t < GTIME  && sensor2 < hud) {
      sensor2 = analogRead(SENSOR2);
      sensor2 = map(sensor2, 0, 1023, 0, 99);
      lcd.setCursor(0, 0);
      lcd.print("Box2: ");
      lcd.print(sensor2);
      lcd.setCursor(7, 1);
      if (millis() - timen > 200) {
        timen = millis();
        if (stat < 3) {
          stat++;
        } else {
          stat = 0;
        }
      }
      switch (stat) {
        case 0:
          lcd.write(5);
          break;
        case 1:
          lcd.write(6);
          break;
        case 2:
          lcd.write(7);
          break;
        case 3:
          lcd.write(8);
          break;
      }
      digitalWrite(S2, dr);
    }
    nulltopin(dr);
    s2 = 0;
    s2t = millis();
  }
}
void sends(byte col, int massive[]) {
  Serial.print("$");
  for (byte i = 0; i < col - 1; i++) {
    Serial.print(massive[i]);
  }
  Serial.println(";");
}
