// Connect Relay VCC pin(+) to Arduino 5V pin
// Connect Relay GND pin(-) to Arduino GND pin
// Connect Relay signal pin(S) to Arduino Digital pin [7]
// Connect power supply 6V (+) to the pump motor red wire(+) and Valve red wire(+)
// Connect power supply 6V (-) to the pump motor black wire(-)
// Connect power supply 6V (-) to relay pin(com)
// Connect Valve black wire (-) to relay pin (NO)


// Definisi pin Arduino
const int relayPin = 7;

void setup() {
  // Atur pin sebagai OUTPUT
  pinMode(relayPin, OUTPUT);

  // Inisialisasi Serial Monitor
  Serial.begin(9600);
}

void loop() {
  // Buka valve dan nyalakan pompa (relay OFF)
  turnOnPumpAndOpenValve();
  delay(5000); // Tunggu 5 detik

  // Tutup valve dan matikan pompa (relay ON)
  turnOffPumpAndCloseValve();
  delay(5000); // Tunggu 5 detik
}

void turnOnPumpAndOpenValve() {
  digitalWrite(relayPin, LOW); // Relay OFF
  Serial.println("Valve terbuka, Pompa dinyalakan");
}

void turnOffPumpAndCloseValve() {
  digitalWrite(relayPin, HIGH); // Relay ON
  Serial.println("Valve tertutup, Pompa dimatikan");
}
