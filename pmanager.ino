#include <ctype.h>
#include "Keyboard.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// #include <Fonts/Org_01.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_DC     18
#define OLED_CS     10
#define OLED_RESET  19
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);



#define BUFSIZE 255
#define PDBSIZE 5
#define IFS " "
#define DEBOUNCE_TIME 50
#define SCREEN_REFRESH 100

#define DEBUG 0

int incomingByte = 0; // for incoming serial data
int serialbytesRead=0;
char mode='s';
char buff[BUFSIZE]="\0";
int bytesRead=0;
unsigned long lastScreenUpdate;

struct pentry 
{
    char tag[7];
    char pass[25];
};

struct key 
{
    byte pin;
    unsigned long lastChangeTime;
    bool prevVal;
    bool inTrans;
    bool active;
    byte debounce;
    bool sticky;
};

struct pentry pdb[PDBSIZE];
struct key keys[] = 
{
  { 4, 0, HIGH, false, false, 30, 0 },
  { 5, 0, HIGH, false, false, 30, 0 },
  { 6, 0, HIGH, false, false, 30, 0 },
  { 7, 0, HIGH, false, false, 30, 0 },
  { 8, 0, HIGH, false, false, 30, 0 },
  { 9, 0, HIGH, false, false, 50, 0 }
};
unsigned short numKeys = sizeof(keys)/sizeof(keys[0]);


void printUsage()
{
  Serial.println("Commands: add | print | help");
  Serial.println("Usage:");
  Serial.println("add INDEX TAG PASSWORD");
  //Serial.println("INDEX := [0..5]; TAG := STRING[10], PASSWORD := STRING[25]");
  //Serial.println("Addin over used index overwrites the entry!");
  Serial.println("print - prints the database.");
  //Serial.println("pk - prints the keys (debug info)");
  //Serial.println("help - prints this.");
}


void serialPrintpdb()
{
  // Print header
  // Serial.println("ID\tTAG\tLOGIN\tPASS");
  Serial.println("ID\tTAG\tPASS");
  for(int i=0;i<PDBSIZE;i++)
  {
    Serial.print(i+1); Serial.print("\t");
    Serial.print(pdb[i].tag); Serial.print("\t");
    if(strlen(pdb[i].pass)) Serial.print("************"); 
    Serial.println();
  }
}

int serialReadln(char * buf, int len)
{
  int result=0;
  char echo[1];
  
  while(Serial.available() > 0) 
  {
    // read the incoming byte:
    incomingByte = Serial.read();
    if( (incomingByte != '\n') && (incomingByte != '\r') && (serialbytesRead < len) )
    {
      buf[serialbytesRead]=incomingByte;
      serialbytesRead++;
      // echo as we type
      sprintf(echo, "%c", incomingByte);
      Serial.print(echo);
      #if (DEBUG > 1)
      Serial.print("%"); Serial.print(buf);
      Serial.print("%");
      Serial.println(incomingByte);
      #endif /* DEBUG */
    }
    else
    {
      buf[serialbytesRead]='\0';
      result=serialbytesRead;
      serialbytesRead=0;
      Serial.println();
      return(result);
    }
  }
  return(0);
 }

