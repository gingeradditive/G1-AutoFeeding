#include <EEPROM.h>
#include <Wire.h>

bool ACCESO_FEED = LOW;
bool PAUSE_FEED = LOW;

// bottone per accendere spegnere il controllo
const int button_feed_pin = 9;
int buttonState_feed = LOW; // on 1 is HIGH
int buttonLastState_feed = HIGH;
unsigned long buttonLastDebounceTime_feed = millis();

// sensore capacitivo
const int probe_pin = 4;
int probeState = HIGH; // high means closed (when probing something is closed)
int probeLastState = HIGH;

// filtro sul bottone
unsigned long debounceDelay = 20;
unsigned long lastDebounceTime = millis();

// filtro di 1sec sul sensore
unsigned long probeDebounceDelay = 800;
unsigned long probeLastDebounceTime = millis();

// ciclo di carico da 1sec
const int cycle_feed = 1000;
unsigned long cycle_feed_started = millis();

// relè da controllare
const int ssr_feed_pin = 2;

// pausa di 20sec dopo 5 cicli
int cont_feed = 0;
const int cont_feed_max = 4; // 5 cicli

int count_pause = 0; 
const int count_pause_max = 2; // 3 cicli

unsigned long pause_feed_started = millis();
const int cycle_feed_pause = 20000; // 20sec

// buzzer sonoro di errore dopo 5 cicli
float power_secco = 0.07;     // percentage
const int cycle_secco = 7000; // ms
unsigned long cycle_started_secco;
const int buzzer_pin = 12;

// LED
const int ssr_led_pin = 3;

void setup()
{
    pinMode(ssr_led_pin, OUTPUT);
    digitalWrite(ssr_led_pin, LOW);

    pinMode(ssr_feed_pin, OUTPUT);
    digitalWrite(ssr_feed_pin, LOW);

    pinMode(buzzer_pin, OUTPUT);
    digitalWrite(buzzer_pin, LOW);

    pinMode(button_feed_pin, INPUT); // ha senso mettere un pullup?
    pinMode(probe_pin, INPUT);

    delay(10000);
}

void loop()
{

    // Debounce del sensore capacitivo
    int probeReading = digitalRead(probe_pin);
    if (probeReading != probeLastState)
        probeLastDebounceTime = millis();
    
    if ((millis() - probeLastDebounceTime) > probeDebounceDelay)
        probeState = probeReading;
    
    probeLastState = probeReading;

    // Debounce del bottone o del PIN di abilitazione 
    int buttonReading_feed = digitalRead(button_feed_pin);
    if (buttonReading_feed != buttonLastState_feed)
        buttonLastDebounceTime_feed = millis();
    
    if ((millis() - buttonLastDebounceTime_feed) > debounceDelay)
        buttonState_feed = buttonReading_feed;

    buttonLastState_feed = buttonReading_feed;

    // LOGICA
    if (buttonState_feed == HIGH && probeState == LOW && ACCESO_FEED == LOW && PAUSE_FEED == LOW)
    {
        ACCESO_FEED = HIGH;
        cycle_feed_started = millis();
    }
    
    if (buttonState_feed == LOW)
    {
        ACCESO_FEED = LOW;
        PAUSE_FEED = LOW;
    }

    // CONTROLLO RELE'

    //Ciclo di carico
    if (ACCESO_FEED == HIGH && PAUSE_FEED == LOW)
    {
        unsigned long delta = millis() - cycle_feed_started; 

        if (delta < cycle_feed)
        {
            //Accendi aspiratore e led 
            digitalWrite(ssr_feed_pin, HIGH);
            digitalWrite(ssr_led_pin, HIGH);
        }
        else if ((delta >= cycle_feed) && probeState == HIGH)
        {
            ACCESO_FEED = LOW;
            cont_feed = 0;
        }
        else
        {
            cycle_feed_started = millis();
            cont_feed++;
        }
    }

    //spegni aspiratore e led 
    if (ACCESO_FEED == LOW)
    {
        digitalWrite(ssr_feed_pin, LOW);
        digitalWrite(ssr_led_pin, LOW);
    }

    //Avvia ciclo di pausa dopo 5 cicli
    if (cont_feed >= cont_feed_max && PAUSE_FEED == LOW)
    {
        PAUSE_FEED = HIGH;
        pause_feed_started = millis();
        cont_feed = 0;
    }

    if (PAUSE_FEED == HIGH)
    {
        count_pause = 0;
        digitalWrite(ssr_feed_pin, LOW);
        digitalWrite(ssr_led_pin, LOW);
        if (millis() - pause_feed_started >= cycle_feed_pause)
            PAUSE_FEED = LOW; 
    }

    if (PAUSE_FEED == LOW)
    {
        digitalWrite(buzzer_pin, LOW);
    }

    if (PAUSE_FEED == HIGH)
    {
        unsigned long delta = millis() - cycle_started_secco; 
        if (delta <= cycle_secco * power_secco)
            if(count_pause < count_pause_max)
                count_pause++;
            else
            {
                count_pause = 0;
                digitalWrite(buzzer_pin, HIGH);
            } 
        else
            digitalWrite(buzzer_pin, LOW);
        
        if (delta >= cycle_secco)
        {
            digitalWrite(buzzer_pin, LOW);
            cycle_started_secco = millis();
        }
    }
}