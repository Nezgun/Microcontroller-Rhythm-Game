/*
//Author: Michael Hoffman
//This is a simple rhythm game using a microntroller.
*/
#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"

#include "queue.h" /* allows for queue data structure */
#include "main_asm.h" /* interface to the assembly module */

//--------------------------------------------------------------

//CONSTANTS
// Define note, pitch, & frequency
#define c     2867      // 261.63 Hz
#define d     2554      // 293.66 Hz
#define e     2276      // 329.63 Hz
#define f     2148      // 349.23 Hz
#define g     1914      // 392.00 Hz
#define a     1705      // 440.00 Hz
#define b     1519      // 493.88 Hz
#define C     1434      // 523.25 Hz
#define D     1277      // 587.33 Hz
#define E     1138      // 659.26 Hz
#define F     1074      // 698.46 Hz
#define G     957       // 783.99 Hz
#define A     853       // 880.00 Hz
#define B     760       // 987.77 Hz
#define CC    717       // 1046.50 Hz
#define DD    639       // 1174.66 Hz

//--------------------------------------------------------------

//Variables
//Hardware Variables
int LEDs; //Tracks blinking LEDs

//Settings Variables
int k;            //song list selecter
int playing;      //Allows for quit from game
int buffer;       //num of chars that will display before the beatmap begins
int selection;    //keypad selection
int songs;        //number of songs
int *activesong;  //selected song

//Gameplay Variables
int lives;        //lives counter
int score;        //score counter
int gametick;     //controls game flow during songs
int ticks;        //RTI Interrupt count
int songLength;   //Length of song indicated in index 0
int keyFlag;      //flag to indicate whether key has been pressed;

//Song Variables
int pitch;          //note pitch value
int noteCounter;    //Counter for Note Data
int durationCounter;//Counter for Sound Length Data
int soundCounter;   //Sound Counter
int bmpCounter;     //Beatmap counter
char newchar;       //placeholder for beatmap character
char activeChar;    //Character that's currently between the two bars |_|
char userChar;      //Character that the user enters

//--------------------------------------------------------------

//Constants
//--Switches--
const SW2 = 8;

//--LEDs--
const red = 16;
const green = 64;

//--Hex Keys--
const char key0 = 0x7D;
const char key1 = 0xEE;
const char key2 = 0xED;
const char key3 = 0xEB;
const char key4 = 0xDE;
const char key5 = 0xDD;
const char key6 = 0xDB;
const char key7 = 0xBE;
const char key8 = 0xBD;
const char key9 = 0xBB;
const char keyA = 0xE7;
const char keyB = 0xD7;
const char keyC = 0xB7;
const char keyD = 0x77;
const char keyE = 0x7E;
const char keyF = 0x7B;

//--------------------------------------------------------------

//Array Variables
const char bmpOptions[] = {'_', 'f', 'j', 'v', 'n'}; //Character options for beatmap.  Can be adjusted as desired
char bmpline[] = "|_|_ _ _ _ _ _ _ _ _ _0"; //Line that displays the beatmap to the player len = 23

//--------------------------------------------------------------

//Song Data

int aozora[] = {
36,
CC, B, A, G, E, F, G, G, A, B, 
CC, G, CC, B, CC, D, E, F, G, E, 
F, G, A, B, DD, CC, E, F, G, DD, 
E, F, G, A, CC, DD,

3, 3, 2, 4, 2, 1, 2, 2, 2, 2, 
8, 2, 2, 1, 2, 1, 9, 3, 3, 2, 
4, 1, 1, 2, 2, 4, 2, 3, 3, 1, 
1, 2, 2, 1, 1, 7,
 
1, 0, 0, 2, 0, 0, 3, 0, 4, 0,
0, 0, 4, 0, 1, 3, 0, 2, 0, 1, 
0, 2, 0, 3, 4, 3, 4, 3, 4, 3,
4, 1, 0, 1, 0, 2, 4, 0, 2, 1, 
0, 0, 0, 0, 0, 0, 0, 0, 3, 0,
0, 4, 0, 0, 1, 0, 3, 0, 2, 0,
0, 0, 1, 1, 3, 0, 3, 0, 4, 0,
0, 0, 1, 0, 3, 1, 3, 4, 2, 4,
1, 2, 3, 0, 4, 0, 3, 4, 1, 0,
0, 0, 0, 0, 0, -1  
};

int odetojoy[] = {
30,
E, E, F, G, G, F, E, D, C, C,
D, E, E, D, D, E, E, F, G, G,
F, E, D, C, C, D, E, D, C, C,

2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 3, 1, 4, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 3, 1, 4,

1, 0, 3, 0, 2, 0, 4, 0, 1, 0, 
2, 0, 3, 0, 4, 0, 3, 0, 4, 0,
1, 0, 1, 0, 2, 0, 0, 4, 4, 0, 
0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 
4, 0, 3, 0, 2, 0, 1, 0, 3, 0,
4, 0, 3, 0, 1, 0, 1, 0, 0, 2,
3, 0, 0, 0, -1 
};