int serialCmd(char * buff)
{
    int tmpid;
    char* token;
    
    token = strtok(buff, IFS);
    Serial.println(token);

    // Add new passwrd entry
    if( strcmp(token,"add")==0 )
    {
      #if (DEBUG > 0)
      Serial.println("Add");
      #endif
      // Read the id
      token = strtok(NULL, IFS);
      // Check if the first argument is a number
      for(int j=0;j<strlen(token);j++)
      {
        if( isdigit(token[j]) )
          continue;
        Serial.println("ID must be a number!");
        return(10);
      }
      tmpid = atoi(token)-1;
      
      if(tmpid>PDBSIZE-1 || tmpid <0)
      {
        Serial.println("entry id out of range!");
        return(20);
      }
      // Read NAME
      token = strtok(NULL, IFS);
      strncpy(pdb[tmpid].tag, token, 6);
      pdb[tmpid].tag[6]='\0';
      /*
      // Read LOGIN
      token = strtok(NULL, IFS);
      strncpy(pdb[tmpid].login, token, 25);    
      */
      // Read PASS
      token = strtok(NULL, IFS);
      strncpy(pdb[tmpid].pass, token, 24);
      pdb[tmpid].pass[24]='\0';
    }
    // Delete entry( probably never gonna be used)
    else if ( strcmp(token,"del")==0)
    {
      #if (DEBUG > 0)
      Serial.println("Delete");
      #endif
      digitalWrite(10, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(30);                       // wait for a second
      digitalWrite(10, LOW);    // turn the LED off by making the voltage LOW
    }
    //Print the database to the serial port
    else if ( strcmp(token, "print")==0 )
    {
      #if (DEBUG > 0)
      Serial.println("PrintDB");
      #endif
      serialPrintpdb();
    }
    else if (strcmp(token,"help")==0)
    {
      printUsage();
    }
    else
    {
      #if (DEBUG > 0)
      Serial.println("Default");
      #endif
    }
}

void readKeys()
{
  unsigned long currentTime=millis();
  
  for(int i=0;i<numKeys;i++)
  {
    boolean currentVal = digitalRead(keys[i].pin);
    unsigned long transTime = currentTime - keys[i].lastChangeTime;

    if(keys[i].sticky==false) keys[i].active=false;
    
    if(currentVal != keys[i].prevVal)
    {
      keys[i].lastChangeTime=currentTime;
      if(currentVal == LOW)
      {
        keys[i].inTrans = true;
        #if (DEBUG > 0)
        Serial.println("[TRANS]");
        #endif
      }
      else
      {
        // Reset transition to debounce
        keys[i].inTrans = false;
        keys[i].active=false;
        #if (DEBUG > 0)
        Serial.println("[RELEASE]");
        #endif
      }
      keys[i].prevVal = currentVal;
      continue;
    }
    // Common debounce for all keys
    //if(keys[i].inTrans && (transTime>DEBOUNCE_TIME) )
    
    // each key can have different debounce time.
    if(keys[i].inTrans && (transTime>keys[i].debounce) )
    {
      keys[i].active=true;
      keys[i].inTrans=false;
      #if (DEBUG > 0)
      Serial.println("[ACTIVE]");
      #endif
    }
 
  }
}

void displayText(byte x, byte y, byte textSize, char mtext[64])
{
      display.clearDisplay();
      display.setTextSize(textSize);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(x, y);
      display.println(mtext);
      display.display();
}

void statusScreen()
{ 
  char tag[5];
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
   
  display.setCursor(0, 54);
  display.print("[1]");
  
  display.setCursor(36, 54);
  display.print("[2]");
  
  display.setCursor(68, 54);
  display.print("[3]");
 
  display.setCursor(105, 54);
  display.print("[4]");

  display.setCursor(105, 28);
  display.print("[5]");

  display.setCursor(85, 0);
  display.print(mode=='k'?"KBD ON":"KBD OFF");
  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("1."); display.print(pdb[0].tag);
  display.setCursor(0,15);
  display.print("2."); display.print(pdb[1].tag);
  display.setCursor(0,30);
  display.print("3."); display.print(pdb[2].tag);
  display.setCursor(55,15);
  display.print("4."); display.print(pdb[3].tag);
  display.setCursor(55,30);
  display.print("5."); display.print(pdb[4].tag);

  
  display.display();
}

// SETUP HERE
void setup() {
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  //display.setFont(&Org_01);
  displayText(0, 0, 1, "Waiting for serial connection...");

  Serial.begin(9600);
  while (!Serial) ;
  Serial.print("Keys on pins: ");
  for(int i;i<numKeys;i++)
  {
    pinMode(keys[i].pin, INPUT_PULLUP);
    Serial.print(keys[i].pin); Serial.print(" ");
  }
  Serial.println();
  Serial.println("Ready.");
  Serial.print( "> " );
}

void loop() {
    
    bytesRead = serialReadln(buff, BUFSIZE);
    if(bytesRead)
    {
      #if (DEBUG > 1)
      Serial.print(bytesRead); Serial.print(":"); Serial.println(buff);
      #endif /* DEBUG */
      bytesRead=0;
      serialCmd(buff);
      Serial.print( "> " );
    }
    readKeys();
    if(keys[0].active && mode=='k')
    {
      Keyboard.print(pdb[0].pass);
    }
    if(keys[1].active && mode=='k')
    {
      Keyboard.print(pdb[1].pass);
    }
    if(keys[2].active && mode=='k')
    {
      Keyboard.print(pdb[2].pass);
    }
    if(keys[3].active && mode=='k')
    {
      Keyboard.print(pdb[3].pass);
    }
    if(keys[4].active && mode=='k')
    {
      Keyboard.print(pdb[4].pass);
    }
    if(keys[5].active && mode=='s')
    {
      Serial.println("Keyboard on.");Serial.print( "> " );
      Keyboard.begin();
      mode='k';
    }
    else if( (keys[5].active) && mode=='k')
    {
      Serial.println("Keyboard off.");Serial.print( "> " );
      Keyboard.end();
      mode='s';
    }

    if((millis() - lastScreenUpdate)> SCREEN_REFRESH)
    {
      statusScreen();
      lastScreenUpdate = millis();
    }
}
