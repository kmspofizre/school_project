#include <SPI.h>
#include <MFRC522.h> // библиотека "RFID".
#define SS_PIN 10
#define RST_PIN 9
#define relay 5
MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long uidDec, uidDecTemp;  // для храниения номера метки в десятичном формате
void setup() {
  Serial.begin(9600);
  Serial.println("Waiting for card...");
  SPI.begin();  //  инициализация SPI / Init SPI bus.
  mfrc522.PCD_Init();     // инициализация MFRC522 / Init MFRC522 card.
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
}
void loop() {
  // Поиск новой метки
  Serial.println("Start");
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
  if (uidDec == 3763966293) // Сравниваем Uid метки, если он равен заданому то серва открывает.
  {
    digitalWrite(relay, HIGH);
    delay(3000); // пауза 3 сек и механизм запирается.
    digitalWrite(relay, LOW);
  }
}
