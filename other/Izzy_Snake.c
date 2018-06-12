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
const int UP = 6;
//Pin number for down button
const int DOWN = 4;
//Pin number for left buttonn 
const int LEFT = 5;
//Pin number for right button
const int RIGHT = 7;

const int POT_PIN = 15;
//The max snake length
const int MAX_LENGTH = 50;

//The amount  of times it checks which direction the player is pressing per game tick
const int DIRECTIONS_PER_TICK = 50;
//The pin for the Matrix Data In
const int DIN = 8;
//The clock pin
const int CLK = 9;
//The load pin
const int LOAD = 10;
//The color that is currently turned off (it alternates colors)
int maxInShutdown=GREEN;                // tells which MAX7221 is currently off
//The IRS time
unsigned long ISRTime;      
//The LedControl variable used to manipulate the matricies
LedControl lc = LedControl(DIN, CLK, LOAD, 2);
//The LCD display control variable
LiquidCrystal_I2C lcd(0x3F, 20, 4);



const int POINT_DELAY = 6;//The ticks after a point vanishes before a new point is generated
const int POINT_LIFETIME = 20;//The amount of ticks before the next point is generated
const int BONOUS_PROBABILITY = 15;//Out of 100




int tickDelay;//In MS
//The LED array
int LED[WIDTH][LENGTH];
//The direction that the snake is moving
int direction = UP;
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

int record = 0;



/**
 The setup function initilizes all the pins, sets up the LedMetrecies
 Initlizes the LiquidCrystal_I2C
**/
void setup() {
  //Resets the game
  resetGame();
  lc.setIntensity(RED, 15);
  lc.setIntensity(GREEN, 15);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  //When the game boots up set it to the GameOver menu
  gameOver();
 
  /**Malp, setup the LedControl stuff here**/
}



//The ISR is from the Internet, it turns on and off the two colors so that they are not on at the same time
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
  //go through each x and y coordnate and prepare to set it
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < LENGTH; y++) {
	  //switch for the matrix that holds the colors and do the proper proccedure for each color
      switch(LED[x][y]) {
        case OFF:
          setLed(RED, x, y, false);
          setLed(GREEN, x, y, false);
          break;
        case RED:
          setLed(RED, x, y, true);
          setLed(GREEN,x, y, false);
          break;
        case GREEN:
          setLed(RED, x, y, false);
          setLed(GREEN, x, y, true);
          break;
        case ORANGE:
          setLed(RED, x, y, true);
          setLed(GREEN, x, y, true);
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
  //clear the LCD
  lcd.clear();
  //Set the cursor to the top left
  lcd.setCursor(0, 0);
  int points = getPoints();
  //Print that the game is over
  lcd.print("Game Over");
  //print("Game Over");
  lcd.setCursor(0, 1);
  //If it is a new record, set the record and display it
   if (points > record) {
  	  lcd.print("New Record: ");
	  record = points;
  } else {
	    lcd.print("Points: ");
  }
  lcd.print(points);
  //Play the trippy game over effect
  for (int x = 0; x < LENGTH; x++) {
  	  for (int y = 0; y < WIDTH; y++) {
	  	  setLed(GREEN, x, y, true);
		  setLed(RED, LENGTH-x-1, WIDTH-y-1, true);
		  delay(10);
	  }
  }
   lcd.setCursor(0, 0);
   //Tell the user to press any button to start the next game
   lcd.print("Press Any Button");
  while (!isPressed(UP) && !isPressed(DOWN) && !isPressed(LEFT) && !isPressed(RIGHT)) {
      delay(10);
	  //Wait for the next button press
	  updateDirection();
  }
  //Reset the game info and start the game
  resetGame();
  /**Fill in by malp**/
}


//A method to tick the point, randomly makes new points and removes them after specified delays
void pointTick() {
  //If the point is not currently on the screan
  if (pointX == -1) {
	//If the point has not been on long enough to add
    if (pointTicks > POINT_DELAY) {
	  //randomly decide if it is a bonous point
      bonousPoint = random(100) <= BONOUS_PROBABILITY;
      do {
        pointX = random(WIDTH);//randomly generate the point coordnates
        pointY = random(LENGTH);
      }
	  //keep doing this until a safe point is generated
      while(!isSafe(pointX, pointY));
	  //reset the point ticks
      pointTicks = 0;
    }
  } else if (pointTicks > POINT_LIFETIME) {
    //call on the method to reset the  points
    resetPoint();
  }
  //increase the point ticks by 1
  pointTicks++;
}
//The method to reset the point to not existing
void resetPoint() {
    pointX = -1;
    pointY = -1;
    //bonousPoint = false;
    pointTicks = 0;
}

//A method for updating the direction the snake is facing
int updateDirection() {
  //check each of the four directions to see if it is pressed
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
  //clear the snake location array
  clearArray(X);
  clearArray(Y);
  //reset the snake length to 1
  length = 1;
  //set the snake's starting position to the center
  X[0] = WIDTH/2;
  Y[0] = LENGTH/2;
  //reset the point
  resetPoint();
  //reset the player's bonous points'
  bonous = 0;
  //update the LCD
  updateLCD();
}
//A method to move the snake to a point
void moveTo(int x, int y) {
  shiftArray(X, length);
  shiftArray(Y, length);
  //shift the array of the snake so that all the points are moved back one and the last point is truncated
  X[length-1] = x;
  Y[length-1] = y;
  //set the new head location to the point that you moved to
}
//A method to add a new point to the snake
void add(int x, int y) {
  //add the given x and y cords to the array
  X[length] = x;
  Y[length] = y;
  if (bonousPoint) {
    bonous++;//if the point they got to grow is a bonous point increment the bonous counter
  }
  length++;//Increase the snake's length
  //update the LCD
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
  tickDelay = (analogRead(POT_PIN)/3)/DIRECTIONS_PER_TICK;
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
  lcd.clear();
  lcd.setCursor(0, 0);
  int points = getPoints();
  if (points < record) {
	lcd.print("Record: ");
	lcd.print(record);
  } else {
  	  lcd.print("New Record!");
  }
  lcd.setCursor(0, 1);
  lcd.print("Points: ");
  lcd.print(points);
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