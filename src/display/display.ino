// 8x8 display pins
// on display component, 9-16 are top row (side with label), 8-1 are bottom row
// COMPONENT:  1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
// PIN NUMBER: 52  50  48  46  44  42  40  38  39  41  43  45  47  49  51  53
// ROW / COL:  c1  c2  r7  c8  r8  c5  c3  r5  r6  r3  c4  r1  c6  c7  r2  r4
// TODO: make the comment pin assignments reflect the ones used in code
const int row[8] = {45,51,41,53,44,39,48,40};
const int col[8] = {52,50,42,43,38,47,49,46};

// inputting the position, these arrays will give you what column/row to enable
// position goes clockwise from the topright corner, 0-27
// -1 is for not enabling anything

const int horiColumns[28] = {0,1,2,3,4,5,6,7,-1,-1,-1,-1,-1,-1,7,6,5,4,3,2,1,0,-1,-1,-1,-1,-1,-1};
const int horiRows[28] = {0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,7,7,7,7,7,7,7,7,-1,-1,-1,-1,-1,-1};

const int vertiColumns[28] = {0,-1,-1,-1,-1,-1,-1,7,7,7,7,7,7,7,7,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0};
const int vertiRows[28] = {0,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,-1,-1,-1,-1,-1,-1,7,6,5,4,3,2,1};


void setup() {
  // setup 8x8 pins for output
  for (int i = 0; i < 8; i++)
  {
    pinMode(row[i], OUTPUT);
    pinMode(col[i], OUTPUT);
  }
}

void loop() {
  for (int i = 0; i < 28; i++)
  {
    for (int j = 0; j < 1000; j++)
    {
      draw(i,3 + i % 2,3 + i % 2);
    }
  }
}

void draw(int paddlePos, int pointX, int pointY) {
  clearDisplay();
  drawPaddleX((paddlePos + 26) % 28);
  drawPaddleX((paddlePos + 27) % 28);
  drawPaddleX(paddlePos);
  drawPaddleX((paddlePos + 1) % 28);
  drawPaddleX((paddlePos + 2) % 28);
  delayMicroseconds(10);
  
  clearDisplay();
  drawPaddleY((paddlePos + 26) % 28);
  drawPaddleY((paddlePos + 27) % 28);
  drawPaddleY(paddlePos);
  drawPaddleY((paddlePos + 1) % 28);
  drawPaddleY((paddlePos + 2) % 28);
  delayMicroseconds(10);

  clearDisplay();
  drawPoint(pointX,pointY);
}

void clearDisplay() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(col[i], HIGH);
    digitalWrite(row[i], LOW);
  }
}

void drawPaddleX(int pos) {
  if (horiColumns[pos] != -1 && horiRows[pos] != -1)
    drawPoint(horiColumns[pos], horiRows[pos]);
}
void drawPaddleY(int pos) {
  if (vertiColumns[pos] != -1 && vertiRows[pos] != -1)
    drawPoint(vertiColumns[pos], vertiRows[pos]);
}

void drawPoint(int x, int y) {
  digitalWrite(col[x], LOW);
  digitalWrite(row[y], HIGH);
}

void displayBorderX() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(row[i], HIGH);
  }

  digitalWrite(col[0], LOW);
  digitalWrite(col[7], LOW);
}
void displayBorderY() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(col[i], LOW);
  }

  digitalWrite(row[0], HIGH);
  digitalWrite(row[7], HIGH);
}
