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
 */
int ballX = 2;
int ballY = 2;
int ballYDirection = 1;
int ballXDirection = 1;
int aiX = 3;
int aiY = 7;
int playerX = 3;
int playerY = 0;

const short WAIT = 5;
const short LOOP_CHECK = 7;
long loops = 0;



void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(redPin, INPUT);
  pinMode(greenPin, INPUT);
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
}

int modulo(int n, int M) {
  /*
   * Modulo in C is actually the remainder, not the modulo
   * https://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers
   * https://stackoverflow.com/questions/1907565/c-and-python-different-behaviour-of-the-modulo-operation
   */
  return ((n % M) + M) % M;
}

void drawBall()
{
  if (ballY != 0 || ballY != 7)
  {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << ballY); //col
    shiftOut(dataPin, clockPin, LSBFIRST, ~(0b00000001 << ballX)); //row
    digitalWrite(latchPin, HIGH);
    delay(WAIT);
  }
}

void drawPaddles()
{
  /*
   * Player Paddle
   */
  digitalWrite(latchPin, LOW); 
  if (ballY == 0)
  {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << playerY); //col
      shiftOut(dataPin, clockPin, LSBFIRST, ~((0b11000000 >> playerX) | 1UL << ballX)); //row, https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit, toggle bit
  }
  else
  {
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << playerY); //col
    shiftOut(dataPin, clockPin, LSBFIRST, ~(0b11000000 >> playerX)); //row
  }
  digitalWrite(latchPin, HIGH);
  delay(WAIT);

  /*
   * AI Paddle
   */

  digitalWrite(latchPin, LOW);
  if (ballY == 7)
  {
      shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << aiY); //col
      shiftOut(dataPin, clockPin, LSBFIRST, ~((0b11000000 >> aiX) | 1UL << ballX)); //row, https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit, toggle bit
  }
  else
  {
    shiftOut(dataPin, clockPin, LSBFIRST, 0b00000001 << aiY); //col
    shiftOut(dataPin, clockPin, LSBFIRST, ~(0b11000000 >> aiX)); //row
  }

  digitalWrite(latchPin, HIGH);
  delay(WAIT);
}

void updateGame()
{
    loops++;
    if (loops % LOOP_CHECK == 0)
    {
       ballX += ballXDirection;
       if (ballX == 0 || ballX == 7)
       {
           ballXDirection *= -1;
       }
       ballY += ballYDirection;
       
       if (ballY == playerY || ballY == aiY)
       {
          if (ballY == playerY)
          {
            #ifdef DEBUG
          Serial.print(playerX);
          Serial.print(", ");
          Serial.print(playerX + 1);
          Serial.print(": ");
          Serial.print(ballX);
          Serial.print("\n\n");
          #endif
          }
          

          ballYDirection *= -1;
//           if (ballY == playerY)
//           {
//              if (playerX + 1 == ballX or playerX + 2 == ballX)
//              {
//                ballYDirection *= -1;
//                #ifdef DEBUG
//                Serial.print("Player Bounce!\n");
//                #endif
//              }
//              else
//              {
//                ballYDirection *= -1;
//              }
//           }
//           else if (ballY == aiY)
//           {
//              if (aiX + 1 == ballX or aiX + 2 == ballX)
//              {
//                ballYDirection *= -1;
//                #ifdef DEBUG
//                Serial.print("AI Bounce!\n");
//                #endif
//              }
//              else
//              {
//                ballYDirection *= -1;
//              }
//           }
           
       }

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

void loop() {
  updateGame();
  drawBall();
  drawPaddles();
}


