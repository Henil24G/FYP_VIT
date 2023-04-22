#include <LiquidCrystal.h>

// Initialize the LiquidCrystal object with the pin numbers for RS, EN, D4, D5, D6, D7.
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

unsigned char msg;
unsigned int dis;
char str[70];
String gpsString="";
char *test = "$ABCDD";
String latitude = "No Range      ";
String longitude = "No Range     ";

boolean gps_status = 0;
unsigned int i;

void setup() {
  // Initialize the serial ports and the LCD screen
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  lcd.begin(16, 2);

  // Set the pin modes
  pinMode(2, OUTPUT);
  pinMode(3, INPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(A8, INPUT);

  // Set the initial state
  digitalWrite(2, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  // Display a PIPE INSPECTION message on the LCD screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PIPE INSPECTION");
  lcd.setCursor(0, 1);
  lcd.print("    ROBOT");
  delay(2000);
}

void loop() {
  // Read any incoming serial data and print it to the serial monitor
  while (Serial.available()) {
    msg = Serial.read();
    Serial.println(msg);  
  }

  // Read the distance from an ultrasonic sensor and display it on the LCD screen
  dis = ultra();
  Serial.print("DIS:");
  Serial.println(dis);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DIS:");
  lcd.print(dis);
  delay(300);

  // Read the gas level from a gas sensor and display it on the LCD screen
  unsigned int g = analogRead(A8);
  Serial.print("GAS:");
  Serial.println(g);
  lcd.setCursor(0, 1);
  lcd.print("GAS:");
  lcd.print(g);
  delay(300);

  // If the gas level is above a 170 threshold, activate the GPS and send the coordinates to IOT module
  if (g >= 170) {
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    Serial.print("GAS DETECTED");
    lcd.setCursor(0, 0);
    lcd.print("GAS DETECTED");
    delay(1000);
    get_gps();
  }

  // Control the robot based on the incoming serial commands
  if (msg == 'A' || msg == 'a') {
    msg = 0;
    Serial.println("EXPAND");
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    digitalWrite(7, LOW);
    //delay(1000);
  }
 
  if (msg == 'B' || msg == 'b') {
    msg = 0;
    Serial.println("REDUCED");
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(7, HIGH);
    //delay(1000);
    //digitalWrite(6,LOW);
    //digitalWrite(7,LOW);
  }
 
  if (msg == 'F' || msg == 'f') {
    msg = 0;
    Serial.println("FORWARD");
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(4, HIGH);
    digitalWrite(5, LOW);
    delay(1000);
  }  
  if (msg == 'R' || msg == 'r') {
    msg = 0;
    Serial.println("REVERSE");
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    delay(1000);
  } 
  if (msg == 'S' || msg == 's') {
    msg = 0;
    Serial.println("STOP");
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    delay(1000);
  }

//calculates the distance of an object from the ultrasonic sensor
int getDistance() {
  long t;
  int distance, i;
  digitalWrite(2, HIGH);
  delayMicroseconds(8);
  digitalWrite(2, LOW);
  delayMicroseconds(10);
  t = pulseIn(3, HIGH);
  distance = (t * 0.034) / 2;
  return distance;
}

//read the GPS data from the GPS module
void gpsEvent() {
  gpsString = "";
  int i = 0;
  while (1) {
    while (Serial2.available() > 0) {
      char inChar = (char)Serial2.read();
      gpsString += inChar;
      i++;
      if (i < 7) {
        if (gpsString[i - 1] != test[i - 1]) {
          i = 0;
          gpsString = "";
        }
      }
      if (inChar == '\r') {
        if (i > 65) {
          gps_status = 1;
          break;
        } else {
          i = 0;
        }
      }
    }
    if (gps_status) break;
  }
}

//gets the GPS coordinates and sends them to the IoT platform
void get_gps() {
  gps_status = 0;
  int x = 0;
  while (gps_status == 0) {
    gpsEvent();
    int str_length = i;
    latitude = "";
    longitude = "";
    int comma = 0;
    while (x < str_length) {
      if (gpsString[x] == ',') {
        comma++;
      }
      if (comma == 2) {
        latitude += gpsString[x + 1];
      } else if (comma == 4) {
        longitude += gpsString[x + 1];
      }
      x++;
    }

    int l1 = latitude.length();
    latitude[l1 - 1] = ' ';
    l1 = longitude.length();
    longitude[l1 - 1] = ' ';
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lat:");
    lcd.print(latitude);
    lcd.setCursor(0, 1);
    lcd.print("Long:");
    lcd.print(longitude);
    Serial3.write('*');
    iot(latitude);
    Serial3.write(',');
    iot(longitude);
    Serial3.write('#');
    i = 0;
    x = 0;
    str_length = 0;
    delay(1000);
  }
}

// write the GPS data
void iot(String data) {
  for (int i = 0; i < data.length(); i++) {
    Serial3.write(data[i]);
  }
}

   
