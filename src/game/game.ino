// score display output

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Debug Mode Defines
#define DEBUG 0
#define CALOUT 0
// 1 for on, 0 for off

// Pin Defines
#define RND_NOISE      A0
#define JOY_X          A1
#define JOY_Y          A2
#define DATA_READY     3
#define TONE_PIN       9
#define SND_TX         18
#define SND_RX         19
#define QUIET_BUTTON   5
#define RESTART_BUTTON 52
#define START_BUTTON   53
// Also not usable:
// SDA = 20, SCL = 21

// Array Constants
#define N_PADDLE_PINS 5
#define N_POSITION_PINS 3

// Joystick Input
#define X_CENTRE 515
#define Y_CENTRE 520
#define J_WINDOW 50
#define X_C_HIGH X_CENTRE+J_WINDOW
#define X_C_LOW  X_CENTRE-J_WINDOW
#define Y_C_HIGH Y_CENTRE+J_WINDOW
#define Y_C_LOW  Y_CENTRE-J_WINDOW

// Drawing Constants
#define FRAME_COUNT 20
#define FRAME_DELAY 10

// Sound defines

// Sound files
#define SND_HIT       0x0101
#define SND_MISS      0x0102
#define SND_HIGHSCORE 0x0103
#define SND_LOSE      0x0104
#define SND_START     0x0105
#define SND_WAIT      0x0106
#define SND_MUSIC     0x0201
#define SND_FOLDER    0x0200

// Volume levels
#define SND_VOLUME 0x18
#define SND_VOLUME_QUIET 0x10

// Quiet levels
#define QUIET_LEVELS      5
#define QUIET_LEVEL_NONE  4
#define QUIET_LEVEL_TONES 3
#define QUIET_LEVEL_NWAIT 2
#define QUIET_LEVEL_LVOL  1
#define QUIET_LEVEL_OFF   0

// Tones, set identical to original pong game
#define TONE_WALL 226
#define DUR_WALL  16
#define TONE_HIT  459
#define DUR_HIT   96
#define TONE_MISS 490
#define DUR_MISS  257

// Modes
#define GAME_MODE_PLAY 0
#define GAME_MODE_WAIT 1

// MP3 Defines
/************Command byte**************************/
/*basic commands*/
#define CMD_PLAY  0X01
#define CMD_PAUSE 0X02
#define CMD_NEXT_SONG 0X03
#define CMD_PREV_SONG 0X04
#define CMD_VOLUME_UP   0X05
#define CMD_VOLUME_DOWN 0X06
#define CMD_FORWARD 0X0A // >>
#define CMD_REWIND  0X0B // <<
#define CMD_STOP 0X0E
#define CMD_STOP_INJECT 0X0F//stop interruptting with a song, just stop the interlude
  
/*5 bytes commands*/
#define CMD_SEL_DEV 0X35
  #define DEV_TF 0X01
#define CMD_IC_MODE 0X35
  #define CMD_SLEEP   0X03
  #define CMD_WAKE_UP 0X02
  #define CMD_RESET   0X05
  
/*6 bytes commands*/  
#define CMD_PLAY_W_INDEX   0X41
#define CMD_PLAY_FILE_NAME 0X42
#define CMD_INJECT_W_INDEX 0X43
  
/*Special commands*/
#define CMD_SET_VOLUME 0X31
#define CMD_PLAY_W_VOL 0X31
  
#define CMD_SET_PLAY_MODE 0X33
  #define ALL_CYCLE 0X00
  #define SINGLE_CYCLE 0X01
  
#define CMD_PLAY_COMBINE 0X45//can play combination up to 15 songs

// MP3 stuff
static int8_t Send_buf[6] = {0};
void sendCommand(int8_t command, int16_t dat );

// Components
LiquidCrystal_I2C lcd(0x38, 16, 2);

// Display output pins
// These decide where to display things
// paddlePins is the bits for a number 0-27
// posPins is for 0-8 on both the X and Y axis

const int paddlePins[5] = {22,24,26,28,30};
const int posPinsX[3] = {23,25,27};
const int posPinsY[3] = {29,31,33};

// Score variables
long currentScore;
long bestScore;

// Joystick input
float angle;
int discreteAngle;

// Framerate counter
int frameCounter;

// Joystick position
int x;
int y;

// Ball position
int ballx;
int bally;
int balldir;

