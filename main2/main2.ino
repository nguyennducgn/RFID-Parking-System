#include <Servo.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <SPI.h> 
#include <MFRC522.h>
#include <SoftwareSerial.h>

void setupExcel() {
  delay(2000);  
  Serial.println("CLEARDATA");                  
  Serial.println("Time,UID,Hành động"); 
}

SoftwareSerial espSerial(A1, A2);  

Servo myservo_vao;
Servo myservo_ra;
int servoPin_vao = 6; 
int servoPin_ra = 7;

#define ir_car1 2
#define ir_car2 3
#define ir_car3 4
#define ir_car4 5

#define buzzer 8
#define flamePin A0
#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define MAX_CARDS 20  
int IDs[MAX_CARDS][4];
bool code_IDs[MAX_CARDS];
int cardCount = 0;  

int UID[4];
int S1 = 0, S2 = 0, S3 = 0, S4 = 0;
int total;
bool fireAlert = false;
bool addNewCardMode = false;
bool showingMessage = false;

unsigned long previousMillis = 0;
const long sendInterval = 1000;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  pinMode(ir_car1, INPUT);
  pinMode(ir_car2, INPUT);
  pinMode(ir_car3, INPUT);333
  pinMode(ir_car4, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(flamePin, INPUT);
  digitalWrite(buzzer, 0);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(6, 0);
  lcd.print("CAR");
  lcd.setCursor(1, 1);
  lcd.print("DOAN3");
  delay(2000);
  lcd.clear();

  myservo_vao.attach(servoPin_vao); 
  myservo_ra.attach(servoPin_ra); 
  myservo_vao.write(90);
  myservo_ra.write(90); 

  SPI.begin();
  mfrc522.PCD_Init();

  
  int defaultIDs[4][4] = {
    {42, 163, 117, 6},
    {77, 169, 118, 6},
    {250, 30, 249, 5},
    {181, 193, 250, 5}
  };
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      IDs[i][j] = defaultIDs[i][j];
    }
    code_IDs[i] = true;
  }
  cardCount = 4;

  Serial.println("He thong san sang!");
}

int getCardIndex() {
  for (int i = 0; i < cardCount; i++) {  
    if (UID[0] == IDs[i][0] && UID[1] == IDs[i][1] && UID[2] == IDs[i][2] && UID[3] == IDs[i][3]) {
      return i;
    }
  }
  return -1;
  setupExcel();  
}

void control_servo_vao() {
  lcd.clear();
  lcd.print("Xin Chao!");
  myservo_vao.write(180);
  digitalWrite(buzzer, 1); delay(100); digitalWrite(buzzer,0);
  delay(2800);
  myservo_vao.write(90);
  lcd.clear();
}

void control_servo_ra() {
  lcd.clear();
  lcd.print("Cam On Quy Khach");
  myservo_ra.write(0);
  digitalWrite(buzzer,1); delay(100); digitalWrite(buzzer,0);
  delay(2800);
  myservo_ra.write(90);
  lcd.clear();
}

void Read_Sensor() {
  
  if (showingMessage) return;
  
  S1 = (digitalRead(ir_car1) == 0) ? 1 : 0;
  S2 = (digitalRead(ir_car2) == 0) ? 1 : 0;
  S3 = (digitalRead(ir_car3) == 0) ? 1 : 0;
  S4 = (digitalRead(ir_car4) == 0) ? 1 : 0;
  total = S1 + S2 + S3 + S4;
 
  lcd.setCursor(0, 0);
  lcd.print("VT: 1 2 3 4  ");
  lcd.setCursor(4, 1);
  lcd.print(S1 ? "x" : "0");
  lcd.setCursor(6, 1);
  lcd.print(S2 ? "x" : "0");
  lcd.setCursor(8, 1);
  lcd.print(S3 ? "x" : "0");
  lcd.setCursor(10, 1);
  lcd.print(S4 ? "x" : "0");
}

void fireEmergency() {
  if (fireAlert) return;
  fireAlert = true;
  showingMessage = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" CO CHAY ");
  lcd.setCursor(1, 1);
  lcd.print("BARRIER DA MO");

  myservo_vao.write(180);
  myservo_ra.write(0);

  while (true) {
    digitalWrite(buzzer, HIGH); delay(150);
    digitalWrite(buzzer, LOW); delay(150);
  }
}
void logToExcel(String action) {
  String uidStr = String(UID[0]) + "." + String(UID[1]) + "." + String(UID[2]) + "." + String(UID[3]);
  Serial.print("DATA,TIME,");        
  Serial.print(uidStr);
  Serial.print(",");
  Serial.println(action);
}


