#include <SoftwareSerial.h>

// -----------------------------
// GSM Configuration
// -----------------------------
const int GSM_RX_PIN = 9;   // Arduino RX (connect to SIM800L TX)
const int GSM_TX_PIN = 10;  // Arduino TX (connect to SIM800L RX)
SoftwareSerial gsmSerial(GSM_RX_PIN, GSM_TX_PIN);

// -----------------------------
// Ultrasonic Sensor Pins
// -----------------------------
const int trigPin1 = 2;
const int echoPin1 = 3;

const int trigPin2 = 4;
const int echoPin2 = 5;

const int trigPin3 = 6;
const int echoPin3 = 7;

// -----------------------------
// PIR Sensor Pin
// -----------------------------
const int pirPin = 8;  // Digital input from PIR sensor

// -----------------------------
// Distance Threshold for Ultrasonic
// -----------------------------
long distanceThreshold = 30;  // cm

// -----------------------------
// Phone Number for Alerts
// -----------------------------
String alertPhoneNumber = "+918543053029"; // Use correct format with '+'

// -----------------------------
// SETUP
// -----------------------------
void setup() {
  Serial.begin(9600);      // Debugging serial
  gsmSerial.begin(9600);   // GSM Serial (make sure SIM800L is set to same baud)

  // Set up ultrasonic pins
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  // Set up PIR pin
  pinMode(pirPin, INPUT);  // Typical PIR output goes HIGH on detection

  // Initialize GSM
  initGSMModule();

  Serial.println("Setup complete. Beginning main loop...");
}

// -----------------------------
// Main Loop
// -----------------------------
void loop() {
  // Read ultrasonic distances
  long distance1 = getDistanceCM(trigPin1, echoPin1);
  long distance2 = getDistanceCM(trigPin2, echoPin2);
  long distance3 = getDistanceCM(trigPin3, echoPin3);

  // Read PIR sensor
  int pirState = digitalRead(pirPin);

  // Debug prints
  Serial.print("Sensor1: "); Serial.print(distance1); Serial.print(" cm, ");
  Serial.print("Sensor2: "); Serial.print(distance2); Serial.print(" cm, ");
  Serial.print("Sensor3: "); Serial.print(distance3); Serial.print(" cm, ");
  Serial.print("PIR: "); Serial.println(pirState);

  // Check if any ultrasonic sensor detects object < threshold
  bool ultrasonicTriggered = (
    (distance1 > 0 && distance1 < distanceThreshold) ||
    (distance2 > 0 && distance2 < distanceThreshold) ||
    (distance3 > 0 && distance3 < distanceThreshold)
  );

  // Check if PIR sensor is triggered
  bool pirTriggered = (pirState == HIGH);

  // If either ultrasonic or PIR triggered
  if (ultrasonicTriggered || pirTriggered) {
    // Send an SMS alert
    sendSMS(alertPhoneNumber, "ALERT! Intruder Detected via Ultrasonic/PIR!");

    // Wait some seconds to avoid spamming multiple SMS
    delay(5000);
  }

  // Small delay before next reading
  delay(1000);
}

// -----------------------------
// Initialize the GSM module
// -----------------------------
void initGSMModule() {
  delay(1000);

  // Basic "AT" check
  gsmSerial.println("AT");
  delay(1000);

  // Optional: enable caller ID
  gsmSerial.println("AT+CLIP=1");
  delay(1000);

  // Set SMS to text mode
  gsmSerial.println("AT+CMGF=1");
  delay(1000);

  // Optional: delete all old SMS
  // gsmSerial.println("AT+CMGD=1,4");
  // delay(1000);

  Serial.println("GSM Module initialized.");
}

// -----------------------------
// Send SMS Function
// -----------------------------
void sendSMS(String number, String message) {
  Serial.println("Sending SMS to " + number + ": " + message);

  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(number);
  gsmSerial.println("\"");
  delay(1000);  // Wait for '>' prompt

  gsmSerial.print(message);
  delay(500);

  // Ctrl+Z to finalize the SMS
  gsmSerial.write(26);
  delay(2000); // wait for SMS to send

  Serial.println("SMS sent (or attempted)...");
}

// -----------------------------
// Get Distance from Ultrasonic
// -----------------------------
long getDistanceCM(int trigPin, int echoPin) {
  // Clear trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Trigger the sensor (HIGH for 10 microseconds)
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo pulse width (microseconds)
  long duration = pulseIn(echoPin, HIGH);

  // Convert to distance (cm):
  // Speed of sound ~34300 cm/s => distance = (duration * 0.034)/2
  long distanceCm = duration * 0.034 / 2;
  return distanceCm;
}
