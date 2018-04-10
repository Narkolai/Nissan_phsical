/*
*Скетч для отключения лентопротяжки автомагнитолы для последующего монтирования Mp3 плеера
*на аудио вход предназначеного для воспризведения кассет. В связи с установкой в штатные магнитолы имеющиее рулевое управление 
*лентопротяжки оборудуются логическим управлением для возможности управлять воспроизведением аудио кассеты с посощью электрических сигналов.
*Для Mp3 плееров имеющих IR приемник реализованна возможность переключения треков.
*Обход зашиты от обрыва пленки (проверка вращения катушек кассеты) реализован на отдельной микросхеме таймере 555.
*Автор narkolai@yandex.ru
*/

#include <IRremote.h>
//BIT1-BIT3 котакты на планке состояния лентопротяжки
int bit1 = 2; //подкл к BIT1
int bit2 = 11; //подкл к BIT2
int bit3 = 6; //подкл к BIT3 (4-я нога ардуина вышла из строя)
int typs = 5; //подкл к проверка кассеты
int mpl = 7;  //подкл к мотор управления + (если брать на входе в контролер TA7291 то питание инверсное)
int mmi = 8;  //подкл к мотор управления - (если брать на входе в контролер TA7291 то питание инверсное)
int mot = 9;  //подкл к мотор протяжки (питание инверсное)
int swt = 10; //подкл к добавленому релле для перключения режима SIDE1 и SIDE2,а также остановка промотки
const int analogInPin = 0;
float sensorValue = 0; 
float outputValue = 0;  

