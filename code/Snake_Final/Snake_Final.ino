//The size of the LED Matrix
const int SIZE = 8;
//The color value for off
const int OFF = -1;
//The color value for RED
const int RED = 0;
//The color value for Green
const int GREEN = 1;
//The color value for orange 
const int ORANGE = 2; //!!!! Orange is not a separate colour, it is red combined with green at the same time
//The color to use for points
const int POINT_COLOR = GREEN;
//The color to use for the snake
const int SNAKE_COLOR = RED;
//The color to use for the bonus points
const int BONUS_COLOR = ORANGE;
//The Pin Number for button up
const int UP = 6;
//Pin number for down button
const int DOWN = 7;
//Pin number for left button
const int LEFT = 8;
//Pin number for right button
const int RIGHT = 9;
//The max snake length
const int MAX_LENGTH = 10;

const int POINT_DELAY = 15;//The ticks after a point vanishes before a new point is generated
const int POINT_LIFETIME = 20;//The amount of ticks before the next point is generated
const int BONUS_PROBABILITY = 25;//Out of 100
const int TICK_DELAY = 500;//In MS
//The LED array
int LED[SIZE][SIZE];
//The direction that the snake is moving
int direction;
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

//The setup functiion
void setup() {
	resetGame();
	/**Malp, setup the LedControl stuff here**/
}

//The  loop function
void loop() {
	//Run the game logic
	gameTick();
	//Update the matrix
	updateMatrix();
	//Delay before next tick
	delay(TICK_DELAY);
}

//The method to update the matrix
void updateMatrix() {
	//Beforee updating the matrix update the Matrix LED array
	resolveMatrixArray();
	/**Fill in by malp
	The array you need is LED[x][y] where x and y = 8
	Values for LED[x][y] will either be OFF, RED, GREEN, ORANGE which are integer constants. LED[x][y] is an 2D int array
	The array goes from 0 to 8 in both x and y where 0, 0 is the bottom left corner and 7, 7 is the top right corner**/
}

 //A method to check if a pin/button is pressed
bool isPressed(int pin) {
	/**Fill in by malp**/
}

//The code to run when the game is over
void gameOver() {
	/**Fill in by malp**/
}
//A method for generating a random number between 0 and bound-1 inclusive
int random(int bound) {
	return rand() % bound;
}

//A method to tick the point, randomly makes new points and removes them after specified delays
void pointTick() {
	if (pointX == -1) {
		if (pointTicks > POINT_DELAY) {
			bonusPoint = random(100) <= BONUS_PROBABILITY;
			do {
				pointX = random(SIZE);
				pointY = random(SIZE);
			}
			while(!isSafe(pointX, pointY);
			pointTicks = 0;
		}
	} else if (pointTicks > POINT_LIFETIME) {
		pointX = -1;
		pointY = -1;
		bonusPoint = false;
		pointTicks = 0;
	}
	pointTicks++;
}
//A method for updating the direction the snake is facing
int updateDirection() {
	if (isPressed(UP)) {
		direction = UP;
	} else if (isPressed(DOWN)) {
		direction = DOWN;
	} else if (isPressed(LEFT)) {
		direction = LEFT;
	} else iif (isPressed(RIGHT)) {
		direction = RIGHT;
	}
}
//A method to resolve the LED Matrix
void resolveMatrixArray() {
	//Set all the points to false
	for (int x = 0; x < SIZE; x++) {
		for (int y = 0; y < SIZE; y++) {
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
void resetGame() {
	clearArray(X);
	clearArray(Y);
	length = 1;
	X[0] = SIZE/2;
	Y[0] = SIZE/2;
	pointX = -1;
	bonus = 0;
	pointTicks = 0;
	bonusPoint = false;
	pointY = -1;
	direction = UP;
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
	if (bonusPoint) {
		bonus++;
	}
	length++;
}
//A method for shifting an array to the left, truncating the value at the left and nulling the value on the right
void shiftArray(int * array) {
	return shiftArray(length(array));
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
	for (int i = 0; i < length(array); i++) {
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
	return x >= 0 && x < SIZE && y>= 0 && y < SIZE;
}
//Get the amount of points a player has
int getPoints() {
	return length + bonus;
}

//Run the game logic
void gameTick() {
	updateDirection();
	pointTick();
	int x = getX();
	int y = getY();
	switch(direction) {
		case UP:y++break;
		case DOWN:y--break;
		case LEFT:x--break;
		case RIGHT:x++break;
	}
	if (isPoint(x, y)) {
		add(x, y);
	} else if (isSafe(x,y)) {
		moveTo(x, y);
	} else {
		gameOver();
	}

}
