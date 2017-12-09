// score display output
// SDA = A4, SCL = A5

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x38, 16, 2);

// display output pins
// these decide where to display things
// paddlePins is the bits for a number 0-27
// posPins is for 0-8 on both the X and Y axis

const int paddlePins[5] = {22,24,26,28,30};
const int posPinsX[3] = {23,25,27};
const int posPinsY[3] = {29,31,33};

// pin for indicating data is ready to receive
const int dataReadyPin = 5;

void setup() {
  Serial.begin(9600);
  // setup paddle reading pins
  for (int i = 0; i < 5; i++)
  {
    pinMode(paddlePins[i], OUTPUT);
  }
  // setup position reading pins
  for (int i = 0; i < 3; i++)
  {
    pinMode(posPinsX[i], OUTPUT);
    pinMode(posPinsY[i], OUTPUT);
  }
  pinMode(dataReadyPin, OUTPUT);

  // scoreboard setup
  lcd.init();
  lcd.backlight();
}

void loop() {
  for (int i = 0; i < 168; i++)
  {
    sendNumber(paddlePins, 5, i % 28);
    sendNumber(posPinsX, 3, i % 6 + 1);
    sendNumber(posPinsY, 3, i % 6 + 1);
    debugDraw(i % 28, i % 6 + 1, i % 6 + 1);
    digitalWrite(dataReadyPin, HIGH);
    delay(2);
    digitalWrite(dataReadyPin, LOW);
    delay(490);
  }

}

void sendNumber(int pins[], int pinSize, int num)
{
  int total = num;
  int toSend[pinSize];
  for (int i = 0; i < pinSize; i++)
  {
    digitalWrite(pins[i], bitRead(num, i));
  }
}

// draw current display board to serial monitor
// is not a clean function, but is functional for debugging
void debugDraw(int paddle, int x, int y)
{
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();

  // top line
  for (int i = 0; i < 8; i++)
  {
    if ((i >= paddle - 2 && i <= paddle + 2) || (paddle == 26 && i == 0) || (paddle == 27 && i <= 1)) 
    {
      Serial.print("X");
    }
    else
    {
      Serial.print("-");
    }
  }
  Serial.println();

  // lines 1-6
  for (int i = 1; i < 7; i++)
  {
    if((28 - i >= paddle - 2 && 28 - i <= paddle + 2) || (paddle == 0 && i <= 2) || (paddle == 1 && i == 1))
    {
      Serial.print("X");
    }
    else
    {
      Serial.print("-");
    }
    for (int j = 1; j < 7; j++)
    {
      if (x == j && y == i)
      {
        Serial.print("O");
      }
      else
      {
        Serial.print(".");
      }
    }
    if(i + 7 >= paddle - 2 && i + 7 <= paddle + 2)
    {
      Serial.print("X");
    }
    else
    {
      Serial.print("-");
    }
    Serial.println();
  }

  // bottom line
  for (int i = 7; i >= 0; i--)
  {
    if (i + 14 >= paddle - 2 && i  + 14 <= paddle + 2)
    {
      Serial.print("X");
    }
    else
    {
      Serial.print("-");
    }
  }
  Serial.println();
}

