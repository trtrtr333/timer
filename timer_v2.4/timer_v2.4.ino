// Версия 2.4
// добавлена запись чисел из еепром в переменные num1 и num2
// добавлен высокий уровень флага long_flag, иначе не работало отключение 1 таймера
// добавлено условие работы одного или другого атймера для возсожности отключения по длинному нажатию
// исправлено условие перехода в режим сна (контроллер не засыпал) вместо равенства кнопок "1" сделано равенство "0"
// исправлена переменная расчета времени до наступления сна с int на unsigned long

#include <EEPROM.h> //библеотека для работы с энергонезависимой памятью

// определяем мнемоники для работы с индикатором
#define DIG1 PIN_PD2 // первый разряд
#define DIG2 PIN_PD1 // второй разряд
// сегменты
#define A PIN_PC0
#define B PIN_PC1
#define C PIN_PC2
#define D PIN_PC3
#define E PIN_PC4
#define FF PIN_PC5
#define G PIN_PC7
#define BB 10 //яркость сегментов 10-макс. 0-мин.
#define T 10 //интервал морганий сегментов

// определяем мнемоники для работы с кнопками
#define bPLUS PIN_PB6   // кнопка +
#define bMINUS PIN_PB2  // кнопка -
#define bSEL PIN_PB1    // кнопка Выбор
#define bSS PIN_PB0     // кнопка Старт/стоп

// определяем мнемоники для работы со светодиодами
#define led_pin_g PIN_PA1  // зеленый светодиод
#define led_pin_r PIN_PA0  // красный светодиод

#define mos_pin PIN_PA2    // MOSFET  !! PD0 нужно изменить на PA2, иначе по истечению 2го таймера будут загораться точки на обоих разрядах !!  

//настройки пределов таймеров
#define min_t1 1   // минимальный предел первого таймера
#define max_t1 20  // максимальный предел первого таймера
#define min_t2 1   // минимальный предел второго таймера
#define max_t2 20  // максимальный предел второго таймера
#define BUTTON_READ_PERIOD 100  // период опроса кнопок

#define SLEEP_TIME 300000    // время "засыпания", 5 минут, в мс //исправлено на long v.2.4  300000
#define LONG_PRESS 5000

// инициализируем переменные
int dig1 = 0;             // переменная для хранения цифры первого разряда индикатора
int dig2 = 0;             // переменная для хранения цифры втоого разряда индикатора
int num_taimer = 1;       // переменная для хранения состояния номера таймера
int count_blink_tmr1 = 0; // счетчик миганий 1 таймера
int count_blink_tmr2 = 0; // счетчик миганий 2 таймера
int co = 0;               // счетчик миганий 

float num1 = 0;  // число 1
float num2 = 0;  // число 2

float num1_read = 0; //прочитанное из EEPROM число 1
float num2_read = 0; //прочитанное из EEPROM число 2

// флаги
bool sel_flag = 0;  // флаг кнопки выбор
bool ss_flag = 0;   // флаг кнопки старт/стоп
bool led_g_flag = 0;  // флаг зеленого светодиода
bool led_r_flag = 0;  // флаг красного светодиода
bool led_flag = 0;    // флаг светодиодов
bool sleep_flag1 = 0;  // флаг засыпания
bool block_b_flag = 1;  // флаг блокировки кнопок

bool timer1Active = 0;   // флаг включения первого таймера
bool timer2Active = 0;   // флаг включения второго таймера

bool buttonState_SS = 0;  // состояние кнопки старт/стоп
bool lastButtonState_SS = 0;  // предыдущее состояние кнопки старт/стоп
bool buttonState_SEL = 0;  // состояние кнопки выбор 
bool lastButtonState_SEL = 0;    // предыдущее состояние кнопки выбор
bool buttonState_PLUS = 0;    // состояние кнопки +
bool lastButtonState_PLUS = 0;    // предыдущее состояние кнопки +
bool buttonState_MINUS = 0;       // состояние кнопки -
bool lastButtonState_MINUS = 0;    // предыдущее состояние кнопки -

bool long_flag = 1;     //флаг длинного нажатия 

