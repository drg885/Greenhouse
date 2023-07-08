#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcdf(0x27,16,2);
DHT dht(DHTPIN, DHTTYPE);
void Dht_begin(){
	dht.begin();
}
byte cels[]{
  B10110,
  B01001,
  B01000,
  B01000,
  B01000,
  B01001,
  B00110};
byte persent[]{
  B11100,
  B11001,
  B10010,
  B00100,
  B01001,
  B10011,
  B00111,
  B00000};
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
int dht_temp(){
	return dht.readTemperature();
}
int dht_hum(){
	return dht.readHumidity();
}
bool day = 0; //1 - day / 0 - night

bool door = 1;  //1 - open
bool windows = 1;  //0 - close

void windows_open(bool dr){
  int timen = millis();
  while(millis() - timen < TIME_TO_OC && digitalRead(SWITCH_OPEN) == 1){
  if(dr == 1){
	if(millis() - timen < TIME_TO_OC && digitalRead(SWITCH_OPEN) != 0){
		digitalWrite(WINDOWo, HIGH);
	}
 if(digitalRead(SWITCH_OPEN) == 0 || millis() - timen >= TIME_TO_OC){
		digitalWrite(WINDOWo, LOW);
   windows = 1;
	}
  }
  if(dr == 0){
  if(millis() - timen < TIME_TO_OC && digitalRead(SWITCH_OPEN) != 0){
    digitalWrite(WINDOWo, LOW);
  }
  if(digitalRead(SWITCH_OPEN) == 0 || millis() - timen >= TIME_TO_OC){
    digitalWrite(WINDOWo, HIGH);
    windows = 1;
  }
  }
  }
}
void windows_close(bool dr){
  lcdf.clear();
  int timen = millis();
  unsigned long t1 = millis();
  byte stat = 0;
  while(digitalRead(SWITCH_CLOSE) == 1 && millis() - timen < TIME_TO_OC){
    lcdf.setCursor(0,0);
    lcdf.print("Windows closing");
    lcdf.setCursor(7,1);
    if(millis() - t1 > 200){
      t1 = millis();
      if(stat < 3){stat++;}else{stat = 0;}
      }
    switch(stat){
      case 0:
        lcdf.write(5);
        break;
      case 1:
        lcdf.write(6);
        break;
      case 2:
        lcdf.write(7);
        break;
      case 3:
        lcdf.write(8);
        break;
      }
  if(dr == 1){
	if(millis() - timen < TIME_TO_OC && digitalRead(SWITCH_CLOSE) != 0){
		digitalWrite(WINDOWc, HIGH);
	}
  if(digitalRead(SWITCH_CLOSE) == 0 || millis() - timen >= TIME_TO_OC){
		digitalWrite(WINDOWc, LOW);
	  windows = 0;
  }
  }
if(dr == 0){
  if(millis() - timen < TIME_TO_OC && digitalRead(SWITCH_CLOSE) != 0){
    digitalWrite(WINDOWc, LOW);
  }
  if(digitalRead(SWITCH_CLOSE) == 0 || millis() - timen >= TIME_TO_OC){
    digitalWrite(WINDOWc, HIGH);
    windows = 0;

  }
}
}
}
void door_close(bool dr){
  int timen = millis();
  while(millis() - timen < TIME_DOOR_OC){
  if(dr == 1){
	if(millis() - timen < TIME_DOOR_OC){
		digitalWrite(DOORc, HIGH);
	}else{
		digitalWrite(DOORc, LOW);
   door = 0;
	}
  
  if(dr == 0){
  if(millis() - timen < TIME_DOOR_OC){
    digitalWrite(DOORc, LOW);
  }else{
    digitalWrite(DOORc, HIGH);
    door = 0;
  }
}
}
}
}
void door_open(bool dr){
  int timen = millis();
  while(millis() - timen < TIME_DOOR_OC){
  if(dr == 1){
	if(millis() - timen < TIME_DOOR_OC){
		digitalWrite(DOORo, HIGH);
    
	}else{
		digitalWrite(DOORo, LOW);
   door = 1;
	}
    if(dr == 0){
  if(millis() - timen < TIME_DOOR_OC){
    digitalWrite(DOORo, LOW);
  }else{
    digitalWrite(DOORo, HIGH);
    door = 1;
  }

}
}
}
}
void work_temp(int setting_temp, bool dr){
  int temp = dht_temp();
  #if (TEMP_MODE == 1)
    if(temp >= setting_temp + 10){ 
      if(temp != setting_temp || temp > setting_temp){
        if(digitalRead(SWITCH_OPEN) == 0){}else{
        windows_open(dr);
        }
        door_open(dr);
      }
    }
    else if(temp >= setting_temp + 3){
      if(temp != setting_temp || temp > setting_temp){
        if(digitalRead(SWITCH_OPEN) == 0){}else{
        windows_open(dr); 
      }
    }
    else{
      if(digitalRead(SWITCH_CLOSE) == 0){}else{     
      windows_close(dr);
      }
      door_close(dr);
    }
  #elif (TEMP_MODE == 2)
    if(temp >= setting_temp + 10){ 
      if(temp != setting_temp || temp > setting_temp - 3){
       if(digitalRead(SWITCH_OPEN) == 0){}else{
        windows_open(dr);
       }
        door_open(dr);
      }
    }
    else if(temp >= setting_temp){
      if(temp != setting_temp || temp > setting_temp - 3){
        if(digitalRead(SWITCH_OPEN) == 0){}else{
        windows_open(dr);
      }
    }
    }
    else{
      if(digitalRead(SWITCH_CLOSE) == 0){}else{
      windows_close(dr);
      }
      door_close(dr);
    }
  #endif
  }
}
void day_fase(int ph){
  int val = analogRead(PHOTORESISTOR);
  val = map(val,0,1023,0,99);
  #if(PHmode == 1)
  if(val < ph){
    day = 1;
    }
   else{
    day = 0;
    }
  #elif(PHmode == 2)
    if(val > ph){
    day = 1;
    }
   else{
    day = 0;
    }
  #endif
}
 
  void temp_work(bool dr, int temp){
    if(day == 0){
      if(digitalRead(SWITCH_OPEN) == 0 || windows == 1){windows_close(dr);}
      if(door == 1){
      door_close(dr);
      }
    }
     if(day == 1){
      work_temp(temp,dr);
      }
    }
bool dayt(){
  return day;
  }
 void nulltopin(bool dr){
    if(dr == 1){
      digitalWrite(DOORc, LOW);
      digitalWrite(DOORo, LOW);
      digitalWrite(WINDOWo, LOW);
      digitalWrite(WINDOWc, LOW);
      }
      if(dr == 0){
      digitalWrite(DOORc, HIGH);
      digitalWrite(DOORo, HIGH);
      digitalWrite(WINDOWo, HIGH);
      digitalWrite(WINDOWc, HIGH);
        }
    }
bool wind(){
  return windows;
  }
bool doors(){return door;}
void lcdf(){
    lcdf.init();
    lcdf.createChar(3, cels);  //celsium symbol
    lcdf.createChar(4, persent); //persent symbol
    lcdf.createChar(5,load25);
    lcdf.createChar(6,load50);
    lcdf.createChar(7,load75);
    lcdf.createChar(8,load100);
  }
  