IRsend irsend;
void setup() {
pinMode(mpl, INPUT); //Врашение М+
pinMode(mmi, INPUT); //Врашение М-
pinMode(mot, INPUT); //Врашение ММ
pinMode(swt, OUTPUT); //SwType
//pinMode(errOff, INPUT); //Flack Check ON/OFF
Serial.begin(9600);
}
void typsc() {pinMode(typs, OUTPUT);  digitalWrite(typs,LOW);return;};//Наличие касеты
void typso() {pinMode(typs, INPUT);return;}//Откр 1 ногу
void bit1c() {pinMode(bit1, OUTPUT);  digitalWrite(bit1,LOW);return;}//закрыть 1 ногу
void bit1o() {pinMode(bit1, INPUT);return;}//Откр 1 ногу
void bit2c() {pinMode(bit2, OUTPUT);digitalWrite(bit2,LOW);return;}//закрыть 2 ногу
void bit2o() {pinMode(bit2, INPUT);return;}//Откр 2 ногу
void bit3c() {pinMode(bit3, OUTPUT); digitalWrite(bit3,LOW);return;} //закрыть 3 ногу
void bit3o() {pinMode(bit3, INPUT);return;}//Откр 3 ногу
//Функции управления Пинами, поочередное замыкаение кантактов на планке определения состаяния лентопротяжки 
void sbit(char nbit){
   switch (nbit) {
         case '0':typsc(); bit1o(); bit2o();bit3o(); break; //0 
         case '1':bit1o(); bit2o(); bit3c();typso(); break; //3
         case '2':bit1c(); bit2o(); bit3c();typso(); break; //1+3
         case '3':bit1c(); bit2o(); bit3o();typso(); break; //1
         case '4':bit1c(); bit2c(); bit3o();typso(); break; //1+2
         case '5':bit1o(); bit2c(); bit3o();typso(); break; //2
         case '6':bit1o(); bit2c(); bit3c();typso(); break; //2+3
         case '7':bit1c(); bit2c(); bit3c();typso(); break; //1+2+3
         case '8':bit1o(); bit2c(); bit3c();typso(); break; //2+3
         case '9':bit1o(); bit2c(); bit3o();typso(); break; //2
         case ':':bit1c(); bit2c(); bit3o();typso(); break; //1+2
         case ';':bit1c(); bit2o(); bit3o();typso(); break; //1
         }
}
char stat = '0';  //статус Откл
char side;
void loop() {
   outputValue = float(analogRead(analogInPin))/204,6;
  if(digitalRead(mpl)==LOW && digitalRead(mmi)==LOW && digitalRead(mot)==HIGH && (stat!='2')){ stat = '0'; sbit(stat); delay(150);}//закрыть 8 ногу(8) Эмитировать вставленную кассету + сброс к 0 при сбое
 typso();delay(50);
//___MOV +__
  if(digitalRead(mpl)==LOW && digitalRead(mmi)==HIGH &&  digitalRead(mot)==HIGH)
  {   stat++;sbit(stat);Serial.print("Stat    "); Serial.println(stat);
  }
//___MOV -___  
  if(digitalRead(mpl)==HIGH && digitalRead(mmi)==LOW && digitalRead(mot)==HIGH)
  {stat--;sbit(stat);Serial.print("Stat    "); Serial.println(stat);
  }
//__PAUSE____
   if(digitalRead(mpl)==LOW && digitalRead(mmi)==LOW && digitalRead(mot)==HIGH && stat=='2')  //PAUSE
  {Serial.print("PAUSE   ");Serial.println(stat);
    irsend.sendNEC(0xFFA25D,32); //IR FLACK OFF
    delay(1000);
    
    while(digitalRead(mpl)==LOW && digitalRead(mmi)==LOW && digitalRead(mot)==HIGH && stat=='2' && outputValue<=1.5);}
//__SIDE_1____
  if(digitalRead(mot)==LOW && stat=='9') //SIDE 1
  {
     outputValue = float(analogRead(analogInPin))/204,6;
  Serial.print("Voltage = " );                     
  Serial.println(outputValue);

    Serial.print("SIDE 1  ");Serial.println(stat); //Serial.println(voltage);
    side ='A';
    if( outputValue>=2)
    {
      irsend.sendNEC(0xFFA25D,32); //IR FLACK ON
      delay(1000);
    }
    while(digitalRead(mot)==LOW && stat=='9');
   }
//__SIDE_2_
  if(digitalRead(mot)==LOW && stat==';')  //SIDE 2
  {
    if( outputValue>=2)
    {
      irsend.sendNEC(0xFFA25D,32); //IR FLACK ON
      delay(1000);
    }
    Serial.print("SIDE 2  ");Serial.println(stat);
    side ='B'; while(digitalRead(mot)==LOW && stat==';');
  }
   //__FF1__
  if(digitalRead(mot)==LOW && stat=='4' && side == 'A')  //FF 
  {
    irsend.sendNEC(0xFFA857,32); //IR FLACK Next
    digitalWrite(swt,HIGH);    delay(50);    digitalWrite(swt,LOW);
    Serial.print("FF1     ");Serial.println(stat);  
    while(digitalRead(mot)==LOW && stat=='4' && side == 'A');
  }
   //__PRV1__
  if(digitalRead(mot)==LOW && stat=='6' && side == 'A')  //PRV 
  {
    irsend.sendNEC(0xFFE01F,32); //IR FLACK Prw
    digitalWrite(swt,HIGH);    delay(50);    digitalWrite(swt,LOW);
    Serial.print("PRV1    ");Serial.println(stat);
    while(digitalRead(mot)==LOW && stat=='6' && side == 'A');
  }
   //__FF2__
  if(digitalRead(mot)==LOW && stat=='6' && side == 'B')  //FF 
  {
    irsend.sendNEC(0xFFA857,32); //IR FLACK Next
    digitalWrite(swt,HIGH);    delay(50);    digitalWrite(swt,LOW);
    Serial.print("FF2     ");Serial.println(stat);  
    while(digitalRead(mot)==LOW && stat=='6' && side == 'B');
  }
   //__PRV2__
  if(digitalRead(mot)==LOW && stat=='4' && side == 'B')  //PRV 
  {
    irsend.sendNEC(0xFFE01F,32); //IR FLACK Prw
    digitalWrite(swt,HIGH);    delay(50);    digitalWrite(swt,LOW);
    Serial.print("PRV2    ");Serial.println(stat);
    while(digitalRead(mot)==LOW && stat=='4' && side == 'B');
  }
  //__SWICH_to_s2__
  if(digitalRead(mpl)==HIGH && digitalRead(mmi)==HIGH && digitalRead(mot)==HIGH && stat=='9' && side == 'B')  //SWICH 
  {
    irsend.sendNEC(0xFF22DD,32); //IR FLACK Mode
    delay(50);
    Serial.print("Mode>S2 ");Serial.print(side); Serial.println(stat);
  }
 //__SWICH_to_s1__
  if(digitalRead(mpl)==HIGH && digitalRead(mmi)==HIGH && digitalRead(mot)==HIGH && stat==';' && side == 'A') //SWICH 
  {
    irsend.sendNEC(0xFF22DD,32); //IR FLACK Mode
    delay(50);
    Serial.print("Mode>S1 ");Serial.print(side); Serial.println(stat);
  } 
}
