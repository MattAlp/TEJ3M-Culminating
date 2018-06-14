/**Made by Izzy and Matthew
   Izzy made the game logic and Matthew programmed the interface with the hardware (i.e Buttons, Matrix, LCD)
   See the header of SnakeTick and DodgeTick for the game logic**/


//Include external libraries for interfacing with LED Matrix, LCD Display
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
//The color to use for the bonus points
const int BONUS_COLOR = ORANGE;
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
const int MAX_LENGTH = 35;

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
const int BONUS_PROBABILITY = 15;//Out of 100

//The static constant for the game Snake
const int SNAKE = 0;
//The static constant for the game Dodge
const int DODGE = 1;
//The variable that stores the current game, -1 for no game selected
int game = -1;


int tickDelay;//In MS
//The LED array
int LED[WIDTH][LENGTH];
//The direction that the snake is moving
int direction = UP;
//The length of the snake
int length;
//The amount of bonus points
int bonus;
//The X positions of the snake
int X[MAX_LENGTH];
//The Y positions of the snake
int Y[MAX_LENGTH];
//The x cords of the point on the matrix for increasing the snake length
int pointX;
//The y cords of the point
int pointY;
//Weather or not the point is a bonus point
bool bonusPoint;
//The amount of ticks since the last point action
int pointTicks;

int records[2];




//The Dodge Variables
//The amount of points you have
int points = 0;
//The position x
int posX;
//The position in y
int posY;
//The directions of each bullet
int directions[MAX_LENGTH];

const int BULLET_SPAWN_PROBABILITY = 33;//the probability of a bullet spawnning in dodge




