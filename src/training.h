#include <Arduino.h>

namespace Timer
{
    int BUZZER_PIN;
    int ROUND_LED_PIN;
    int REST_LED_PIN;

    struct RoutineConfiguration
    {
        unsigned int INT_TIME = 1;
        unsigned int ROUND_TIME = 2;
        unsigned int REST_TIME = 3;
        unsigned int ROUNDS = 4;
    };

    RoutineConfiguration routine;

    volatile int CURRENT_TIME;

    int TOTAL_TIME = 0;

    bool TRAINING_ENABLED = false;
    bool DISPLAY_ROUND = false;

    void (*timeDisplayCallback)(int);
    void (*roundDisplayCallback)(int);
    void (*endCallback)(int);

    void (*timerAction)();

    void setup(RoutineConfiguration routineCfg, int buzzerPin, int roundLedPin, int restLedPin, void (*tDisplayCallback)(int), void (*rDisplayCallback)(int), void (*doneCallback)(int))
    {
        Timer::routine = routineCfg;

        Timer::BUZZER_PIN = buzzerPin;
        Timer::ROUND_LED_PIN = roundLedPin;
        Timer::REST_LED_PIN = restLedPin;

        Timer::timeDisplayCallback = tDisplayCallback;
        Timer::roundDisplayCallback = rDisplayCallback;
        Timer::endCallback = doneCallback;

        pinMode(BUZZER_PIN, OUTPUT);
        pinMode(ROUND_LED_PIN, OUTPUT);
        pinMode(REST_LED_PIN, OUTPUT);
        digitalWrite(ROUND_LED_PIN, LOW);
        digitalWrite(REST_LED_PIN, LOW);

        noTone(BUZZER_PIN);
    }

    void enableTimer()
    {
        cli();
        TCCR1A = 0;                          //Rejestr kontrolny A do 0, piny OC1A i OC1B wyłączone
        TCCR1B = 0;                          //Wyczyść rejestrator
        TCCR1B |= (1 << CS10) | (1 << CS12); //Ustaw preskaler na 1024: CS12 = 1 i CS10 = 1
        TCNT1 = 0xC2F8;                      //Uruchom licznik czasu przepełnienia na 1 sekundę
                                             //65536-(16MHz/1024/1Hz - 1) = 49912 = 0xC2F8 w systemie szesnastkowym
        TIMSK1 |= (1 << TOIE1);              //Włącz przerwanie dla Timera1
        sei();                               //Włącz globalne przerwania
    }

    ISR(TIMER1_OVF_vect)
    {
        TOTAL_TIME++;
        (*timerAction)();
        TCNT1 = 0xC2F8; // for 1 sec at 16 MHz
    }

    void timerAfterTrainingAction(){
        (*timeDisplayCallback)(TOTAL_TIME);
    }

    void timerTrainingAction()
    {
        digitalWrite(Timer::BUZZER_PIN, LOW);
        CURRENT_TIME--;
        if (CURRENT_TIME <= 3 && CURRENT_TIME > 0)
            tone(Timer::BUZZER_PIN, 3000, 100);

        if (CURRENT_TIME == 0)
            tone(Timer::BUZZER_PIN, 4000, 500);

        if (TOTAL_TIME % 2 == 0 && DISPLAY_ROUND)
            DISPLAY_ROUND = false;
    }

    void disableTimer()
    {
        TIMSK1 &= ~(1 << TOIE1);
    }

    void switchLed(int pin)
    {
        digitalWrite(ROUND_LED_PIN, LOW);
        digitalWrite(REST_LED_PIN, LOW);
        if (pin != NULL && pin > 0)
        {
            digitalWrite(pin, HIGH);
        }
    }

    void displayRound(int round)
    {
        DISPLAY_ROUND = true;
        do
        {
            (*roundDisplayCallback)(round);
        } while (DISPLAY_ROUND);
    }

    void displayTime(int time)
    {
        (*timeDisplayCallback)(time);
    }

    void beginTraining()
    {
        TRAINING_ENABLED = true;
        TOTAL_TIME = 0;
        timerAction = timerTrainingAction;

        // INITIAL TIME
        CURRENT_TIME = routine.INT_TIME;
        tone(BUZZER_PIN, 800, 100);
        unsigned long startMilis = millis();
        Serial.print("Start time: ");
        Serial.println(startMilis);
        enableTimer();
        do
        {
            (*timeDisplayCallback)(CURRENT_TIME);
        } while (CURRENT_TIME > 0);

        for (int round = 0; round < (int)routine.ROUNDS; round++)
        {
            // ROUND TIME
            CURRENT_TIME = routine.ROUND_TIME;
            switchLed(ROUND_LED_PIN);
            displayRound(round + 1);
            while (CURRENT_TIME > 0)
            {
                displayTime(CURRENT_TIME);
            }

            // REST TIME
            CURRENT_TIME = routine.REST_TIME;
            switchLed(REST_LED_PIN);
            while (CURRENT_TIME > 0)
            {
                displayTime(CURRENT_TIME);
            }
        }
        timerAction = timerAfterTrainingAction;
        (*endCallback)(TOTAL_TIME);
        TRAINING_ENABLED = false;
        digitalWrite(ROUND_LED_PIN, HIGH);
        digitalWrite(REST_LED_PIN, HIGH);
        tone(Timer::BUZZER_PIN, 4000, 2000);

        unsigned long endMilis = millis();
        Serial.print("End time: ");
        Serial.println(endMilis);
        unsigned long totalMilis = (endMilis - startMilis) / 1000;
        Serial.print("Total time: ");
        Serial.println(totalMilis);
    }
    
} // namespace Timer
