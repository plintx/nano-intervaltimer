/* Common Anode 74HC595 6-digit 7-segment display */
/*
-- Segments
FIRST 8 BITS (ZERO TO ENABLE SEGMENT)
ALL with DOT - 0, 0, 0, 0, 0, 0, 0, 0
0 - 1, 1, 0, 0, 0, 0, 0, 0
1 - 1, 1, 1, 1, 1, 0, 0, 1
2 - 1, 0, 1, 0, 0, 1, 0, 0
3 - 1, 0, 1, 1, 0, 0, 0, 0
4 - 1, 0, 0, 1, 1, 0, 0, 1
5 - 1, 0, 0, 1, 0, 0, 1, 0
6 - 1, 0, 0, 0, 0, 0, 1, 0
7 - 1, 1, 1, 1, 1, 0, 0, 0
8 - 1, 0, 0, 0, 0, 0, 0, 0
9 - 1, 0, 0, 1, 0, 0, 0, 0


  -8-
3|   |7
  -2-
4|   |6
  -5-
1 - dot


// Digits
// SECOND 8 BITS
// 1 - 0, 0, 0, 0, 0, 1, 0, 0
// 2 - 0, 0, 0, 0, 0, 0, 1, 0
// 3 - 0, 0, 0, 0, 0, 0, 0, 1
// 4 - 0, 1, 0, 0, 0, 0, 0, 0
// 5 - 0, 0, 1, 0, 0, 0, 0, 0
// 6 - 0, 0, 0, 1, 0, 0, 0, 0
*/

#include <Arduino.h>

namespace HC595D
{

    typedef struct
    {
        char character;
        int segments[8];
    } segment_letter;

    int SEGMENT_VALUE[10][8] =
        {
            {1, 1, 0, 0, 0, 0, 0, 0},
            {1, 1, 1, 1, 1, 0, 0, 1},
            {1, 0, 1, 0, 0, 1, 0, 0},
            {1, 0, 1, 1, 0, 0, 0, 0},
            {1, 0, 0, 1, 1, 0, 0, 1},
            {1, 0, 0, 1, 0, 0, 1, 0},
            {1, 0, 0, 0, 0, 0, 1, 0},
            {1, 1, 1, 1, 1, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 1, 0, 0, 0, 0}};

    segment_letter SEGMENT_LETTERS[] = {
        {' ', {1, 1, 1, 1, 1, 1, 1, 1}},
        {'P', {1, 0, 0, 0, 1, 1, 0, 0}},
        {'L', {1, 1, 0, 0, 0, 1, 1, 1}},
        {'A', {1, 0, 0, 0, 1, 0, 0, 0}},
        {'Y', {1, 0, 0, 1, 1, 0, 0, 1}},
        {'R', {1, 0, 0, 0, 1, 0, 0, 0}},
        {'T', {1, 1, 1, 1, 1, 0, 0, 0}},
        {'^', {1, 1, 1, 1, 1, 1, 1, 0}},
        {'-', {1, 0, 1, 1, 1, 1, 1, 1}},
        {'_', {1, 1, 1, 1, 0, 1, 1, 1}}};

    int DISPLAY_DIGIT[6][8] = {
        {0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 0}};

    class HC595Display
    {
    public:
        HC595Display(int dioPin, int sckPin, int rckPin)
        {
            DS = dioPin;
            STCP = sckPin;
            SHCP = rckPin;
            pinMode(DS, OUTPUT);
            pinMode(STCP, OUTPUT);
            pinMode(SHCP, OUTPUT);
            digitalWrite(DS, LOW);
            digitalWrite(STCP, LOW);
            digitalWrite(SHCP, LOW);
            clear();
        }

        void writeValue(int digit, int order, bool dot)
        {
            if (digit < 0 || digit > 9)
                return;

            int registry[16];

            for (int i = 0; i < 8; i++)
            {
                // SET DIGIT SEGMENTS ENABLED
                registry[i] = SEGMENT_VALUE[digit][i];
                // SET DIGIT ENABLED
                registry[i + 8] = DISPLAY_DIGIT[order][i];
            }

            if (dot == true)
            {
                registry[0] = 0;
            }

            digitalWrite(SHCP, LOW);
            for (int i = 0; i < 16; i++)
            {
                digitalWrite(STCP, LOW);
                digitalWrite(DS, registry[i]);
                digitalWrite(STCP, HIGH);
            }

            digitalWrite(SHCP, HIGH);
        }

        void writeChar(char chr, int order, bool dot)
        {
            int registry[16];
            segment_letter segments = getLetter(chr);

            for (int i = 0; i < 8; i++)
            {
                // SET DIGIT SEGMENTS ENABLED
                registry[i] = segments.segments[i];
                // SET DIGIT ENABLED
                registry[i + 8] = DISPLAY_DIGIT[order][i];
            }

            if (dot == true)
            {
                registry[0] = 0;
            }
            digitalWrite(SHCP, LOW);
            for (int i = 0; i < 16; i++)
            {
                digitalWrite(STCP, LOW);
                digitalWrite(DS, registry[i]);
                digitalWrite(STCP, HIGH);
            }

            digitalWrite(SHCP, HIGH);
        }

        void writeTime(int sec)
        {
            int seconds = sec % 60;
            int minutes = ((sec - seconds) / 60) % 60;
            char buf[7] = {
                0,
            };
            snprintf(buf, 7, "%02d.%02d", minutes, seconds);
            writeValue(buf);
        }

        void writeValue(int value)
        {
            char buf[7] = {
                0,
            };
            snprintf(buf, 7, "%6d", value);

            for (int i = 0; i < 6; i++)
            {
                int num = (buf[i] - '0') & 0xFF;
                writeValue(num, i, false);
            }
        }

        void writeValue(String value)
        {
            for (int i = value.length(), d = 6; i >= 0; i--)
            {
                if (value[i] == '.')
                    continue;

                bool dot = false;
                int num = -1;
                if (value[i + 1] == '.')
                {
                    dot = true;
                }
                if (isdigit(value[i]))
                {
                    num = (value[i] - '0') & 0xFF;
                    writeValue(num, d--, dot);
                }
                else
                {
                    writeChar(value[i], d--, dot);
                }
            }
        }
        void test()
        {
            digitalWrite(SHCP, LOW);

            for (int i = 16; i >= 0; i--)
            {
                registry[i] = HIGH;
                for (int x = 6; x >= 0; x--)
                {
                    digitalWrite(STCP, LOW);
                    digitalWrite(DS, HIGH); // will output HIGH // common anode
                    digitalWrite(STCP, HIGH);
                }
            }
            digitalWrite(SHCP, HIGH);
        }
        void clear()
        {
            digitalWrite(SHCP, LOW);

            for (int i = 16; i >= 0; i--)
            {
                registry[i] = HIGH;
                for (int x = 6; x >= 0; x--)
                {
                    digitalWrite(STCP, LOW);
                    digitalWrite(DS, HIGH); // will output HIGH // common anode
                    digitalWrite(STCP, HIGH);
                }
            }
            digitalWrite(SHCP, HIGH);
        }

    private:
        int DS;   //DS PIN
        int STCP; //STCP PIN
        int SHCP; //SHCP PIN

        int registry[16];

        segment_letter getLetter(char chr)
        {
            for (int i = 0; i < sizeof(SEGMENT_LETTERS) / sizeof(segment_letter); ++i)
            {
                if (SEGMENT_LETTERS[i].character == chr)
                {
                    return SEGMENT_LETTERS[i];
                }
            }
            return SEGMENT_LETTERS[0];
        }
    };
} // namespace HC595D