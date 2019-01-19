/*
 * Matthew Alp
 * TEJ4M
 * Mr. Naccarrato
 * January 21st, 2019
 * This program runs a simple game of Pong on an Arduino Uno,
 * which is connected to 2 shift registers, a pair of buttons,
 * and an 8x8 LED matrix. The player competes against an AI paddle
 * by controlling his own through the two buttons. Upon either paddle
 * missing the ball, the matrix will light up on the winner's side,
 * after which the game will reset.
 */


// A DEBUG macro for Serial.print debugging purposes
#define DEBUG 1

//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 10;
////Pin connected to DS of 74HC595
int dataPin = 11;
//unsigned int datablargh = 65535;

int redPin = 4;
int greenPin = 5;

/*
 * X is columns, Y is rows, and everything is flipped
 * The direction variables determine whether the ball goes up/down or left/right
 * the X and Y coordinates both range from 0 to 7 (inclusive)
 */
int ballX = 3;
int ballY;
int ballYDirection = 1;
int ballXDirection = 1;
int aiX = 3;
int aiY = 7;
int playerX = 3;
int playerY = 0;

/*
 * WAIT is the amount of milliseconds to wait between turning a row on/off
 * LOOP_CHECK is the number of loops that need to pass for button input to 
 * be taken, and allows the input rate to be independent of the matrix
 * refresh rate
 * loops is used as a counter for how many times loop() has been called
 * gameOver is used to determine whether or not someone has lost and
 * a winner must be displayed
 */
const short WAIT = 6;
const short LOOP_CHECK = 12;
long loops = 0;
bool gameOver = false;

void setup()
{
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(redPin, INPUT);
  pinMode(greenPin, INPUT);
  //sets a random seed by reading the unplugged value from pin 0
  randomSeed(analogRead(0));
  //starts the ball off somewhere in the middle
  ballY = random(3, 5);
#ifdef DEBUG
  Serial.begin(9600);
#endif
}

int modulo(int n, int M)
{
  /*
   * Modulo in C is actually the remainder, not the modulo
   * https://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers
   * https://stackoverflow.com/questions/1907565/c-and-python-different-behaviour-of-the-modulo-operation
   */
  return ((n % M) + M) % M;
}

void drawBall()
{
  /*
   * Draws the ball by shifting a single bit to the right by ballY
   * Draws the appropriate column by shifting a single bit from the
   * left to the right by ballX and getting the inverse (to turn on 
   * a column it must be grounded (0, not 1)
   */
  if (ballY != 0 || ballY != 7)
  {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << ballY);    //col
    shiftOut(dataPin, clockPin, LSBFIRST, ~(0b10000000 >> ballX)); //row
    digitalWrite(latchPin, HIGH);
    delay(WAIT);
  }
}

void drawPaddles()
{
  /*
   * Draws the two paddles on the matrix
   * Before bits are shifted out, the latch pin is first set 
   * to low to signal that data is about to be sent in
   * After the data is sent out (16 bytes for 2 shift registers)
   * the latch pin is set to high again to signal that all the 
   * data has been sent out, and the Arduino waits a few
   * milliseconds
   */

  /*
   * Draws the player's paddle
   * Works in the same way as the drawBall() method
   * If the ball is in the top or bottom row, the method also 
   * combines a bitshifted 0b11000000 >> n with a single byte 
   * shifted to the right by ballX through a bitwise or
   */
  digitalWrite(latchPin, LOW);
  if (ballY == 0)
  {
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << playerY);                            //col
    shiftOut(dataPin, clockPin, LSBFIRST, ~((0b11000000 >> playerX) | 0b10000000 >> ballX)); //row, https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit, toggle bit
  }
  else
  {
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << playerY);    //col
    shiftOut(dataPin, clockPin, LSBFIRST, ~(0b11000000 >> playerX)); //row
  }
  digitalWrite(latchPin, HIGH);
  delay(WAIT);

  /*
   * Draws the AI paddle, works in the same way as the code snippet above
   */

  digitalWrite(latchPin, LOW);
  if (ballY == 7)
  {
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << aiY);                            //col
    shiftOut(dataPin, clockPin, LSBFIRST, ~((0b11000000 >> aiX) | 0b10000000 >> ballX)); //row, https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit, toggle bit
  }
  else
  {
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << aiY);    //col
    shiftOut(dataPin, clockPin, LSBFIRST, ~(0b11000000 >> aiX)); //row
  }

  digitalWrite(latchPin, HIGH);
  delay(WAIT);
}

