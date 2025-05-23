#include <mbed.h>

DigitalOut led1(LED1); // LED pour l'interruption 1

// Initialisation des PIN
AnalogIn analog_alim_tension(PB_0);
float tension_micro_lu = 0;
float tension_batterie = 0;

AnalogIn analog_hall_effect(PB_1);
float image_courant_lu_micro = 0;
float mesure_courant_batt = 0;
float vitesse = 0;

InterruptIn encoder_pin(PA_7);

volatile int compteur_tick = 0;
float distance_m = 0;
float distance_prec = 0;
#define SIZE_WHEEL_DIAMETER_m 0.0025
#define TIC_PER_TOUR 1

Ticker t1;
Ticker t2;

volatile bool flag_led = false;
volatile bool flag_controle = false;
#define T_FLAG_LED 1          // en seconde
#define T_FLAG_CONTROLE 0.001 // en seconde

// caca
void LED()
{
    led1 = !led1;
    flag_led = true;
}

void encoder_tick()
{
    compteur_tick++;
}

void control()
{

    flag_controle = true;
}

int main()
{
    double temps = 0;
    t1.attach(&LED, T_FLAG_LED);
    t2.attach(&control, T_FLAG_CONTROLE);
    encoder_pin.rise(&encoder_tick);
    printf("Initialisation des tache effectuer\n");

    while (true)
    {
        static int i = 0;

        // printf("i %d", i++);
        // printf(" flag_led %d", flag_led);
        // printf("\n");

        if (flag_controle)
        {

            // // Mesure tension
            tension_micro_lu = analog_alim_tension.read() * (3.3 / 1.0);       // Conversion vers tension analogique
            tension_batterie = tension_micro_lu * (15000.0 + 1000.0) / 1000.0; // Diviseur de tension

            // Mesure du courant
            image_courant_lu_micro = analog_hall_effect.read() * (4096.0 / 1.0); // Lecture ADC brut (0-4096)
            mesure_courant_batt = 0.0065 * image_courant_lu_micro - 19.946;      // Diviseur de tension

            // Mesure de la vitesse
            distance_m = (2.0 * M_PI * (SIZE_WHEEL_DIAMETER_m / 2.0) / TIC_PER_TOUR) * compteur_tick;
            distance_prec = distance_m;
            vitesse = (distance_m - distance_prec) / T_FLAG_CONTROLE;
            printf("Ubatt %3.1f ", tension_batterie);
            printf("Ibatt %3.1f ", mesure_courant_batt);
            printf("cpt_Tick %d, ", compteur_tick);
            printf("distance_m %f, ", distance_m);
                        printf("distance_m %f, ", distance_prec);

            printf("delta_d%3.1f ", (distance_m - distance_prec));

            printf("Vitesse %3.1f ", vitesse);

            printf("\n");
            flag_controle = false;
        }
    }
}
