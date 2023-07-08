// Code Author: Norokh Nikita
// 08.07.2023

#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#define ONE_WIRE_BUS 2
#define ASCII_CONVERT '0'
//Relay to water bucket filling
#define fill 3

//relay to pump

//water lavels
#define levelH 8
#define levelL 7
//Times
#define pumpT 5000
#define velvT 5000
#define SENSORt 5000
//SHIFT
#define latchPin  9
#define clockPin  13
#define dataPin  10
//SHIFT

//SENSORS
#define SENSOR1 A0
#define SENSOR2 A1
#define SENSOR3 A2
#define SENSOR4 A3
#define SENSOR5 A4
#define SENSOR6 A5
#define SENSOR7 A6
#define SENSOR8 A7



#define pumpW 9
/* Table of serial commands
    1 - start data sending
    2 - stop data sending
    3 - recieve new set water temp
    6 - d open switch
    7 - d close switch
*/




#define D_OPEN 5  //limit switch to open door
#define D_CLOSE 6  //limit switch to close door

#define Do 3  //relay output to open door
#define Dc 4  //relay output to close door

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temp(&oneWire);

bool dr = 0;
bool ddr = 0;
bool wt_send = 0;
bool shift_changed = 1;

uint32_t wts = 0;

uint32_t tm1 = millis();
uint32_t tm2 = millis();
uint32_t tm3 = millis();
uint32_t tm4 = millis();
uint32_t tm5 = millis();
uint32_t tm6 = millis();
uint32_t tm7 = millis();
uint32_t tm8 = millis();

uint32_t lcht = millis();

bool s1 = 0;
bool s2 = 0;
bool s3 = 0;
bool s4 = 0;
bool s5 = 0;
bool s6 = 0;
bool s7 = 0;
bool s8 = 0;

int sensor1 = analogRead(SENSOR1);
int sensor2 = analogRead(SENSOR2);
int sensor3 = analogRead(SENSOR3);
int sensor4 = analogRead(SENSOR4);
int sensor5 = analogRead(SENSOR5);
int sensor6 = analogRead(SENSOR6);
int sensor7 = analogRead(SENSOR7);
int sensor8 = analogRead(SENSOR8);

byte stemp = eeprom_read_byte(2);
byte hud = eeprom_read_byte(3);

bool water = 1;
char todo[6];
void setup() {
  Serial.begin(19200);
  temp.begin();
  Serial.println(stemp);
  Serial.println(hud);

  //digital pins
  pinMode(levelH, INPUT_PULLUP);
  pinMode(levelL, INPUT_PULLUP);

  pinMode(D_OPEN, INPUT_PULLUP);
  pinMode(D_CLOSE, INPUT_PULLUP);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  digitalWrite(latchPin, HIGH);

  pinMode(13, OUTPUT);

  //relays
  pinMode(Do, OUTPUT);
  pinMode(Dc, OUTPUT);
  pinMode(pumpW, OUTPUT);


  digitalWrite(Do, HIGH);
  digitalWrite(Dc, HIGH);


  //SENSORS
  pinMode(SENSOR1, INPUT);
  pinMode(SENSOR2, INPUT);
  pinMode(SENSOR3, INPUT);
  pinMode(SENSOR4, INPUT);
  pinMode(SENSOR5, INPUT);
  pinMode(SENSOR6, INPUT);
  pinMode(SENSOR7, INPUT);
  pinMode(SENSOR8, INPUT);



  bucket_control();
  nulltopin();
  stemp = eeprom_read_byte(2);
  hud = eeprom_read_byte(3);
}
uint32_t t1 = millis();
uint32_t t2 = millis();
byte sr = 0;

byte buffer[8];


bool door = 0;
bool led = 0;
int t = 0;

bool dsend = 0;
bool diag = 0;

