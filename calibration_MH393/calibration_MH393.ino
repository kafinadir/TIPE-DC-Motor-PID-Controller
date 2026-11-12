// calibration_MH393.ino
// Branchez AO du MH-393 sur A0
// Le moteur doit tourner pendant la calibration

#define PIN_PWM  9
#define PIN_IN1  7
#define PIN_IN2  8
#define PIN_AO   A0

int valeur  = 0;
int val_min = 1023;
int val_max = 0;

unsigned long temps_debut = 0;
const int DUREE = 10000;  // 10 secondes
bool termine = false;

void setup() {
  Serial.begin(9600);

  // Faire tourner le moteur
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_PWM, OUTPUT);
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_PWM, 150);

  Serial.println("================================");
  Serial.println("Calibration MH-393 — 10 secondes");
  Serial.println("================================");

  temps_debut = millis();
}

void loop() {

  if (!termine) {

    valeur = analogRead(PIN_AO);

    if (valeur < val_min) val_min = valeur;
    if (valeur > val_max) val_max = valeur;

    Serial.print("AO=");
    Serial.print(valeur);
    Serial.print("  min=");
    Serial.print(val_min);
    Serial.print("  max=");
    Serial.println(val_max);

    delay(5);

    if (millis() - temps_debut >= DUREE) {
      int seuil = (val_min + val_max) / 2;

      Serial.println();
      Serial.println("================================");
      Serial.println("RESULTAT :");
      Serial.print("  min   = "); Serial.println(val_min);
      Serial.print("  max   = "); Serial.println(val_max);
      Serial.print("  SEUIL = "); Serial.println(seuil);
      Serial.println("================================");
      Serial.println("Notez le SEUIL pour le code PID");

      // Arrêter le moteur
      analogWrite(PIN_PWM, 0);

      termine = true;
    }
  }
}