#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "LedControl.h"
#define ISR_FREQ 190     //190=650Hz    // Sets the speed of the ISR - LOWER IS FASTER
//The size of the LED Matrix
const int WIDTH = 8;
const int LENGTH = 8;
//The color value for off
const int OFF = -1;
//The color value for REd
const int RED = 0;
//The color value for Green
const int GREEN = 1;
//The color value for orange
const int ORANGE = 2;
//The color to use for points
const int POINT_COLOR = GREEN;
//The color to use for the snake
const int SNAKE_COLOR = RED;
//The color to use for the bonous points
const int BONOUS_COLOR = ORANGE;
//The Pin Number for button up
const int UP = 4;
//Pin number for down button
const int DOWN = 5;
//Pin number for left buttonn 
const int LEFT = 7;
//Pin number for right button
const int RIGHT = 6;

const int POT_PIN = 15;
//The max snake length
const int MAX_LENGTH = 10;

const int DIRECTIONS_PER_TICK = 10;

const int DIN = 8;
const int CLK = 9;
const int LOAD = 10;

int maxInShutdown=GREEN;                // tells which MAX7221 is currently off
unsigned long ISRTime;      

LedControl lc = LedControl(DIN, CLK, LOAD, 2);
LiquidCrystal_I2C lcd(0x3F, 20, 4);



const int POINT_DELAY = 6;//The ticks after a point vanishes before a new point is generated
const int POINT_LIFETIME = 20;//The amount of ticks before the next point is generated
const int BONOUS_PROBABILITY = 15;//Out of 100




int tickDelay;//In MS
//The LED array
int LED[WIDTH][LENGTH];
//The direction that the snake is moving
int direction;
//The length of the snake
int length;
//The amount of bonous points
int bonous;
//The X positions of the snake
int X[MAX_LENGTH];
//The Y positions of the snake
int Y[MAX_LENGTH];
//The x cords of the point on the matrix for increasing the snake length
int pointX;
//The y cords of the point
int pointY;
//Weather or not the point is a bonous point
bool bonousPoint;
//The amount of ticks since the last point action
int pointTicks;

int GND[1] = {23};
int VCC[1] = {22};




//The setup functiion
void setup() {
  resetGame();
  setupLC(RED);
  setupLC(GREEN);
  setupLC(RED+2);
  setupLC(GREEN+2);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  //srand(null);
  Serial.begin(9600);
  Serial.print("Number of matrices: ");
  Serial.print((int) lc.getDeviceCount());
  Serial.print("\n");
  for (int i = 0; i < sizeOf(GND); i++) {
  	  pinMode(GND[i], OUTPUT);
	  digitalWrite(GND[i], LOW);
	  Serial.print(GND[i]);
	  Serial.print(" = LOW\n");
  }
  for (int i = 0; i < sizeOf(VCC); i++) {
  	  pinMode(VCC[i], OUTPUT);
	  digitalWrite(VCC[i], HIGH);
	  Serial.print(VCC[i]);
	  Serial.print(" = HIGH\n");
  }
  /**Malp, setup the LedControl stuff here**/
}

void setupLC(int LC) {
  //lc.shutdown(LC, false);
  lc.setIntensity(LC, 15);
  //lc.clearDisplay(LC);
}

/////////////////////////////ISR Timer Functions ///////////////////////////
ISR(TIMER2_COMPA_vect) {  //This ISR toggles shutdown between the 2MAX7221's
  if(maxInShutdown==RED){
  		lc.shutdown(GREEN,true);  // The order here is critical - Shutdown first!
  		lc.shutdown(RED,false);   // . . . Then restart the other.
		maxInShutdown=GREEN;
  }
  else  {
  		lc.shutdown(RED,true);
		lc.shutdown(GREEN,false);
		maxInShutdown=RED;
  }
}  
 
void setISRtimer(){  // setup ISR timer controling toggleing
  TCCR2A = 0x02;                        // WGM22=0 + WGM21=1 + WGM20=0 = Mode2 (CTC)
  TCCR2B = 0x05;                // CS22=1 + CS21=0 + CS20=1 = /128 prescaler (125kHz)
  TCNT2 = 0;                            // clear counter
  OCR2A = ISR_FREQ;                     // set TOP (divisor) - see #define
}
 