int  tempp = 0;
uint32_t ltemp = millis();
void loop() {
  if (diag == 0) {
    nulltopin();
  }
  if (diag == 1 && millis() - t2 > 15000) {
    diag = 0;
  }

  if (parsing()) {
    switch (buffer[0]) {
      case 1:
        if (buffer[2] == 1) {
          t = (String(buffer[3]) + String(buffer[4])).toInt();
        }
        else {
          t = buffer[3];
        }
        door_open(buffer[1], t * 1000);
        dr = buffer[1];
        break;
      case 2:
        Serial.println("dooring");
        if (buffer[2] == 1) {
          t = (String(buffer[3]) + String(buffer[4])).toInt();
        }
        else {
          t = buffer[3];
        }
        door_close(buffer[1], t * 1000);
        dr = buffer[1];
        break;
      case 3:
        if (buffer[1] == 1) {
          dsend = 1;
        } else {
          dsend = 0;
        }
        break;
      case 4:
        diag  = 1;
        t2 = millis();
        if (buffer[1] == 0) {
          if (buffer[2] == 0) {
            digitalWrite(Dc, LOW);
          } else {
            digitalWrite(Dc, HIGH);
            Serial.println("dcnp");
          }
        } else {
          if (buffer[2] == 0) {
            digitalWrite(Do, LOW);
          } else {
            digitalWrite(Do, HIGH);
          }
        }

        break;
      case 5:
        if (buffer[1] == 1) {
          Serial.print("$");
          Serial.print(2);
          Serial.print(1);
          Serial.print(!digitalRead(D_OPEN));
          Serial.println(";");
        } else {
          Serial.print("$");
          Serial.print(2);
          Serial.print(0);
          Serial.print(!digitalRead(D_CLOSE));
          Serial.println(";");
        }
        break;
      case 6:
        if (buffer[1]) {
          byte val = (String(buffer[2]) + String(buffer[3])).toInt();
          eeprom_write_byte(2, val);
          stemp = eeprom_read_byte(2);
          if (buffer[4]) {
            byte val = (String(buffer[5]) + String(buffer[6])).toInt();
            eeprom_write_byte(3, val);
            hud = eeprom_read_byte(3);
          } else {
            eeprom_write_byte(3, buffer[5]);
            hud = eeprom_read_byte(3);
          }
        } else {
          eeprom_write_byte(2, buffer[2]);
          stemp = eeprom_read_byte(2);
          if (buffer[3]) {
            byte val = (String(buffer[4]) + String(buffer[5])).toInt();
            eeprom_write_byte(3, val);
            hud = eeprom_read_byte(3);
          } else {
            eeprom_write_byte(3, buffer[4]);
            hud = eeprom_read_byte(3);
          }
        }
        break;
    }

  }
  if (dsend == 1 && millis() - t1 > 1500) {
    tempp = tempg();
    t1 = millis();
    Serial.print("$");
    Serial.print(1);
    if (tempp > 9) {
      Serial.print(1);
      Serial.print(tempp);
    } else {
      Serial.print(0);
      Serial.print(tempp);
    }
    Serial.print(!digitalRead(levelH));
    Serial.print(!digitalRead(levelL));
    Serial.println(";");
  }
  bucket_control();
  sensors();
  //Serial.println("----------");
  //Serial.println(analogRead(A4));
  //Serial.println(analogRead(A5));
  //Serial.println("----------");
  //delay(200);
  if(todo[0] == 2){
    door_open(todo[1], todo[2] * 1000);
    todo[0] = 0;
    }
      if(todo[3] == 2){
    door_open(todo[4], todo[5] * 1000);
    todo[3] = 0;
    }
}


