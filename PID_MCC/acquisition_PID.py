# acquisition_PID.py
# Reçoit les données Arduino via port série
# Trace et sauvegarde la courbe pour le TIPE

import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import csv
import time

# ======================================================
# CONFIGURATION — modifiez le port selon votre PC
# Windows : 'COM3', 'COM4'...
# Linux/Mac : '/dev/ttyUSB0', '/dev/ttyACM0'
# ======================================================
PORT     = 'COM11'
BAUDRATE = 9600
DUREE    = 50        # secondes d'acquisition
bleu     = '#19327A'

# ======================================================
# Connexion Arduino
# ======================================================
print(f"Connexion sur {PORT}...")
try:
    arduino = serial.Serial(PORT, BAUDRATE, timeout=1)
    time.sleep(2)  # attendre Arduino reset
    arduino.readline()  # ignorer l'en-tête CSV
    print("Connecté !")
except Exception as e:
    print(f"Erreur : {e}")
    exit()

# ======================================================
# Buffers de données
# ======================================================
temps_data    = deque()
vitesse_data  = deque()
consigne_data = deque()
pwm_data      = deque()

# ======================================================
# Acquisition en temps réel
# ======================================================
print(f"Acquisition pendant {DUREE} secondes...")
print("Appuyez Ctrl+C pour arrêter")

t_debut = time.time()

try:
    while (time.time() - t_debut) < DUREE:
        ligne = arduino.readline().decode('utf-8').strip()
        if not ligne:
            continue
        try:
            parts = ligne.split(',')
            if len(parts) == 4:
                t   = float(parts[0]) / 1000.0  # ms → s
                rpm = float(parts[1])
                ref = float(parts[2])
                pwm = float(parts[3])

                temps_data.append(t)
                vitesse_data.append(rpm)
                consigne_data.append(ref)
                pwm_data.append(pwm)

                print(f"t={t:.1f}s  RPM={rpm:.0f}  "
                      f"Ref={ref:.0f}  PWM={pwm:.0f}")
        except:
            continue

except KeyboardInterrupt:
    print("\nAcquisition arrêtée")

arduino.close()

# ======================================================
# Sauvegarde CSV
# ======================================================
with open('donnees_experience.csv', 'w',
          newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['temps_s','vitesse_RPM',
                     'consigne_RPM','PWM'])
    for row in zip(temps_data, vitesse_data,
                   consigne_data, pwm_data):
        writer.writerow(row)
print("Données sauvegardées : donnees_experience.csv")

# ======================================================
# Tracé de la courbe finale
# ======================================================
t   = list(temps_data)
rpm = list(vitesse_data)
ref = list(consigne_data)
pwm = list(pwm_data)

# Normaliser le temps depuis 0
if t:
    t0 = t[0]
    t  = [ti - t0 for ti in t]

fig, (ax1, ax2) = plt.subplots(2, 1,
                                figsize=(8, 5),
                                sharex=True)

# ---- Graphe 1 : Vitesse ----
ax1.plot(t, rpm, color=bleu,
         linewidth=2.0, label='$\\Omega$ mesurée')
ax1.plot(t, ref, color='gray',
         linewidth=1.5, linestyle='--',
         label='Consigne $\\Omega_{ref}$')
ax1.set_ylabel('Vitesse (RPM)', fontsize=11)
ax1.set_title('Asservissement MCC — Résultats expérimentaux',
              fontsize=13, fontweight='bold', color=bleu)
ax1.legend(fontsize=10)
ax1.grid(True, alpha=0.25, linestyle='--')
ax1.spines['top'].set_visible(False)
ax1.spines['right'].set_visible(False)

# ---- Graphe 2 : PWM ----
ax2.plot(t, pwm, color='orange',
         linewidth=1.8, label='Commande PWM')
ax2.set_ylabel('PWM (0--255)', fontsize=11)
ax2.set_xlabel('Temps (s)', fontsize=11)
ax2.legend(fontsize=10)
ax2.grid(True, alpha=0.25, linestyle='--')
ax2.spines['top'].set_visible(False)
ax2.spines['right'].set_visible(False)

plt.tight_layout()
plt.savefig('courbe_experience.png',
            dpi=200, bbox_inches='tight')
print("Courbe sauvegardée : courbe_experience.png")
plt.show()