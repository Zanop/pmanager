#define BUFSIZE 255
#define PDBSIZE 10

char key[20]="";
int incomingByte = 0; // for incoming serial data

struct pentry {
    int id;
    char name[10]="Empty";
    char pass[25]="****";  
};

struct pentry pdb[PDBSIZE];

void printpdb()
{
  // Print header
  Serial.println("ID\tNAME\tPASS");
  for(int i=0;i<PDBSIZE;i++)
  {
    Serial.print(pdb[i].id);
    Serial.print("\t");
    Serial.print(pdb[i].name);
    Serial.print("\t");
    Serial.print(pdb[i].pass);
    Serial.println();
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
    Serial.println("Waiting user...");
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
  Serial.println("Started.");
  printpdb();
}

void loop() {
    int bytesRead=0;
    char buff[BUFSIZE]= { 0 };
    bytesRead = readln(buff, BUFSIZE);
    Serial.println(buff);
    delay(100);
}
