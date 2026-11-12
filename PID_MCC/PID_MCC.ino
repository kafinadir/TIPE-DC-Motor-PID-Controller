// ====================================================
// PID_MCC.ino — Version douce
// Consigne : 2000 RPM
// ====================================================

#define PIN_PWM  9
#define PIN_IN1  7
#define PIN_IN2  8
#define PIN_AO   A0

// ---- Seuil calibré ----
const int SEUIL = 202;

// ---- Paramètres PID adoucis ----
float Kp = 0.05;   // très réduit
float Ki = 0.002;  // très réduit
float Kd = 0.001;  // très réduit

// ---- Consigne ----
float consigne_RPM = 2000.0;

// ---- Variables PID ----
float erreur      = 0;
float erreur_prev = 0;
float integrale   = 0;
float derivee     = 0;
float commande    = 80;  // PWM de départ non nul

// ---- Mesure vitesse ----
volatile int  compteur    = 0;
float         vitesse_RPM = 0;
bool          etat_prev   = false;

// ---- Temps ----
const float   Te        = 0.1;    // 100ms — plus lent
unsigned long temps_PID = 0;
unsigned long temps_RPM = 0;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_PWM, OUTPUT);

  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);

  // Démarrage doux — PWM initial
  analogWrite(PIN_PWM, 80);

  temps_PID = millis();
  temps_RPM = millis();

  Serial.println("temps_ms,vitesse_RPM,consigne_RPM,PWM");
}

void loop() {

  unsigned long maintenant = millis();

  // ---- Détection impulsion AO ----
  int  valAO = analogRead(PIN_AO);
  bool etat  = (valAO > SEUIL);

  if (etat && !etat_prev) {
    compteur++;
  }
  etat_prev = etat;

  // ---- Calcul vitesse toutes les 200ms ----
  if (maintenant - temps_RPM >= 200) {
    vitesse_RPM = (compteur / 2.0) * 300.0;
    compteur    = 0;
    temps_RPM   = maintenant;
  }

  // ---- PID toutes les 100ms ----
  if (maintenant - temps_PID >=
      (unsigned long)(Te * 1000)) {

    erreur    = consigne_RPM - vitesse_RPM;

    // Intégrale avec anti-windup
    integrale = integrale + erreur * Te;
    integrale = constrain(integrale, -2000, 2000);

    // Dérivée
    derivee   = (erreur - erreur_prev) / Te;

    // Commande PID — incrémentale
    float delta = Kp * erreur
                + Ki * integrale
                + Kd * derivee;

    // On incrémente doucement — max ±3 par cycle
    delta    = constrain(delta, -1, 1);
    commande = commande + delta;
    commande = constrain(commande, 0, 255);

    analogWrite(PIN_PWM, (int)commande);

    erreur_prev = erreur;
    temps_PID   = maintenant;

    // Envoi données PC
    Serial.print(maintenant);
    Serial.print(",");
    Serial.print(vitesse_RPM);
    Serial.print(",");
    Serial.print(consigne_RPM);
    Serial.print(",");
    Serial.println((int)commande);
  }
}