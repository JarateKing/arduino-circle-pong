// score display output

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>

LiquidCrystal_I2C lcd(0x38, 16, 2);

// Debug Mode Define
#define DEBUG 1
// 1 for on, 0 for off

// Pin Defines
#define JOY_X A1
#define JOY_Y A2
#define RESTART 3
#define DATA_READY 5
#define RND_NOISE A0
#define ETH_LINE 10
#define SD_LINE 4
// Also not usable:
// To verify: SD_MOSI = 11, SD_MISO = 12, SD_CLK = 13
// SDA = 20, SCL = 21

// Array Constants
#define N_PADDLE_PINS 5
#define N_POSITION_PINS 3

// Joystick Input
#define X_IN_MAX 540
#define X_IN_CNT 520
#define X_IN_MIN 500
#define Y_IN_MAX 540
#define Y_IN_CNT 520
#define Y_IN_MIN 500

// Drawing Constants
#define FRAME_DELAY 20

// Math Defines

// score variables
long currentScore;
long bestScore;
File scoreFile;
static String SCORE_FILE_NAME = "high_scores.sav";
char charBuf[10];

// joystick input
float angle;
int discreteAngle;

// display output pins
// these decide where to display things
// paddlePins is the bits for a number 0-27
// posPins is for 0-8 on both the X and Y axis

const int paddlePins[5] = {22,24,26,28,30};
const int posPinsX[3] = {23,25,27};
const int posPinsY[3] = {29,31,33};

// framerate input
int frameCounter;

// ball position
int ballx;
int bally;
int balldir;

void setup() {
  #if DEBUG > 0
    // If DEBUG is defined to 1 in the defines (top) setup the serial out for debugging
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
  #endif
  
  // Disable the Ethernet interface
  pinMode(ETH_LINE, OUTPUT);
  digitalWrite(ETH_LINE, HIGH);
  
  // setup paddle reading pins
  for (int i = 0; i < N_PADDLE_PINS; i++)
  {
    pinMode(paddlePins[i], OUTPUT);
  }
  // setup position reading pins
  for (int i = 0; i < N_POSITION_PINS; i++)
  {
    pinMode(posPinsX[i], OUTPUT);
    pinMode(posPinsY[i], OUTPUT);
  }
  
  pinMode(DATA_READY, OUTPUT);

  // scoreboard setup
  lcd.init();
  lcd.backlight();
  bestScore = getHighScore();

  // joystick setup
  angle = 0.0;
  discreteAngle = 0;

  // game setup
  gameStart();

  // rng setup
  randomSeed(analogRead(RND_NOISE));
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
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);
  // Migrated constants to defines mostly for consistency, I can undo it if you'd rather
  if ((x > X_IN_MAX || x < X_IN_MIN) || (y > Y_IN_MAX || y < Y_IN_MIN)) // Are we sure about this?
  {
    angle = atan2(x-X_IN_CNT,y-Y_IN_CNT); // -pi to pi // Does this need math.h import?
    // This should be faster and equivalent
    discreteAngle = (int)(53 - (angle + 3.14159) * 4.45634) % 28; //(int)(25 - (angle + 3.14) / 6.28 * 28 + 28) % 28;
  }

  if (digitalRead(RESTART))
  {
    gameStart();
  }
  else
  {
    drawScore();
    sendNumber(paddlePins, 5, discreteAngle);
    sendNumber(posPinsX, 3, ballx);
    sendNumber(posPinsY, 3, bally);
    
    digitalWrite(DATA_READY, HIGH);
    delay(2);
    digitalWrite(DATA_READY, LOW);
  
    // perform updates
    frameCounter++;
    if (frameCounter > FRAME_DELAY)
    {
      frameCounter = 0;

      // determine where the ball hit & if the paddle is in range
      // each corner & each side has their own opposing angle, that will have variance added on
      if ((ballx == 1 && bally == 1) && (discreteAngle <= 2 || discreteAngle >= 26))
      {
        balldir = nextDirection(1);
        currentScore++;
      }
      else if ((ballx == 6 && bally == 1) && (discreteAngle >= 5 && discreteAngle <= 9))
      {
        balldir = nextDirection(3);
        currentScore++;
      }
      else if ((ballx == 6 && bally == 6) && (discreteAngle >= 12 && discreteAngle <= 16))
      {
        balldir = nextDirection(5);
        currentScore++;
      }
      else if ((ballx == 1 && bally == 6) && (discreteAngle >= 19 && discreteAngle <= 23))
      {
        balldir = nextDirection(7);
        currentScore++;
      }
      else if (ballx == 1 && ((discreteAngle >= 28 - bally - 2 && discreteAngle <= 28 - bally + 2)
                              || (discreteAngle == 0 && bally == 2)))
      {
        balldir = nextDirection(0);
        currentScore++;
      }
      else if (bally == 1 && (discreteAngle >= ballx - 2 && discreteAngle <= ballx + 2))
      {
        balldir = nextDirection(2);
        currentScore++;
      }
      else if (ballx == 6 && (discreteAngle >= 7 + bally - 2 && discreteAngle <= 7 + bally + 2))
      {
        balldir = nextDirection(4);
        currentScore++;
      }
      else if (bally == 6 && (discreteAngle >= 21 - ballx - 2 && discreteAngle <= 21 - ballx + 2))
      {
        balldir = nextDirection(6); //(6 + random(-1,1) + 8) % 8;
        currentScore++;
      }
      else if (ballx == 1 || bally == 1 || ballx == 6 || bally == 6)
      {
        delay(5);
        // Preprocessor commands for safety
        #if DEBUG > 0
          Serial.println("x:" + String(ballx));
          Serial.println("y:" + String(bally));
        #endif
        saveScore();
        delay(2000);
        gameStart();
      }
      
      switch (balldir) {
        case 0:
          ballx++;
          break;
        case 1:
          ballx++;
          bally++;
          break;
        case 2:
          bally++;
          break;
        case 3:
          ballx--;
          bally++;
          break;
        case 4:
          ballx--;
          break;
        case 5:
          ballx--;
          bally--;
          break;
        case 6:
          bally--;
          break;
        case 7:
          ballx++;
          bally--;
          break;
      }
      
      #if DEBUG > 0
        debugDraw(discreteAngle, ballx, bally);
      #endif
    }
  }

  delay(5);
}