// счетчики времени
unsigned long t1 = 0;
unsigned long btnTimer = 0;
unsigned long sleepTimer = 0;
unsigned long timer1PreviousMillis = 0;
unsigned long timer2PreviousMillis = 0;
unsigned long buttonPressStartTime = 0;
unsigned long lastButtonActivityTime = 0;
unsigned long buttonReadTime = 0;
unsigned long currentMillis = 0;


void setup() {
  
lastButtonActivityTime = millis(); // Инициализация времени последней активности кнопки

// инициализация выходов
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(FF, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(DIG1, OUTPUT);
  pinMode(DIG2, OUTPUT);
  pinMode(led_pin_g, OUTPUT);
  pinMode(led_pin_r, OUTPUT);
  pinMode(mos_pin, OUTPUT);

// инициализация входов
  pinMode(bPLUS, INPUT);
  pinMode(bMINUS, INPUT);
  pinMode(bSEL, INPUT);
  pinMode(bSS, INPUT);
  
// начальная инициализация пинов
  digitalWrite(A,0); 
  digitalWrite(B,0);
  digitalWrite(C,0);
  digitalWrite(D,0);
  digitalWrite(E,0);
  digitalWrite(FF,0);
  digitalWrite(G,0);
  digitalWrite(DIG1,0);
  digitalWrite(DIG2,0);
  digitalWrite(mos_pin ,0);
}

void loop() {
  
currentMillis = millis();  // Получение текущего времени в миллисекундах

if (currentMillis - buttonReadTime > BUTTON_READ_PERIOD)  // изменено в v.2.4, считываем кнопки каждые 100 мс
  {
    buttonReadTime = currentMillis;
    buttonState_SS = digitalRead(bSS);  // Считывание состояния кнопки SS
    buttonState_PLUS = digitalRead(bPLUS);  // Считывание состояния кнопки PLUS
    buttonState_MINUS = digitalRead(bMINUS);  // Считывание состояния кнопки MINUS
    buttonState_SEL = digitalRead(bSEL);  // Считывание состояния кнопки SEL
  }

// Проверка, прошло ли определенное время (SLEEP_TIME) с последней активности кнопок
if (currentMillis - lastButtonActivityTime > SLEEP_TIME) {
    sleep_flag1 = 1;  // Установка флага сна
}

if (long_flag)
{
// Проверка, если кнопка SS нажата и она была в состоянии "нажата" (lastButtonState_SS == 0)
if (buttonState_SS == 0 && lastButtonState_SS == 0 ) 
  {
  
    // Проверка, если кнопка SS была нажата в течение longPress миллисекунд
    if ((currentMillis - buttonPressStartTime >= LONG_PRESS)&& (timer1Active || timer2Active)) 
      { 
        timer1Active = 0;  // Отключение таймера 1
        timer2Active = 0;  // Отключение таймера 2
        ss_flag = 1;  // Установка флага
      }
  } else  
      {
        buttonPressStartTime = currentMillis;  // Обновление времени начала нажатия кнопки
      }
}

// Если установлен флаг ss_flag, то выполняется следующий блок
if (ss_flag){
    long_flag = 0;
    // Если переменная co меньше 8
    if (co < 8){
        // Если прошло более 500 миллисекунд с момента последнего выполнения этого блока
        if ((currentMillis - t1) > 500){
            t1 = currentMillis;  // Обновление времени последнего выполнения блока
            led_flag = !led_flag;  // Переключение состояния флага led_flag
            digitalWrite(led_pin_g ,!led_flag);  // Переключение состояния светодиода led_pin_g
            digitalWrite(led_pin_r ,!led_flag);  // Переключение состояния светодиода led_pin_r
            co++;  // Увеличение значения переменной co
        }             
    } else {
      co = 0;  // Сброс значения переменной co
      block_b_flag = 1;  // Установка флага
      long_flag = 1;// Установка флага
      ss_flag = 0;  // Сброс флага
    }
}

if (block_b_flag) {
    // Если кнопка SS нажата (buttonState_SS == 0), и предыдущее состояние кнопки было "не нажато" (lastButtonState_SS == 1)
    // и при этом таймер 1 не активен
    if (buttonState_SS == 0 && lastButtonState_SS == 1 && !timer1Active) {
        timer1PreviousMillis = currentMillis;  // Запоминаем текущее время в переменной timer1PreviousMillis
        timer1Active = 1;  // Включаем таймер 1
        ss_flag = 0;  // Сбрасываем флаг 
        block_b_flag = 0;  // Сбрасываем флаг 
        sleep_flag1 = 0;  // Сбрасываем флаг 
        long_flag = 1;// Установка флага // добавлено в v.2.4, иначе не выключался таймер 1
    }
    // Если кнопка SEL нажата (buttonState_SEL == 0), и предыдущее состояние кнопки было "не нажато" (lastButtonState_SEL == 1)
    if (buttonState_SEL == 0 && lastButtonState_SEL == 1) {
         sel_flag = !sel_flag;  // Инвертируем флаг 
         sleep_flag1 = 0;  // Сбрасываем флаг 
    }
    // Если кнопка PLUS нажата (buttonState_PLUS == 0), и предыдущее состояние кнопки было "не нажато" (lastButtonState_PLUS == 1)
    if (buttonState_PLUS == 0 && lastButtonState_PLUS == 1) {
        sleep_flag1 = 0;  // Сбрасываем флаг 
        if (!sel_flag) {
            num1++;  // Увеличиваем значение 
            eeprom_update();  // Обновляем EEPROM с новым значением num1
        }
        if (sel_flag) {
            num2++;  // Увеличиваем значение 
            eeprom_update();  // Обновляем EEPROM с новым значением num2
        }
    }
    // Если кнопка MINUS нажата (buttonState_MINUS == 0), и предыдущее состояние кнопки было "не нажато" (lastButtonState_MINUS == 1)
    if (buttonState_MINUS == 0 && lastButtonState_MINUS == 1) {
        sleep_flag1 = 0;  // Сбрасываем флаг 
        if (!sel_flag) {
            num1--;  // Уменьшаем значение 
            eeprom_update();  // Обновляем EEPROM с новым значением num1
        }
        if (sel_flag) {
            num2--;  // Уменьшаем значение 
            eeprom_update();  // Обновляем EEPROM с новым значением num2
        }
    }
}

// Если активен таймер 1 (timer1Active)
if (timer1Active) {
    eeprom_read();  // Чтение данных из EEPROM
    // Проверка, прошло ли достаточно времени для таймера 1
    if (currentMillis - timer1PreviousMillis >= num1_read * 60000) {   //*60 000 - минуты
        timer1PreviousMillis = currentMillis;  // Сброс таймера 1
        timer2PreviousMillis = currentMillis;  // Сброс таймера 2
        timer2Active = 1;  // Включение таймера 2
        timer1Active = 0;  // Выключение таймера 1
        long_flag = 1;// Установка флага

    }
    // Периодическое моргание светодиодом led_pin_g
    if (currentMillis - t1 >= 500) {
       t1 = currentMillis;  
       led_g_flag = !led_g_flag;  
       digitalWrite(led_pin_g ,!led_g_flag);
    }    
}      

// Если активен таймер 2 (timer2Active)
if (timer2Active) {
    digitalWrite(led_pin_g ,0);
    eeprom_read();  // Чтение данных из EEPROM
    // Проверка, прошло ли достаточно времени для таймера 2
    if (currentMillis - timer2PreviousMillis >= num2_read * 60000) {   //*60 000 - минуты
        timer2PreviousMillis = currentMillis;  // Сброс таймера 2
        timer2Active = 0;  // Выключение таймера 2
        block_b_flag = 1;  // Установка флага 
        long_flag = 1;// Установка флага
        digitalWrite(mos_pin , 1);  // Установка высокого уровня на пине mos_pin
        DisplayShow();  // Вывод информации на дисплей
    }
    // Периодическое моргание светодиодом led_pin_r
    if (currentMillis - t1 >= 500) {
       t1 = currentMillis;  
       led_r_flag = !led_r_flag;  
       digitalWrite(led_pin_r ,!led_r_flag);
    }
}

if (!timer2Active && !timer1Active && !ss_flag){
  digitalWrite(led_pin_r ,0); // Выключить красный светодиод (установить его пин в LOW).
  digitalWrite(led_pin_g ,1); // Включить зеленый светодиод (установить его пин в HIGH).
}

// Если не выбран таймер (sel_flag == false)
if (!sel_flag) {
    // Если переменная count_blink_tmr1 меньше 4
    if (count_blink_tmr1 < 4) {
        // Проверка, прошло ли достаточно времени для моргания
        if (currentMillis - t1 >= 500) {
            t1 = currentMillis;
            num_taimer = 1;  // Установка значения num_taimer в 1
            count_blink_tmr1++;  // Увеличение счетчика морганий
        }
        if (!sleep_flag1) { 
            // Если счетчик морганий таймера 1 нечетный, отображаем моргающие сегменты
            if (count_blink_tmr1 % 2 != 0) {
                DisplayShowTaimer();
            }
        }
    } else {
        if (!sleep_flag1) {
            eeprom_read();  // Чтение данных из EEPROM
            DisplayMath(num1_read);  // Вычисление цифр для отображения
            DisplayShow();  // Вывод информации на дисплей
        }
    }
   count_blink_tmr2 = 0;  // Сброс счетчика морганий таймера 2
}
// Если выбран таймер (sel_flag == true)
if (sel_flag) {
    // Если переменная count_blink_tmr2 меньше 4
    if (count_blink_tmr2 < 4) {
        // Проверка, прошло ли достаточно времени для моргания
        if (currentMillis - t1 >= 500) {
            t1 = currentMillis;
            num_taimer = 2;  // Установка значения num_taimer в 2
            count_blink_tmr2++;  // Увеличение счетчика морганий
        }
        if (!sleep_flag1) {
            // Если счетчик морганий таймера 2 нечетный, отображаем моргающие сегменты
            if (count_blink_tmr2 % 2 != 0) {
                DisplayShowTaimer();
            }
        }
    } else {
        if (!sleep_flag1) {
            eeprom_read();  // Чтение данных из EEPROM
            DisplayMath(num2_read);  // Вычисление цифр для отображения
            DisplayShow();  // Вывод информации на дисплей
        }
    }
    count_blink_tmr1 = 0;  // Сброс счетчика морганий таймера 1
}

// Если кнопка SS, SEL, PLUS или MINUS нажата (хотя бы одна из них), обновляем время последнего активного взаимодействия с кнопками v.2.4
if (buttonState_SS == 0 || buttonState_SEL == 0 || buttonState_PLUS == 0 || buttonState_MINUS == 0) {
    lastButtonActivityTime = currentMillis;
}

// Сохранение состояний кнопок для следующего цикла
lastButtonState_SS = buttonState_SS;
lastButtonState_SEL = buttonState_SEL;
lastButtonState_PLUS = buttonState_PLUS;
lastButtonState_MINUS = buttonState_MINUS;
}

// Функция для записи значений num1 и num2 в EEPROM после их ограничения в заданных пределах
void eeprom_update() {
  EEPROM.put(2, num1 = constrain(num1, min_t1, max_t1));  // Записываем num1 в EEPROM, ограниченное в заданных пределах
  EEPROM.put(6, num2 = constrain(num2, min_t2, max_t2));  // Записываем num2 в EEPROM, ограниченное в заданных пределах
}

// Функция для чтения значений num1_read и num2_read из EEPROM
void eeprom_read() {
  EEPROM.get(2, num1_read);  // Чтение num1_read из EEPROM
  num1 = num1_read;
  EEPROM.get(6, num2_read);  // Чтение num2_read из EEPROM
  num2 = num2_read;
}

// Функция для отображения таймера на дисплее
void DisplayShowTaimer() {
    digitalWrite(DIG1, HIGH);  // Включаем первый разряд дисплея
    ShowTaimer(4);  // Отображаем 4 кейс на дисплее
    delay(BB);  // Пауза
    Clean();  // Очищаем дисплей
    digitalWrite(DIG1, LOW);  // Выключаем первый разряд дисплея
    delay(T - BB);  // Пауза

    digitalWrite(DIG2, HIGH);  // Включаем второй разряд дисплея
    ShowTaimer(num_taimer);  // Отображаем значение num_taimer на дисплее
    delay(BB);  // Пауза
    Clean();  // Очищаем дисплей
    digitalWrite(DIG2, LOW);  // Выключаем второй разряд дисплея
    delay(T - BB);  // Пауза
}

// Функция для отображения цифры таймера на дисплее
void ShowTaimer(int digitTaimer) {
  switch(digitTaimer) {
    case 1: {
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
    }
    break;
    case 2: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(E, HIGH);  // Включаем сегмент E
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
    case 4: {
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
  }
}

// Функция для разбиения числа на десятки и единицы и сохранения их в переменных dig1 и dig2
void DisplayMath(int data) {
  dig1 = dig2 = 0;  // Обнуляем переменные для хранения десятков и единиц
  if (data < 100) {  // Проверяем, что число меньше 100
    while (data >= 10) {  // Пока число больше или равно 10
      data -= 10;  // Вычитаем 10 из числа
      dig1++;  // Увеличиваем значение переменной для десятков
    }
    dig2 = data;  // Оставшееся число становится единицами
  }
}

// Функция для отображения чисел на семисегментном дисплее
void DisplayShow() {
  digitalWrite(DIG1, HIGH);  // Включаем первый разряд
  Show(dig1);  // Отображаем десятки
  delay(BB);  // Делаем паузу
  Clean();  // Очищаем дисплей
  digitalWrite(DIG1, LOW);  // Выключаем первый разряд
  delay(T - BB);  // Делаем паузу

  digitalWrite(DIG2, HIGH);  // Включаем второй разряд
  Show(dig2);  // Отображаем единицы
  delay(BB);  // Делаем паузу
  Clean();  // Очищаем дисплей
  digitalWrite(DIG2, LOW);  // Выключаем второй разряд
  delay(T - BB);  // Делаем паузу
}

// Функция для отображения цифры на семисегментном дисплее
void Show(int digit) {
  switch(digit) {
    case 0: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(E, HIGH);  // Включаем сегмент E
      digitalWrite(FF, HIGH);  // Включаем сегмент FF
    }
    break;
    case 1: {
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
    }
    break;
    case 2: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(E, HIGH);  // Включаем сегмент E
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
    case 3: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
    case 4: {
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
      digitalWrite(FF, HIGH);  // Включаем сегмент FF
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
    case 5: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(C, HIGH);  // Включаем сегмент C
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(FF, HIGH);  // Включаем сегмент FF
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
    case 6: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(C, HIGH);  // Включаем сегмент C
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(E, HIGH);  // Включаем сегмент E
      digitalWrite(FF, HIGH);  // Включаем сегмент FF
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
    case 7: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
    }
    break;
    case 8: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(E, HIGH);  // Включаем сегмент E
      digitalWrite(FF, HIGH);  // Включаем сегмент FF
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
    case 9: {
      digitalWrite(A, HIGH);  // Включаем сегмент A
      digitalWrite(B, HIGH);  // Включаем сегмент B
      digitalWrite(C, HIGH);  // Включаем сегмент C
      digitalWrite(D, HIGH);  // Включаем сегмент D
      digitalWrite(FF, HIGH);  // Включаем сегмент FF
      digitalWrite(G, HIGH);  // Включаем сегмент G
    }
    break;
  }
}

// Функция для очистки семисегментного дисплея, выключает все сегменты
void Clean() {
    digitalWrite(A, LOW);   // Выключаем сегмент A
    digitalWrite(B, LOW);   // Выключаем сегмент B
    digitalWrite(C, LOW);   // Выключаем сегмент C
    digitalWrite(D, LOW);   // Выключаем сегмент D
    digitalWrite(E, LOW);   // Выключаем сегмент E
    digitalWrite(FF, LOW);  // Выключаем сегмент FF
    digitalWrite(G, LOW);   // Выключаем сегмент G
}