/**
 The setup function initilizes all the pins, sets up the LedMetrecies
 Initlizes the LiquidCrystal_I2C
**/
void setup() {
  //Resets the game
  reset();
  lc.setIntensity(RED, 15);
  lc.setIntensity(GREEN, 15);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  Serial.print("Game has started up\n");
  //When the game boots up set it to the GameOver menu
  //selectGame();
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

/**
  The core method that ticks in a loop. If checks which game is
  currently active and then deffers the game logic to that game's tick method. 
  After that it updates the LCD and runs the delay. Those methods are common for both games
  If no game is selected it will go to the game selection menu**/
void loop() {
  //Run the game logic
  if (game == SNAKE) {
     snakeTick();
  } else if (game == DODGE) {
     dodgeTick();
  }
  if (game == -1) {
  	  selectGame();
	  updateLCD();
  } else {
	  //Update the matrix
	  updateMatrix();
	  //updateLCD();
	  //Delay before next tick
	  runDelay();
  }
}

//The method to update the matrix
void updateMatrix() {
  //Beforee updating the matrix update the Matrix LED array
  if (game == SNAKE) {
     resolveMatrixArraySnake();
  } else if (game == DODGE) {
      resolveMatrixArrayDodge();
  }
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
  /**
  Values for LED[x][y] will either be OFF, RED, GREEN, ORANGE which are integer constants. LED[x][y] is an 2D int array
  The array goes from 0 to 8 in both x and y where 0, 0 is the bottom left corner and 7, 7 is the top right corne**/
}

 //A method to check if a pin/button is pressed
bool isPressed(int pin) {
  return digitalRead(pin) == LOW;
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
   if (points > records[game]) {
      lcd.print("New Record: ");
      records[game] = points;
  } else {
      lcd.print("Points: ");
  }
  lcd.print(points);
  //Play the trippy game over effect
  for (int x = 0; x < LENGTH; x++) {
      for (int y = 0; y < WIDTH; y++) {
		  setLed(GREEN, x, y, true);
		  setLed(RED, LENGTH-x-1, WIDTH-y-1, true);
		  delay(15);
      }
  }
  delay(1000);
  //Reset the game info and start the game
  //selectGame();
  game = -1;
}


void reset() {
   resetDodge();
   resetSnake();
}


//A method to tick the point, randomly makes new points and removes them after specified delays
void pointTick() {
  //If the point is not currently on the screan
  if (pointX == -1) {
  //If the point has not been on long enough to add
    if (pointTicks > POINT_DELAY) {
    //randomly decide if it is a bonus point
      bonusPoint = random(100) <= BONUS_PROBABILITY;
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
    //bonusPoint = false;
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
void resolveMatrixArraySnake() {
  //Set all the points to false
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < LENGTH; y++) {
      LED[x][y] = OFF;
    }
  }
  //Set the point on the screen to it's color if there is one'
  if (pointX >= 0 && pointY >= 0) {
    int color = POINT_COLOR;
    if (bonusPoint) { 
      color = BONUS_COLOR;
    }
    LED[pointX][pointY] = color;
  }
  //Add the snake to the  array
  for (int i = 0; i < length; i++) {
    int color = SNAKE_COLOR;
    if (i < bonus) {
      color = BONUS_COLOR;
    }
    LED[X[i]][Y[i]] = color;
  }
}

//The method to clear/reset the game, initilizes all the variables
void resetSnake() {
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
  //reset the player's bonus points'
  bonus = 0;
  //update the LCD
  //updateLCD();
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
  if (bonusPoint) {
    bonus++;//if the point they got to grow is a bonus point increment the bonus counter
  }
  length++;//Increase the snake's length
  //update the LCD
  //updateLCD();
  
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
//Take the size of the array and divide it by the size of the first element to get the amount of elements
int sizeOf(int * a) {
  return sizeof(a) / sizeof(a[0]);
}

//Get the amount of points a player has
int getPoints() {
  if (game == SNAKE) {//If snake the points is the length + the bonus points
     return length + bonus;
  } else if (game == DODGE) {//If the game is doge its the amount of points variable
      return points;
  }
}
//Run the delay between game ticks
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
//Update the LCD and clear the previous display
void updateLCD() {
	updateLCD(true);
}
//Update the LCD Display
void updateLCD(bool clear) {
  if (clear) {
      lcd.clear();
  }
  lcd.setCursor(0, 0);
  int points = getPoints();
  if (points < records[game]) {
  lcd.print("Record: ");
  lcd.print(records[game]);
  } else {
      lcd.print("New Record!");
  }
  lcd.setCursor(0, 1);
  lcd.print("Points: ");
  lcd.print(points);
}

/**
  The core method that decides how the snake game logic should work
  First updates the food pellets on the screen that the snake needs to eat to grow
  Then it gets the current position of the snake head and decides where the snake is moving to
  If that position is a point, add it to the SNAKE
  If it is not a point but it is a valid location then move the snake there
  If it is not safe then the game is over so call the universal game over method that runs for both games
  Update the LCD without clearing it so there is no flickering, due to the number on the screen always growing there is no underflow
**/
void snakeTick() {
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
	//return;
  }
  updateLCD(false);

}
//The method to select which game is active
void selectGame() {
   lcd.clear();//Print to the screen the game options
   lcd.setCursor(0, 0);
   //Tell the user to press any button to start the next game
   lcd.print("Right = SNAKE");
   lcd.setCursor(0, 1);
   lcd.print("Left = DODGE");
  while (!isPressed(UP) && !isPressed(DOWN)) {//wait for a press that decides which game to select
      delay(10);
  }
  updateDirection();//update the direction based on the clicked button
  if (direction == UP) {//if they selected snake, the game is snake
  	  game = SNAKE;
  } else if (direction == DOWN) {//if they selected dodge, the game is dodge
      game = DODGE;
	  updateLCD();//update the LCD for dodge
  }
  reset();//reset the game logic
}
//A method to spawn in a bullet in dodge randomly
void spawnBullet() {
  int pos = random(WIDTH);//choose how far along the grid it is spawning
  int x;
  int y;
  int dir;
  int r = random(4);//choose which side it is spawning from and adjust the direction and position acordingly
  switch (r) {
    case 0:
       dir = DOWN;
       y = LENGTH-1;
       x = pos;
       break;
    case 1:
       y = 0;
       x = pos;
       dir = UP;
       break;
    case 2:
       y = pos;
       x = 0;
       dir = RIGHT;
       break;
    case 3:
       y = pos;
       x = WIDTH - 1;
       dir = LEFT;
       break;
    default: break;
  }
  int empty = -1;//the ID of the first empty position in the snake array
  for (int i = 0; i < length+1 && i < MAX_LENGTH; i++) {//go through each position
    if (X[i] == -1 || i == length) {//if the position is empty
      empty = i;//Set the empty to the current value
      if (empty >= length) {
        length = empty+1;//increase the length of the array if needed
      }//escape from the loop
      break;
    }
  }
  X[empty] = x;//set the bullet array to store the bullet location
  Y[empty] = y;
  directions[empty] = dir;//store the bullet velocity
}

/**
The core function of the Dodge game, the game moves the players, checks if they hit a bullet, moves the bullets, checks if they hit a player, adds new bullets, and updates the screen
**/
void dodgeTick() {
   movePlayer();//Update the player position
   moveBullets();//Move all the bullets
   spawnBullets();//Spawn the new bullets if needed
   updateLCD(false);//Update the LCD without clearing to account for rapid point changes
}

//The spawn bullets command
void spawnBullets() {
	if (random(100) < BULLET_SPAWN_PROBABILITY + points/10) {//Randomly decides if it should spawn a bullet based on probability
		spawnBullet();//If it did decide to spawn one, actually call the spawn function
	}
}
//Move the player's position'
void movePlayer() {
  int x = posX;
  int y = posY;//get their current positions
  switch(direction) {//Update their position based on the clicked direction
    case UP:y++;break;
    case DOWN:y--;break;
    case LEFT:x--;break;
    case RIGHT:x++;break;
  }
  if (!isSafe(x, y)) {//If the new position is not a valid position then the game is over, call the universal game over method
    gameOver();
	return;
  } else {
     posX = x;//If the position is valid, update the position and record they survived another tick/point
     posY = y;
     points++;
  }
}
//A function to move the bullets
void moveBullets() {
  for (int i = 0; i < length; i++) {
    if (X[i] != -1) {//go through each bullet, if it is not null then select it
      int x = X[i];
      int y = Y[i];//get its position and direction
      int dir = directions[i];
      switch(dir) {//update its position based on direction
        case UP:y++;break;
        case DOWN:y--;break;
        case LEFT:x--;break;
        case RIGHT:x++;break;
      }
	  if (x == posX && y == posY) {//If they crashed into the player, game is over
	  	  gameOver();
		  return;
	  } else if (isSafe(x, y)) {//If they are safe, update the position
        X[i] = x;
        Y[i] = y;
      } else {
        X[i] = -1;//If it is not safe (i.e off screen or hit another bullet) then remove the bullet from the array
        Y[i] = -1;
      }
    }
  }
}
//Resolve the dodge game logic into an array to display on the matrix
void resolveMatrixArrayDodge() {
  resolveMatrixArraySnake();//Beacuse dodge stores bullets in the same array as Snake stores the snake, it can calll tghe Snake resolve function without re-programming it
  LED[posX][posY] = POINT_COLOR;//Add thhe dodge player to the array
}

void resetDodge() {//To reset dodge reset the position and clear the direcctions array and reset the points
 // resetSnake();
  posX = WIDTH/2;
  posY = LENGTH/2;
  points = 0;
  clearArray(directions);
}
