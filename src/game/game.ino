// score display output

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Debug Mode Define
#define DEBUG 1
// 1 for on, 0 for off

// Pin Defines
#define JOY_X A1
#define JOY_Y A2
#define RESTART 3
#define DATA_READY 5
#define RND_NOISE A0
#define SND_RX 8
#define SND_TX 7
// Also not usable:
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

// Component Drivers

LiquidCrystal_I2C lcd(0x38, 16, 2);
SoftwareSerial mp3(SND_RX, SND_TX);

// MP3 Controls
#define SND_START 0x0304
#define SND_HIT 0x0304
#define SND_LOSE 0x0304
#define SND_HIGHSCORE 0x0304

static int8_t Send_buf[6] = {0};
#define CMD_PLAY  0X01
#define CMD_PAUSE 0X02
#define CMD_NEXT_SONG 0X03
#define CMD_PREV_SONG 0X04
#define CMD_VOLUME_UP   0X05
#define CMD_VOLUME_DOWN 0X06
#define CMD_FORWARD 0X0A
#define CMD_REWIND  0X0B
#define CMD_STOP 0X0E
#define CMD_STOP_INJECT 0X0F
#define CMD_SEL_DEV 0X35
#define DEV_TF 0X01
#define CMD_IC_MODE 0X35
#define CMD_SLEEP   0X03
#define CMD_WAKE_UP 0X02
#define CMD_RESET   0X05
#define CMD_PLAY_W_INDEX   0X41
#define CMD_PLAY_FILE_NAME 0X42
#define CMD_INJECT_W_INDEX 0X43
#define CMD_SET_VOLUME 0X31
#define CMD_PLAY_W_VOL 0X31
#define CMD_SET_PLAY_MODE 0X33
#define ALL_CYCLE 0X00
#define SINGLE_CYCLE 0X01
#define CMD_PLAY_COMBINE 0X45
void sendCommand(int8_t command, int16_t dat );

// Math Defines

// score variables
long currentScore;
long bestScore;

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
  bestScore = 0;

  // joystick setup
  angle = 0.0;
  discreteAngle = 0;

  // game setup
  gameStart();

  // rng setup
  randomSeed(analogRead(RND_NOISE));

  // audio setup
  mp3.begin(9600);
  delay(500);
  sendCommand(CMD_SEL_DEV, DEV_TF);
  delay(500);
}

void gameStart()
{
  if (currentScore > bestScore)
    bestScore = currentScore;
  
  clearScore();
  currentScore = 0;
  
  frameCounter = 0;

  ballx = 3;
  bally = 3;
  balldir = 0;

  // start sound
  playWithFolderAndVolume(SND_START, 0x01);
  delay(1000);
  sendCommand(CMD_SEL_DEV, DEV_TF);
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

        if (currentScore > bestScore)
          playWithFolderAndVolume(SND_HIGHSCORE, 0x01);
        else
          playWithFolderAndVolume(SND_LOSE, 0x01);
        
        delay(1000);
        sendCommand(CMD_SEL_DEV, DEV_TF);
        
        // Preprocessor commands for safety
        #if DEBUG > 0
          Serial.println("x:" + String(ballx));
          Serial.println("y:" + String(bally));
        #endif
        
        delay(1000);
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
  playWithFolderAndVolume(SND_HIT, 0x01);
  delay(1000);
  sendCommand(CMD_SEL_DEV, DEV_TF);
  
  return (in + random(-1,2) + 8) % 8;
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

void setVolume(int8_t vol)
{
  mp3_5bytes(CMD_SET_VOLUME, vol);
}

void playWithFolderAndVolume(int16_t dat, int8_t vol)
{
  // dat represents the command portion after Play with Folder and Filename
  setVolume(vol); // call helper method, above
  mp3_6bytes(CMD_PLAY_FILE_NAME, dat); // play as directed
}

void playWithVolume(int16_t dat)
{
  mp3_6bytes(CMD_PLAY_W_VOL, dat);
}

/*cycle play with an index*/
void cyclePlay(int16_t index)
{
  mp3_6bytes(CMD_SET_PLAY_MODE,index);
}

void setCyleMode(int8_t AllSingle)
{
  mp3_5bytes(CMD_SET_PLAY_MODE,AllSingle);
}


void playCombine(int8_t song[][2], int8_t number)
{
  if(number > 15) return;//number of songs combined can not be more than 15
  uint8_t nbytes;//the number of bytes of the command with starting byte and ending byte
  nbytes = 2*number + 4;
  int8_t Send_buf[nbytes];
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = nbytes - 2; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = CMD_PLAY_COMBINE; 
  for(uint8_t i=0; i < number; i++)//
  {
    Send_buf[i*2+3] = song[i][0];
  Send_buf[i*2+4] = song[i][1];
  }
  Send_buf[nbytes - 1] = 0xef;
  sendBytes(nbytes);
}


void sendCommand(int8_t command, int16_t dat = 0)
{
  delay(20);
  if((command == CMD_PLAY_W_VOL)||(command == CMD_SET_PLAY_MODE)||(command == CMD_PLAY_COMBINE))
    return;
  else if(command < 0x10) 
  {
  mp3Basic(command);
  }
  else if(command < 0x40)
  { 
  mp3_5bytes(command, dat);
  }
  else if(command < 0x50)
  { 
  mp3_6bytes(command, dat);
  }
  else return;
 
}

void mp3Basic(int8_t command)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x02; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = 0xef; //
  sendBytes(4);
}
void mp3_5bytes(int8_t command, uint8_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x03; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = dat; //
  Send_buf[4] = 0xef; //
  sendBytes(5);
}
void mp3_6bytes(int8_t command, int16_t dat)
{
  Send_buf[0] = 0x7e; //starting byte
  Send_buf[1] = 0x04; //the number of bytes of the command without starting byte and ending byte
  Send_buf[2] = command; 
  Send_buf[3] = (int8_t)(dat >> 8);//data high-order byte (shifted)
  Send_buf[4] = (int8_t)(dat); //data low-order byte
  Send_buf[5] = 0xef; //
  sendBytes(6);
}
void sendBytes(uint8_t nbytes)
{
  for(uint8_t i=0; i < nbytes; i++)//
  {
    mp3.write(Send_buf[i]) ;
  }
}

