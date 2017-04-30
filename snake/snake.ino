//www.elegoo.com
//2016.12.9

//We always have to include the library
#include "LedControl.h"

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to LOAD(CS)
 pin 10 is connected to the CLK 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(12,10,11,1);

/* we always wait a bit between updates of the display */
unsigned long delaytime1=500;
unsigned long delaytime2=50;
unsigned long delaytime3=300;

// Arduino pin numbers
const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = 0; // analog pin connected to X output
const int Y_pin = 1; // analog pin connected to Y output

int snake[64][2] = { {0, 0}, {1, 0} };
int snakeHead = 1;
int snakeTail = 0;

int snakeMoveX = 1;
int snakeMoveY = 0;

int cherry[2] = { 0, 0 };

// lc.setLed(0, row, col, turnOn);
void setup() {
  randomSeed(analogRead(0));

  Serial.begin(9600);
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,3);
  /* and clear the display */
  startGame();
}

bool detectMove() {
  int x = analogRead(X_pin);
  int y = analogRead(Y_pin);

  int moveX = 0;
  int moveY = 0;
  
  if (x > 600) {
    // move down
    moveX = 1;
  }

  if (x < 400) {
    moveX = -1;
  }

  if (y > 600) {
    // move down
    moveY = 1;
  }

  if (y < 400) {
    moveY = -1;
  }

 
  if (snakeMoveY != 0 && moveX != 0) {
    snakeMoveX = moveX;
    snakeMoveY = 0;
    return true;
  } else if (snakeMoveX != 0 && moveY != 0) {
    snakeMoveY = - moveY;
    snakeMoveX = 0;
    return true;
  }   
  return false;
}

void generateCherry() {
  boolean generated = false;
  do {
    cherry[0] = random(8);
    cherry[1] = random(8);

    generated = true;
    // check if cherry wasn't generated into the snake
    for (int snakeIndex = snakeTail; snakeIndex != (snakeHead + 1) % 64; snakeIndex = (snakeIndex + 1) % 64) {
      int snakePoint[2] = { snake[snakeIndex][0], snake[snakeIndex][1] };

      if (snakePoint[0] == cherry[0] && snakePoint[1] == cherry[1]) {
        generated = false;
      }
    }  
  } while(!generated);

  Serial.print("Cherry at position: ");  
  Serial.print(cherry[0]);  
  Serial.print(", ");  
  Serial.println(cherry[1]);  
}

void startGame() {
  snakeMoveX = 1;
  snakeMoveY = 0;

  snake[0][0] = 0;
  snake[0][1] = 0;
  snake[1][0] = 1;
  snake[1][1] = 0;
  snakeHead = 1;
  snakeTail = 0;

  generateCherry();
}

void restartGame() {
  lc.setRow(0,0,B10000001);
  lc.setRow(0,1,B01000010);
  lc.setRow(0,2,B00100100);
  lc.setRow(0,3,B00011000);
  lc.setRow(0,4,B00011000);
  lc.setRow(0,5,B00100100);
  lc.setRow(0,6,B01000010);
  lc.setRow(0,7,B10000001);

  delay(1000);
  startGame();
}


bool moveSnake() {
  
  int snakePoint[2] = { snake[snakeHead][0], snake[snakeHead][1] };
  snakeHead = (snakeHead + 1) % 64;

  snake[snakeHead][0] = (snakePoint[0] + snakeMoveX + 8) % 8;
  snake[snakeHead][1] = (snakePoint[1] + snakeMoveY + 8) % 8;

  // check if snake did not hit itself
  for (int snakeIndex = snakeTail; snakeIndex != (snakeHead) % 64; snakeIndex = (snakeIndex + 1) % 64) {
    int snakePoint[2] = { snake[snakeIndex][0], snake[snakeIndex][1] };

    if (snakePoint[0] == snake[snakeHead][0] && snakePoint[1] == snake[snakeHead][1]) {
      // snake has hit itself
      restartGame();
      return true;
    }
  }
  if (snake[snakeHead][0] != cherry[0] || snake[snakeHead][1] != cherry [1]) {
    snakeTail = (snakeTail + 1) % 64;
  } else {
    // cherry was eaten, generate new cherry
    generateCherry();
  }
  return false;
  
}
// int snake[64][2] = { {0, 0}, {0, 1} };
// int snakeHead = 1;
// int snakeTail = 0;
void drawSnake() {
  for (int snakeIndex = snakeTail; snakeIndex != (snakeHead + 1) % 64; snakeIndex = (snakeIndex + 1) % 64) {
    int snakePoint[2] = { snake[snakeIndex][0], snake[snakeIndex][1] };

    lc.setLed(0, snakePoint[0], snakePoint[1], true);
  }
}

void drawCherry(bool on) {
  lc.setLed(0, cherry[0], cherry[1], on);
}

void loop() {

  lc.clearDisplay(0);

  drawCherry(true);
  if (moveSnake()) {
    // if we have restarted, start the function over
    return;
  }
  drawSnake();
  
  bool moveMade = false;
  for (int i = 0; i < 5; i++) {
    if (i == 3) {
      drawCherry(false);
    }
    if (!moveMade) {
      moveMade = detectMove();
    }
    delay(100);
  }
}
