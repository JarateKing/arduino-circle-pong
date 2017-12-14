// score display output
// SDA = 20, SCL = 21

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x38, 16, 2);

// score variables
long currentScore;
long bestScore;

// joystick input
const int joyXPin = A1;
const int joyYPin = A2;
float angle;
int discreteAngle;

// display output pins
// these decide where to display things
// paddlePins is the bits for a number 0-27
// posPins is for 0-8 on both the X and Y axis

const int paddlePins[5] = {22,24,26,28,30};
const int posPinsX[3] = {23,25,27};
const int posPinsY[3] = {29,31,33};

// pin for indicating data is ready to receive
const int dataReadyPin = 5;

// framerate input
int frameCounter;

// ball position
int ballx;
int bally;
int balldir;

// random noise pin
const int randomNoisePin = A0;

// restart button
const int restartPin = 3;

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
  bestScore = 0;

  // joystick setup
  angle = 0.0;
  discreteAngle = 0;

  // game setup
  gameStart();

  // rng setup
  randomSeed(analogRead(randomNoisePin));
}

void gameStart()
{
  clearScore();
  currentScore = 0;
  
  frameCounter = 0;

  ballx = 3;
  bally = 3;
  balldir = 0;
}

void loop() {
  int x = analogRead(joyXPin);
  int y = analogRead(joyYPin);
  if ((x > 540 || x < 500) || (y > 540 || y < 500))
  {
    angle = atan2(x-520,y-520);
    discreteAngle = (int)(25 - (angle + 3.14) / 6.28 * 28 + 28) % 28;
  }

  if (digitalRead(restartPin))
  {
    gameStart();
  }
  else
  {
    drawScore();
    sendNumber(paddlePins, 5, discreteAngle);
    sendNumber(posPinsX, ballx, 1);
    sendNumber(posPinsY, bally, 1);
    
    digitalWrite(dataReadyPin, HIGH);
    delay(2);
    digitalWrite(dataReadyPin, LOW);
  
    // perform updates
    frameCounter++;
    if (frameCounter > 20)
    {
      frameCounter = 0;
  
      if ((ballx == 1 && bally == 1) && (discreteAngle <= 2 || discreteAngle >= 26))
      {
        balldir = (1 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if ((ballx == 6 && bally == 1) && (discreteAngle >= 5 && discreteAngle <= 9))
      {
        balldir = (3 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if ((ballx == 6 && bally == 6) && (discreteAngle >= 12 && discreteAngle <= 16))
      {
        balldir = (5 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if ((ballx == 1 && bally == 6) && (discreteAngle >= 19 && discreteAngle <= 23))
      {
        balldir = (7 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if (ballx == 1 && ((discreteAngle >= 28 - bally - 2 && discreteAngle <= 28 - bally + 2)
                              || (discreteAngle == 0 && bally == 2)))
      {
        balldir = (0 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if (bally == 1 && (discreteAngle >= ballx - 2 && discreteAngle <= ballx + 2))
      {
        balldir = (2 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if (ballx == 6 && (discreteAngle >= 7 + bally - 2 && discreteAngle <= 7 + bally + 2))
      {
        balldir = (4 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if (bally == 6 && (discreteAngle >= 21 - ballx - 2 && discreteAngle <= 21 - ballx + 2))
      {
        balldir = (6 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if (ballx == 1 || bally == 1 || ballx == 6 || bally == 6)
      {
        delay(5);
        Serial.println("x:" + String(ballx));
        Serial.println("y:" + String(bally));
        saveScore();
        delay(2000);
        gameStart();
      }
      
  
      if (balldir == 0)
      {
        ballx++;
      }
      else if (balldir == 1)
      {
        ballx++;
        bally++;
      }
      else if (balldir == 2)
      {
        bally++;
      }
      else if (balldir == 3)
      {
        ballx--;
        bally++;
      }
      else if (balldir == 4)
      {
        ballx--;
      }
      else if (balldir == 5)
      {
        ballx--;
        bally--;
      }
      else if (balldir == 6)
      {
        bally--;
      }
      else if (balldir == 7)
      {
        ballx++;
        bally--;
      }
      
      debugDraw(discreteAngle, ballx, bally);
    }
  }

  delay(5);
}

void saveScore()
{
  if (currentScore > bestScore)
  {
    // TODO: implement this part
  }
}

void drawScore()
{
  lcd.setCursor(0,0);
  lcd.print("BEST");
  if (bestScore == 0)
    lcd.setCursor(15,0);
  else
    lcd.setCursor(16 - floor(log10(bestScore))-1,0);
  lcd.print(bestScore);
  lcd.setCursor(0,1);
  lcd.print("SCORE");
  if (currentScore == 0)
    lcd.setCursor(15,1);
  else
    lcd.setCursor(16 - floor(log10(currentScore))-1,1);
  lcd.print(currentScore);
}

void clearScore()
{
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
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
  Serial.println(discreteAngle);
  Serial.println(angle);
  Serial.println();
  Serial.println("BEST   " + String(bestScore));
  Serial.println("SCORE  " + String(currentScore));
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

