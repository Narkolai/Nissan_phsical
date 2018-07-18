//код подключения магнитолы JVC к РЕЗЕТИВНЫМ рулевым кнопкам Nissan (X-trail)
//Оптрон от старой мыши с резистором на фотодиод ивыход ~20кОм
//оптрон подключается между 8pin цыфровым и входом в магнитолу
//между +5в ардуина и 5, 6 добавленны еще два сопративления 1,5кОм для адаптации 
//под омметр нужного диапазона сопративлений кнопок на руле
//который выступает в качестве определения нажатой кнопки
// вся информация взята из сети hттps://ввв.avforums.com/threads/jvc-stalk-adapter-diy.248455/ 
//а, также hттps://pastebin.com/wqzggjUd


#define VOLUP       0x04 //адресс ir для jvc
#define VOLDOWN     0x05 //адресс ir для jvc
#define SOURCE      0x08 //адресс ir для jvc
#define EQUALIZER   0x0D //адресс ir для jvc
#define MUTE        0x0E
#define TRACKFORW   0x12
#define TRACKBACK   0x13
#define FOLDERFORW  0x14
#define FOLDERBACK  0x15
#define LEDPIN     13 // Светодиод

#define PULSEWIDTH 538 //я подбирал под оптопару
#define OUTPUTPIN   8 // D8 вход с оптрона
#define REMA    5 // A5 Анлоговый вход для считывания напряжения (с делителя напряжения)
#define REMB    6 // A6 Анлоговый вход для считывания напряжения (с делителя напряжения)
#define ADDRESS 0x47
#define HOLDTIME 400 //Назначяем время, по прошествию которого кнопка считается зажатой
#define ALLDOWN  1023 //Все кнопки отпущены
#define DELTA    10   // Погрешность в измерении приходящего сигнала, если показания скачут более чем на 5 едениц, то можно увеличить

//зависят от сопративлений между +5в и 5,6м аналоговым входом (между стендовыми значениями 
//возникла разница значений, после установки, из-за разнице в длине магистрали от кнопок к адаптеру)
#define KVUP 315 //315 - значение получено при отладке с нажатой кнопки
#define KVDW 315
#define KNTR 104
#define KPTR 104
#define KSOR 0
#define KMUT 0



boolean flag;  //Флаг, что ранее зажатая кнопка работает с повтором
float analogA = 0; // для чтения аналогового сигнала A
float analogB = 0; // для чтения аналогового сигнала B
unsigned long time;// переменная для хранения времени

void setup() {
 // Serial.begin(9600); // Подготовка Serial Monitor для вывода информации (Для Отладки)
  pinMode(OUTPUTPIN, OUTPUT);    // Set the proper pin as output
  digitalWrite(OUTPUTPIN, LOW);  // Output LOW to make sure optocoupler is off
  pinMode(LEDPIN, OUTPUT);                  // Set pin connected to on-board LED as output...
  digitalWrite(LEDPIN, LOW);                // ...and turn LED off
  for (unsigned char i = 0; i <= 7; i++) {  // Flash on-board LED a few times so it's easy to see when the Arduino is ready
    delay(100);
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  }
  delay(100);
  digitalWrite(LEDPIN, LOW);                // Make sure LED ends up being off

}

