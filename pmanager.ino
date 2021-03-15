#include <ctype.h>

#define BUFSIZE 255
#define PDBSIZE 10
#define IFS " ,;"
#define DEBOUNCE_TIME 50

#define DEBUG 1

int incomingByte = 0; // for incoming serial data
int serialbytesRead=0;

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
};

struct pentry pdb[PDBSIZE];
struct key keys[] = 
{
  { 2, 0, HIGH, false, false, 100 }
 // { 3, 0, HIGH, false, false }
};
unsigned short numKeys = sizeof(keys)/sizeof(keys[0]);

void serialPrintpdb()
{
  // Print header
  Serial.println("ID\tNAME\tLOGIN\tPASS");
  for(int i=0;i<PDBSIZE;i++)
  {
    Serial.print(i); Serial.print("\t");
    Serial.print(pdb[i].name); Serial.print("\t");
    Serial.print(pdb[i].login); Serial.print("\t");
    Serial.print(pdb[i].pass); Serial.println();
  }
}

int serialReadln(char * buf, int len)
{
  int result=0;
  while(Serial.available() > 0) 
  {
    // read the incoming byte:
    incomingByte = Serial.read();
    if( (incomingByte != '\n') && (serialbytesRead < len) )
    {
      buf[serialbytesRead]=incomingByte;
      serialbytesRead++;
      #if (DEBUG > 1)
      Serial.print(".");
      #endif /* DEBUG */
    }
    else
    {
      buf[serialbytesRead]='\0';
      result=serialbytesRead;
      serialbytesRead=0;
      return(result);
    }
  }
  return(0);
 }

void serialCmd(char * buff)
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
    
    if(currentVal != keys[i].prevVal)
    {
      keys[i].lastChangeTime=currentTime;
      if(currentVal == LOW)
      {
        keys[i].inTrans = true;
        Serial.println("[TRANS]");
      }
      else
      {
        // Reset transition to debounce
        keys[i].inTrans = false;
        Serial.println("[RELEASE]");
/*
        if(transTime>keys[i].debounce)
        {
          //keys[i].inTrans = false;
          keys[i].active=true;
          Serial.println("[ACTIVE]");
        }
        else
        {
          Serial.println("[DEACT]");
        }
*/
      }
      keys[i].prevVal = currentVal;
      continue;
    }
    //if(keys[i].inTrans && (transTime>DEBOUNCE_TIME) )
 
    if(keys[i].inTrans && (transTime>keys[i].debounce) )
    {
      keys[i].active=true;
      keys[i].inTrans=false;
      Serial.println("[ACTIVE]");
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
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(incomingByte==0)
  {
    Serial.println("Waiting user.");
    if (Serial.available() > 0) 
    {
      // read the incoming byte:
      incomingByte = Serial.read();
    }
    delay(1000);
  }
  for(int i;i<numKeys;i++)
  {
    pinMode(keys[i].pin, INPUT);
    Serial.println(keys[i].pin);
  }
  Serial.println("Ready.");
  pinMode(10, OUTPUT);
  Serial.print( "> " );
}

void loop() {
    int bytesRead=0;
    char buff[BUFSIZE]= { 0 };

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
    if(keys[0].active)
    {
      Serial.println("[2]");
      keys[0].active=false;
    }
    //Serial.println(digitalRead(2));     
}
