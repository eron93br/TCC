// MCU To MPU
#include <Bridge.h>
#include <SPI.h>

const int chipSelectPinADC = 6;
unsigned int result = 0;
byte inByte = 0;
int saida;

const int PIN_CS = 7;
const int GAIN_1 = 0x1;
const int GAIN_2 = 0x0;

void setup() 
{
  Serial.begin(9600); 
  Bridge.begin();
  pinMode(4,OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  pinMode(chipSelectPinADC, OUTPUT);
  digitalWrite(chipSelectPinADC, HIGH);
}

// coeficientes do filtro PF

float filtro1[26] = {-0.0029   , -0.0048   , -0.0085   ,-0.0136  , -0.0181  , -0.0183 ,  -0.0103  ,  0.0086  ,  0.0387 ,   0.0765,    0.1156,
0.1479 ,   0.1662  ,  0.1662  ,  0.1479  ,  0.1156 ,   0.0765 ,   0.0387 ,   0.0086,   -0.0103,   -0.0183 ,  -0.0181, -0.0136,   -0.0085,   -0.0048,  -0.0029};

int filtro2[5] = {2, 1, 0, -1, -2};

int x=0;
int flag=1;
int M=20;

unsigned long t1=0;
unsigned long t0 = 0; 
unsigned long t2=0;
unsigned long deltat=0; 
float w=0;
float h=0; 
float t=0;
float x1[26]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
float x2[6]={0,0,0,0,0, 0};
float x3[50]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0};

float y=0;
float b0=0; float b1=0; float b2=0;
int i, j, k;
int cont=0;

float aux=0;
float aux2=0;
float aux3=0; 
int bpm=80;
int bpm_anterior=0;
float hn0, hn1, hn2; 


void loop()
{
   flag=0;

      /// ------------------------------------FILTRAGEM 1 -------------------------------------- 
   for(i=1; i<26; i++)
   {
      x1[(26-i)] = x1[(25-i)];
   }


   x1[0] = analog_to_digital();
 

    for(int k=0;k<26;k++)   //Convolution
    {
      aux = filtro1[k]*x1[k];
      y = aux + y;
    }

   ///------------------------------------ FILTRAGEM 2 --------------------------------------
   for(i=1; i<6; i++)
   {
      x2[(6-i)] = x2[(5-i)];
   }
   
   x2[0] = y;

    for(int k=0;k<5;k++)   //Convolution
    {
      aux2 = filtro2[k]*x2[k]; 
      w = aux2 + w;
    }

   //t = w;
    ///------------------------------------ FILTRAGEM 3 --------------------------------------

   for(i=(M-1); i>0; i=i-1)
   {
      x3[i] = x3[i-1];
   }
   x3[0] = abs(w); 
   

   //h = 0.1*x3[9] + 0.1*x3[8] +  0.1*x3[7] +  0.1*x3[6] +  0.1*x3[5] +  0.1*x3[4] +  0.1*x3[3] +  0.1*x3[2] +  0.1*x3[1] +  0.1*x3[0];
   /*h = 0.05*x3[9] + 0.05*x3[8] +  0.05*x3[7] +  0.05*x3[6] +  0.05*x3[5] +  0.05*x3[4] +  0.05*x3[3] +  0.05*x3[2] +  0.05*x3[1] +  0.05*x3[0] +
      + 0.05*x3[19] + 0.05*x3[18] +  0.05*x3[17] +  0.05*x3[16] +  0.05*x3[15] +  0.05*x3[14] +  0.05*x3[13] +  0.05*x3[12] +  0.05*x3[11] +  0.05*x3[10];*/

   for(i==0; i< M; i++)
   {
     h += x3[i];
   }

   h=h/M; 
   
   t0 = millis();             // calcula  e salva Tempo atual!



   // ---------------------------------- CALCULO DO BPM ----------------------------------------------- //
   b2=b1;
   b1=b0;
   b0=h;         // pega amostra atual
   
   
   if( (b2<b1) && (b1>b0) && (b0>225) && (cont>35) )
   {
     t1=t2;
     t2=t0;
     deltat = abs(t2-t1);
     flag=1;
     cont = 0;
   }
  

   if(flag==1)
   { 
    bpm_anterior = bpm; 
    bpm = 60000/deltat;
    //if( bpm>28 && bpm<140){Serial.println(bpm);}
    Serial.println(bpm);
   }

    // MCU To MPU
    Bridge.put("Humidity", String(bpm));
    Bridge.put("Temperature", String(bpm-bpm_anterior));
   
   //Serial.println(h);
   cont = cont + 1;
   aux=0;  aux2=0; aux3=0;
   y=0; h=0; w=0;
 
}

// ------------ Funcoes conversao A/D & D/A ------------

int analog_to_digital()
{
  digitalWrite(chipSelectPinADC, LOW);
  result = SPI.transfer(0x00);
  result = result << 8;
  inByte = SPI.transfer(0x00);
  result = result | inByte;
  digitalWrite(chipSelectPinADC, HIGH);
  result = result >> 2;
  result = result & 0b0000111111111111;
  return result; 
}


