#define BUFSIZE 255
#define PDBSIZE 10
#define IFS " ,;"

char key[20]="";
int incomingByte = 0; // for incoming serial data

struct pentry {
    char name[10]="Empty";
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
  incomingByte = 0;
  int bytesRead=0;
  while( (incomingByte != '\n') && (bytesRead < len) )
  {
    if (Serial.available() > 0) 
    {
      // read the incoming byte:
      incomingByte = Serial.read();
      buf[bytesRead]=incomingByte;
      bytesRead++;
    }
  }
  buf[bytesRead-1]='\0';
  return(bytesRead);
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
  Serial.println("Starting...");
  printpdb();
}

void loop() {
    int bytesRead=0, tmpid;
    char buff[BUFSIZE]= { 0 };
    char* token;
    

    // Print the prompt
    Serial.print("> ");
    bytesRead = readln(buff, BUFSIZE);
    Serial.println(buff);
    token = strtok(buff, IFS);
    Serial.println(token);
    if( strcmp(token,"add")==0 )
    {
      Serial.println("Add");
      // Read the id
      token = strtok(NULL, IFS);
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
    } 
    else if ( strcmp(token, "print")==0 )
    {
      printpdb();
    }
    else
    {
      Serial.println("Default");
    }
    delay(100);
}