void bucket_control() {
  if (millis() - ltemp > 30000) {
    tempp = tempg();
    if (tempp < stemp && water == 1) {
      water = 0;
      Serial.print("$");
      Serial.print(3);
      Serial.print(0);
      Serial.println(";");
    }
    if (tempp >= stemp && water == 0) {
      water = 1;
      Serial.print("$");
      Serial.print(3);
      Serial.print(1);
      Serial.println(";");
    }
    ltemp = millis();
  }
  if (digitalRead(levelH) == 1 && digitalRead(levelL) == 1) {
    water = 0;
    Serial.print("$");
    Serial.print(3);
    Serial.print(0);
    Serial.println(";");
    uint32_t t = millis();
    while (digitalRead(levelH) == 1 && digitalRead(levelL) == 1 && millis() - t < pumpT) {
      digitalWrite(pumpW, dr);
    }
  }
}




int tempg() {
  temp.requestTemperatures();
  int temps = round(temp.getTempCByIndex(0));
  return temps;
}


void door_open(bool dr, int TIME_DOOR_OC) {

  uint32_t timen = millis();
  while (millis() - timen < TIME_DOOR_OC && digitalRead(D_OPEN) != 0) {
    if (dr == 1) {
      if (millis() - timen < TIME_DOOR_OC && digitalRead(D_OPEN) != 0) {
        digitalWrite(Do, HIGH);

      } else {
        Serial.print("$");
        Serial.print(3);
        Serial.println(";");
        digitalWrite(Do, LOW);
        door = 1;
      }
    }
    if (dr == 0) {
      if (millis() - timen < TIME_DOOR_OC && digitalRead(D_OPEN) != 0) {
        digitalWrite(Do, LOW);
      } else {
        Serial.print("$");
        Serial.print(3);
        Serial.println(";");
        digitalWrite(Do, HIGH);
        door = 1;
      }

    }
  }
}

