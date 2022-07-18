//Mastermind Game for Lake House Puzzlefest
//June 22nd 2022, Max + Bea

//Buzzer notes
#define nC3 131
#define nD3 147
#define nE3 164
#define nF3 175
#define nG3 196
#define nGs3 208
#define nA3 220
#define nB3 247
#define nC4 261
#define nD4 294
#define nE4 330
#define nF4 349
#define nG4 392
#define nGs4 415
#define nA4 440
#define nB4 494
#define nC5 523
#define nD5 587
#define nE5 660
//above this sounds bad
#define nF5 698
#define nG5 784
#define nA5 880

//note information
int quarterNoteDuration = 500;

//component pins
int buzzerPin = 14;
int buttonPin = A1;
int lightPins[6][2] = {{2,8},{3,9},{4,10},{5,11},{6,12},{7,13}};
//Change the light pins depending on where you've connected the LEDs into the arduino.
//The positions are: lightPins[6][2] = {{g1,w1},{g2,w2},{g3,w3},{g4,w4},{g5,w5},{g6,w6}};
//g1 = green light in column 1
//w2 = white light in column 2, etc.

//-----------button detection variables----------------------------------
#define BUTTONLISTLEN 6 
#define REQSTREAKLEN 5
int currentState = 0; //values 0,1,2,3,4,5 in real-time
int previousState = 0; //values 0,1,2,3,4,5 for what was previously pressed
int previousStreak = 1; //how many of the same value in a row we've seen
int buttonState = 0; //values 0,1,2,3,4,5 for what button is currently pressed

int buttonList[BUTTONLISTLEN]; //array for remembering the buttons in the current sequence
int buttonCounter = 0; //counts the number of buttons that have been pushed

int secretCode[BUTTONLISTLEN]; //array that tracks the secret code that you try to guess
int currentGuesses = 0; //how many guesses the players have made
char allGreen[BUTTONLISTLEN] = {'g','g','g','g','g','g'}; //all green code for comparisons

int activationCode[BUTTONLISTLEN] = {2,2,2,3,3,4}; //code needed to activate the game
bool isActivated; //blocks the game from working until set to true
bool isEscapeGame; //if the game is part of the escape game challenge
//-----------------------------------------------------------------

//------function prototypes------
bool isMatch(int *arr1, int *arr2);
bool isCharMatch(char *arr1, char *arr2);
void turnOnLights(char *lightArray);

void diagonalSnakeLights();
//--------------------------------

void setup() {
  Serial.begin(9600); //start communication from the arduino
  
  
  //setup escape game (set to true if part of escape room game,
  //set to false to just play the mastermind game itself)
  isEscapeGame = false;
  //start deactivated if part of escape game, start activated if just playing for fun.
  isActivated = !isEscapeGame; 
  
  //generate a random secret code
  randomSeed(analogRead(A5)); //set the seed based on an unconnected pin 
  generateSecretCode();

  //set up the LED+buzzer pin modes
  for (int i=2;i<=13;i++) {
    pinMode(i,OUTPUT); //LEDs
  }
  pinMode(buzzerPin,OUTPUT); //buzzer
  
} //end setup()

void loop() {
  //always check if a new button has been pressed
  currentState = detectButton(buttonPin); //1,2,3,4,5, or 0 (no button)
  //compare the button state to the previous state
  if (currentState == previousState) {
    //the states match, increment the streak counter
    previousStreak++;
    if (previousStreak == REQSTREAKLEN) {
      //an equilibrium has been reached, check if we need to update
      if (currentState != buttonState) {
        //This is a new state!
        buttonState = currentState;
        if (buttonState != 0) {
          //if this new state isn't returning us back to zero, trigger a button press
          updateButtonList(buttonState);
          printList(buttonList);
          if (!(buttonList[BUTTONLISTLEN-1] == 0)) { //if the last digit of the list isn't a 0, it's full!
            //update the LEDs to show the guess result
            if (isActivated) {
              //update LEDs for the new guess
              updateLEDs();
            }
            //check if we need to activate the game
            if (isActivated == false) {
              if (isMatch(buttonList,activationCode)) {
                activateTheGame(); //activate the game
              } else {
                //activation code was wrong
                playFailureTune();
              }
            }
            //now clear the button list
            clearButtonList();
          }
        } else {
          //testing 
          //Serial.println("zeroed");
          //printList(buttonList);
        }
      }
    }
  } else {
    //The state doesn't match the past state
    previousState = currentState; //update the new previous state
    previousStreak = 1; //reset the streak to 1
  }
} //end void loop()

//######## Button List Functions #########//