unsigned char GetInput(void) {
  analogA = analogRead(REMA);
  analogB = analogRead(REMB);

  //----------------------KEYGROOP_A-------------------------------------------------------
  if ((abs(ALLDOWN - analogA) < DELTA) && (abs(ALLDOWN - analogB) < DELTA)) // для того, чтобы отпущенная кнопка снова обрабатывалась скидываем time на ноль
  {
    time = 0;
    flag = false;
  }
  //Читаем значение кнопки, если нажата, и до этого нажата не была (time равен нулю)
  if (abs(ALLDOWN - analogA) > DELTA && time == 0)
  {
    time = millis();  // отмечаем время нажатия записью в переменную time
    do
    {
      delay(50);  //ждем, набираем holdtime
    }
    while (abs(ALLDOWN - analogRead(REMA)) > DELTA && millis() - time <= HOLDTIME); // пока что-то нажато и время нажатия не превысило holdtime

    if (millis() < time + HOLDTIME && flag == false) //если время удержания меньше чем holdtime, шлем сигналы одиночного нажатия
      //flag нужен, чтобы отпущенная, ранее зажатая кнопка с повтором не обработалась как однократно нажатая
    {
      if (abs(analogA - KVUP) < DELTA) //241 кнопка вверх на руле VOL+
      {
        // return VOLUP;
      //  delay(100);
        return VOLUP;
      }
      if (abs(analogA - KNTR) < DELTA) //146 кнопка < на руле NEXT
      {
        //return TRACKFORW;
       // delay(100);
        // flag = false;
        return TRACKFORW;
      }
      if (abs(analogA - KSOR) < DELTA) //91 кнопка > на руле KSOR
      {
        //return SOURCE;
       // delay(50);
        return SOURCE;
      }
    }
    //-----------------------удерживание_A---------------------------
    else if (millis() >= time + HOLDTIME) // в другом случае, когда кнопка удерживалась, шлем сигналы удержания
    {
      if (abs(analogA - KNTR) < DELTA) //146 кнопка < на руле NEXT
      {
       // flag = true;
        return FOLDERFORW;
      }
      if (abs(analogA - KVUP) < DELTA) //241 кнопка вверх на руле VOL+
      {
        
        time = 0; //зажатые кнопки громкости повторяются
        flag = true; //Признак, что кнопка может повторяться
        return VOLUP;
      }

      if (abs(analogA - KSOR) < DELTA) //91 кнопка > на руле KSOR
      {
        //flag = true;
        return SOURCE;
      }
    }

  }

  //------------------------KEYGROOP_B------------------------------------------------------
  //Читаем значение кнопки, если нажата, и до этого нажата не была (time равен нулю)
  if (abs(ALLDOWN - analogB) > DELTA && time == 0)
  {
    time = millis();  // отмечаем время нажатия записью в переменную time
    do
    {
      delay(50);  //ждем, набираем holdtime
    }
    while (abs(ALLDOWN - analogRead(REMB)) > DELTA && millis() - time <= HOLDTIME); // пока что-то нажато и время нажатия не превысило holdtime

    if (millis() < time + HOLDTIME && flag == false) //если время удержания меньше чем holdtime, шлем сигналы одиночного нажатия
      //flag нужен, чтобы отпущенная, ранее зажатая кнопка с повтором не обработалась как однократно нажатая
    {
      if (abs(analogB - KVDW) < DELTA) //241 кнопка вверх на руле VOL+
      {
        return VOLDOWN;
      }
      if (abs(analogB - KPTR) < DELTA) //146 кнопка < на руле NEXT
      {
        return TRACKBACK;
      }
      if (abs(analogB - KMUT) < DELTA) //91 кнопка > на руле KSOR
      {
        return MUTE;
      }
    }
    //-----------------------удерживание_B---------------------------
    else if (millis() >= time + HOLDTIME) // в другом случае, когда кнопка удерживалась, шлем сигналы удержания
    {
      if (abs(analogB - KVDW) < DELTA) //241 кнопка вверх на руле VOL+
      {
        time = 0; //зажатые кнопки громкости повторяются
        flag = true; //Признак, что кнопка может повторяться
        return VOLDOWN;
      }
      if (abs(analogB - KPTR) < DELTA) //146 кнопка < на руле NEXT
      {
        return FOLDERBACK;
      }
      if (abs(analogB - KMUT) < DELTA) //91 кнопка > на руле KSOR
      {
        return EQUALIZER;
      }
    }
  }
  return 0;
}
//---------------------------------------------------------------------------------------------------
void loop() {

  unsigned char Key = GetInput();  // If any buttons are being pressed the GetInput() function will return the appropriate command code

  if (Key) {  // If no buttons are being pressed the function will have returned 0 and no command will be sent
    SendCommand(Key);
  }
 // delay(100); (Для Отладки)
 //   Serial.print("KEY_A ");
 //   Serial.println(analogA); //Раскомментировать для того, чтобы видеть текущие значения сопративлений, (Для Отладки)
 //   Serial.print("KEY_B ");
 //   Serial.println(analogB); //Раскомментировать для того, чтобы видеть текущие значения сопративлений, (Для Отладки)
}