void startISR(){  // Starts the ISR
  TCNT2 = 0;                            // clear counter (needed here also)
  TIMSK2|=(1<<OCIE2A);                  // set interrupts=enabled (calls ISR(TIMER2_COMPA_vect)
}
 
void stopISR(){    // Stops the ISR
  TIMSK2&=~(1<<OCIE2A);                  // disable interrupts
}

//The  loop function
void loop() {
  //Run the game logic
  gameTick();
  //Update the matrix
  updateMatrix();
  //updateLCD();
  //Delay before next tick
  runDelay();
}

//The method to update the matrix
void updateMatrix() {
  //Beforee updating the matrix update the Matrix LED array
  resolveMatrixArray();
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < LENGTH; y++) {
	  int x2 = 7-x;
	  int y2 = 7-(y % 8);
	  int green = GREEN;
	  int red = RED;
	  if (y >= 8) {
		green = green+2;
		red = red + 2;
	  }
	  /*Serial.print(x);
	  Serial.print(", ");
	  Serial.print(y);
	  Serial.print(" = ");
	  Serial.print(x2);
	  Serial.print(", ");
	  Serial.print(y2);
	  Serial.print(" = ");
	  Serial.print(red);
	  Serial.print("&");
	  Serial.print(green);
	  Serial.print(" = ");
	  Serial.print(LED[x][y]);
	  Serial.print("\n");*/
      switch(LED[x][y]) {
        case OFF:
          setLed(red, x2, y2, false);
          setLed(green, x2, y2, false);
          break;
        case RED:
          setLed(red, x2, y2, true);
          setLed(green,x2, y2, false);
          break;
        case GREEN:
          setLed(red, x2, y2, false);
          setLed(green, x2, y2, true);
          break;
        case ORANGE:
          setLed(red, x2, y2, true);
          setLed(green, x2, y2, true);
          break;
      }
    }
  } 
  /**Fill in by malp
  The array you need is LED[x][y] where x and y = 8
  Values for LED[x][y] will either be OFF, RED, GREEN, ORANGE which are integer constants. LED[x][y] is an 2D int array
  The array goes from 0 to 8 in both x and y where 0, 0 is the bottom left corner and 7, 7 is the top right corner**/
}

 //A method to check if a pin/button is pressed
bool isPressed(int pin) {
  return digitalRead(pin) == LOW;
  /**Fill in by malp**/
}

//The code to run when the game is over
void gameOver() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Over");
  //print("Game Over");
  lcd.setCursor(0, 1);
  lcd.print("Points ");
  lcd.print(getPoints());
  delay(3000);
  while (!isPressed(UP) && !isPressed(DOWN) && !isPressed(LEFT) && !isPressed(RIGHT)) {
      delay(500);
  }
  resetGame();
  /**Fill in by malp**/
}


//A method to tick the point, randomly makes new points and removes them after specified delays
void pointTick() {
  if (pointX == -1) {
    if (pointTicks > POINT_DELAY) {
      bonousPoint = random(100) <= BONOUS_PROBABILITY;
      do {
        pointX = random(WIDTH);
        pointY = random(LENGTH);
      }
      while(!isSafe(pointX, pointY));
      pointTicks = 0;
    }
  } else if (pointTicks > POINT_LIFETIME) {
    resetPoint();
  }
  pointTicks++;
}

void resetPoint() {
    pointX = -1;
    pointY = -1;
    //bonousPoint = false;
    pointTicks = 0;
}

//A method for updating the direction the snake is facing
int updateDirection() {
  if (isPressed(UP)) {
    direction = UP;
  } else if (isPressed(DOWN)) {
    direction = DOWN;
  } else if (isPressed(LEFT)) {
    direction = LEFT;
  } else if (isPressed(RIGHT)) {
    direction = RIGHT;
  }
}
//A method to resolve the LED Matrix
void resolveMatrixArray() {
  //Set all the points to false
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < LENGTH; y++) {
      LED[x][y] = OFF;
    }
  }
  //Set the point on the screen to it's color if there is one'
  if (pointX >= 0 && pointY >= 0) {
    int color = POINT_COLOR;
    if (bonousPoint) { 
      color = BONOUS_COLOR;
    }
    LED[pointX][pointY] = color;
  }
  //Add the snake to the  array
  for (int i = 0; i < length; i++) {
    int color = SNAKE_COLOR;
    if (i < bonous) {
      color = BONOUS_COLOR;
    }
    LED[X[i]][Y[i]] = color;
  }
}