// determine the ball's direction, with variance
// given the "perfect" direction for where it hit as its input
inline int nextDirection(int in)
{
  return (in + random(-1,2) + 8) % 8;
}

// This always overrides previous high score
void saveScore()
{
  if (currentScore > bestScore)
  {
    #if DEBUG > 0
      Serial.print(F("Initializing SD card communications..."));
    #endif
    if (!SD.begin(SD_LINE)) {
      #if DEBUG > 0
        Serial.println(F(" initialization failed!"));
      #endif
      return;
    }
    #if DEBUG > 0
      Serial.println(F(" initialization done."));
    #endif
  
    if (SD.exists(SCORE_FILE_NAME)) {
      SD.remove(SCORE_FILE_NAME);
    }

    // Create a new file
    scoreFile = SD.open(SCORE_FILE_NAME, O_CREAT | O_WRITE);

    // If successful:
    if (scoreFile) {
      #if DEBUG > 0
        Serial.print(F("Writing high score..."));
      #endif

      // ltoa apparently converts longs to char arrays, if it doesn't work switch to String(currentScore)
      ltoa(currentScore,charBuf,10);
      scoreFile.println(charBuf); // String(currentScore)
  
      // Close (and flush) the file
      scoreFile.close();
      
      #if DEBUG > 0
        Serial.println(F(" done."));
      #endif

      bestScore = currentScore;
    } else {
      // if the file didn't open, print an error:
      #if DEBUG > 0
        Serial.println(F("Error opening high score file."));
      #endif
    }
  }
}

int getHighScore()
{
  #if DEBUG > 0
    Serial.print(F("Initializing SD card communications..."));
  #endif
  if (!SD.begin(SD_LINE)) {
    #if DEBUG > 0
      Serial.println(F(" initialization failed!"));
    #endif
    return 0;
  }
  #if DEBUG > 0
    Serial.println(F(" initialization done."));
  #endif

  // If there is no high score file, return 0
  if (!SD.exists(SCORE_FILE_NAME)) {
    return 0;
  }

  scoreFile = SD.open(SCORE_FILE_NAME, O_READ);

  // If successful:
  if (scoreFile) {
    #if DEBUG > 0
      Serial.print(F("Reading high score..."));
    #endif

    int readScore;
    while (scoreFile.available()) {
      // atol is a C++ function that converts strings to a long, if it doesn't work, try stol or String.toInt() (no toLong())
      readScore = atol(scoreFile.read());//.toInt(); // This needs to be tested
    }

    // Close the file
    scoreFile.close();
    
    #if DEBUG > 0
      Serial.println(F(" done."));
    #endif

    bestScore = currentScore;
  } else {
    // if the file didn't open, print an error:
    #if DEBUG > 0
      Serial.println(F("Error opening high score file."));
    #endif
  }
}

// Can this be split into two methods to improve performance?
// (The high score only needs to be updated sometimes.)
void drawScore()
{
  lcd.setCursor(0,0);
  //  F("some string literal") is a provided macro that moves strings to flash memory,
  //  which helps if memory becomes a problem.
  lcd.print(F("BEST"));
  drawNumberOn(bestScore, 0);
  
  lcd.setCursor(0,1);
  //  F("some string literal") is a provided macro that moves strings to flash memory,
  //  which helps if memory becomes a problem.
  lcd.print(F("SCORE"));
  drawNumberOn(currentScore, 1);
}

inline void drawNumberOn(long number, int row)
{
  // ltoa would be better, but unsure how to best use it
  // Since it needs to be printed, it is fastest to geth the length this way I think
  String asString = String(number);
  lcd.setCursor(16 - asString.length(),row);
  lcd.print(asString);
}

void clearScore()
{
  //  F("some string literal") is a provided macro that moves strings to flash memory,
  //  which helps if memory becomes a problem.
  lcd.setCursor(0,0);
  lcd.print(F("                "));
  lcd.setCursor(0,1);
  lcd.print(F("                "));
}

void sendNumber(int pins[], int pinSize, int num)
{
  for (int i = 0; i < pinSize; i++)
  {
    digitalWrite(pins[i], bitRead(num, i));
  }
}

// draw current display board to serial monitor
// is not a clean function, but is functional for debugging
// Preprocessor commands added for safety
void debugDraw(int paddle, int x, int y)
{
  #if DEBUG > 0
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
  #endif
}

