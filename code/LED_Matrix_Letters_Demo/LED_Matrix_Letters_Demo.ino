
//We always have to include the library
#include "LedControl.h"

#define RED 0
#define GREEN 1
//pin 12 is connected to the DataIn 
//pin 11 is connected to the CLK 
//pin 10 is connected to LOAD 

//LedControl(int dataPin, int clkPin, int csPin, int numDevices = 2 for red and green 7219's);
LedControl lc=LedControl(12,11,10,2);

void setup() {
  //The MAX72XX is in power-saving mode on startup, we have to do a wakeup call   
  lc.shutdown(RED,false);
  lc.shutdown(GREEN,false);
  //Set the brightness to a medium values (0~15 possible values)
  lc.setIntensity(0,15);
  //and clear the display
  lc.clearDisplay(RED);
  lc.clearDisplay(GREEN);
}
#define CHAR_WIDTH 5
#define SP {0, 0, 0, 0, 0}               //Space
#define EX {0, 125, 0, 0, 0}             //Exclamation !
#define EX2 {0, 125, 0, 125, 0}             //Exclamation !
#define A {31, 36, 68, 36, 31}
#define B {127, 73, 73, 73, 54,}
#define C {62, 65, 65, 65, 34}
#define D {127, 65, 65, 34, 28}
#define E {127, 73, 73, 65, 65}
#define F {127, 72, 72, 72, 64}
#define G {62, 65, 65, 69, 38}
#define H {127, 8, 8, 8, 127}
#define I {0, 65, 127, 65, 0}
#define J {2, 1, 1, 1, 126}
#define K {127, 8, 20, 34, 65}
#define L {127, 1, 1, 1, 1}
#define M {127, 32, 16, 32, 127}
#define N {127, 32, 16, 8, 127}
#define O {62, 65, 65, 65, 62}
#define P {127, 72, 72, 72, 48}
#define Q {62, 65, 69, 66, 61}
#define R {127, 72, 76, 74, 49}
#define S {50, 73, 73, 73, 38}
#define T {64, 64, 127, 64, 64}
#define U {126, 1, 1, 1, 126}
#define V {124, 2, 1, 2, 124}
#define W {126, 1, 6, 1, 126}
#define X {99, 20, 8, 20, 99}
#define Y {96, 16, 15, 16, 96}
#define Z {67, 69, 73, 81, 97}

#define BLOCK_WIDTH 15
// Write the sentence with commas separating each letter
// In the first issue we will adapt the length of the sentence (characters)
byte phrase[13][CHAR_WIDTH]={M,A,L,P,M,A,D,E,T,H,I,S,EX};
int len = sizeof(phrase)/CHAR_WIDTH;
//int gnd[18]={-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12};
int gnd[BLOCK_WIDTH]={10,9,8,7,6,5,4,3,2,1,0,-1,-2,-3,-4};

void loop() { 

  for (int letter=0; letter<len; letter++){
    for (int moves = BLOCK_WIDTH; moves>=CHAR_WIDTH; moves--) {
      for (int i=0; i<CHAR_WIDTH; i++){ 
        lc.setRow(RED,gnd[i+moves-CHAR_WIDTH],phrase[letter][i]); 
      }
      delay(100); // speed of animation
      lc.clearDisplay(0);
    } 
    if(letter == len){
      letter = 0;
    }
  }//end for letter red
//   
//  for (int letter=0; letter<len; letter++){
//    for (int moves = BLOCK_WIDTH; moves>=CHAR_WIDTH; moves--) {
//      for (int i=0; i<CHAR_WIDTH; i++){ 
//        lc.setRow(1,gnd[i+moves-CHAR_WIDTH],phrase[letter][i]); 
//      }
//      delay(100); // speed of animation
//      lc.clearDisplay(0);
//    } 
//    if(letter == len){
//      letter = 0;
//    }
//  }//end for letter green (DOESN'T WORK, LAST COLUMN CARRIES OVER UNTIL THE NEXT LETTER)
     //TODO fix the ghosting effect / get a different board

}