int detectButton(int pin) {
  //reads the value from the analog pin and determines what button is being pressed
  //WIP maybe make these values a list or array to be changed more easily
  int val = analogRead(pin); //the value of the analog pin, 0-1023
  if (val >= 760 && val <= 932) {
    //Serial.println(1); 
    return 1; //button 1, White
  } else if (val >= 588) {
    //Serial.println(2);
    return 2; //button 2, Red
  } else if (val >= 418) {
    //Serial.println(3);
    return 3; //button 3, Yellow
  } else if (val >= 250) {
    //Serial.println(4);
    return 4; //button 4, Green
  } else if (val >= 130 ) { //should be 82, but we were getting noise in the 80-100 range.
    //Serial.println(5);
    return 5; //button 5, Blue
  } else {
    return 0; //no button is being pressed
  }
} //end int detectButton()

void updateButtonList(int newButton) {
  //update the button list with the new button and the press counter
  buttonList[buttonCounter] = newButton; //add the new button in the next spot
  buttonCounter++; //increment the spot counter
}

void clearButtonList() {
  //clear the button list with zeros, and reset the counter
  memset(buttonList, 0, sizeof(buttonList));
  buttonCounter = 0;
}

void printList(int *listToPrint) {
  //prints the list of buttons in the sequence to the console for testing
  for (int i = 0; i < BUTTONLISTLEN; i++) {
    Serial.print(listToPrint[i]);
  }
  Serial.print("\n");
}

//######### End Button Functions #########//

void activateTheGame() {
  isActivated = true; 
  Serial.println("Activated!");
  playActivationTune();
  for (int i=0;i<2;i++) {
    diagonalSnakeLights();
  }
  
}

void generateSecretCode() {
  //generate a random secret code
  for (int i = 0; i < BUTTONLISTLEN; i++) {
    secretCode[i] = int(random(1,6)); //random number between 1 and 5 (min, max-1)
  }
  Serial.print("new secret code is: ");
  printList(secretCode);
}

bool isMatch(int *arr1, int *arr2) {
  //checks if two int arrays match
  //returns if the current button list matches the activation code
  bool match = true;
  for (int i = 0; i < BUTTONLISTLEN; i++) {
    if (arr1[i] != arr2[i]) {
      match = false;
      break;
    }
  }
  return match;
}

bool isCharMatch(char *arr1, char *arr2) {
  //checks if two char arrays match
  //returns if the current button list matches the activation code
  bool match = true;
  for (int i = 0; i < BUTTONLISTLEN; i++) {
    if (arr1[i] != arr2[i]) {
      match = false;
      break;
    }
  }
  return match;
}
//####### LED Functions ########///

void updateLEDs() {
  //update the display of the LEDs based on the player's guess
  //sets up an array of 6 characters and sends those to the turnOnLights function
  
  bool isUsed[BUTTONLISTLEN]; //if the place has been used already, set it to true
  memset(isUsed, false, sizeof(isUsed)); //set all places to not used
  char lightArray[BUTTONLISTLEN]; //light status ('w'hite,'g'reen,'o'ff)
  memset(lightArray, 'o', sizeof(lightArray)); //mark all lights as off
 
  // turn off all the lights
  turnOffLights();
  
  //compare the places, checking for direct matches
  for (int i = 0; i< BUTTONLISTLEN; i++) {
    if (buttonList[i] == secretCode[i]) {
      //if they match, put a green light in the light array and mark that place as used
      lightArray[i] = 'g';
      isUsed[i] = true;
    }
  }
  
  //once you've gotten through all 6 places checking greens, start checking for whites

  for (int i = 0; i<BUTTONLISTLEN; i++) {
    //for each place, check each other position in the code
    for (int k = 0; k<BUTTONLISTLEN; k++) {
      if ( (buttonList[i] == secretCode[k]) && (isUsed[k] == false) && (lightArray[i] == 'o') ) {
        //if it does, mark it with a white light and mark that other place as used
        lightArray[i] = 'w';
        isUsed[k] = true;
      }
    }
  }

  //turn on the lights based on the light array
  turnOnLights(lightArray);
  
  //check if all lights are green
  if (isCharMatch(lightArray,allGreen)) {
    //if they are, do something good
    playVictoryTune();
    turnOffLights();
    if (isEscapeGame == true) {
      showSecretCodeLights(); //show the secret code lights twice
      showSecretCodeLights();
    }
    turnOffLights();
    playActivationTune();
    diagonalSnakeLights();
    generateSecretCode();
    currentGuesses = 0;
  } else {
    //play the tune that says they were incorrect in their guess
    playIncorrectTune();
    //update the number of guesses that have been made
    currentGuesses+=1;
  }
  
  
  //if the number of guesses are 4, do something bad, reset the secret code  
  if (currentGuesses >= 4) {
    //reset the secret code
    generateSecretCode(); 
    currentGuesses = 0;
    playCodeResetTune();
    turnOffLights();
  }
}

