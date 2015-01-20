#include <Adafruit_GFX.h>   // Core graphics library  https://github.com/adafruit/Adafruit-GFX-Library 
#include <RGBmatrixPanel.h> // Hardware-specific library https://github.com/adafruit/RGB-matrix-Panel
#include <Wire.h> //standard analog control libraire, used for master/slave communication using A4 and A5

//global setups--------------------------------------------------------------------------------

//hardware control Pins
#define CLK 8  //clock
#define LAT A3 //data latch
#define OE  9  
#define A   A0  //control A
#define B   A1  //control B
#define C   A2  //control C

//RGB color values
#define YELLOW 7, 7, 0
#define CYAN 0, 7, 7
#define PURPLE 4, 0, 7
#define BLUE 0, 0, 7
#define ORANGE 7, 1, 0
#define RED 7, 0, 0
#define GREEN 0, 7, 0
#define ERASE 0, 0, 0

// Colors for encoding
enum colors {none_e = 0, square_e = 1, line_e = 2, rightL_e = 3,leftL_e = 4, rightS_e = 5, leftS_e = 6, tshape_e = 7};

// The game state contains the colors values for each pixel square of a line.
unsigned long gameState[16];

//setup matrix with control pins
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

//global for pixel values;
char vP1, hP1, vP2, hP2, vP3, hP3, vP4, hP4;

//Score
short score = 0;

//Speed & acceleration
int _speed = 825;  //current speed
int speedSave = 825; //real speed
int acc = 50;  //accelerated speed mode

//Encoding/Decoding----------------------------------------------------------------------------------------------------------

// MAX INT
unsigned long mask = 4294967295;

// Decode a square pixel from a line
unsigned long decode(unsigned long encoded,char i){
	unsigned long temp;
	temp = encoded << (29-((7-i)*3));
	return (mask & temp>>29);
}

// Encode an array of 8 pixel squares to represent a line
unsigned long encode(unsigned long* decoded){
	unsigned long temp = 0;
	unsigned long encoded = 0;
	for(unsigned long i = 0; i < 8; ++i){
		temp = decoded[i] << (21-(i*3));
		encoded += temp;
	}
	return encoded;	 
}

//Functions-------------------------------------------------------------------------------------------------------------------------

//inital setup
void setup() {
  matrix.begin(); //initialize de LED panel

  Wire.begin(); //begin communication with other arduino, start as master
 }
 
   
//Draw a shape according to the global pixel positions(which defines the shape and it's rotation) and the provided offsets and colors
void drawShape(short verticalOffset, short horizontalOffset, char c1, char c2, char c3)
 {
     matrix.fillRect(vP1 - verticalOffset, hP1 + horizontalOffset, 2, 2, matrix.Color333(c1, c2, c3));
     matrix.fillRect(vP2 - verticalOffset, hP2 + horizontalOffset, 2, 2, matrix.Color333(c1, c2, c3));
     matrix.fillRect(vP3 - verticalOffset, hP3 + horizontalOffset, 2, 2, matrix.Color333(c1, c2, c3));
     matrix.fillRect(vP4 - verticalOffset, hP4 + horizontalOffset, 2, 2, matrix.Color333(c1, c2, c3));   
 }

//Draw a Pixel 
void draw(short verticalOffset, short horizontalOffset, char c1, char c2, char c3){
     matrix.fillRect(30 - verticalOffset, horizontalOffset, 2, 2, matrix.Color333(c1, c2, c3)); 
}
 
//Blinking red effect when loosing a game
void lostDraw(){
       matrix.fillRect(16,0, 16, 16, matrix.Color333(RED));
       matrix.fillRect(0,0, 16, 16, matrix.Color333(RED)); 
       
       short value;
       
       do  //keep game paused until joystick is pressed
       {
         Wire.requestFrom(2,1);
         value = Wire.read();      
       }while(value > 7);
 
       matrix.fillRect(16,0, 16, 16, matrix.Color333(ERASE));
       matrix.fillRect(0,0, 16, 16, matrix.Color333(ERASE));      
}

// Erase previous parts of the shape (while moving or rotating)
void erase(short Voffset, short Hoffset, short Vvalue, short Hvalue){
     matrix.fillRect(vP1 - Voffset  + Vvalue, hP1 + Hoffset + Hvalue, 2, 2, matrix.Color333(ERASE));
     matrix.fillRect(vP2 - Voffset  + Vvalue, hP2 + Hoffset + Hvalue, 2, 2, matrix.Color333(ERASE));
     matrix.fillRect(vP3 - Voffset  + Vvalue, hP3 + Hoffset + Hvalue, 2, 2, matrix.Color333(ERASE));
     matrix.fillRect(vP4 - Voffset  + Vvalue, hP4 + Hoffset + Hvalue, 2, 2, matrix.Color333(ERASE)); 
}

//define the initial value of a shape pixel's 
void defineShape(char P1v, char P1h, char P2v, char P2h, char P3v, char P3h, char P4v, char P4h) 
{      
       
        vP1 = P1v;
        hP1 = P1h;
        vP2 = P2v;
        hP2 = P2h;
        vP3 = P3v;
        hP3 = P3h;
        vP4 = P4v;
        hP4 = P4h;
}

//modifies acceleration if players pushes down on the joystick
void setAcceleration(int value)
{ 
   if (8 <= value && value < 15)
             _speed = acc;
   else 
             _speed = speedSave;
} 

