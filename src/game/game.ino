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
}

void loop() {
  for (int i = 0; i < 28; i++)
  {
    sendNumber(paddlePins, 5, i);
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

