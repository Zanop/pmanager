#include <ctype.h>
#include "Keyboard.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI  20
#define OLED_CLK   21
#define OLED_DC    18
#define OLED_CS    15
#define OLED_RESET 19
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
 
#define BUFSIZE 255
#define PDBSIZE 6
#define IFS " "
#define DEBOUNCE_TIME 50

#define DEBUG 0

int incomingByte = 0; // for incoming serial data
int serialbytesRead=0;
char mode='s';
char buff[BUFSIZE]="\0";
int bytesRead=0;

struct pentry 
{
    char name[10];
    char login[25];
    char pass[25];
};

struct key 
{
    unsigned short pin;
    unsigned long lastChangeTime;
    bool prevVal;
    bool inTrans;
    bool active;
    unsigned int debounce;
    bool sticky;
};

struct pentry pdb[PDBSIZE];
struct key keys[] = 
{
  { 4, 0, HIGH, false, false, 50, 0 },
  { 5, 0, HIGH, false, false, 50, 0 },
  { 6, 0, HIGH, false, false, 50, 0 },
  { 7, 0, HIGH, false, false, 50, 0 },
  { 8, 0, HIGH, false, false, 50, 0 },
  { 9, 0, HIGH, false, false, 50, 0 }
};
unsigned short numKeys = sizeof(keys)/sizeof(keys[0]);


void printUsage()
{
  Serial.println("Commands: add | print | pk | help");
  Serial.println("Usage:");
  Serial.println(" add INDEX NAME LOGIN PASSWORD");
  Serial.println("INDEX := [0..9]; NAME := STRING[10], LOGIN := STRING[25], PASSWORD := STRING[25]");
  Serial.println("Addin over used index overwrites the entry!");
  Serial.println("print - prints the database.");
  Serial.println("pk - prints the keys (debug info)");
  Serial.println("help - prints this.");
}

void serialPrintpdb()
{
  // Print header
  Serial.println("ID\tNAME\tLOGIN\tPASS");
  for(int i=0;i<PDBSIZE;i++)
  {
    Serial.print(i); Serial.print("\t");
    Serial.print(pdb[i].name); Serial.print("\t");
    Serial.print(pdb[i].login); Serial.print("\t");
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
    //Serial.println(token);

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
      tmpid = atoi(token);
      
      if(tmpid>PDBSIZE-1)
      {
        Serial.println("entry id out of range!");
        return(20);
      }
      // Read NAME
      token = strtok(NULL, IFS);
      strncpy(pdb[tmpid].name, token, 10);
      // Read LOGIN
      token = strtok(NULL, IFS);
      strncpy(pdb[tmpid].login, token, 25);    
      // Read PASS
      token = strtok(NULL, IFS);
      strncpy(pdb[tmpid].pass, token, 25);    
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
    else if (strcmp(token,"pk")==0)
    {
      #if (DEBUG > 0)
      Serial.println("PrintKeys");
      #endif
      printKeys();
    } 
    else if (strcmp(token,"help")==0)
    {
      //printUsage();
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

void printKeys()
{
  for(int i=0;i<numKeys;i++)
  {
    Serial.print(keys[i].pin);Serial.print(" ");
    Serial.print(keys[i].lastChangeTime);Serial.print(" ");
    Serial.print(keys[i].prevVal);Serial.print(" ");
    Serial.print(keys[i].inTrans);Serial.print(" ");
    Serial.print(keys[i].active);Serial.print(" ");
    Serial.println();
  }  
}

// SETUP HERE
void setup() {
  Serial.begin(9600);
  while (!Serial) ;

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
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
    int sum=0;
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
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(10, 0);
      display.println(F("Keys ON"));
      display.display();      // Show in
    }
    else if( (keys[5].active) && mode=='k')
    {
      Serial.println("Keyboard off.");Serial.print( "> " );
      Keyboard.end();
      mode='s';
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(10, 0);
      display.println(F("Keys OFF"));
      display.display();      // Show in
    }

//    display.display();
}