// State variables
int mode;
int quietLevel;

void setup() {
  #if DEBUG > 0 || CALOUT > 0
    // If DEBUG is defined to 1 in the defines (top) setup the serial out for debugging
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
  #endif
  
  // Setup paddle reading pins
  for (int i = 0; i < N_PADDLE_PINS; i++)
  {
    pinMode(paddlePins[i], OUTPUT);
  }
  // Setup position reading pins
  for (int i = 0; i < N_POSITION_PINS; i++)
  {
    pinMode(posPinsX[i], OUTPUT);
    pinMode(posPinsY[i], OUTPUT);
  }

  // Setup display communication pin
  pinMode(DATA_READY, OUTPUT);

  // Setup buttons
  pinMode(START_BUTTON, INPUT);
  pinMode(RESTART_BUTTON, INPUT);
  pinMode(QUIET_BUTTON, INPUT);

  // Setup scoreboard
  lcd.init();
  lcd.backlight();
  bestScore = 0;
  drawHighScore();
  
  // Setup audio
  
  // MP3 uses Serial1
  Serial1.begin(9600);
  delay(500);
  sendCommand(CMD_SEL_DEV, DEV_TF);
  delay(500);
  setVolume(SND_VOLUME);
  delay(500);
  
  // Passive Buzzer for Pong sounds
  pinMode(TONE_PIN, OUTPUT);

  // Initialize quiet level
  quietLevel = 0;

  // Setup random generator
  randomSeed(analogRead(RND_NOISE));

  // Initialize joysitck values
  angle = 0.0;
  discreteAngle = 0;

  // game setup
  startWaiting();
}