void loop() {
  unsigned long currentMillis = millis();

  
  if (currentMillis - previousMillis >= sendInterval) {
    previousMillis = currentMillis;
    espSerial.println("STATUS:" + String(S1) + "," + String(S2) + "," + String(S3) + "," + String(S4) + "," + String(fireAlert ? 1 : 0));
  }

  
  if (digitalRead(flamePin) == HIGH) {  
    if (!fireAlert) {  
      fireAlert = true;
      showingMessage = true;
      
     
      lcd.clear();
      lcd.print("CO CHAY");
      lcd.setCursor(0, 1);
      lcd.print("BARRIER DA MO");

      myservo_vao.write(180);
      myservo_ra.write(0);

      digitalWrite(buzzer, HIGH);  
    }
  } else {  
    if (fireAlert) {  
      fireAlert = false;
       showingMessage = false;
      digitalWrite(buzzer, LOW);  
      myservo_vao.write(90);
      myservo_ra.write(90);
      lcd.clear();  
    }
  }

 
  if (espSerial.available()) {
    String cmd = espSerial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "OPEN_VAO") {
      showingMessage = true;
      control_servo_vao();    
      showingMessage = false;
    }

    if (cmd == "OPEN_RA") {
      showingMessage = true;
      control_servo_ra();     
      showingMessage = false;
    }
    if (cmd == "ADD_CARD:ON") {
      addNewCardMode = true;
      showingMessage = true;
      lcd.clear();
      lcd.print("QUET THE MOI...");
      digitalWrite(buzzer,1); delay(200); digitalWrite(buzzer,0); delay(200);
      digitalWrite(buzzer,1); delay(200); digitalWrite(buzzer,0);
    }
    if (cmd == "ADD_CARD:OFF") {
      addNewCardMode = false;
      lcd.clear();
    }
  }

  Read_Sensor();

  // RFID - Đọc thẻ
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  for (byte i = 0; i < 4; i++) UID[i] = mfrc522.uid.uidByte[i];

  Serial.print("UID: ");
  for (byte i = 0; i < 4; i++) {
    Serial.print(UID[i]);
    Serial.print(" ");
  }
  Serial.println();

  
  if (addNewCardMode) {
    showingMessage = true;
    if (cardCount < MAX_CARDS) {
      for (int i = 0; i < 4; i++) IDs[cardCount][i] = UID[i];
      code_IDs[cardCount] = true;
      
      cardCount++;

      
      lcd.clear();
      lcd.print("DA THEM THE");
      digitalWrite(buzzer,1); delay(200); digitalWrite(buzzer,0); delay(200);
      digitalWrite(buzzer,1); delay(200); digitalWrite(buzzer,0); delay(200);
      digitalWrite(buzzer,1); delay(200); digitalWrite(buzzer,0);
      delay(2000);
      lcd.clear();
       

      espSerial.println("CARD_ADDED");  
      addNewCardMode = false;
      showingMessage = false;
    } else {
      
      lcd.clear();
      lcd.print("DAY BO NHO!");
      delay(2000);
      lcd.clear();
    }
    mfrc522.PICC_HaltA();
    return;  
  }

  
  int index = getCardIndex();
  if (index != -1) {
    if (total < 4) {
      if (code_IDs[index]) {
        control_servo_vao();
        code_IDs[index] = false;
        logToExcel("VAO");
      } else {
        control_servo_ra();
        code_IDs[index] = true;
        logToExcel("RA");
      }
    } else {
      if (code_IDs[index]) {
        lcd.clear();
        lcd.print("Da day xe!");
        digitalWrite(buzzer, 1); delay(1000); digitalWrite(buzzer, 0);
        lcd.clear();
      } else {
        control_servo_ra();
        code_IDs[index] = true;
      }
    }
  } else {
    
    lcd.clear();
    lcd.print("The Sai!");
      digitalWrite(buzzer, 1); delay(500); digitalWrite(buzzer, 0); delay(500);
    digitalWrite(buzzer, 1); delay(500); digitalWrite(buzzer, 0);
    lcd.clear();
  }

 
  mfrc522.PICC_HaltA();
  delay(500);
}
       