//The method to clear/reset the game, initilizes all the variables
void resetGame() {
  clearArray(X);
  clearArray(Y);
  length = 1;
  X[0] = WIDTH/2;
  Y[0] = LENGTH/2;
  pointX = -1;
  bonous = 0;
  pointTicks = 0;
  bonousPoint = false;
  pointY = -1;
  direction = UP;
  updateLCD();
}
//A method to move the snake to a point
void moveTo(int x, int y) {
  shiftArray(X, length);
  shiftArray(Y, length);
  X[length-1] = x;
  Y[length-1] = y;
}
//A method to add a new point to the snake
void add(int x, int y) {
  X[length] = x;
  Y[length] = y;
  if (bonousPoint) {
    bonous++;
  }
  length++;
  updateLCD();
  
}



void setLed(byte Color, byte Row,byte Col, byte State){
  stopISR();            // disable interrupts - stop toggling shutdown when updating
  lc.setLed(Color,Row,Col,State);
  startISR();           // enable interrupts again
}
 
void setRow(byte Color, byte Row, byte State){
  stopISR();            // disable interrupts - stop toggling shutdown when updating
  lc.setRow(Color,Row,State);
  startISR();           // enable interrupts again
}
 
void setColumn(byte Color, byte Col, byte State){
  stopISR();            // disable interrupts - stop toggling shutdown when updating
  lc.setColumn(Color,Col,State);
  startISR();           // enable interrupts again
}
 
void clearMatrix(){
  stopISR();            // disable interrupts - stop toggling shutdown when updating
  lc.clearDisplay(GREEN);
  lc.clearDisplay(RED);
  startISR();           // enable interrupts again
}


//A method for shifting an array to the left, truncating the value at the left and nulling the value on the right
void shiftArray(int * array) {
  return shiftArray(array, sizeOf(array));
}
//A method for shifting an array to the left, truncating the value at the left and nulling the value on the right where the rightmost point is the length given
void shiftArray(int * array, int length) {
  for (int i = 0; i < length-1; i++) {
    array[i] = array[i+1];
  }
  array[length-1] = -1;
}
//A method for clearing an array
void clearArray(int * array) {
  for (int i = 0; i < sizeOf(array); i++) {
    array[i] = -1;
  }
}
//Get the x cord of the snake head
int getX() {
  return X[length-1];
}
//Get the y cord of the snake head
int getY() {
  return Y[length-1];
}
//Checks if a given cord is the point
bool isPoint(int x, int y) {
  return pointX == x && pointY == y;
}
//Checks if a set of cords are a valid point to move to
bool isSafe(int x, int y) {
  for (int i = 0; i < length; i++) {
    if (X[i] == x && Y[i] == y) {
      return false;
    }
  }
  return x >= 0 && x < WIDTH && y>= 0 && y < LENGTH;
}

int sizeOf(int * a) {
  return sizeof(a) / sizeof(a[0]);
}

//Get the amount of points a player has
int getPoints() {
  return length + bonous;
}

void runDelay() {
  tickDelay = analogRead(POT_PIN)/DIRECTIONS_PER_TICK;
  //Serial.print(tickDelay);
  //Serial.print("\n");
  for (int i = 0; i < DIRECTIONS_PER_TICK; i++) {
      updateDirection();
	  delay(tickDelay);
	  //Serial.print(tickDelay);
	  //Serial.print("\n");
  }
}

void updateLCD() {
	lcd.setCursor(0, 0);
	lcd.print("Snake Length: ");
	lcd.print(length);
	lcd.setCursor(0, 1);
	lcd.print("Points ");
	lcd.print(getPoints());
}

//Run the game logic
void gameTick() {
  pointTick();
  int x = getX();
  int y = getY();
  switch(direction) {
    case UP:y++;break;
    case DOWN:y--;break;
    case LEFT:x--;break;
    case RIGHT:x++;break;
  }
  if (isPoint(x, y)) {
    add(x, y);
  resetPoint();
  } else if (isSafe(x,y)) {
    moveTo(x, y);
  } else {
    gameOver();
  }

}