void moveAI()
{
  /*
   * If the AI is at least 2 "pixels" (LEDs) away from the ball,
   * it moves in the appropriate direction to catch up
   */
  if (ballX > aiX - 1)
  {
    aiX++;
  }
  else if (ballX < aiX + 1)
  {
    aiX--;
  }
}

void updateGame()
{
  /*
   * Updates the game state if enough loops have occurred
   */
  if (loops % LOOP_CHECK == 0)
  {
    // Increments/decrements the ball position by 1/-1
    ballX += ballXDirection;
    // "Bounces" the ball if it hits a wall
    if (ballX == 0 || ballX == 7)
    {
      ballXDirection *= -1;
    }
    ballY += ballYDirection;

    //Checks that the ball is in either one of the endzones
    if (ballY == playerY || ballY == aiY)
    {
      if (ballY == playerY)
      {
        if (playerX == ballX)
        {
          // The ball has collided with one of the two paddle pixels
          ballYDirection *= -1;
          ballXDirection = -1;
#ifdef DEBUG
          Serial.print("Player Bounce!\n");
#endif
        }
        else if (playerX + 1 == ballX)
        {
          // The ball has collided with the other of the two paddle pixels
          ballYDirection *= -1;
          ballXDirection = 1;
#ifdef DEBUG
          Serial.print("Player Bounce!\n");
#endif
        }
        else
        {
#ifdef DEBUG
          // The paddle has missed the ball
          Serial.print("Player Lost!\n");
#endif
          gameOver = true;
        }
      }
      else if (ballY == aiY)
      {
        if (aiX == ballX)
        {
          // The ball has collided with one of the two paddle pixels
          ballYDirection *= -1;
          ballXDirection = 1;
#ifdef DEBUG
          Serial.print("AI Bounce!\n");
#endif
        }
        else if (aiX + 1 == ballX)
        {
          // The ball has collided with the other of the two paddle pixels
          ballYDirection *= -1;
          ballXDirection = -1;
#ifdef DEBUG
          Serial.print("AI Bounce!\n");
#endif
        }
        else
        {
          // The paddle has missed the ball
#ifdef DEBUG
          Serial.print("AI Lost!\n");
#endif
          gameOver = true;
        }
      }
    }

    // Lets the AI react to the ball location
    moveAI();

    /*
     * If either of the buttons are pressed, the player's paddle is moved
     * If the paddle reaches either end of the matrix and the player keeps
     * moving in that direction, the paddle loops around to the other side 
     * of the matrix
     */
    if (digitalRead(greenPin) == HIGH)
    {
      playerX--;
      playerX = modulo(playerX, 7);
    }
    else if (digitalRead(redPin) == HIGH)
    {
      playerX++;
      playerX = modulo(playerX, 7);
    }
  }
}

void loop()
{
  // Increments the loop counter for independent refresh and update rate
  loops++;

  drawBall();
  drawPaddles();
  if (gameOver)
  {
    // If the game is over, either the player or the AI has missed the ball
    if (ballY == playerY)
    {
      // If the ball is on the player's side, the AI has won, and we need to
      // light up the entire row on the AI's side
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, LSBFIRST, 0b10000000);  //col
      shiftOut(dataPin, clockPin, LSBFIRST, ~0b11111111); //row
      digitalWrite(latchPin, HIGH);
    }
    else if (ballY == aiY)
    {
      // Otherwise, the ball is on the AI's side, and the winning player's row
      // must light up to signify that they won
      digitalWrite(latchPin, LOW);
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001);  //col
      shiftOut(dataPin, clockPin, LSBFIRST, ~0b11111111); //row
      digitalWrite(latchPin, HIGH);
    }

    // As there is only one row lighting up, a simple delay is enough to keep it
    // on (as no persistent vision effect is required here)
    delay(2000);

    // In order to reset the game, a simple jump to 0 is called in assembly,
    // essentially performing a software reset of the entire Arduino
    asm volatile("jmp 0");
  }
  // Only start the game after 200 loops so that the player can see the ball's starting location
  if (loops > 200)  
  {
    updateGame();
  }
}
