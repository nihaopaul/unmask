#include "animation.h"
byte col = 0;
byte leds[8][8];

// pin[xx] on led matrix connected to nn on Arduino (-1 is dummy to make array start at pos 1)
int pins[17]= {
  5, 4, 3, 2, 14, 15, 16, 17, 13, 12, 11, 10, 9, 8, 7, 6
};
//1, 2, 3, 4,  5,  6,  7,  8,  9, 10, 11, 12,13,14,15,16 
// col[xx] of leds = pin yy on led matrix
int cols[8] = {
  pins[13], pins[3], pins[4], pins[10], pins[06], pins[11], pins[15], pins[16]
};
// PB1,     PD3,     PD2,      PB4,      PC1,      PB3,      PD7,      PD6
volatile uint8_t* colp[8] = {
  &PORTB, &PORTD, &PORTD, &PORTB, &PORTC, &PORTB, &PORTD, &PORTD
};
uint8_t colb[8] = {
  bit(1), bit(3), bit(2), bit(4), bit(1), bit(3), bit(7), bit(6)
};


// row[xx] of leds = pin yy on led matrix
int rows[8] = {
  pins[9], pins[14], pins[8], pins[12], pins[1], pins[7], pins[2], pins[5]
};
//PB5,      PB0,     PC3,      PB2,     PD5,     PC2,     PD4,     PC0
volatile uint8_t* rowp[8] = {
  &PORTB, &PORTB, &PORTC, &PORTB, &PORTD, &PORTC, &PORTD, &PORTC
};
uint8_t rowb[8] = {
  bit(5), bit(0), bit(3), bit(2), bit(5), bit(2), bit(4), bit(0)

};                 


//mask stuff
int led= 1; //green
// boolean to know if the balance has been set
boolean balanceSet = false;
int green = 0;
//floats to hold colour arrays
float colour = 0;
float white = 0;
float black = 0;
int avgRead;
long calibration = 0;
int LDR = A4;


void setup() {
  // sets the pins as output
  for (int i = 0; i < 16; i++) {
    pinMode(pins[i], OUTPUT);
  }

  // set up cols and rows
  for (int i = 1; i <= 8; i++) {
    digitalWrite(cols[i - 1], LOW);
  }

  for (int i = 1; i <= 8; i++) {
    digitalWrite(rows[i - 1], LOW);
  }
  
  pinMode(led,OUTPUT); //green
  pinMode(LDR,INPUT);

}


void loop(){

  //mask stuff
  checkBalance();
  checkColour();
  outputScreen();
}

void display(uint8_t leds[8], int t){
  long start = millis();
  while(millis() - start < t) _display(leds);
}

// Interrupt routine
void _display(uint8_t leds[8]) {
  for(uint8_t col = 0; col < 8; col++){
    uint8_t p = 1;
    for (uint8_t row = 0; row < 8; row++) {
      if (leds[col] & p) {
        *rowp[row] &= ~rowb[row]; //digitalWrite(rows[row], LOW);  // Turn on this led
      } 
      else {
        *rowp[row] |= rowb[row];  //digitalWrite(rows[row], HIGH); // Turn off this led
      }
      p = p << 1;
    }
    *colp[col] |= colb[col];
    if(col == 4) delayMicroseconds(60);
    delayMicroseconds(10);
    *colp[col] &= ~colb[col];
    if(col != 4)delayMicroseconds(10);
  }
}


//mask stuff

void checkBalance(){
  //check if the balance has been set, if not, set it
  if(balanceSet == false){
    setBalance();
    calibration = (colour - black)/(white - black)*255;
  }
}

void setBalance(){

  for (int t = 0; t <= 20; t++) {
    
      digitalWrite(led,HIGH);
      delay(100);
      getReading(5);          //number is the number of scans to take for average, this whole function is redundant, one reading works just as well.
      white = min(white, avgRead);
      black = max(black, avgRead);
      digitalWrite(led,LOW);


    
    delay(100);
  }
  
  
  //set boolean value so we know that balance is set
  balanceSet = true;

}

void checkColour(){
    display(expresssion[4], 0);
    digitalWrite(led,HIGH);  //turn or the LED, red, green or blue depending which iteration
    delay(100);                      //delay to allow CdS to stabalize, they are slow
    getReading(5);                  //take a reading however many times
    colour = avgRead;        //set the current colour in the array to the average reading
    float greyDiff = white - black;                    //the highest possible return minus the lowest returns the area for values in between
    colour = (colour - black)/(greyDiff)*255; //the reading returned minus the lowest value divided by the possible range multiplied by 255 will give us a value roughly between 0-255 representing the value for the current reflectivity(for the colour it is exposed to) of what is being scanned
    digitalWrite(led,LOW);   //turn off the current LED


}
void getReading(int times){
  
  int reading;
  int tally=0;
  //take the reading however many times was requested and add them up
  for(int i = 0;i < times;i++){
    reading = analogRead(LDR);
    tally = reading + tally;
    delay(10);
  }
  //calculate the average and set it
  avgRead = (tally)/times;
}

void outputScreen() {
  
  long thisValue = double(((double)colour / (double)calibration)*100);
  if (thisValue > 15) {
    //mouth open
    display(expresssion[2], 300);
    //Serial.println(":o");
  } else if(thisValue >= 3) {
    //smile
//    Serial.println(":)");
    display(expresssion[1], 300);
    
  } else if(thisValue >= -5) {
    //normal
//    Serial.println(":|");
    display(expresssion[3], 300);
  } else {
//    Serial.println(":x");
    display(expresssion[0], 300);
  }
  
  
}


