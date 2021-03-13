#include <ctype.h>

#define BUFSIZE 255
#define PDBSIZE 10
#define IFS " ,;"

char key[20]="";
int incomingByte = 0; // for incoming serial data
int serialbytesRead=0;

struct pentry {
    char name[10];
    char login[25];
    char pass[25];
};

struct pentry pdb[PDBSIZE];

void printpdb()
{
  // Print header
  Serial.println("NAME\tLOGIN\tPASS");
  for(int i=0;i<PDBSIZE;i++)
  {
    Serial.print(pdb[i].name); Serial.print("\t");
    Serial.print(pdb[i].login); Serial.print("\t");
    Serial.print(pdb[i].pass); Serial.println();
  }
}

int readln(char * buf, int len)
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
      Serial.print(".");
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
    if( strcmp(token,"add")==0 )
    {
      Serial.println("Add");
      // Read the id
      token = strtok(NULL, IFS);
      for(int j=0;j<strlen(token);j++)
      {
        if( isdigit(token[j]) )
          continue;
        Serial.println("ID must be a number!");
        return(10);
      }
      tmpid = atoi(token);
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
    else if ( strcmp(token,"del")==0)
    {
      Serial.println("Delete");
      digitalWrite(10, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(30);                       // wait for a second
      digitalWrite(10, LOW);    // turn the LED off by making the voltage LOW
    } 
    else if ( strcmp(token, "print")==0 )
    {
      printpdb();
    }
    else
    {
      Serial.println("Default");
    }
}

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
  
      // say what you got:
      // Serial.print("I received: ");
    // Serial.println(incomingByte, DEC);
    }
    delay(1000);
  }
  Serial.println("Ready.");
  pinMode(10, OUTPUT);
  Serial.print( "> " );
}

void loop() {
    int bytesRead=0;
    char buff[BUFSIZE]= { 0 };

    bytesRead = readln(buff, BUFSIZE);
    if(bytesRead)
    {
      Serial.print(bytesRead); Serial.print(":"); Serial.println(buff);
      
      serialCmd(buff);
      Serial.print( "> " );
    }
    bytesRead=0;
}
