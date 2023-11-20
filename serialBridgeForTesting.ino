#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX, TX

void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  // read from port 1, send to port 0:
  if (mySerial.available()) {
    String content = mySerial.readString();
    Serial.println(content);
  }

  // read from port 0, send to port 1:
  if (Serial.available()) {
    String content = Serial.readString();
    mySerial.println(content);
  }
}
