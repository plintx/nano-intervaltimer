#include <Arduino.h>

namespace Encoder
{
    class RotaryEncoder
    {
    public:
        RotaryEncoder(int clkPIN, int dtPIN, int swPIN)
        {
            CLK_PIN = clkPIN;
            DT_PIN = dtPIN;
            SW_PIN = swPIN;
            pinMode(CLK_PIN, INPUT);
            pinMode(DT_PIN, INPUT);
            pinMode(SW_PIN, INPUT);
            sw_lastState = HIGH;
        }

        void (*changeCallback)(int);
        void (*btnCallback)(unsigned int);

        void read()
        {
            readEncoderState();
            readButtonState();
        }

    private:
        int CLK_PIN;
        int DT_PIN;
        int SW_PIN;
        uint8_t prevNextCode = 0;
        uint16_t store = 0;

        int sw_lastState;
        int sw_currentState;
        unsigned long sw_pressedTime = 0;
        unsigned long sw_releasedTime = 0;

        void readEncoderState()
        {
            static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

            prevNextCode <<= 2;
            if (digitalRead(DT_PIN))
                prevNextCode |= 0x02;
            if (digitalRead(CLK_PIN))
                prevNextCode |= 0x01;
            prevNextCode &= 0x0f;

            // If valid then store as 16 bit data.
            if (rot_enc_table[prevNextCode])
            {
                store <<= 4;
                store |= prevNextCode;
                //if (store==0xd42b) return 1;
                //if (store==0xe817) return -1;
                if ((store & 0xff) == 0x2b)
                    (*changeCallback)(-1);
                if ((store & 0xff) == 0x17)
                    (*changeCallback)(1);
            }
        }

        void readButtonState()
        {
            sw_currentState = digitalRead(SW_PIN);

            if (sw_lastState == HIGH && sw_currentState == LOW) // button is pressed
                sw_pressedTime = millis();
            else if (sw_lastState == LOW && sw_currentState == HIGH)
            {
                sw_releasedTime = millis();
                unsigned long pressDuration = sw_releasedTime - sw_pressedTime;
                (*btnCallback)(pressDuration);   
            }
            sw_lastState = sw_currentState;
        }
    };
} // namespace Encoder