//--------------------------------------------------------------

//Song Options
char *songlist[] = {"0: Quit", "1: Aozora", "2: Ode to Joy"};
int *songoptions[] = {aozora, odetojoy};

//--------------------------------------------------------------

//Function Declarations
void SCI_println(char *line);

void SCI_clearln(void);

void addNewChar(char *line, char newChar);

void shiftLeft(char *line);

void lineUpdate(char *line, char newChar); 

void LCD_gameDisplay(int lives,int score);

int checkKey(char char1, char char2);

void lineReset(char *line, char newChar,int buffer);

void init(void);

//---------------------------------------------------------------

//Interrupt routines
//Timer interrupt that controlles game flow.  
//Game can be sped up or slowed down by adjusting
//the ticks that it counts to
void interrupt 7 tickTimer() {
  ticks++;
  if(ticks == 15){
    set_servo76(6000); 
  }
  if(ticks == 30){
    gametick = 1;
    LEDs = (LEDs*2)%257; 
    PORTB ^= LEDs;
    set_servo76(3000);
    ticks = 0;
  }
  clear_RTI_flag();
}

//Sound Interrupt
void interrupt 13 soundHandler() {
  tone(pitch);
}

//SCI Interrupt that checks pressed key against
//Key currently in place
void interrupt 20 keyInput() {
  userChar = read_SCI0_Rx();
  if(checkKey(userChar, activeChar)){
    score += 10; 
    keyFlag = 0;
  } else {
    lives -= 1;
    keyFlag = 0; 
  }
}

//---------------------------------------------------------------

void main(void) {//start main
//Inner initialization
  PLL_init();             //Set phase lock loop
  playing = 1;
  
  //------------------------------------------------------------
  
  while(playing){// start while 0
    init();         //Initializes all hardware
    DDRP = 0xFF;    //Important for RGB       
    buffer = 10;
    songs = 3;  //hard coded number of songs + 1  for quit menu
    LEDs = 1;   //Init to start off flashing lights
    
    //Introduction Sequence
    clear_lcd();
    set_lcd_addr(0x00);
    type_lcd("Press SW2");
    set_lcd_addr(0x40);
    type_lcd("to begin!");
    while(PTH&SW2); //Press a button to start the game
    
    SCI_println("Welcome!\n0");
    ms_delay(1000);
    SCI_println("Using the keypad, select your choice!\n0");
    ms_delay(1000);
    SCI_println("Press '*' on the keypad to scroll forward.\n0");
    SCI_println("---------------------------------------\n0");
    ms_delay(1000);
    
    //Song Selection Loop
    while(1){//start while 1     
      //write out songs to LCD
      clear_lcd();
      set_lcd_addr(0x00);
      type_lcd(songlist[k]);
      set_lcd_addr(0x40);
      type_lcd(songlist[(k+1)%songs]); //wraps around songlist
               
      selection = getkey();
      wait_keyup();
      //Menu Selection.  More options can be added later for more songs.            
      switch(selection){
        case 0: //Quit
          playing = 0;
          clear_lcd();
          set_lcd_addr(0x00);
          type_lcd("Quitting...");
          return;
        case 1: //Choose Song 1
          activesong = songoptions[0];
          break;
        case 2:
          activesong = songoptions[1];//Choose Song 2
          break;
        case 3:
          continue;
        case 4:
          continue;
        case 5:
          continue;
        case 6:
          continue;
        case 7:
          continue;
        case 8:
          continue;
        case 9:
          continue;
        case 10:
          continue;
        case 11:
          continue;
        case 12:
          continue;
        case 13:
          continue;
        case 14:
          k = (k+1)%songs;
          break;
        case 15:
          continue; 
      } //End Switch
      if(selection != 14){        
        //Confirm Choice
        SCI_println("Press '*' to begin or any other key to change selection.\n0");
        selection = getkey();
        wait_keyup();
        if(selection == 14){ 
          break;
        }
        SCI_println("Please make a new selection.\n0");
      }  
    }//end while 1
    
    //--------------------------------------------------------------
    
    //GAME LOOP initialization
    lives = 10;
    score = 0;
    songLength = activesong[0];
    clear_lcd();
    set_lcd_addr(0x00);
    type_lcd("Score: ");
    set_lcd_addr(0x40);
    type_lcd("Lives: ");
    LCD_gameDisplay(lives, score);
                                                             
    //start sequence
    SCI_println("3...\n0");
    ms_delay(1000);
    SCI_println("2...\n0");
    ms_delay(1000);
    SCI_println("1...\n0");
    ms_delay(1000);
    SCI_println("START!\n0");
    ms_delay(500);
    SCI_println("******************************\n0");
    SCI_println(bmpline);
    
    
    //Initialize Game Variables
    bmpCounter = 1 + 2 * songLength;  //Counter for beatmap 
    noteCounter = 1;
    durationCounter = songLength + 1;  //Duration counter is the counter for note duration information;
    gametick = 0;
    soundCounter = 1;
    keyFlag = 0;
    
    while(1){ //while 3
    
      //Things to do during gametick
      if(gametick){ //tick operations
      
        //If lights were on, turn them off
        PTP &= green;
        PTP &= red;
        
        //Update Score
        LCD_gameDisplay(lives, score);
        
        //Choose Between Buffer and Main        
        if(buffer){
          lineUpdate(bmpline, bmpOptions[activesong[bmpCounter]]);
          activeChar = bmpline[1];
          bmpCounter++;
          buffer--;    
        } else { //post buffer operations
          if(keyFlag){
            PTP |= red;
            lives --; 
          }
          //BMP Update
          if(activesong[bmpCounter] >= 0){ //only updates while characters exist in beatmap
            lineUpdate(bmpline, bmpOptions[activesong[bmpCounter]]);
            activeChar = bmpline[1];
            
            //Makes it so that you don't lose lives for not pressing anything on blanks
            //While you will lose lives if you don't press the key on a normal option
            if(activeChar == bmpOptions[0]){
              keyFlag = 0; 
            } else {
              keyFlag = 1; 
            }
            
            bmpCounter++;  
          } else {  //If reached the end of the beatmap, game ends and returns to start
            lineUpdate(bmpline, bmpOptions[0]);
            activeChar = bmpline[1]; 
          }
          //---------------------------------------------------------
          
          //Sound Update
          soundCounter--;
          
          //If the sound has played for its duration, the note and sound counter
          //are updated to the next and the played note adjusts
          if(soundCounter == 0){
            pitch = activesong[noteCounter];
            soundCounter = activesong[durationCounter];
            sound_init();
            sound_on();
            
            
            durationCounter++;
            noteCounter++;    
          }
          //----------------------------------------------------------
          
          if(noteCounter == songLength + 2){
            SCI_println("\nSong Cleared!\n0");
            sound_off();
            ms_delay(5000);
            break; 
          }
                                                             
        }
        //Reset Tick Variables
        gametick = 0;
      } //end tick operation
      
      //If you run out of lives, game ends and returns to main menu
      if(lives <= 0){
        SCI_println("\nGame Over!\n0");
        sound_off();
        ms_delay(5000);
        break; 
      }
      //---------------------------------------------------------------
      
    } // end while 3
    
    //Reset Line and return to start
    lineReset(bmpline, bmpOptions[0], buffer);
    SCI_println("Returning to start menu...\n0");
    ms_delay(500);
    SCI_println("******************************\n0");
  }//end while0
}//end main

