#include <Wire.h> //i2c communication
#include <OneWire.h> //one bus digital tempature sensor 
#include <LiquidCrystal.h> //LCD display

//CONSTANT AND VARIABLES------------------------------------------------------------------------------
int DS18S20_Pin = 13; //DS18S20 tempature sensor
LiquidCrystal lcd(2, 3, 4, 5, 6, 7, 8); //iniate LCD control pins on D2, D3, D4, D5, D6, D7, D8

//for LCD display
int score = 0;
int level = 0;

//for rand fixing, see improved rand for details
int numbers[] = {5, 5, 5, 5};

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2

//define analog control pins


//FUNCTIONS-------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onRequest(requestEvent); //data request from master
  Wire.onReceive(receiveEvent); //data receive from master
  
  lcd.begin(16, 2); //set lcd lines and char per lines
  
  //seed generation
  float temperature = getTemp();
  randomSeed(temperature); //seeding with current tempature with 2 decimal point accuracy
  
  //initial LCD setup
  lcd.print("Score: ");
  lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print("Level: ");
  lcd.print(level);
}

//when data is requested by the master
void requestEvent()
{ 
  int ySensorValue = analogRead(A2);
  int xSensorValue = analogRead(A1);
  int sSensorValue = analogRead(A0);
  
  int value;
  
  if(sSensorValue == 0) //select pin
   value = improvedRand(0, 7, 0);

  
  if(ySensorValue == 0) //down       
    value = improvedRand(8, 15, 0); 

    
  if(xSensorValue == 1023) //right    
    value = improvedRand(16, 23, 0); 
  else if(xSensorValue == 0 ) //left
    value = improvedRand(24, 31, 0);

  
  if(sSensorValue != 0 && xSensorValue != 1023 && xSensorValue != 0 && ySensorValue != 0) //if no value is sent from the analog input, send garbage data for piece randomisation
    value = improvedRand(32, 39, 0); 

  Wire.write(value);
  
}

//returns the temperature from one DS18S20 in DEG Celsius
float getTemp(){
  
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
}

//calculate level to display
int calculateLevel()
{
  int temp;
  
  temp = score/2;
  
  if(temp > 10) //LEVEL MAX 10
    temp = 10;
 
  return temp;
}

//Due to Arduino's random() tendancy to cluster numbers, we will keep record of previous generated piece, and stop
//a piece from appearing if it was one of the 4 last piece.
int improvedRand(int low, int maxi, int found)
{
  bool inDataSet = false;
  int count = found;
  
  int value = random(low, maxi);
  int temp = value%7;
  
  for(int i = 0; i < 4; i++)
  {
    if(temp == numbers[i])
      inDataSet = true;
  }
  
  if(inDataSet && count < 4)
  {    
    return improvedRand(low, maxi, count+1);
  }
  else
  {
   return value;   
  }
    
}

//receive data from the master (in this case, the score value)
void receiveEvent(int howMany)
{
  int data = Wire.read();
  

  
  //verify if the data is shape data 
  if(data >= 247 && data <= 254)
  {
    int temp = data - 247;
    //shape data, save it to bias random number generation
    numbers[0] = numbers[1];
    numbers[1] = numbers[2];
    numbers[2] = numbers[3];
    numbers[3] = temp;
   
     return;   
  }
  
  
  //if we reach this point, the data send by the master was score data 
  score = data;
  level = calculateLevel();
  
  if(score == 0) // if we receive score 0, a new game is starting
    lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("Score: ");
  lcd.print(score);
  
  lcd.setCursor(0, 1);
  lcd.print("Level: ");
  lcd.print(level);
}

//MAIN LOOP------------------------------------------------------------------------

void loop() 
{
  delay(100); // for stability
}