void loop() {
  // Read in joystick position
  x = analogRead(JOY_X);
  y = analogRead(JOY_Y);

  #if CALOUT > 0
    Serial.println("Joystick: "+String(x)+" "+String(y));
  #endif
  
  // Determin paddle position
  if ((x > X_C_HIGH || x < X_C_LOW) || (y > Y_C_HIGH || y < Y_C_LOW))
  {
    // Find angle
    angle = atan2(x-X_CENTRE,y-Y_CENTRE); // -pi to pi
    discreteAngle = (int)(53 - (angle + 3.14159) * 4.45634) % 28; //means: (int)(25 - (angle + 3.14) / 6.28 * 28 + 28) % 28;
  }

  if (debounced(QUIET_BUTTON))
  {
    // Set quiet level and volume (as necessary)
    quietLevel = (quietLevel + 1) % QUIET_LEVELS;
    if (quietLevel == QUIET_LEVEL_OFF)
    {
      setVolume(SND_VOLUME);
      if (isWaiting) playWithFolder(SND_MUSIC);
    }
    else if (quietLevel == QUIET_LEVEL_LVOL) setVolume(SND_VOLUME_QUIET);
    else if (quietLevel == QUIET_LEVEL_TONES || quietLevel == QUIET_LEVEL_NWAIT && isWaiting()) sendCommand(CMD_STOP);

    #if DEBUG > 0
      Serial.println("Quiet level: "+String(quietLevel));
    #endif
  }
  
  // Check for (re)start or run the game
  if (debounced(START_BUTTON) || (isWaiting() && debouncedNeg(RESTART_BUTTON)))
  {
    gameStart();
  }
  else
  {
    // Send
    sendNumber(paddlePins, 5, discreteAngle);
    sendNumber(posPinsX, 3, ballx);
    sendNumber(posPinsY, 3, bally);

    // Advize of new information
    digitalWrite(DATA_READY, HIGH);
    delay(2);
    digitalWrite(DATA_READY, LOW);
  
    // Update state
    frameCounter++;
    if (frameCounter > FRAME_COUNT)
    {
      frameCounter = 0; // Reinitialize frame count

      if (isWaiting())
      {
          doWaiting(); // Change waiting display
      }
      else
      {
        // Determine where the ball hit & if the paddle is in range
        // Each corner & each side has their own opposing angle, that will have variance added on
        if ((ballx == 1 && bally == 1) && (discreteAngle <= 2 || discreteAngle >= 26))
        {
          hit(); // Register hit
          balldir = nextDirection(1); // Reflect
        }
        else if ((ballx == 6 && bally == 1) && (discreteAngle >= 5 && discreteAngle <= 9))
        {
          hit(); // Register hit
          balldir = nextDirection(3); // Reflect
        }
        else if ((ballx == 6 && bally == 6) && (discreteAngle >= 12 && discreteAngle <= 16))
        {
          hit(); // Register hit
          balldir = nextDirection(5); // Reflect
        }
        else if ((ballx == 1 && bally == 6) && (discreteAngle >= 19 && discreteAngle <= 23))
        {
          hit(); // Register hit
          balldir = nextDirection(7); // Reflect
        }
        else if (ballx == 1 && ((discreteAngle >= 28 - bally - 2 && discreteAngle <= 28 - bally + 2)
                                || (discreteAngle == 0 && bally == 2)))
        {
          hit(); // Register hit
          balldir = nextDirection(0); // Reflect
        }
        else if (bally == 1 && (discreteAngle >= ballx - 2 && discreteAngle <= ballx + 2))
        {
          hit(); // Register hit
          balldir = nextDirection(2); // Reflect
        }
        else if (ballx == 6 && (discreteAngle >= 7 + bally - 2 && discreteAngle <= 7 + bally + 2))
        {
          hit(); // Register hit
          balldir = nextDirection(4); // Reflect
        }
        else if (bally == 6 && (discreteAngle >= 21 - ballx - 2 && discreteAngle <= 21 - ballx + 2))
        {
          hit(); // Register hit
          balldir = nextDirection(6); // Reflect
        }
        else if (ballx == 1 || bally == 1 || ballx == 6 || bally == 6)
        {
          miss(); // Register miss
          
          if (currentScore > bestScore)
            highscore(); // Register highscore
          else
            loss(); // Register loss
          
          // Debug output
          #if DEBUG > 0
            Serial.println("x:" + String(ballx));
            Serial.println("y:" + String(bally));
          #endif

          // Move to waiting mode
          startWaiting();
        }

        // Move the ball
        switch (balldir)
        {
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
        
        // Debug output
        #if DEBUG > 0
          debugDraw(discreteAngle, ballx, bally);
        #endif
      }
    }
  }

  // Keep the game sufficiently slow
  delay(FRAME_DELAY);
}

// Run helper methods

// Determine the ball's direction, with variance
// Given the "perfect" direction for where it hit as its input
inline int nextDirection(int in)
{
  return (in + random(-1,2) + 8) % 8;
}

// Play hit tone
inline void hit()
{
  if (quietLevel < QUIET_LEVEL_NONE)
  {
    // Tone identical to original Pong game
    tone(TONE_PIN, TONE_HIT);
    delay(DUR_HIT);
    noTone(TONE_PIN);
    
    //playWithFolder(SND_HIT);
    //delay(100);
  } else {
    delay(DUR_HIT); 
  }
  currentScore++;
  drawScore();
}

// Play miss tone
inline void miss()
{
  if (quietLevel < QUIET_LEVEL_NONE)
  {
    // Tone identical to original Pong game
    tone(TONE_PIN, TONE_MISS);
    delay(DUR_MISS);
    noTone(TONE_PIN);
  } else {
    delay(DUR_MISS); 
  }
}

// Register high score
inline void highscore()
{
  // Play loss sound
  if (quietLevel < QUIET_LEVEL_TONES) playWithFolder(SND_HIGHSCORE);

  // Redraw highscore
  clearHighScore();
  bestScore = currentScore;
  drawHighScore();

  // Delay
  delay(2000);
}

// Register loss
inline void loss()
{
  // Play loss sound
  if (quietLevel < QUIET_LEVEL_TONES) playWithFolder(SND_LOSE);

  // Delay
  delay(1000);
}

// Debounce button
inline boolean debounced(int pin)
{
  if (digitalRead(pin))
  {
    delay(100);
    return digitalRead(pin);
  }
  return false;
}

// Debounce button
inline boolean debouncedNeg(int pin)
{
  if (!digitalRead(pin))
  {
    delay(100);
    return !digitalRead(pin);
  }
  return false;
}

// If is waiting
inline boolean isWaiting()
{
  return mode == GAME_MODE_WAIT;
}

// If is running a game
inline boolean isRunning()
{
  return mode == GAME_MODE_PLAY;
}

// Game modes

// Start new game
void gameStart()
{
  // If waiting, ensure tat it is finished
  if (isWaiting()) stopWaiting();
  
  // Play start sound
  if (quietLevel < QUIET_LEVEL_TONES) playWithFolder(SND_START);
  
  // Clear the display and draw the score
  clearScore();
  currentScore = 0;
  drawScore();
  
  // Initialize frame counter
  frameCounter = 0;

  // Initialize positions
  ballx = 3; // random(3,5);
  bally = 3; // random(3,5);
  balldir = 0; // random(0,8);

  // Set mode
  mode = GAME_MODE_PLAY;

  // Give the player a chance to start
  delay(1000);
}

// Start waiting for a new game
void startWaiting()
{
  // Start waiting music (Composed by C. Vessey :))
  if (quietLevel < QUIET_LEVEL_NWAIT) playWithFolder(SND_MUSIC);

  // Tried and failed:
  //playWithFolder(SND_MUSIC);
  //setCyleMode(ALL_CYCLE);
  //setCyleMode(SINGLE_CYCLE);

  // Clear the score and draw the waiting message
  clearScore();
  currentScore = 0;
  drawWaiting();

  // Initialize frame counter
  frameCounter = 0;

  // Initialize positions
  ballx = random(3,5);
  bally = random(3,5);
  balldir = 0;

  // Set mode
  mode = GAME_MODE_WAIT;
}

// Increment waiting
void doWaiting()
{
  // Set new random position
  ballx = random(1,7); //((ballx + random(0,2) + 7) % 6) + 1;
  bally = random(1,7); //((bally + random(0,2) + 11) % 6) + 1;
}

// Stop waiting
void stopWaiting()
{
  sendCommand(CMD_STOP);
}


// LCD methods

// Clear the player score
inline void clearScore()
{
  lcd.setCursor(0,1);
  lcd.print(F("                "));
}

// Clear the high score
inline void clearHighScore()
{
  lcd.setCursor(0,0);
  lcd.print(F("                "));
}

// Draw the player score
inline void drawScore()
{
  lcd.setCursor(0,1);
  lcd.print(F("SCORE"));
  drawNumberOn(currentScore, 1);
}

// Draw the high score
inline void drawHighScore()
{
  lcd.setCursor(0,0);
  lcd.print(F("BEST"));
  drawNumberOn(bestScore, 0);
}

// Draw the waiting message
inline void drawWaiting()
{
  lcd.setCursor(0,1);
  lcd.print(F("Press start now!"));
}

// Draws a number on the given row of the LCD screen
inline void drawNumberOn(long number, int row)
{
  // ltoa would be better, but unsure how to best use it
  String asString = String(number);
  lcd.setCursor(16 - asString.length(),row);
  lcd.print(asString);
}

// Send a value to the given pins as a binary number
inline void sendNumber(int pins[], int pinSize, int num)
{
  for (int i = 0; i < pinSize; i++)
  {
    digitalWrite(pins[i], bitRead(num, i));
  }
}

// Debug functions

// Draws current display board to serial monitor
// Is not a clean function, but is functional for debugging
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

// MP3 Methods

inline void setVolume(int8_t vol)
{
  mp3_5bytes(CMD_SET_VOLUME, vol);
}

inline void playWithFolderAndVolume(int16_t dat, int8_t vol)
{
  // dat represents the command portion after Play with Folder and Filename
  setVolume(vol); // call helper method, above
  mp3_6bytes(CMD_PLAY_FILE_NAME, dat); // play as directed
}

inline void playWithFolder(int16_t dat)
{
  mp3_6bytes(CMD_PLAY_FILE_NAME, dat);
}

inline void playWithVolume(int16_t dat)
{
  mp3_6bytes(CMD_PLAY_W_VOL, dat);
}

/*cycle play with an index*/
inline void cyclePlay(int16_t index)
{
  mp3_6bytes(CMD_SET_PLAY_MODE,index);
}

inline void setCyleMode(int8_t AllSingle)
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

void sendCommand(int8_t command)
{
  delay(20);
  if((command == CMD_PLAY_W_VOL)||(command == CMD_SET_PLAY_MODE)||(command == CMD_PLAY_COMBINE))
    return;
  else if(command < 0x10) 
  {
  mp3Basic(command);
  }
  else return;
 
}

void sendCommand(int8_t command, int16_t dat)
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
inline void sendBytes(uint8_t nbytes)
{
  for(uint8_t i=0; i < nbytes; i++)//
  {
    Serial1.write(Send_buf[i]) ;
  }
}

