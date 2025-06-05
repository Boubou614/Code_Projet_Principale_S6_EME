#include <mbed.h>

// Initialisation des PIN

// Initialisation Tension

AnalogIn analog_alim_tension(PB_0);
float tension_micro_lu = 0;
float tension_batterie = 0;

// Initialisation Courant
AnalogIn analog_hall_effect(PB_1);
float image_courant_lu_micro = 0;
float mesure_courant_batt = 0;

// Initialisation CNY70
InterruptIn encoder_pin(PA_7);
volatile int compteur_tick = 0;
float distance_m = 0;
#define SIZE_WHEEL_DIAMETER_m 0.3
#define TIC_PER_TOUR 1
Ticker t1;
Ticker t2;
Ticker t_vitesse;

// Variables courant
float x1 = 0;
float x2 = 0;
float y_1 = 0;
float y2 = 0;
float x = 0;
float y = 0;
float a = 0;
float b = 0;

volatile bool flag_controle = false;
#define T_FLAG_CONTROLE 0.001 // en seconde
#define T_VITESSE 200e-6      // Intervalle de mesure de vitesse

// Variables internes pour vitesse
float ancienne_distance = 0;
float ancien_temps = 0;
float vitesse = 0;

Timer timer; // pour mesurer le temps écoulé

void encoder_tick()
{
    compteur_tick++;
}

void control()
{
    flag_controle = true;
}

// Calcul de vitesse
void calcul_vitesse()
{
    float nouvelle_distance = (2.0 * M_PI * (SIZE_WHEEL_DIAMETER_m / 2.0) / TIC_PER_TOUR) * compteur_tick;
    float nouvelle_temps = chrono::duration<float>(timer.elapsed_time()).count();

    float delta_distance = nouvelle_distance - ancienne_distance;
    float delta_temps = nouvelle_temps - ancien_temps;

    if (delta_temps > 0.0f)
    {
        vitesse = delta_distance / delta_temps; // m/s
    }

    ancienne_distance = nouvelle_distance;
    ancien_temps = nouvelle_temps;
}

int main()
{
    timer.start(); // On lance le timer dès le début

    t2.attach(&control, T_FLAG_CONTROLE);
    t_vitesse.attach(&calcul_vitesse, T_VITESSE); // Ticker pour la vitesse

    encoder_pin.rise(&encoder_tick);

    printf("Initialisation des taches effectuee\n");

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

            // Distance totale depuis le démarrage
            distance_m = (2.0 * M_PI * (SIZE_WHEEL_DIAMETER_m / 2.0) / TIC_PER_TOUR) * compteur_tick;

            // Affichage
            printf("Ubatt %3.1f ", tension_batterie);
            printf("Ibatt %3.1f ", mesure_courant_batt);
            printf("cpt_Tick %d, ", compteur_tick);
            printf("distance_m %f, ", distance_m);
            printf("Vitesse %3.2f m/s", vitesse);
            printf("\n");

            flag_controle = false;
        }
    }
}