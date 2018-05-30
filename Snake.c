const int SIZE = 8;

const int OFF = 0;
const int RED = 1;
const int GREEN = 2;
const int ORANGE = 3;

const int POINT_COLOR = GREEN;
const int SNAKE_COLOR = RED;
const int BONOUS_COLOR = ORANGE;

const int UP = 6;
const int DOWN = 7;
const int LEFT = 8;
const int RIGHT = 9;

const int MAX_LENGTH = 10;

const int POINT_DELAY = 15;//The ticks after a point vanishes before a new point is generated
const int POINT_LIFETIME = 20;//The amount of ticks before the next point is generated
const int BONOUS_PROBABILITY = 25;//Out of 100
const int TICK_DELAY = 500;//In MS

int LED[SIZE][SIZE];
int direction;
int length;
int bonous;
int X[MAX_LENGTH];
int Y[MAX_LENGTH];

int pointX;
int pointY;
bool bonousPoint;
int pointTicks;

void setup() {
	resetGame();
	/**Malp, setup the LedControl stuff here**/
}

void loop() {
	gameTick();
	updateMatrix();
	delay(TICK_DELAY);
}

void updateMatrix() {
	resolveMatrixArray();
	/**Fill in by malp
	The array you need is LED[x][y] where x and y = 8
	Values for LED[x][y] will either be OFF, RED, GREEN, ORANGE which are integer constants. LED[x][y] is an 2D int array
	The array goes from 0 to 8 in both x and y where 0, 0 is the bottom left corner and 7, 7 is the top right corner**/
}

 
bool isPressed(int pin) {
	/**Fill in by malp**/
}


void gameOver() {
	/**Fill in by malp**/
}

int random(int bound) {
	return rand() % bound;
}

void pointTick() {
	if (pointX == -1) {
		if (pointTicks > POINT_DELAY) {
			bonousPoint = random(100) <= BONOUS_PROBABILITY;
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
		bonousPoint = false;
		pointTicks = 0;
	}
	pointTicks++;
}

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

void resolveMatrixArray() {
	for (int x = 0; x < SIZE; x++) {
		for (int y = 0; y < SIZE; y++) {
			LED[x][y] = OFF;
		}
	}
	if (pointX >= 0 && pointY >= 0) {
		int color = POINT_COLOR;
		if (bonousPoint) {
			color = BONOUS_COLOR;
		}
		LED[pointX][pointY] = color;
	}
	for (int i = 0; i < length; i++) {
		int color = SNAKE_COLOR;
		if (i < bonous) {
			color = BONOUS_COLOR;
		}
		LED[X[i]][Y[i]] = color;
	}
}

void resetGame() {
	clearArray(X);
	clearArray(Y);
	length = 1;
	X[0] = SIZE/2;
	Y[0] = SIZE/2;
	pointX = -1;
	bonous = 0;
	pointTicks = 0;
	bonousPoint = false;
	pointY = -1;
	direction = UP;
}

void moveTo(int x, int y) {
	shiftArray(X, length);
	shiftArray(Y, length);
	X[length-1] = x;
	Y[length-1] = y;
}

void add(int x, int y) {
	X[length] = x;
	Y[length] = y;
	if (bonousPoint) {
		bonous++;
	}
	length++;
}

void shiftArray(int * array) {
	return shiftArray(length(array));
}
void shiftArray(int * array, int length) {
	for (int i = 0; i < length-1; i++) {
		array[i] = array[i+1];
	}
	array[length-1] = -1;
}


void clearArray(int * array) {
	for (int i = 0; i < length(array); i++) {
		array[i] = -1;
	}
}

int getX() {
	return X[length-1];
}
int getY() {
	return Y[length-1];
}

bool isPoint(int x, int y) {
	return pointX == x && pointY == y;
}

bool isSafe(int x, int y) {
	for (int i = 0; i < length; i++) {
		if (X[i] == x && Y[i] == y) {
			return false;
		}
	}
	return x >= 0 && x < SIZE && y>= 0 && y < SIZE;
}

int getPoints() {
	return length + bonous;
}


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