//-----------FUNCTIONS--------------------------------------------------

// Checks to see if the keys match
int checkKey(char char1, char char2){
  if(char1 == char2){
    PTP |= green;
    return 1; 
  } else {
    PTP |= red;
    return 0; 
  }
}

//Function to initialize various hardware
void init(void){
  SCI0_int_init(9600);    //Enable SCI0
  RTI_init();             //Enable interrupts
  SW_enable();            //Enable switches
  led_enable();           //Enable LEDs
  lcd_init();             //Enable LCD
  keypad_enable();        //Enable keypad
  servo76_init();         //Enable Servo 
  
}

//Function that displays game state to LCD
void LCD_gameDisplay(int lives,int score){
  set_lcd_addr(0x0A);
  write_int_lcd(score);
  set_lcd_addr(0x4A);
  write_int_lcd(lives);  
}

//Function that prints out a line to SCI
void SCI_println(char *line) {
  int i = 0;
  while(line[i] != '0') {
    outchar0(line[i]);
    i++;
  }
}

//Function to clear line in SCI
void SCI_clearln(void) {
  int i;
  for(i = 0; i < 22; i++){
    outchar0(0x08); 
  }
}

//Function to add a new character to the beatmap display
void addNewChar(char *line, char newChar){
  shiftLeft(line);
  line[21] = newChar;
}

//Function to shift all the active characters left in the beatmap
void shiftLeft(char *line){
  int i;
  for(i = 1; i <= 19; i += 2){
    line[i] = line[i+2];  
  }
}

//Function that performs a full line update to the beatmap display
void lineUpdate(char *line, char newChar){
  SCI_clearln();
  addNewChar(line, newChar);
  SCI_println(line);
}

//Function to reset the bmpline after each game
void lineReset(char *line, char newChar,int buffer){
  int i;
  for(i = 0; i < buffer; i++){
    addNewChar(line, newChar); 
  }
}