void turnOnLights (char *newLightArray) {
  //turn on lights according to the recieved character array
  for (int i = 0; i<BUTTONLISTLEN; i++) {
    switch (newLightArray[i]) {
      case 'g': 
        digitalWrite(lightPins[i][0], HIGH); //turn on green light
        digitalWrite(lightPins[i][1], LOW);
        break;
      case 'w':
        digitalWrite(lightPins[i][0], LOW); //turn on white light
        digitalWrite(lightPins[i][1], HIGH);
        break;
      case 'o':
        digitalWrite(lightPins[i][0], LOW); //turn off both lights
        digitalWrite(lightPins[i][1], LOW);
        break;
      case 'b':
        digitalWrite(lightPins[i][0], HIGH); //turn on both lights
        digitalWrite(lightPins[i][1], HIGH);  
        break;
    }
  }
}

void turnOffLights() {
  char lightArray[6] = {'o','o','o','o','o','o'};
  turnOnLights(lightArray);
}

void diagonalSnakeLights() {
  int delayms = 50;
  turnOffLights();
  for (int i=0;i<6;i++) {
    digitalWrite(lightPins[i][i%2], HIGH);
    delay(delayms);
  }
  for (int i=0;i<6;i++) {
    digitalWrite(lightPins[5-i][i%2], HIGH);
    delay(delayms);
  }
  for (int i=0;i<6;i++) {
    digitalWrite(lightPins[i][i%2], LOW);
    delay(delayms);
  }
  for (int i=0;i<6;i++) {
    digitalWrite(lightPins[5-i][i%2], LOW);
    delay(delayms);
  }
}

void showSecretCodeLights() {
  //show the lights that will allow them to decode the secret code grid
  char lightArray1[6] = {'g','o','w','o','o','o'}; //look
  turnOnLights(lightArray1);
  playNote(nC4,0.5);
  delay(5000);
  char lightArray2[6] = {'o','o','o','o','g','w'}; //under
  turnOnLights(lightArray2);
  playNote(nD4,0.5);
  delay(5000);
  char lightArray3[6] = {'o','g','o','o','w','o'}; //couch
  turnOnLights(lightArray3);
  playNote(nE4,0.5);
  delay(5000);
  char lightArray4[6] = {'w','g','o','o','o','o'}; //can
  turnOnLights(lightArray4);
  playNote(nF4,0.5);
  delay(5000);
  char lightArray5[6] = {'o','o','o','g','w','o'}; //talk
  turnOnLights(lightArray5);
  playNote(nG4,0.5);
  delay(5000);
}

void playNote(int freq,float dur) {
  //duration in terms of quarter notes (1 = one quarter note)
  tone(buzzerPin,freq);
  delay(dur*quarterNoteDuration);
  noTone(buzzerPin);
  delay(50);
}

void playVictoryTune() {
  //G chord
  playNote(nF4,0.5);
  playNote(nD4,0.5);
  playNote(nB3,0.5);
  playNote(nD4,0.5);
  playNote(nB3,0.5);
  playNote(nG3,0.5);
  //switch to C chord
  playNote(nC3,0.5);
  playNote(nE3,0.5);
  playNote(nG3,0.5);
  playNote(nE3,0.6);
  playNote(nG3,0.6);
  playNote(nC4,0.6);
  playNote(nG3,0.7);
  playNote(nC4,0.7);
  playNote(nE4,0.7);
  playNote(nC4,0.8);
  playNote(nE4,0.85);
  playNote(nG4,0.9);
  playNote(nC5,2);
}

void playFailureTune() {
  //played when the wrong activation code is entered
  playNote(nE3,0.5);
  playNote(nE3,0.5);
}

void playCodeResetTune() {
  //played when the secret code is resetting after missing 4 guesses
  playNote(nA3,1);
  playNote(nF3,0.5);
  playNote(nE3,0.5);
  playNote(nD3,1);
  playNote(nGs3,1);
  playNote(nA3,1);
}

void playIncorrectTune() {
  //played when an incorrect guess is submitted
  playNote(nA4,0.5);
  playNote(nB3,0.5);
  playNote(nGs4,0.5);
  playNote(nD4,0.5);
  //playNote(nE4,1);
}

void playActivationTune() {
  //played when the game activates by the correct code being entered
  playNote(nG4,0.33);
  playNote(nC5,0.33);
  playNote(nG5,1.33);
}
