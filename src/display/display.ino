// inputting the position, these arrays will give you what column & row to enable
// hori is for the top and bottom display cycle, verti is for the left and right sides
// position goes clockwise from the topright corner, 0-27
// -1 is for not enabling anything
int horiColumns[28] = {0,1,2,3,4,5,6,7,-1,-1,-1,-1,-1,-1,7,6,5,4,3,2,1,0,-1,-1,-1,-1,-1,-1};
int horiRows[28] = {0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,7,7,7,7,7,7,7,7,-1,-1,-1,-1,-1,-1};

int vertiColumns[28] = {0,-1,-1,-1,-1,-1,-1,7,7,7,7,7,7,7,7,-1,-1,-1,-1,-1,-1,0,0,0,0,0,0,0};
int vertiRows[28] = {0,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,-1,-1,-1,-1,-1,-1,7,6,5,4,3,2,1};

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