// refresh the game state and LED matrix with the line above the current line
void fullLine(char line){  
  if(line > 0)
    gameState[line] = gameState[line-1];
  else
    gameState[line] = 0;
    
    for(char i = 0; i < 8; ++i){
      if(decode(gameState[line],i) == 0)
        draw(2*line,2*i,ERASE);
      else if(decode(gameState[line],i) == 1){
        draw(2*line,2*i,YELLOW);
      }
      else if(decode(gameState[line],i) == 2){
        draw(2*line,2*i,CYAN);
      }
      else if(decode(gameState[line],i) == 3){
        draw(2*line,2*i,PURPLE);
      }
      else if(decode(gameState[line],i) == 4){
        draw(2*line,2*i,BLUE);
      }
      else if(decode(gameState[line],i) == 5){
        draw(2*line,2*i,RED);
      }
      else if(decode(gameState[line],i) == 6){
        draw(2*line,2*i,GREEN);
      }
      else{
        draw(2*line,2*i,ORANGE);
      }
    }  
}

void sendData(short data)
{
  Wire.beginTransmission(2); // transmit to device #2
  Wire.write(data);              // sends score to slave device 
  Wire.endTransmission();    // stop transmitting
}

void SaveL(char e,unsigned long* line,unsigned long* line2,unsigned long* line3,char Hoffset,char Voffset,char hP1,char hP2,char hP3,char hP4,unsigned long* gameState,char state){
 for(unsigned long i = 0; i < 8; ++i){
             line[i] = 0;
             line2[i] = 0;
             line3[i] = 0;     
       }
       if(state%4 == 0){
         line[(Hoffset+hP2)/2] = e;
         line[(Hoffset+hP1)/2] = e;
         line2[(Hoffset+hP3)/2] = e;
         line3[(Hoffset+hP4)/2] = e;
         
         gameState[Voffset/2] += encode(line);
         gameState[(Voffset/2)+1] += encode(line2);
         gameState[(Voffset/2)+2] += encode(line3);
       }  
       else if(state%4 == 1 && e == leftL_e){
         line[(Hoffset+hP3)/2] = e;
         line2[(Hoffset+hP2)/2] = e;
         line2[(Hoffset+hP1)/2] = e;
         line2[(Hoffset+hP4)/2] = e;
         
         gameState[(Voffset+2)/2] += encode(line);
         gameState[(Voffset+4)/2] += encode(line2);
       }
       else if(state%4 == 1 && e == rightL_e){
         line[(Hoffset+hP1)/2] = e;
         line[(Hoffset+hP2)/2] = e;
         line[(Hoffset+hP3)/2] = e;
         line2[(Hoffset+hP4)/2] = e;
         
         gameState[Voffset/2] += encode(line);
         gameState[(Voffset+2)/2] += encode(line2);
       }
       else if(state%4 == 2){
         line[(Hoffset+hP4)/2] = e;
         line2[(Hoffset+hP3)/2] = e;
         line3[(Hoffset+hP1)/2] = e;
         line3[(Hoffset+hP2)/2] = e;
         
         gameState[Voffset/2] += encode(line);
         gameState[(Voffset+2)/2] += encode(line2);
         gameState[(Voffset+4)/2] += encode(line3);
       }
       else if(state%4 == 2 && e == rightL_e){
         line[(Hoffset+hP3)/2] = e;
         line2[(Hoffset+hP4)/2] = e;

         gameState[Voffset/2] += encode(line);
         gameState[(Voffset+2)/2] += encode(line2);
         gameState[(Voffset+4)/2] += encode(line3);
       }
       else{
         line[(Hoffset+hP3)/2] = e;
         line2[(Hoffset+hP4)/2] = e;
         line2[(Hoffset+hP2)/2] = e;
         line2[(Hoffset+hP1)/2] = e;
         

         if(e == rightL_e){
         gameState[(Voffset+2)/2] += encode(line);
         gameState[(Voffset+4)/2] += encode(line2);
         }
         else{
         gameState[Voffset/2] += encode(line2);
         gameState[(Voffset+2)/2] += encode(line);
        }
     }
}
//Game loop----------------------------------------------------------------------------------------------------------
 void loop() {
  
  
  Wire.requestFrom(2,1);
  short shape = Wire.read()%7; //choose a shape
  sendData(247+shape);
  short Voffset = 0, Hoffset = 0, count, state = 0; //set shape to starting point
  bool lost = false;


  //verify if a full line is present in game board
  for(char i = 0; i < 16; ++i){
      bool full = true;
      for(char j = 0; j < 8; ++j){		
         if(decode(gameState[i],j)== 0){
           full = false; 
         }
      }
      if(full){// If there's a full line, refresh every higher lines(the ones below do not change)
        for(int j = i; j >0; j--)
          fullLine(j);
        score+=1;
        if(score%2 == 0){
          if(speedSave > 75) 
          speedSave-=75; 
        }
      }
  }

//verify if the player is game over  
 for(char k = 0; k < 8; k++){
    if (decode(gameState[0],k) != 0){// if the last line is full, well too bad you just lost
       lostDraw();
        //wait for joystick click before starting next game
       //Reset the game state
       for(char i = 0; i < 16; ++i){
         gameState[i] = 0;
       }
       //Reset score and speed
       score = 0;
       speedSave = 750;
       break;
     }
  }
  sendData(score);
  //set speed
  _speed = speedSave;
  
  
  // Shape generation

 // General Statements for the inner workings of the shape generation and movement:

  //First we verify the value on PIN_X, with a value of over 1000 we must move right, with a value of less then 20 we move left. We use a
  //count %100 for fluidity of the game (delay the movement or rotation of the shape) this stops the shape from moving to one side of the board in a single stroke
  //Next we verify that the shape is not trying to it's boundaries (i.e: Hoffset < 6). Lastly, we verify that the shape is able to move according to the shapes
  //already in play. We do this by decoding the state of the game at a specific position, comparing if it is empty, if it is, we can move. We must verify more then one
  //pixel before allowing a single move.

  //To make a shape move we simply erase the current shape on screen, and redraw the shape with the new offset. The shape rotation simply involve changing the starting position
  //of each pixel, while keeping the current offsets. Once a shape can no long move (Reached the bottom line or is in a vertical colision with another shape) we add this shape to the game
  //state.
  
  switch(shape){
    case 0:
    //set pixel positions initial state;
        defineShape(30, 6, 28, 6, 30, 8, 28, 8);

        //YELLOW Square
        for(int i = 0; i < 16; i++){
         count = 0;
         drawShape(Voffset,Hoffset, YELLOW); //initial shape draw
         while(count < (_speed)){// use of _speed variable to determine how fast the shapes goes down(the smaller _speed is the faster the shape goes down)
           drawShape(Voffset,Hoffset, YELLOW);
            Wire.requestFrom(2,1);
            int value = Wire.read();
           if((16 <= value && value < 23) && (count%100 == 0) && (Hoffset < 6) && (decode(gameState[i+1],(hP3 + Hoffset + 2)/2) == 0)&& (decode(gameState[i],(hP3 + Hoffset + 2)/2) == 0)){//move right
             Hoffset += 2;
             matrix.fillRect(vP1 - Voffset, hP1 + Hoffset -2, 2, 2, matrix.Color333(ERASE));
             matrix.fillRect(vP2 - Voffset, hP2 + Hoffset -2, 2, 2, matrix.Color333(ERASE)); 
           }
           else if((24 <= value && value < 31) && (count%100 == 0) && (Hoffset > -6) && (decode(gameState[i+1],(hP1 + Hoffset - 2)/2) == 0)&& (decode(gameState[i],(hP1 + Hoffset - 2)/2) == 0)){//move left
             Hoffset -= 2;
             matrix.fillRect(vP3 - Voffset, hP3 + Hoffset +2, 2, 2, matrix.Color333(ERASE));
             matrix.fillRect(vP4 - Voffset, hP4 + Hoffset +2, 2, 2, matrix.Color333(ERASE)); 
           }

          setAcceleration(value);
           
         count++;
         }
         
           if((i < 14) && ((decode(gameState[i+2],(hP1+Hoffset)/2) == 0) && ((decode(gameState[i+2],(hP3+Hoffset)/2) == 0)))){//move down
           Voffset += 2;
           matrix.fillRect(vP1 - Voffset +2, hP1 + Hoffset, 2, 2, matrix.Color333(ERASE));
           matrix.fillRect(vP3 - Voffset +2, hP3 + Hoffset, 2, 2, matrix.Color333(ERASE));
           }
           else{
             break;
           } 
       }

       // Filling the game state with the newly inserted shape
       unsigned long line[8];
       for(unsigned long i = 0; i < 8; ++i)
             line[i] = 0;
       line[(Hoffset/2)+3] = square_e;
       line[(Hoffset/2)+4] = square_e;
       
       gameState[Voffset/2] += encode(line);
       gameState[(Voffset/2)+1] += encode(line);
       break;
  case 1:
        //set pixels initial state;
        defineShape(30, 6, 28, 6, 26, 6, 24, 6);
        //CYAN line
        for(int i = 0; i < 17; ++i){
         count = 0;
         drawShape(Voffset,Hoffset, CYAN); //initial shape draw
         while(count < _speed){
           Wire.requestFrom(2,1);
           int value = Wire.read();
           drawShape(Voffset,Hoffset, CYAN);
           if(
           ((state%2 == 1) && ((16 <= value && value < 23) && (count%100 == 0 && Hoffset < 4)) && (decode(gameState[i],(Hoffset+hP3+2)/2) == 0))
           ||
           ((state%2 == 0) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 7)) 
               && (decode(gameState[i],(Hoffset+hP1+2)/2) == 0) 	//checking pixels to the right
               && (decode(gameState[i+1],(Hoffset+hP1+2)/2) == 0) 
               && (decode(gameState[i+2],(Hoffset+hP1+2)/2) == 0)
               && (decode(gameState[i+3],(Hoffset+hP1+2)/2) == 0) ))){//move right
            
             Hoffset += 2;
             erase(Voffset, Hoffset, 0, -2);
           }
           else if(
           ((state%2 == 1) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -4)) && (decode(gameState[i],(Hoffset+hP4-2)/2) == 0))
           ||
           ((state%2 == 0) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -6))
             && (decode(gameState[i],(Hoffset+hP1-2)/2) == 0) 		//checking pixels to the left
             && (decode(gameState[i+1],(Hoffset+hP1-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP1-2)/2) == 0)
             && (decode(gameState[i+3],(Hoffset+hP1-2)/2) == 0) ))){//move left
           
            Hoffset -= 2;
            erase(Voffset, Hoffset, 0, 2);
           }
           else if((
           			//verify neighboring pixels before rotation 1
                    (state%2 == 0) &&  
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    (decode(gameState[i],(hP1+Hoffset-2)/2) == 0) &&
                    (decode(gameState[i],(hP1+Hoffset+2)/2) == 0) &&
                    (decode(gameState[i],(hP1+Hoffset+4)/2) == 0) &&
                    (((hP1+Hoffset-2)/2) >= 0) &&
                    (((hP1+Hoffset+4)/2) <= 7))
                    
                    ||

                    //verify neighboring pixels before rotation 2
                    ((state%2 == 1) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+3) < 16) &&
                    (decode(gameState[i+1],(hP1+Hoffset)/2) == 0) &&
                    (decode(gameState[i+2],(hP1+Hoffset)/2) == 0) &&
                    (decode(gameState[i+3],(hP1+Hoffset)/2) == 0))){
             
                       erase(Voffset, Hoffset, 0, 0);
                       switch(state % 2)
                       {
                       	//change initial pixel states for rotation 1
                         case 0:
                         defineShape(30, 6, 30, 8, 30, 10, 30, 4);
                           break;
                        //change initial pixel states for rotation 1
                        case 1:
                        defineShape(30, 6, 28, 6, 26, 6, 24, 6);
                       }
                       state++;
                     }
                     setAcceleration(value);
                     count++;
         }
           if( 
           	//moving down, according to shape's current rotation
           ((i < 15) && (state%2 == 1) && (decode(gameState[i+1],(hP1+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP3+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP4+Hoffset)/2) == 0))
           ||
           ((i < 12) && (state%2 == 0) &&  (decode(gameState[i+1],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+2],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+4],(hP2+Hoffset)/2) == 0))){//move down
             Voffset += 2;
              erase(Voffset, Hoffset, 2, 0);
           }
           else{
             break;
           }
       }
       //save shape to game state
       for(unsigned long i = 0; i < 8; ++i)
             line[i] = 0;
       if(state%2 == 0){
         line[(Hoffset+hP1)/2] = line_e;
         
         gameState[Voffset/2] += encode(line);
         gameState[(Voffset/2)+1] += encode(line);
         gameState[(Voffset/2)+2] += encode(line);
         gameState[(Voffset/2)+3] += encode(line);
       }  
       else{
         line[(Hoffset+hP1)/2] = line_e;
         line[(Hoffset+hP2)/2] = line_e;
         line[(Hoffset+hP3)/2] = line_e;
         line[(Hoffset+hP4)/2] = line_e;
         
         gameState[Voffset/2] += encode(line);
       }
      break;
      
   case 2:
        //set pixels initial state;
        defineShape(30, 8, 30, 6, 28, 6, 26, 6);
        
        //PURPLE right L shape
        for(int i = 0; i < 16; ++i){
         count = 0;
         drawShape(Voffset,Hoffset, PURPLE); //initial shape draw
         while(count < _speed){
         Wire.requestFrom(2,1);
         int value = Wire.read();
           drawShape(Voffset,Hoffset, PURPLE);
           if(//move right
           	//verifying right movement according to current shape rotation
           ((state%4 == 1) && ((16 <= value && value < 23) && (count%100 == 0 && Hoffset < 4)) && (decode(gameState[i],(Hoffset+hP3+2)/2) == 0)&& (decode(gameState[i+1],(Hoffset+hP4+2)/2) == 0) )

           ||

           ((state%4 == 0) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 6)) 
               && (decode(gameState[i],(Hoffset+hP1+2)/2) == 0) 
               && (decode(gameState[i+1],(Hoffset+hP3+2)/2) == 0) 
               && (decode(gameState[i+2],(Hoffset+hP4+2)/2) == 0)))

           ||

           ((state%4 == 3) && ((16 <= value && value < 23) && (count%100 == 0 && Hoffset < 4)) && (decode(gameState[i+1],(Hoffset+hP1+2)/2) == 0)&& (decode(gameState[i],(Hoffset+hP3+2)/2) == 0))

           ||

           ((state%4 == 2) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 4)) 
               && (decode(gameState[i],(Hoffset+hP3+2)/2) == 0) 
               && (decode(gameState[i+1],(Hoffset+hP4+2)/2) == 0) 
               && (decode(gameState[i+2],(Hoffset+hP1+2)/2) == 0)))){
                 Hoffset += 2;
                 erase(Voffset, Hoffset, 0, -2);
           }
           else if(//move left
           //verifying left movement according to current shape rotation
           ((state%4 == 1) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -6)) && (decode(gameState[i],(Hoffset+hP2-2)/2) == 0) &&(decode(gameState[i+1],(Hoffset+hP4-2)/2) == 0))

           ||

           ((state%4 == 0) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -6))
             && (decode(gameState[i],(Hoffset+hP2-2)/2) == 0) 
             && (decode(gameState[i+1],(Hoffset+hP3-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP4-2)/2) == 0))

           ||

           ((state%4 == 3) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -6)) && (decode(gameState[i+1],(Hoffset+hP3-2)/2) == 0)&& (decode(gameState[i+2],(Hoffset+hP4-2)/2) == 0))

           ||

           ((state%4 == 2) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -8))
             && (decode(gameState[i],(Hoffset+hP3-2)/2) == 0) 
             && (decode(gameState[i+1],(Hoffset+hP4-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP2-2)/2) == 0))))){
                Hoffset -= 2;
                erase(Voffset, Hoffset, 0, 2);
           }
           else if(( //rotations
                    (state%4 == 0) &&  
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    (decode(gameState[i],(hP1+Hoffset+2)/2) == 0) &&
                    (decode(gameState[i+1],(hP1+Hoffset+2)/2) == 0) &&
                    (((hP1+Hoffset+2)/2) <= 7))
                    
                    ||
                    
                    ((state%4 == 1) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+2) < 16) &&
                    (decode(gameState[i+2],(hP4+Hoffset)/2) == 0) &&
                    (decode(gameState[i+2],(hP4+Hoffset-2)/2) == 0))
                    
                    ||
                    
                    ((state%4 == 2) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+3) < 16) &&
                    (decode(gameState[i+1],(hP2+Hoffset-2)/2) == 0) &&
                    (decode(gameState[i+2],(hP2+Hoffset-2)/2) == 0) &&
                    (((hP2+Hoffset-2)/2) >= 0))

                    ||
                    
                    ((state%4 == 3) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+2) < 16) &&
                    (decode(gameState[i],(hP3+Hoffset)/2) == 0) &&
                    (decode(gameState[i],(hP3+Hoffset+2)/2) == 0)
                    )){
             
                       erase(Voffset, Hoffset, 0, 0);
             switch(state % 4)
             {
             	//set initial position for shape rotation
              case 0:
                defineShape(30, 8, 30, 6, 30, 10, 28, 10);
                break;
              case 1:
                defineShape(26, 10, 26, 8, 30, 10, 28, 10);
                break;
              case 2:
                defineShape(26, 10, 26, 8, 28, 6, 26, 6);
                break; 
             case 3:
                defineShape(30, 8, 30, 6, 28, 6, 26, 6);
              }
               state++;
           }
         count++;
         setAcceleration(value);

         }
         if( 
         	//check according to current shape rotation if we can move down
         ((i < 14) && (state%4 == 1) && (decode(gameState[i+1],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP1+Hoffset)/2) == 0) && (decode(gameState[i+2],(hP4+Hoffset)/2) == 0))
         ||
         ((i < 13) && (state%4 == 0) &&  (decode(gameState[i+3],(hP4+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP1+Hoffset)/2) == 0))
         ||
         ((i < 13) && (state%4 == 3) &&  (decode(gameState[i+3],(hP4+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP1+Hoffset)/2) == 0))
         ||
         ((i < 13) && (state%4 == 2) &&  (decode(gameState[i+3],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP1+Hoffset)/2) == 0))
         ){//move down
           Voffset += 2;
            erase(Voffset, Hoffset, 2, 0);
         }
         else{
           break;
         }
       }
       //save shape to game state according to current rotation
       unsigned long line2[8];
       unsigned long line3[8];
        SaveL(rightL_e,line,line2,line3,Hoffset,Voffset,hP1,hP2,hP3,hP4,gameState,state);
        break;
      case 3:
        //set pixels initial state;
        defineShape(30, 6, 30, 8, 28, 8, 26, 8);

        //BLUE left L shape 
        for(int i = 0; i < 16; ++i){
         count = 0;
         drawShape(Voffset,Hoffset, BLUE); //initial draw
         while(count < _speed){
          Wire.requestFrom(2,1);
          int value = Wire.read();
          drawShape(Voffset,Hoffset, BLUE);
           if(//verify if we can move right according to current rotation
           ((state%4 == 1) && ((16 <= value && value < 23) && (count%100 == 0 && Hoffset < 6)) && (decode(gameState[i+1],(Hoffset+hP3+2)/2) == 0)&& (decode(gameState[i+2],(Hoffset+hP4+2)/2) == 0) )

           ||

           ((state%4 == 0) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 6)) 
               && (decode(gameState[i],(Hoffset+hP2+2)/2) == 0) 
               && (decode(gameState[i+1],(Hoffset+hP3+2)/2) == 0) 
               && (decode(gameState[i+2],(Hoffset+hP4+2)/2) == 0)))
           ||

           ((state%4 == 3) && ((16 <= value && value < 23) && (count%100 == 0 && Hoffset < 6)) && (decode(gameState[i],(Hoffset+hP2+2)/2) == 0)&& (decode(gameState[i+1],(Hoffset+hP3+2)/2) == 0))

           ||

           ((state%4 == 2) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 8)) 
               && (decode(gameState[i],(Hoffset+hP4+2)/2) == 0) 
               && (decode(gameState[i+1],(Hoffset+hP3+2)/2) == 0) 
               && (decode(gameState[i+2],(Hoffset+hP2+2)/2) == 0)))){//move right

                 Hoffset += 2;
                 erase(Voffset, Hoffset, 0, -2);
           }
           else if(//verify if we can move left according to current rotation
           ((state%4 == 1) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -4)) && (decode(gameState[i+1],(Hoffset+hP3-2)/2) == 0) &&(decode(gameState[i+2],(Hoffset+hP1-2)/2) == 0))

           ||

           ((state%4 == 0) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -6))
             && (decode(gameState[i],(Hoffset+hP1-2)/2) == 0) 
             && (decode(gameState[i+1],(Hoffset+hP3-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP4-2)/2) == 0))

           ||

           ((state%4 == 3) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -4)) && (decode(gameState[i+1],(Hoffset+hP3-2)/2) == 0)&& (decode(gameState[i],(Hoffset+hP4-2)/2) == 0))

           ||

           ((state%4 == 2) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -4))
             && (decode(gameState[i],(Hoffset+hP4-2)/2) == 0) 
             && (decode(gameState[i+1],(Hoffset+hP3-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP1-2)/2) == 0))))){//move left

                Hoffset -= 2;
                erase(Voffset, Hoffset, 0, 2);
           }
           else if((
           	//checking if we can rotate according to current shape state
                    (state%4 == 0) &&  
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    (decode(gameState[i+2],(hP4+Hoffset-2)/2) == 0) &&
                    (decode(gameState[i+2],(hP4+Hoffset-4)/2) == 0) &&
                    (((hP1+Hoffset+4)/2) <= 7)&&
                    (((hP1+Hoffset-2)/2) >= 0))
                    
                    ||
                    
                    ((state%4 == 1) && 
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+2) < 16) &&
                    (decode(gameState[i],(hP1+Hoffset)/2) == 0) &&
                    (decode(gameState[i+1],(hP1+Hoffset)/2) == 0))
                    
                    ||
                    
                    ((state%4 == 2) && 
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+3) < 16) &&
                    (decode(gameState[i],(hP4+Hoffset+2)/2) == 0) &&
                    (decode(gameState[i],(hP4+Hoffset+4)/2) == 0) &&
                    (((hP1+Hoffset+4)/2) <= 7))
                    
                    ||
                    
                    ((state%4 == 3) && 
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+2) < 16) &&
                    (decode(gameState[i+1],(hP2+Hoffset)/2) == 0) &&
                    (decode(gameState[i+2],(hP2+Hoffset)/2) == 0)
                    )){//change rotation
             
                       erase(Voffset, Hoffset, 0, 0);
             switch(state % 4)
             {
             	//set pixel initial values for new rotation
              case 0:
                defineShape(26, 4, 26, 6, 28, 8, 26, 8);
                 break;
              case 1:
                defineShape(26, 4, 26, 6, 28, 4, 30, 4);
                 break;
              case 2:
                defineShape(30, 6, 30, 8, 28, 4, 30, 4);
                break; 
              case 3:
                defineShape(30, 6, 30, 8, 28, 8, 26, 8);
             }
               state++;
           }
         count++;
         setAcceleration(value);

         }
         if( //check if we can move verticaly according to current shape rotation
         ((i < 13) && (state%4 == 1) && (decode(gameState[i+3],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP1+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP4+Hoffset)/2) == 0))
         
         ||

         ((i < 13) && (state%4 == 0) &&  (decode(gameState[i+3],(hP4+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP1+Hoffset)/2) == 0))
         ||
         ((i < 14) && (state%4 == 3) &&  (decode(gameState[i+2],(hP3+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP1+Hoffset)/2) == 0))
         ||
         ((i < 13) && (state%4 == 2) &&  (decode(gameState[i+3],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP1+Hoffset)/2) == 0))
         ){//move down
           Voffset += 2;
            erase(Voffset, Hoffset, 2, 0);
         }
         else{
           break;
         }
       }
       //save shape to game state
       SaveL(leftL_e,line,line2,line3,Hoffset,Voffset,hP1,hP2,hP3,hP4,gameState,state);
      break;
    case 4:
      //set pixels initial state;
      defineShape(26, 4, 26, 6, 26, 8, 28, 6);
        //ORANGE T shape
        for(int i = 0; i < 16; ++i){
         count = 0;
         drawShape(Voffset,Hoffset, ORANGE); //initial shape draw
         while(count < _speed){
          Wire.requestFrom(2,1);
          int value = Wire.read();
           drawShape(Voffset,Hoffset, ORANGE);
           if(//check if we can move right according to current shape rotation
           ((state%4 == 1) && ((16 <= value && value < 23) && (count%100 == 0 && Hoffset < 8)) && (decode(gameState[i],(Hoffset+hP3+2)/2) == 0)&& (decode(gameState[i+1],(Hoffset+hP4+2)/2) == 0) && (decode(gameState[i+2],(Hoffset+hP1+2)/2) == 0) )

           ||

           ((state%4 == 0) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 6)) 
               && (decode(gameState[i+1],(Hoffset+hP4+2)/2) == 0) 
               && (decode(gameState[i+2],(Hoffset+hP3+2)/2) == 0)))

           ||

           ((state%4 == 3) && ((16 <= value && value < 23) && (count%100 == 0 && Hoffset < 6)) && (decode(gameState[i],(Hoffset+hP1+2)/2) == 0)&& (decode(gameState[i+1],(Hoffset+hP2+2)/2) == 0)&& (decode(gameState[i+2],(Hoffset+hP3+2)/2) == 0))

           ||

           ((state%4 == 2) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 6)) 
               && (decode(gameState[i],(Hoffset+hP1+2)/2) == 0) 
               && (decode(gameState[i+1],(Hoffset+hP4+2)/2) == 0)))){//move right

                 Hoffset += 2;
                 erase(Voffset, Hoffset, 0, -2);
           }
           else if(//check if we can move left according to current shape rotation
           ((state%4 == 1) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -4)) && (decode(gameState[i],(Hoffset+hP3-2)/2) == 0) &&(decode(gameState[i+1],(Hoffset+hP2-2)/2) == 0)&&(decode(gameState[i+2],(Hoffset+hP1-2)/2) == 0))

           ||

           ((state%4 == 0) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -4))
             && (decode(gameState[i+1],(Hoffset+hP4-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP1-2)/2) == 0))

           ||

           ((state%4 == 3) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -6)) && (decode(gameState[i+2],(Hoffset+hP4-2)/2) == 0) && (decode(gameState[i+1],(Hoffset+hP4-2)/2) == 0)&& (decode(gameState[i],(Hoffset+hP1-2)/2) == 0))
          
           ||

           ((state%4 == 2) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -4))
             && (decode(gameState[i],(Hoffset+hP4-2)/2) == 0) 
             && (decode(gameState[i+1],(Hoffset+hP3-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP4-2)/2) == 0))))){//move left
               
                Hoffset -= 2;
                erase(Voffset, Hoffset, 0, 2);
           }
           else if((
           			//check if we can rotate the shape according to shape current rotation
                    (state%4 == 0) &&  
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    (decode(gameState[i],(hP4+Hoffset-2)/2) == 0) &&
                    (decode(gameState[i+1],(hP4+Hoffset-1)/2) == 0))
                    
                    ||
                    
                    ((state%4 == 1) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+2) < 16) &&
                    (decode(gameState[i],(hP4+Hoffset)/2) == 0) &&
                    (decode(gameState[i],(hP4+Hoffset+2)/2) == 0)&&
                    (((hP1+Hoffset+4)/2) <= 7))
                    
                    ||
                    
                    ((state%4 == 2) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+3) < 16) &&
                    (decode(gameState[i+1],(hP4+Hoffset+2)/2) == 0) &&
                    (decode(gameState[i+2],(hP4+Hoffset+2)/2) == 0))
                    
                    ||
                    
                    ((state%4 == 3) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+2) < 16) &&
                    (decode(gameState[i+2],(hP4+Hoffset)/2) == 0) &&
                    (decode(gameState[i+2],(hP4+Hoffset-2)/2) == 0)&&
                    (((hP4+Hoffset-2)/2) >= 0)
                    )){//change rotation
             
                       erase(Voffset, Hoffset, 0, 0);
             switch(state % 4)
             {
             	//set new initial pixel state for the new rotation
              case 0:
                defineShape(26, 4, 28, 4, 30, 4, 28, 6);
                 break;
              case 1:
                defineShape(30, 8, 30, 6, 30, 4, 28, 6);
                 break;
              case 2:
                defineShape(30, 8, 28, 8, 26, 8, 28, 6);
                break; 
              case 3:
                defineShape(26, 4, 26, 6, 26, 8, 28, 6);
                break;
             }
               state++;
           }
         count++;
         setAcceleration(value);

         }
         if( //check if we can move verticaly according to shape current rotation
         ((i < 13) && (state%4 == 1) && (decode(gameState[i+3],(hP1+Hoffset)/2) == 0) && (decode(gameState[i+2],(hP4+Hoffset)/2) == 0))

         ||

         ((i < 13) && (state%4 == 0) &&  (decode(gameState[i+3],(hP1+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP3+Hoffset)/2) == 0))

         ||

         ((i < 14) && (state%4 == 2) &&  (decode(gameState[i+2],(hP4+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP1+Hoffset)/2) == 0) && (decode(gameState[i+1],(hP3+Hoffset)/2) == 0))

         ||

         ((i < 13) && (state%4 == 3) &&  (decode(gameState[i+2],(hP4+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP3+Hoffset)/2) == 0))
         
         ){//move down
           Voffset += 2;
            erase(Voffset, Hoffset, 2, 0);
         }
         else{
           break;
         }
       }
       //save shape to game state
       for(unsigned long i = 0; i < 8; ++i){
             line[i] = 0;
             line2[i] = 0;
             line3[i] = 0;     
       }
       if(state%4 == 0){
         line[(Hoffset+hP4)/2] = tshape_e;
         line2[(Hoffset+hP1)/2] = tshape_e;
         line2[(Hoffset+hP2)/2] = tshape_e;
         line2[(Hoffset+hP3)/2] = tshape_e;
         
         gameState[(Voffset+2)/2] += encode(line);
         gameState[(Voffset+4)/2] += encode(line2);
       }  
       else if(state%4 == 1){
         line[(Hoffset+hP3)/2] = tshape_e;
         line2[(Hoffset+hP2)/2] = tshape_e;
         line2[(Hoffset+hP4)/2] = tshape_e;
         line3[(Hoffset+hP1)/2] = tshape_e;
         
         gameState[(Voffset)/2] += encode(line);
         gameState[(Voffset+2)/2] += encode(line2);
         gameState[(Voffset+4)/2] += encode(line3);
       }
       else if(state%4 == 2){
         line[(Hoffset+hP3)/2] = tshape_e;
         line[(Hoffset+hP2)/2] = tshape_e;
         line[(Hoffset+hP1)/2] = tshape_e;
         line2[(Hoffset+hP4)/2] = tshape_e;
         
         gameState[Voffset/2] += encode(line);
         gameState[(Voffset+2)/2] += encode(line2);
       }
       else{
         line[(Hoffset+hP1)/2] = tshape_e;
         line2[(Hoffset+hP4)/2] = tshape_e;
         line2[(Hoffset+hP2)/2] = tshape_e;
         line3[(Hoffset+hP3)/2] = tshape_e;
         
         gameState[Voffset/2] += encode(line);
         gameState[(Voffset+2)/2] += encode(line2);
         gameState[(Voffset+4)/2] += encode(line3);
       }
      
      break;
    case 5:
        //set pixels initial state;
        defineShape(26, 4, 28, 4, 28, 6, 30, 6);
        //RED S right shape 
        for(int i = 0; i < 16; ++i){
         count = 0;
        drawShape(Voffset,Hoffset, RED); //initial draw
         while(count < _speed){
          Wire.requestFrom(2,1);
          int value = Wire.read();
           drawShape(Voffset,Hoffset, RED);
           if(//check if we can move right according to current rotation
           ((state%2 == 1) && ((16 <= value && value < 23)  && (count%100 == 0 && Hoffset < 6)) && (decode(gameState[i+1],(Hoffset+hP3+2)/2) == 0) && (decode(gameState[i+2],(Hoffset+hP4+2)/2) == 0) )

           ||

           ((state%2 == 0) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 7)) 
               && (decode(gameState[i],(Hoffset+hP4+2)/2) == 0) 
               && (decode(gameState[i+1],(Hoffset+hP3+2)/2) == 0)
               && (decode(gameState[i+2],(Hoffset+hP1+2)/2) == 0) ))){//move right

             Hoffset += 2;
            erase(Voffset, Hoffset, 0, -2);
           }
           else if(//check if we can move left according to current rotation
           ((state%2 == 1) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -4)) && (decode(gameState[i+1],(Hoffset+hP1-2)/2) == 0) && (decode(gameState[i+2],(Hoffset+hP2-2)/2) == 0) )

           ||

           ((state%2 == 0) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -4))
             && (decode(gameState[i],(Hoffset+hP4-2)/2) == 0) 
             && (decode(gameState[i+1],(Hoffset+hP2-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP1-2)/2) == 0) ))
             ){//move left

                Hoffset -= 2;
                erase(Voffset, Hoffset, 0, 2);
           }
           else if((
           			//check if we can rotate according to current shape rotation
                    (state%2 == 0) &&  
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    (decode(gameState[i+2],(hP3+Hoffset)/2) == 0) &&
                    (decode(gameState[i+2],(hP3+Hoffset+2)/2) == 0) &&
                    (((hP3+Hoffset+2)/2) <= 7))
                    
                    ||
                    
                    ((state%2 == 1) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+3) < 16) &&
                    (decode(gameState[i+2],(hP3+Hoffset-2)/2) == 0)&&
                    (decode(gameState[i],(hP3+Hoffset)/2) == 0))){//change rotation
             
                     erase(Voffset, Hoffset, 0 , 0);
                     switch(state % 2)
                     {
                       case 0:
                         defineShape(28, 4, 26, 6, 28, 6, 26, 8);
                         break;
                       case 1:
                         defineShape(26, 4, 28, 4, 28, 6, 30, 6);
                        break;
                    }
                        state++;
                     }
         count++;
         setAcceleration(value);
         }       
         if( //check if we can move verticaly according to shape current rotation
           ((i < 13) && (state%2 == 1) && (decode(gameState[i+3],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP4+Hoffset)/2) == 0) && (decode(gameState[i+2],(hP1+Hoffset)/2) == 0))
           
           ||

           ((i < 13) && (state%2 == 0) &&  (decode(gameState[i+3],(hP1+Hoffset)/2) == 0) && (decode(gameState[i+2],(hP3+Hoffset)/2) == 0))){//move down
             Voffset += 2;
             erase(Voffset, Hoffset, 2, 0);
           }
           else{
             break;
           }
       }// save to game state
       for(unsigned long i = 0; i < 8; ++i){
             line[i] = 0;
             line2[i] = 0;
             line3[i] = 0;
       }
       if(state%2 == 0){
         line[(Hoffset+hP4)/2] = rightS_e;
         line2[(Hoffset+hP2)/2] = rightS_e;
         line2[(Hoffset+hP3)/2] = rightS_e;
         line3[(Hoffset+hP1)/2] = rightS_e;
         
         gameState[(Voffset)/2] += encode(line);
         gameState[(Voffset+2)/2] += encode(line2);
         gameState[(Voffset+4)/2] += encode(line3);
         }  
       else{
         line[(Hoffset+hP1)/2] = rightS_e;
         line[(Hoffset+hP3)/2] = rightS_e;
         line2[(Hoffset+hP2)/2] = rightS_e;
         line2[(Hoffset+hP4)/2] = rightS_e;
         
         gameState[(Voffset+2)/2] += encode(line);
         gameState[(Voffset+4)/2] += encode(line2);
       }
      break;  
    case 6:
        //set pixels initial state;
        defineShape(30, 6, 28, 6, 28, 8, 26, 8);
        //GREEN S left shape
        for(int i = 0; i < 16; ++i){
         count = 0;
         drawShape(Voffset,Hoffset, GREEN); //initial shape draw
         while(count < _speed){   
          Wire.requestFrom(2,1);
          int value = Wire.read();
           drawShape(Voffset,Hoffset, GREEN);
           if(//verify if we can move right according to current rotation
           ((state%2 == 1) && ((16 <= value && value < 23) && (count%100 == 0 && Hoffset < 4)) && (decode(gameState[i+1],(Hoffset+hP1+2)/2) == 0) && (decode(gameState[i+2],(Hoffset+hP4+2)/2) == 0) )

           ||

           ((state%2 == 0) && ((16 <= value && value < 23) && (count%100 == 0 && (Hoffset < 6)) 
               && (decode(gameState[i],(Hoffset+hP1+2)/2) == 0) 
               && (decode(gameState[i+1],(Hoffset+hP3+2)/2) == 0)
               && (decode(gameState[i+2],(Hoffset+hP4+2)/2) == 0) ))){//move right
             Hoffset += 2;
             erase(Voffset, Hoffset, 0, -2); 
           }
           else if(//check if we can move left according to current rotation
           ((state%2 == 1) && ((24 <= value && value < 31) && (count%100 == 0 && Hoffset > -6)) && (decode(gameState[i],(Hoffset+hP3-2)/2) == 0) && (decode(gameState[i+1],(Hoffset+hP2)/2) == 0) )

           ||

           ((state%2 == 0) && ((24 <= value && value < 31) && (count%100 == 0 && (Hoffset > -6))
             && (decode(gameState[i],(Hoffset+hP1-2)/2) == 0) 
             && (decode(gameState[i+1],(Hoffset+hP2-2)/2) == 0) 
             && (decode(gameState[i+2],(Hoffset+hP4-2)/2) == 0) ))
             ){//move left

                Hoffset -= 2;
                erase(Voffset, Hoffset, 0, 2); 
           }
           else if(( //check if we can rotate the shape according to current rotation
                    (state%2 == 0) &&  
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    (decode(gameState[i+1],(hP3+Hoffset+2)/2) == 0) &&
                    (decode(gameState[i+2],(hP3+Hoffset-2)/2) == 0) &&
                    (((hP3+Hoffset+2)/2) <= 7))
                    
                    ||
                    
                    ((state%2 == 1) &&
                    ((0 <= value && value < 7)) && 
                    (count%100 == 0) &&
                    ((i+3) < 16) &&
                    (decode(gameState[i],(hP3+Hoffset-2)/2) == 0) &&
                    (decode(gameState[i+1],(hP3+Hoffset-2)/2) == 0))){//change rotation
             
                     erase(Voffset, Hoffset, 0 ,0);
                     switch(state % 2)
                     {
                     	//set new pixel position
                       case 0:
                       defineShape(28, 10, 26, 6, 28, 8, 26, 8);
                         break;
                      case 1:
                      defineShape(30, 6, 28, 6, 28, 8, 26, 8);
                       break;
                            }
                        state++;
                     }
         count++;
         setAcceleration(value);
         }       
         if( //check if we can move verticaly according to current rotation
           ((i < 13) && (state%2 == 1) && (decode(gameState[i+3],(hP2+Hoffset)/2) == 0) && (decode(gameState[i+2],(hP1+Hoffset)/2) == 0) && (decode(gameState[i+3],(hP4+Hoffset)/2) == 0))
          
           ||

           ((i < 13) && (state%2 == 0) &&  (decode(gameState[i+3],(hP4+Hoffset)/2) == 0) && (decode(gameState[i+2],(hP2+Hoffset)/2) == 0))){//move down
             Voffset += 2;
             erase(Voffset, Hoffset, 2, 0);
           }
           else{
             break;
           }
       }
       //add to game state
       for(unsigned long i = 0; i < 8; ++i){
             line[i] = 0;
             line2[i] = 0;
             line3[i] = 0;
       }
       if(state%2 == 0){
         line[(Hoffset+hP1)/2] = leftS_e;
         line2[(Hoffset+hP2)/2] = leftS_e;
         line2[(Hoffset+hP3)/2] = leftS_e;
         line3[(Hoffset+hP4)/2] = leftS_e;
         
         gameState[(Voffset)/2] += encode(line);
         gameState[(Voffset+2)/2] += encode(line2);
         gameState[(Voffset+4)/2] += encode(line3);
         }  
       else{
         line[(Hoffset+hP3)/2] = leftS_e;
         line[(Hoffset+hP1)/2] = leftS_e;
         line2[(Hoffset+hP4)/2] = leftS_e;
         line2[(Hoffset+hP2)/2] = leftS_e;
         
         gameState[(Voffset+2)/2] += encode(line);
         gameState[(Voffset+4)/2] += encode(line2);
       }
      break;
  }
  
 }