// Send a value (7 bits, LSB is sent first, value can be an address or command)
void SendValue(unsigned char value) {
  unsigned char i, tmp = 1;
  for (i = 0; i < sizeof(value) * 8 - 1; i++) {
    if (value & tmp)  // Do a bitwise AND on the value and tmp
      SendOne();
    else
      SendZero();
    tmp = tmp << 1; // Bitshift left by 1
  }
}

// Send a command to the radio, including the header, start bit, address and stop bits
void SendCommand(unsigned char value) {
  unsigned char i;
  Preamble();                         // Send signals to precede a command to the radio
  for (i = 0; i < 3; i++) {           // Repeat address, command and stop bits three times so radio will pick them up properly
    SendValue(ADDRESS);               // Send the address
    SendValue((unsigned char)value);  // Send the command
    Postamble();                      // Send signals to follow a command to the radio

  //  Serial.print("SendCommand "); //инфо о срабатывании адресса (Для Отладки)
 //   Serial.println(value, HEX); //инфо о срабатывании адресса (Для Отладки)
  }
}

// Signals to transmit a '0' bit
void SendZero() {
  digitalWrite(OUTPUTPIN, HIGH);      // Output HIGH for 1 pulse width
  digitalWrite(LEDPIN, HIGH);         // Turn on on-board LED
  delayMicroseconds(PULSEWIDTH);
  digitalWrite(OUTPUTPIN, LOW);       // Output LOW for 1 pulse width
  digitalWrite(LEDPIN, LOW);          // Turn off on-board LED
  delayMicroseconds(PULSEWIDTH);
}

// Signals to transmit a '1' bit
void SendOne() {
  digitalWrite(OUTPUTPIN, HIGH);      // Output HIGH for 1 pulse width
  digitalWrite(LEDPIN, HIGH);         // Turn on on-board LED
  delayMicroseconds(PULSEWIDTH);
  digitalWrite(OUTPUTPIN, LOW);       // Output LOW for 3 pulse widths
  digitalWrite(LEDPIN, LOW);          // Turn off on-board LED
  delayMicroseconds(PULSEWIDTH * 3);

}

// Signals to precede a command to the radio
void Preamble() {
  // HEADER: always LOW (1 pulse width), HIGH (16 pulse widths), LOW (8 pulse widths)
  digitalWrite(OUTPUTPIN, LOW);       // Make sure output is LOW for 1 pulse width, so the header starts with a rising edge
  digitalWrite(LEDPIN, LOW);          // Turn off on-board LED
  delayMicroseconds(PULSEWIDTH * 1);
  digitalWrite(OUTPUTPIN, HIGH);      // Start of header, output HIGH for 16 pulse widths
  digitalWrite(LEDPIN, HIGH);         // Turn on on-board LED
  delayMicroseconds(PULSEWIDTH * 16);
  digitalWrite(OUTPUTPIN, LOW);       // Second part of header, output LOW 8 pulse widths
  digitalWrite(LEDPIN, LOW);          // Turn off on-board LED
  delayMicroseconds(PULSEWIDTH * 8);

  // START BIT: always 1
  SendOne();

}

// Signals to follow a command to the radio
void Postamble() {
  // STOP BITS: always 1
  SendOne();
  SendOne();
}

