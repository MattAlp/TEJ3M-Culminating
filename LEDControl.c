#include <LedControl.h>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(12,11,10,1);

/* we always wait a bit between updates of the display */
unsigned long delaytime=250;
const int SIZE = 8;
const int OBJECTS = 3;
bool LED[OBJECTS][SIZE][SIZE];
bool PRINT[SIZE][SIZE];

void setup() {
  Serial.begin(9600);
  Serial.print("Number of matrices: ");
  Serial.print((int) lc.getDeviceCount());
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,15);
  /* and clear the display */
  lc.clearDisplay(0);
  LED[0][0][0] = true;
  LED[0][0][1] = true;
  LED[1][2][4] = true;
  LED[1][0][1] = true;
  LED[2][6][7] = true;
}


void updateMatrix() {
  resolve();
   for (int i = 0; i < SIZE; i++) {
     for (int k = 0; k < SIZE; k++) {
        lc.setLed(0, i, k, PRINT[i][k]);
      }
  } 
}

void resolve() {
    for (int r = 0; r < SIZE; r++) {
      for (int c = 0; c < SIZE; c++) {
    for (int i = 0; i < OBJECTS; i++) {
      if (LED[i][r][c]) {
        PRINT[r][SIZE-1-c] = true;
         /* Serial.print(r);
          Serial.print(" , ");
          Serial.print(c);
          Serial.print("\n");*/
          break;
      } else {
        PRINT[r][SIZE-1-c] = false;
      }
    }
       
      }
    }
}



void shift(int object, int x, int y) {
    if (x != 0) {
    for (int c = 0; c < SIZE; c++) {
        if (x > 0) {
          for (int r = SIZE-1-x; r>=0; r--) {
            LED[object][r+x][c] = LED[object][r][c];
            LED[object][r][c] = false;
          }
        } else if (x < 0) {
          for (int r = 0; r < SIZE-1+x; r++) {
            LED[object][r][c] = LED[object][r-x][c];
            LED[object][r-x][c] = false;
          }
        }
    }
  }
  if (y != 0) {
    for (int r = 0; r < SIZE; r++) {
        if (y > 0) {
          for (int c = SIZE-1-y; c>=0; c--) {
            LED[object][r][c+y] = LED[object][r][c];
            LED[object][r][c] = false;
          }
        } else if (y < 0) {
          for (int c = 0; c < SIZE-1+y; c++) {
            LED[object][r][c] = LED[object][r][c-y];
            LED[object][r][c-y] = false;
          }
        }
    }
  }
  
}
int count = 0;
void loop() { 
    //lc.setLed(0, 0, 0, true);
  /*for (int i = 0; i < SIZE-1; i++) {
    shift(0, 1, 1);
    delay(1000);
  }*/
  int wait = 200;
  for (int i = 0; i < SIZE-2; i++) {
    shift(0, 1, 0);
    updateMatrix();
    delay(wait);
  }
  for (int i = 0; i < SIZE-2; i++) {
    shift(0, -1, 0);
    updateMatrix();
    delay(wait);
  }
  shift(0, 0, 1);
  
 
  /*for (int i = 0; i < SIZE-1; i++) {
    shift(0, -1, 0);
    updateMatrix();
    delay(1000);
  }
  for (int i = 0; i < SIZE-1; i++) {
    shift(0, 0, -1);
    updateMatrix();
    delay(1000);
  }*/
  
  
}


const int PADDLE1 = 0;
const int PADDLE2 = 1;
const int BALL = 2;
