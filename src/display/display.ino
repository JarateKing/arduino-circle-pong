// Pin Defines
#define DATA_READY 3

// Input pins
// These decide where to display things
// paddlePins is the bits for a number 0-27
// posPins is for 0-8 on both the X and Y axis

const int paddlePins[5] = {22,24,26,28,30};
const int posPinsX[3] = {23,25,27};
const int posPinsY[3] = {29,31,33};

// 8x8 display pins
// on display component, 9-16 are top row (side with label), 8-1 are bottom row
// COMPONENT:  1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
// PIN NUMBER: 52  50  48  46  44  42  40  38  39  41  43  45  47  49  51  53
// ROW / COL:  c1  c2  r7  c8  r8  c5  c3  r5  r6  r3  c4  r1  c6  c7  r2  r4

// Row and column pins
const int row[8] = {45,51,41,53,44,39,48,40};
const int col[8] = {52,50,42,43,38,47,49,46};

// Inputting the position, these arrays will give you what column/row to enable
// Position goes clockwise from the topright corner, 0-27
// -1 is for not enabling anything
const int horiColumns[28] = {0,1,2,3,4,5,6,7,-1,-1,-1,-1,-1,-1,7,6,5,4,3,2,1,0,-1,-1,-1,-1,-1,-1};
const int horiRows[28] = {0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,7,7,7,7,7,7,7,7,-1,-1,-1,-1,-1,-1};
const int vertiColumns[28] = {0,-1,-1,-1,-1,-1,-1,7,7,7,7,7,7,7,7,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0};
const int vertiRows[28] = {0,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,-1,-1,-1,-1,-1,-1,7,6,5,4,3,2,1};

// Position variables
int paddle = 0;
int ballx = 0;
int bally = 0;

void setup() {
  // Setup 8x8 pins for output
  for (int i = 0; i < 8; i++)
  {
    pinMode(row[i], OUTPUT);
    pinMode(col[i], OUTPUT);
  }
  // Setup paddle reading pins
  for (int i = 0; i < 5; i++)
  {
    pinMode(paddlePins[i], INPUT);
  }
  // Setup position reading pins
  for (int i = 0; i < 3; i++)
  {
    pinMode(posPinsX[i], INPUT);
    pinMode(posPinsY[i], INPUT);
  }
  
  pinMode(DATA_READY, INPUT);
}

void loop() {
  if (digitalRead(DATA_READY))
  {
    paddle = convertPins(paddlePins, 5);
    ballx = convertPins(posPinsX, 3);
    bally = convertPins(posPinsY, 3);
  }
  
  draw(paddle,ballx,bally);
}

// Converts an encoded binary number to a value
inline int convertPins(int pins[], int pinSize)
{
  int i = pinSize-1;
  int result = digitalRead(pins[i--]);
  for (; i >= 0; i--)
  {
    result = (result << 1) | digitalRead(pins[i]);
  }
  return result;
}

// Draws the given coordinates
inline void draw(int paddlePos, int pointX, int pointY) {
  clearDisplay();
  drawPaddleX((paddlePos + 26) % 28);
  drawPaddleX((paddlePos + 27) % 28);
  drawPaddleX(paddlePos);
  drawPaddleX((paddlePos + 1) % 28);
  drawPaddleX((paddlePos + 2) % 28);
  delayMicroseconds(20);
  
  clearDisplay();
  drawPaddleY((paddlePos + 26) % 28);
  drawPaddleY((paddlePos + 27) % 28);
  drawPaddleY(paddlePos);
  drawPaddleY((paddlePos + 1) % 28);
  drawPaddleY((paddlePos + 2) % 28);
  delayMicroseconds(35);

  clearDisplay();
  drawPoint(pointX,pointY);
  delayMicroseconds(1);
}

// Clears the display
inline void clearDisplay() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(col[i], HIGH);
    digitalWrite(row[i], LOW);
  }
}

// Sets the paddle X value
inline void drawPaddleX(int pos) {
  if (horiColumns[pos] != -1 && horiRows[pos] != -1)
    drawPoint(horiColumns[pos], horiRows[pos]);
}

// Sets the paddle Y value
inline void drawPaddleY(int pos) {
  if (vertiColumns[pos] != -1 && vertiRows[pos] != -1)
    drawPoint(vertiColumns[pos], vertiRows[pos]);
}

// Draws a point
inline void drawPoint(int x, int y) {
  digitalWrite(col[x], LOW);
  digitalWrite(row[y], HIGH);
}

// Displays an X border
inline void displayBorderX() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(row[i], HIGH);
  }

  digitalWrite(col[0], LOW);
  digitalWrite(col[7], LOW);
}

// Displays a Y border
inline void displayBorderY() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(col[i], LOW);
  }

  digitalWrite(row[0], HIGH);
  digitalWrite(row[7], HIGH);
}
