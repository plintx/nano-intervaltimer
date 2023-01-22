#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "hc595.h"
#include "encoder.h"
#include "training.h"

const int DISPLAY_DIO_PIN = 10;
const int DISPLAY_SCK_PIN = 11;
const int DISPLAY_RCK_PIN = 12;

const int ENCODER_CLK_PIN = 7;
const int ENCODER_DT_PIN = 8;
const int ENCODER_SW_PIN = 9;

//const int BUZZER_S_PIN = A3;
const int BUZZER_S_PIN = A2;
const int ROUND_LED_PIN = 6;
const int REST_LED_PIN = 5;

HC595D::HC595Display *display;
Encoder::RotaryEncoder *encoder;

unsigned int MENU_OPTION = 0;
bool VALUES_CHANGED = false;

void changeValue(unsigned int &a, int b)
{
  if (a + b > 0)
    a = a + b;
  VALUES_CHANGED = true;
}

void saveToEEPROM()
{
  if (VALUES_CHANGED)
  {
    eeprom_write_block(&Timer::routine, 0, sizeof(Timer::routine));
    Serial.println("Configuraton saved");
  }
}

void displayMenu()
{
  switch (MENU_OPTION)
  {
  case 0:
    display->writeTime(Timer::routine.INT_TIME);
    encoder->changeCallback = [](int a) {
      changeValue(Timer::routine.INT_TIME, a);
    };
    break;
  case 1:
    display->writeTime(Timer::routine.ROUND_TIME);
    encoder->changeCallback = [](int a) {
      changeValue(Timer::routine.ROUND_TIME, a);
    };
    break;
  case 2:
    display->writeTime(Timer::routine.REST_TIME);
    encoder->changeCallback = [](int a) {
      changeValue(Timer::routine.REST_TIME, a);
    };
    break;
  case 3:
    display->writeValue(Timer::routine.ROUNDS);
    encoder->changeCallback = [](int a) {
      changeValue(Timer::routine.ROUNDS, a);
    };
    break;
  case 4:
    display->writeValue("PLAY");
    encoder->changeCallback = [](int a) {
      saveToEEPROM();
      Timer::beginTraining();
    };
    break;
  default:
    display->writeTime(Timer::TOTAL_TIME);
    encoder->changeCallback = [](int a) {};
    break;
  }
}

void encoderBtnAction(unsigned int press_time)
{
  if (press_time < 1000)
  {
    MENU_OPTION++;
    if (MENU_OPTION > 4)
      MENU_OPTION = 0;
  }
  if (press_time > 5000)
  {
    Timer::RoutineConfiguration cfg;
    Timer::routine = cfg;
  }
}

Timer::RoutineConfiguration loadFromEEPROM()
{
  Timer::RoutineConfiguration cfg;

  Serial.println("Loading from eeprom...");
  eeprom_read_block(&cfg, 0, sizeof(cfg));
  Serial.print("IN_TIME: ");
  Serial.println(cfg.INT_TIME);
  Serial.print("ROUND_TIME: ");
  Serial.println(cfg.ROUND_TIME);
  Serial.print("REST_TIME: ");
  Serial.println(cfg.REST_TIME);
  Serial.print("ROUNDS: ");
  Serial.println(cfg.ROUNDS);
  return cfg;
}

void setup()
{
  Serial.begin(9600);

  display = new HC595D::HC595Display(DISPLAY_DIO_PIN, DISPLAY_SCK_PIN, DISPLAY_RCK_PIN);

  encoder = new Encoder::RotaryEncoder(ENCODER_CLK_PIN, ENCODER_DT_PIN, ENCODER_SW_PIN);
  encoder->btnCallback = &encoderBtnAction;

  Timer::setup(loadFromEEPROM(), BUZZER_S_PIN, ROUND_LED_PIN, REST_LED_PIN, NULL, NULL, NULL);

  Timer::timeDisplayCallback = [](int a) {
    display->writeTime(a);
  };

  Timer::roundDisplayCallback = [](int a) {
    display->writeValue(a);
  };

  Timer::endCallback = [](int a) {
    MENU_OPTION = 99;
  };
}

void loop()
{
  if (!Timer::TRAINING_ENABLED)
  {
    encoder->read();
    displayMenu();
  }
}