void door_close(bool dr, int TIME_DOOR_OC) {
  uint32_t timen = millis();
  while (millis() - timen < TIME_DOOR_OC && digitalRead(D_CLOSE) != 0) {
    if (dr == 1) {
      if (millis() - timen < TIME_DOOR_OC && digitalRead(D_CLOSE) != 0) {
        digitalWrite(Dc, HIGH);
      } else {
        Serial.print("$");
        Serial.print(3);
        Serial.println(";");
        digitalWrite(Dc, LOW);
        door = 0;
      }
    }

    if (dr == 0) {
      if (millis() - timen < TIME_DOOR_OC && digitalRead(D_CLOSE) != 0) {
        digitalWrite(Dc, LOW);
      } else {
        Serial.print("$");
        Serial.print(3);
        Serial.println(";");
        digitalWrite(Dc, HIGH);
        door = 0;
      }
    }

  }
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
void nulltopin() {
  if (dr == 0) {
    digitalWrite(Do, 1);
    digitalWrite(Dc, 1);
  } else {
    digitalWrite(Do, 0);
    digitalWrite(Dc, 0);
  }
  if (ddr == 0) {
    if (shift_changed) {
      byte data = 0b11111111;
      shiftOut(dataPin, clockPin, LSBFIRST, data);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
      Serial.println("0000");
      shift_changed = 0;
      delay(100);
    }
  } else {
    if (shift_changed) {
      byte data = 0b00000000;
      shiftOut(dataPin, clockPin, LSBFIRST, data);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
      Serial.println("1111");
      shift_changed = 0;
      delay(100);
    }
  }
  digitalWrite(pumpW, !dr);
}
void sends(byte col, int massive[]) {
  Serial.print("$");
  for (byte i = 0; i < col - 1; i++) {
    Serial.print(massive[i]);
  }
  Serial.println(";");
}


void shift(byte data) {
  shiftOut(dataPin, clockPin, LSBFIRST, data);
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
  Serial.println(data);
  delay(100);
}

uint32_t timerr = millis();
uint32_t looper = millis();

void sensors() {
  if (s1 == 1 && millis() - tm1 > SENSORt * 3) {
    s1 = 0;
  }
  if (s2 == 1 && millis() - tm2 > SENSORt * 3) {
    s2 = 0;
  }
  if (s3 == 1 && millis() - tm3 > SENSORt * 3) {
    s3 = 0;
  }
  if (s4 == 1 && millis() - tm4 > SENSORt * 3) {
    s4 = 0;
  }
  if (s5 == 1 && millis() - tm5 > SENSORt * 3) {
    s5 = 0;
  }
  if (s6 == 1 && millis() - tm6 > SENSORt * 3) {
    s6 = 0;
  }
  if (s7 == 1 && millis() - tm7 > SENSORt * 3) {
    s7 = 0;
  }
  if (s8 == 1 && millis() - tm8 > SENSORt * 3) {
    s8 = 0;
  }
  sensor1 = analogRead(SENSOR1);
  sensor2 = analogRead(SENSOR2);
  sensor3 = analogRead(SENSOR3);
  sensor4 = analogRead(SENSOR4);
  sensor5 = analogRead(SENSOR5);
  sensor6 = analogRead(SENSOR6);
  sensor7 = analogRead(SENSOR7);
  sensor8 = analogRead(SENSOR8);

  sensor1 = map(sensor1, 0, 1023, 0, 99);
  sensor2 = map(sensor2, 0, 1023, 0, 99);
  sensor3 = map(sensor3, 0, 1023, 0, 99);
  sensor4 = map(sensor4, 0, 1023, 0, 99);
  sensor5 = map(sensor5, 0, 1023, 0, 99);
  sensor6 = map(sensor6, 0, 1023, 0, 99);
  sensor7 = map(sensor7, 0, 1023, 0, 99);
  sensor8 = map(sensor8, 0, 1023, 0, 99);

  //-------------------------------------------1------------------------------------------------------------
  if (sensor1 < hud - 7 && s1 == 0) {
    Serial.print("$");
    Serial.print(4);
    Serial.print(1);
    Serial.print(3);   //Sensor 3 started
    if (sensor1 > 9) {
      Serial.print(1);
      Serial.print(sensor1);
    } else {
      Serial.print(0);
      Serial.print(sensor1);
    }
    Serial.println(";");
    timerr = millis();
    while (millis() - timerr < 2000) {
      if (millis() - looper > 200) {

        looper = millis();
        Serial.print("$");
        Serial.print(4);
        Serial.print(1);
        Serial.print(3);   //Sensor 3 started
        if (sensor1 > 9) {
          Serial.print(1);
          Serial.print(sensor1);
        } else {
          Serial.print(0);
          Serial.print(sensor1);
        }
        Serial.println(";");

      }
      if (parsing()) {
        if (buffer[0] == 9 && buffer[1] == 1) {
          break;
        }
      }
    }

    if (ddr) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b10000000);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    } else {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b01111111);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    }
    shift_changed = 1;
    uint32_t t = millis();
    while (sensor1 < hud + 7  && millis() - t < SENSORt && s1 == 0) {
      sensor1 = analogRead(SENSOR1);
      sensor1 = map(sensor1, 0, 1023, 0, 99);
      Serial.print("$");
      Serial.print(5);
      if (sensor1 > 9) {
        Serial.print(1);
        Serial.print(sensor1);
      } else {
        Serial.print(0);
        Serial.print(sensor1);
      }
      Serial.println(";");
      if(Serial.available()){chfd();}
      delay(100);
    }
    Serial.print("$");
    Serial.print(4);
    Serial.print(0);
    Serial.print(3);   //Sensor 3 started
    Serial.println(";");
    s1 = 1;
    tm1 = millis();
  }



  //------------------------------------------/1\-----------------------------------------------------------
  //-------------------------------------------2------------------------------------------------------------
  if (sensor2 < hud - 7 && s2 == 0) {
    Serial.print("$");
    Serial.print(4);
    Serial.print(1);
    Serial.print(4);   //Sensor 3 started
    if (sensor2 > 9) {
      Serial.print(1);
      Serial.print(sensor2);
    } else {
      Serial.print(0);
      Serial.print(sensor2);
    }
    Serial.println(";");
    timerr = millis();
    while (millis() - timerr < 2000) {
      if (millis() - looper > 200) {

        looper = millis();
        Serial.print("$");
        Serial.print(4);
        Serial.print(1);
        Serial.print(4);   //Sensor 3 started
        if (sensor2 > 9) {
          Serial.print(1);
          Serial.print(sensor2);
        } else {
          Serial.print(0);
          Serial.print(sensor2);
        }
        Serial.println(";");

      }
      if (parsing()) {
        if (buffer[0] == 9 && buffer[1] == 1) {
          break;
        }
      }
    }
    if (ddr) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b01000000);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    } else {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b10111111);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    }
    shift_changed = 1;
    uint32_t t = millis();
    while (sensor2 < hud + 7  && millis() - t < SENSORt && s2 == 0) {
      sensor2 = analogRead(SENSOR2);
      sensor2 = map(sensor2, 0, 1023, 0, 99);
      Serial.print("$");
      Serial.print(5);
      if (sensor1 > 9) {
        Serial.print(1);
        Serial.print(sensor2);
      } else {
        Serial.print(0);
        Serial.print(sensor2);
      }
      Serial.println(";");
      if(Serial.available()){chfd();}
      delay(100);
    }
    Serial.print("$");
    Serial.print(4);
    Serial.print(0);
    Serial.print(4);   //Sensor 3 started
    Serial.println(";");
    s2 = 1;
    tm2 = millis();
  }

  //------------------------------------------/2\-----------------------------------------------------------


  //-------------------------------------------3------------------------------------------------------------
  if (sensor3 < hud - 7 && s3 == 0) {
    Serial.print("$");
    Serial.print(4);
    Serial.print(1);
    Serial.print(5);   //Sensor 3 started
    if (sensor2 > 9) {
      Serial.print(1);
      Serial.print(sensor3);
    } else {
      Serial.print(0);
      Serial.print(sensor3);
    }
    Serial.println(";");
    //BOMB
    timerr = millis();
    while (millis() - timerr < 2000) {
      if (millis() - looper > 200) {

        looper = millis();
        Serial.print("$");
        Serial.print(4);
        Serial.print(1);
        Serial.print(5);   //Sensor 3 started
        if (sensor2 > 9) {
          Serial.print(1);
          Serial.print(sensor3);
        } else {
          Serial.print(0);
          Serial.print(sensor3);
        }
        Serial.println(";");

      }
      if (parsing()) {
        if (buffer[0] == 9 && buffer[1] == 1) {
          break;
        }
      }
    }
    if (ddr) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00100000);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    } else {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b11011111);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    }
    shift_changed = 1;
    uint32_t t = millis();
    while (sensor3 < hud + 7  && millis() - t < SENSORt && s3 == 0) {
      sensor3 = analogRead(SENSOR3);
      sensor3 = map(sensor3, 0, 1023, 0, 99);
      Serial.print("$");
      Serial.print(5);
      if (sensor1 > 9) {
        Serial.print(1);
        Serial.print(sensor3);
      } else {
        Serial.print(0);
        Serial.print(sensor3);
      }
      Serial.println(";");
      if(Serial.available()){chfd();}
      delay(100);
    }
    Serial.print("$");
    Serial.print(4);
    Serial.print(0);
    Serial.print(5);   //Sensor 3 started
    Serial.println(";");
    s3 = 1;
    tm3 = millis();
  }
  //------------------------------------------/3\-----------------------------------------------------------


  //-------------------------------------------4------------------------------------------------------------
  if (sensor4 < hud - 7 && s4 == 0) {
    Serial.print("$");
    Serial.print(4);
    Serial.print(1);
    Serial.print(6);   //Sensor 3 started
    if (sensor2 > 9) {
      Serial.print(1);
      Serial.print(sensor4);
    } else {
      Serial.print(0);
      Serial.print(sensor4);
    }
    Serial.println(";");
    //BOMB
    timerr = millis();
    while (millis() - timerr < 2000) {
      if (millis() - looper > 200) {

        looper = millis();
        Serial.print("$");
        Serial.print(4);
        Serial.print(1);
        Serial.print(6);   //Sensor 3 started
        if (sensor2 > 9) {
          Serial.print(1);
          Serial.print(sensor4);
        } else {
          Serial.print(0);
          Serial.print(sensor4);
        }
        Serial.println(";");

      }
      if (parsing()) {
        if (buffer[0] == 9 && buffer[1] == 1) {
          break;
        }
      }
    }
    if (ddr) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00010000);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    } else {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b11101111);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    }
    shift_changed = 1;
    uint32_t t = millis();
    while (sensor4 < hud + 7  && millis() - t < SENSORt && s4 == 0) {
      sensor4 = analogRead(SENSOR4);
      sensor4 = map(sensor4, 0, 1023, 0, 99);
      Serial.print("$");
      Serial.print(5);
      if (sensor1 > 9) {
        Serial.print(1);
        Serial.print(sensor4);
      } else {
        Serial.print(0);
        Serial.print(sensor4);
      }
      Serial.println(";");
      if(Serial.available()){chfd();}
      delay(100);
    }
    Serial.print("$");
    Serial.print(4);
    Serial.print(0);
    Serial.print(6);   //Sensor 3 started
    Serial.println(";");
    s4 = 1;
    tm4 = millis();
  }
  //------------------------------------------/4\-----------------------------------------------------------


  //-------------------------------------------5------------------------------------------------------------
  if (sensor5 < hud - 7 && s5 == 0) {
    Serial.print("$");
    Serial.print(4);
    Serial.print(1);
    Serial.print(7);   //Sensor 3 started
    if (sensor2 > 9) {
      Serial.print(1);
      Serial.print(sensor5);
    } else {
      Serial.print(0);
      Serial.print(sensor5);
    }
    Serial.println(";");
    //BOMB
    timerr = millis();
    while (millis() - timerr < 2000) {
      if (millis() - looper > 200) {

        looper = millis();
        Serial.print("$");
        Serial.print(4);
        Serial.print(1);
        Serial.print(7);   //Sensor 3 started
        if (sensor2 > 9) {
          Serial.print(1);
          Serial.print(sensor5);
        } else {
          Serial.print(0);
          Serial.print(sensor5);
        }
        Serial.println(";");

      }
      if (parsing()) {
        if (buffer[0] == 9 && buffer[1] == 1) {
          break;
        }
      }
    }
    if (ddr) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00001000);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    } else {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b11110111);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    }
    shift_changed = 1;
    uint32_t t = millis();
    while (sensor5 < hud + 7  && millis() - t < SENSORt && s5 == 0) {
      sensor5 = analogRead(SENSOR5);
      sensor5 = map(sensor5, 0, 1023, 0, 99);
      Serial.print("$");
      Serial.print(5);
      if (sensor1 > 9) {
        Serial.print(1);
        Serial.print(sensor5);
      } else {
        Serial.print(0);
        Serial.print(sensor5);
      }
      Serial.println(";");
      if(Serial.available()){chfd();}
      delay(100);
    }
    Serial.print("$");
    Serial.print(4);
    Serial.print(0);
    Serial.print(7);   //Sensor 3 started
    Serial.println(";");
    s5 = 1;
    tm5 = millis();
  }
  //------------------------------------------/5\-----------------------------------------------------------


  //-------------------------------------------6------------------------------------------------------------
  if (sensor6 < hud - 7 && s6 == 0) {
    Serial.print("$");
    Serial.print(4);
    Serial.print(1);
    Serial.print(8);   //Sensor 3 started
    if (sensor2 > 9) {
      Serial.print(1);
      Serial.print(sensor6);
    } else {
      Serial.print(0);
      Serial.print(sensor6);
    }
    Serial.println(";");
    //BOMB
    timerr = millis();
    while (millis() - timerr < 2000) {
      if (millis() - looper > 200) {

        looper = millis();
        Serial.print("$");
        Serial.print(4);
        Serial.print(1);
        Serial.print(8);   //Sensor 3 started
        if (sensor2 > 9) {
          Serial.print(1);
          Serial.print(sensor6);
        } else {
          Serial.print(0);
          Serial.print(sensor6);
        }
        Serial.println(";");

      }
      if (parsing()) {
        if (buffer[0] == 9 && buffer[1] == 1) {
          break;
        }
      }
    }
    if (ddr) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000100);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    } else {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b11111011);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    }
    shift_changed = 1;
    uint32_t t = millis();
    while (sensor6 < hud + 7  && millis() - t < SENSORt && s6 == 0) {
      sensor6 = analogRead(SENSOR6);
      sensor6 = map(sensor6, 0, 1023, 0, 99);
      Serial.print("$");
      Serial.print(5);
      if (sensor1 > 9) {
        Serial.print(1);
        Serial.print(sensor6);
      } else {
        Serial.print(0);
        Serial.print(sensor6);
      }
      Serial.println(";");
      if(Serial.available()){chfd();}
      delay(100);
    }
    Serial.print("$");
    Serial.print(4);
    Serial.print(0);
    Serial.print(8);   //Sensor 3 started
    Serial.println(";");
    s6 = 1;
    tm6 = millis();
  }
  //------------------------------------------/6\-----------------------------------------------------------


  //-------------------------------------------7------------------------------------------------------------
  if (sensor7 < hud - 7 && s7 == 0) {
    Serial.print("$");
    Serial.print(4);
    Serial.print(1);
    Serial.print(9);   //Sensor 3 started
    if (sensor2 > 9) {
      Serial.print(1);
      Serial.print(sensor7);
    } else {
      Serial.print(0);
      Serial.print(sensor7);
    }
    Serial.println(";");
    //BOMB
    timerr = millis();
    while (millis() - timerr < 2000) {
      if (millis() - looper > 200) {

        looper = millis();
        Serial.print("$");
        Serial.print(4);
        Serial.print(1);
        Serial.print(9);   //Sensor 3 started
        if (sensor2 > 9) {
          Serial.print(1);
          Serial.print(sensor7);
        } else {
          Serial.print(0);
          Serial.print(sensor7);
        }
        Serial.println(";");

      }
      if (parsing()) {
        if (buffer[0] == 9 && buffer[1] == 1) {
          break;
        }
      }
    }
    if (ddr) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000010);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    } else {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b11111101);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    }
    shift_changed = 1;
    uint32_t t = millis();
    while (sensor7 < hud + 7  && millis() - t < SENSORt && s7 == 0) {
      sensor7 = analogRead(SENSOR7);
      sensor7 = map(sensor7, 0, 1023, 0, 99);
      Serial.print("$");
      Serial.print(5);
      if (sensor1 > 9) {
        Serial.print(1);
        Serial.print(sensor7);
      } else {
        Serial.print(0);
        Serial.print(sensor7);
      }
      Serial.println(";");
      if(Serial.available()){chfd();}
      delay(100);
    }
    Serial.print("$");
    Serial.print(4);
    Serial.print(0);
    Serial.print(9);   //Sensor 3 started
    Serial.println(";");
    s7 = 1;
    tm7 = millis();
  }
  //------------------------------------------/7\-----------------------------------------------------------


  //-------------------------------------------8------------------------------------------------------------
  if (sensor8 < hud - 7 && s8 == 0) {
    Serial.print("$");
    Serial.print(4);
    Serial.print(1);
    Serial.print(10);   //Sensor 3 started
    if (sensor2 > 9) {
      Serial.print(1);
      Serial.print(sensor8);
    } else {
      Serial.print(0);
      Serial.print(sensor8);
    }
    Serial.println(";");
    //BOMB
    timerr = millis();
    while (millis() - timerr < 2000) {
      if (millis() - looper > 200) {

        looper = millis();
        Serial.print("$");
        Serial.print(4);
        Serial.print(1);
        Serial.print(10);   //Sensor 3 started
        if (sensor2 > 9) {
          Serial.print(1);
          Serial.print(sensor8);
        } else {
          Serial.print(0);
          Serial.print(sensor8);
        }
        Serial.println(";");

      }
      if (parsing()) {
        if (buffer[0] == 9 && buffer[1] == 1) {
          break;
        }
      }
    }
    if (ddr) {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    } else {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b11111110);
      digitalWrite(latchPin, LOW);
      digitalWrite(latchPin, HIGH);
    }
    shift_changed = 1;
    uint32_t t = millis();
    while (sensor8 < hud + 7 && millis() - t < SENSORt && s8 == 0) {
      sensor8 = analogRead(SENSOR8);
      sensor8 = map(sensor8, 0, 1023, 0, 99);
      Serial.print("$");
      Serial.print(5);
      if (sensor1 > 9) {
        Serial.print(1);
        Serial.print(sensor8);
      } else {
        Serial.print(0);
        Serial.print(sensor8);
      }
      Serial.println(";");
      if(Serial.available()){chfd();}
      delay(100);
    }
    Serial.print("$");
    Serial.print(4);
    Serial.print(0);
    Serial.print(10);   //Sensor 3 started
    Serial.println(";");
    s8 = 1;
    tm8 = millis();
  }
  //------------------------------------------/8\-----------------------------------------------------------


}
void chfd() {
  Serial.println("sSDSDSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDSSSSSSSSSSS");
  if (parsing()) {
//    switch (buffer[0]) {
//      case 1:
  if(buffer[0] == 1){
        if (buffer[2] == 1) {
          t = (String(buffer[3]) + String(buffer[4])).toInt();
        }
        else {
          t = buffer[3];
        }
        todo[0] = 2;
        todo[1] = buffer[1];
        todo[2] = t;
        dr = buffer[1];
  }
//        break;
//      case 2:
//        if (buffer[2] == 1) {
//          t = (String(buffer[3]) + String(buffer[4])).toInt();
//        }
//        else {
//          t = buffer[3];
//        }
//        todo[3] = 2;
//        todo[4] = buffer[1];
//        todo[5] = t;
//        dr = buffer[1];
//        break;
//      case 3:
//        if (buffer[1] == 1) {
//          dsend = 1;
//        } else {
//          dsend = 0;
//        }
//        break;
//      case 5:
//        if (buffer[1] == 1) {
//          Serial.print("$");
//          Serial.print(2);
//          Serial.print(1);
//          Serial.print(!digitalRead(D_OPEN));
//          Serial.println(";");
//        } else {
//          Serial.print("$");
//          Serial.print(2);
//          Serial.print(0);
//          Serial.print(!digitalRead(D_CLOSE));
//          Serial.println(";");
//        }
//        break;
//      case 6:
//        if (buffer[1]) {
//          byte val = (String(buffer[2]) + String(buffer[3])).toInt();
//          eeprom_write_byte(2, val);
//          stemp = eeprom_read_byte(2);
//          if (buffer[4]) {
//            byte val = (String(buffer[5]) + String(buffer[6])).toInt();
//            eeprom_write_byte(3, val);
//            hud = eeprom_read_byte(3);
//          } else {
//            eeprom_write_byte(3, buffer[5]);
//            hud = eeprom_read_byte(3);
//          }
//        } else {
//          eeprom_write_byte(2, buffer[2]);
//          stemp = eeprom_read_byte(2);
//          if (buffer[3]) {
//            byte val = (String(buffer[4]) + String(buffer[5])).toInt();
//            eeprom_write_byte(3, val);
//            hud = eeprom_read_byte(3);
//          } else {
//            eeprom_write_byte(3, buffer[4]);
//            hud = eeprom_read_byte(3);
//          }
//        }
//        break;
//    }
//
  }
}
