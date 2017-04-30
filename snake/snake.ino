#include "LedControl.h"

/*
 Initialize the LedControl Module
 pin 12 is connected to the DataIn 
 pin 11 is connected to LOAD(CS)
 pin 10 is connected to the CLK 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(12,10,11,1);

// Pin numbers for the Joystick
const int SW_pin = 2; // digital pin connected to switch output
const int X_pin = 0; // analog pin connected to X output
const int Y_pin = 1; // analog pin connected to Y output

// Array holding the positions of the snake
int snake[64][2] = { {0, 0}, {1, 0} };
// index of the snake head in the snake array
int snakeHead = 1;
// index of the snake tail in the snake array
int snakeTail = 0;

// -1, 0, 1 number, showing the movement in the X axis
int snakeMoveX = 1;
// -1, 0, 1 number, showing the movement in the Y axis
int snakeMoveY = 0;

// position of the cherry that the snake can eat
int cherry[2] = { 0, 0 };

void setup() {
  randomSeed(analogRead(5));

  // have a serial for debugging
  Serial.begin(9600);
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,3);
  // start the game
  startGame();
}

/*
 * Detects a move of the joystick and update
 * the snakeMoveX and snakeMoveY variables. 
 * The function allows to snake only to change its direction, 
 * but not to move backward.
 * Returns true, if a move from joystick was detected.
 */
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

/*
 * Generates a new cherry on a random position outside of the snake.
 * The new cherry coordinates are stored in cherry variable.
 */
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

/*
 * Starts a new game (initializes all variables)
 */
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

/*
 * Restarts the game (show a cross and call startGame)
 */
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

/*
 * Moves the snake - remove tail, add head.
 * Also detects if the snake doesn't hit itself or when it eats a cherry.
 */
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
  // check if we have not eaten a cherry
  if (snake[snakeHead][0] != cherry[0] || snake[snakeHead][1] != cherry [1]) {
    snakeTail = (snakeTail + 1) % 64;
  } else {
    // cherry was eaten, generate new cherry and don't move tail
    generateCherry();
  }
  return false;
  
}

/*
 * Draws the whole snake on the screen.
 */
void drawSnake() {
  for (int snakeIndex = snakeTail; snakeIndex != (snakeHead + 1) % 64; snakeIndex = (snakeIndex + 1) % 64) {
    int snakePoint[2] = { snake[snakeIndex][0], snake[snakeIndex][1] };

    lc.setLed(0, snakePoint[0], snakePoint[1], true);
  }
}

// Draws the cherry
void drawCherry(bool on) {
  lc.setLed(0, cherry[0], cherry[1], on);
}

/*
 * Main arduino loop
 */
void loop() {

  lc.clearDisplay(0);

  drawCherry(true);
  if (moveSnake()) {
    // if we have restarted, start the function over
    return;
  }
  
  drawSnake();

  // detect if the user hasn't changed the direction
  bool moveMade = false;
  for (int i = 0; i < 5; i++) {
    // make the cherry blink
    if (i == 3) {
      drawCherry(false);
    }
    if (!moveMade) {
      moveMade = detectMove();
    }
    delay(100);
  }
}
