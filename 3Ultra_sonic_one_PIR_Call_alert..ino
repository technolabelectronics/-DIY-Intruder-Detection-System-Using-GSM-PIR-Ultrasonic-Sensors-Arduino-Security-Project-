#include <SoftwareSerial.h>

// -----------------------------
// GSM Configuration
// -----------------------------
const int GSM_RX_PIN = 9;   // Arduino RX  (connect to SIM800L TX)
const int GSM_TX_PIN = 10;  // Arduino TX  (connect to SIM800L RX)
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
const int pirPin = 8; // PIR output goes HIGH on motion

// -----------------------------
// Distance Threshold
// -----------------------------
long distanceThreshold = 30; // in centimeters

// -----------------------------
// Phone Number to Call
// -----------------------------
String alertPhoneNumber = "+918543053029"; // Use full international format

// -----------------------------
// SETUP
// -----------------------------
void setup() {
  Serial.begin(9600);      // Debug serial
  gsmSerial.begin(9600);   // SIM800L baud (ensure SIM800L is set to 9600 or use auto-baud)

  // Ultrasonic pins
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  // PIR pin
  pinMode(pirPin, INPUT);

  // Initialize GSM
  initGSMModule();

  Serial.println("Setup complete. Beginning main loop...");
}

// -----------------------------
// Main Loop
// -----------------------------
void loop() {
  // Read distances
  long distance1 = getDistanceCM(trigPin1, echoPin1);
  long distance2 = getDistanceCM(trigPin2, echoPin2);
  long distance3 = getDistanceCM(trigPin3, echoPin3);

  // Read PIR
  int pirState = digitalRead(pirPin);

  // Debug prints
  Serial.print("Sensor1: "); Serial.print(distance1); Serial.print(" cm, ");
  Serial.print("Sensor2: "); Serial.print(distance2); Serial.print(" cm, ");
  Serial.print("Sensor3: "); Serial.print(distance3); Serial.print(" cm, ");
  Serial.print("PIR: "); Serial.println(pirState);

  // Check triggers
  bool ultrasonicTriggered = (
       (distance1 > 0 && distance1 < distanceThreshold) ||
       (distance2 > 0 && distance2 < distanceThreshold) ||
       (distance3 > 0 && distance3 < distanceThreshold)
  );

  bool pirTriggered = (pirState == HIGH);

  // If either ultrasonic or PIR triggered
  if (ultrasonicTriggered || pirTriggered) {
    // Make a call
    callNumber(alertPhoneNumber);

    // Wait to avoid repeated calls
    delay(5000);
  }

  // Small delay between checks
  delay(1000);
}

// -----------------------------
// Initialize GSM Module
// -----------------------------
void initGSMModule() {
  delay(1000);

  // Basic AT check
  gsmSerial.println("AT");
  delay(1000);

  // Optional: enable caller ID display
  gsmSerial.println("AT+CLIP=1");
  delay(1000);

  // The module is ready to dial calls without needing to set SMS mode
  Serial.println("GSM Module initialized for calling.");
}

// -----------------------------
// Make a Phone Call
// -----------------------------
void callNumber(String phoneNumber) {
  Serial.println("Dialing " + phoneNumber);

  // "ATD" command with a semicolon for voice call
  gsmSerial.print("ATD");
  gsmSerial.print(phoneNumber);
  gsmSerial.println(";");

  // Wait some time to let it ring (e.g., 10 seconds)
  delay(10000);

  // Hang up
  gsmSerial.println("ATH");
  delay(1000);

  Serial.println("Call ended.");
}

// -----------------------------
// Get Distance from Ultrasonic
// -----------------------------
long getDistanceCM(int trigPin, int echoPin) {
  // Clear trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Trigger the sensor for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo pulse
  long duration = pulseIn(echoPin, HIGH);

  // Convert to cm (speed of sound ~34300 cm/s)
  long distanceCm = duration * 0.034 / 2;
  return distanceCm;
}
