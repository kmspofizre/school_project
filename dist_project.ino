#include <SPI.h>
#include <MFRC522.h> // библиотека "RFID".
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long uidDec, uidDecTemp;  // для храниения номера метки в десятичном формате


bool f = false;
bool flip = false;

int MotorPins[4] = {5, 6, 7, 8};
/*Целочисленная константа, показывающая количество фаз подачи сигналов для одного шага мотора. Для полушагового режима - 8
  Для шагового - 4*/
const int OneTurnPhasesCount = 8;

/*Целочисленная константа, показывающая задержку в миллисекундах между фазами подачи сигналов мотору. Для полушагового режима - 2,
  для шагового - 3*/
const int TurnPhasesDelay = 2;

/*Целочисленная константа, показывающая количество шагов, которые должен выполнить двигатель за полный оборот на 90 град.
  Внутренний вал мотора совершает 64 шага за полный оборот, с учётом передаточного числа редуктора 64:1, то мотор должен совершать 64x64/4=1024 шагов*/
const int CountStepsOneDirection = 1024;

/*Целочисленная переменная, показывающая количество шагов, которые выполнил двигатель в одном направлении*/
int CurrentStepOneDirection = 0;

/*Целочисленная переменная, показывающая номер текущей фазы*/
int CurrentPhase = 0;

/*Целочисленная переменная, показывающая направление вращения мотора: 1 - по часовой стрелке, -1 - против*/
int TurnDirection = 1;

// Для полушагового режима

/*Массив, в котором указано какие сигналы подавать на контакты мотора в той или иной фазе. [фаза][контакт]. Контакты даются в порядке, перечисленном в массиве MotorPins - оранж., жёлт., розов., син. 0 - нет сигнала, 1 - есть сигнал*/
bool MotorTurnPhases[8][4] = {
  { 1, 0, 0, 0},
  { 1, 1, 0, 0},
  { 0, 1, 0, 0},
  { 0, 1, 1, 0},
  { 0, 0, 1, 0},
  { 0, 0, 1, 1},
  { 0, 0, 0, 1},
  { 1, 0, 0, 1}
};

/*Функция CheckLastPhase проверяет не вышел ли номер текущей фазы за пределы размера массива MotorTurnPhases, который определяется переменной OneTurnPhasesCount и не пора ли поменять направление вращения*/
void CheckLastPhase()
{
  if (CurrentPhase >= OneTurnPhasesCount)
  {
    CurrentPhase = 0;
  }
  if (CurrentPhase < 0)
  {
    CurrentPhase = (OneTurnPhasesCount - 1);
  }

  //Увеличиваем шаг на 1
  CurrentStepOneDirection++;

  //проверяем не совершил ли мотор полный оборот
  if (CurrentStepOneDirection == CountStepsOneDirection)
  {
    CurrentStepOneDirection = 0;
    TurnDirection *= -1;
    flip = true;
  }
}


void Flipping() {
  while (not flip) {
    /*подаём напряжения на контакты мотора соответственно фазе, заданной в массиве MotorTurnPhases*/
    for (int i = 0; i < 4; i++) {
      digitalWrite(MotorPins[i], ( (MotorTurnPhases[CurrentPhase][i] == 1) ? HIGH : LOW) );
    }

    //переходим к другой фазе
    CurrentPhase += TurnDirection;

    // Пауза между фазами
    delay(TurnPhasesDelay);
    CheckLastPhase();
  }
}


void setup() {
  Serial.println("Start");
  Serial.begin(9600);
  SPI.begin();  //  инициализация SPI / Init SPI bus.
  mfrc522.PCD_Init();     // инициализация MFRC522 / Init MFRC522 card.
  for (int i = 0; i < 4; i++)
    pinMode(MotorPins[i], OUTPUT);
}


void loop() {
  // Поиск новой метки
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Выбор метки
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  uidDec = 0;
  // Выдача серийного номера метки.
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    uidDecTemp = mfrc522.uid.uidByte[i];
    uidDec = uidDec * 256 + uidDecTemp;
  }
  Serial.println("Card UID: ");
  Serial.println(uidDec); // Выводим UID метки в консоль.
  if ((uidDec == 1199345248) and (not f)) {
    Serial.println("Podoshlo");
    flip = false;
    f = true;
    Flipping();
    flip = false;
    delay(5000);
    Serial.println("Back");
    Flipping();
  }
  else
    f = false;
}
