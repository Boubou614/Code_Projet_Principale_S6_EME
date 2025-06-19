#include <mbed.h>
#include <cmath>

// Initialisation Tension

AnalogIn analog_alim_tension(PB_0);
float tension_micro_lu = 0;
float tension_batterie = 0;

// Initialisation Courant
AnalogIn analog_hall_effect(PB_1);

float image_courant_lu_micro = 0;
float mesure_courant_batt = 0;
float x1 = 0;
float x2 = 0;
float y_1 = 0;
float y2 = 0;
float x = 0;
float y = 0;
float a = 0;
float b = 0;

// Définition variables CN70
InterruptIn encoder_pin(PA_7);
volatile float freq_tick = 0.0f; // fréquence impulsion CN70
volatile float vitesse = 0.0f;   // En km/h

Timer tick_timer;
Ticker t2;

volatile bool flag_controle = false;
#define T_FLAG_CONTROLE 0.1f // Affichage tous les 100 ms

// Paramètres physiques
const float diametre = 0.25f; // en mètres
const int ticks_par_tour = 5;

// Traitement du capteur CNY70
void encoder_tick()
{
    static float temps_precedent = 0.0f;

    float temps_actuel = chrono::duration<float>(tick_timer.elapsed_time()).count();
    float delta_temps = temps_actuel - temps_precedent; // Temps entre deux impulsions
    temps_precedent = temps_actuel;

    if (delta_temps > 0.0f)
    {
        freq_tick = 1.0f / delta_temps;

        // Calcul vitesse en m/s
        float circonference = M_PI * 0.25;
        vitesse = (freq_tick * (circonference / ticks_par_tour)) * 3.6; // Vitesse en km/h
    }
}

void control()
{
    flag_controle = true;
}

int main()
{
    tick_timer.start();
    t2.attach(&control, T_FLAG_CONTROLE);
    encoder_pin.rise(&encoder_tick);

    printf("Initialisation terminee.\n");

    while (true)
    {
        if (flag_controle)
        {
            // Mesure tension
            tension_micro_lu = analog_alim_tension.read() * (3.3 / 1.0);       // Conversion vers tension analogique
            tension_batterie = tension_micro_lu * (15000.0 + 1000.0) / 1000.0; // Diviseur de tension

            // Mesure du courant
            image_courant_lu_micro = analog_hall_effect.read() * (3.3 / 1.0); // Lecture ADC brut (0-4096)

            x1 = 1.63;
            y_1 = -63.63;
            x2 = 3.275;
            y2 = 60.69;
            x = image_courant_lu_micro;
            a = (y2 - y_1) / (x2 - x1);
            b = y_1 - a * x1;
            y = a * x + b;
            mesure_courant_batt = y;

            // Affichage
            printf("Freq: %3.2f Hz | Vitesse: %3.3f km/h\n", freq_tick, vitesse);
            flag_controle = false;
        }
    }
}