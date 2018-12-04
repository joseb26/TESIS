// FUNCIONA BELLLO
#include <DHT11.h>
int pin=5;
DHT11 dht11(pin);
const int MTRX =  2;  // HIGH = Driver / LOW = Receptor
const byte STX = 0X02,
           ETX = 0X03,
           ACK = 0X06,
           NAK = 0X15,
           F_C1 = 0X00,
           F_C2 = 0X00; // F_C1 Y FC2 Son los de funcion de caracteres, por ahora no se usara pero para futuro se dejara en la trama
union inDato{
   int ival;
   byte  b[2];     
};
union miDato{
   float fval;
   byte  b[4];     
}etemp,ehum;

union ulDato{
   unsigned long val;
   byte b[4];     
};
struct Cadena
  {byte STX;
   inDato Direccion;
   byte ENQ;
   byte Fun;
   miDato dato0;// temperatura
   miDato dato1;
   inDato dato2;
   inDato dato3;
   byte ETX;  
   ulDato CHK; 
  } Recibir;

int lum=0,hum=0;
//---------------------
int id=10;
//-----------------------
unsigned long Tmedida = 0,CHK=0;
void setup() {

Serial.begin(9600); // send and receive at 9600 baud

pinMode(13,OUTPUT);
pinMode(12,OUTPUT);
pinMode(11,OUTPUT);
digitalWrite(11,HIGH);// ESCLAVO
pinMode(A0,INPUT);//LDR
pinMode(A1,INPUT);//humedad
pinMode(MTRX,OUTPUT);
digitalWrite(MTRX,LOW);// ESCLAVO
Tmedida = millis();
}

void loop() 
{ 
  if(millis() > (Tmedida +1500))
  { 
  MedirTemperatura(etemp.fval,ehum.fval);
  lum=analogRead(A0);
  hum=analogRead(A1);
  Tmedida = millis();
  }
  
  if (Serial.available() >= sizeof(Recibir))
  {   
  recibirEstructura((byte*)&Recibir,sizeof(Recibir));
  
  //REVISO QUE LA INFORMACIÓN llego de forma correcta
  CHK = CalculoCHK(Recibir);
  for(int i=0;i<4;i++)Recibir.CHK.b[i]-=0x30;
  //-----------------------------------------------
  for(int i=0;i<2;i++)Recibir.Direccion.b[i]-=0x30;
  if(Recibir.Direccion.ival==id)
    {
    byte r_ack=0;
    if(CHK==Recibir.CHK.val){ 
                             LedOnOff(13);
                             r_ack = ACK;
                             }
    else r_ack = NAK;
    digitalWrite(MTRX,HIGH);// ESCLAVO-ENVIA
    delay(50);//tiempo de espera para que se estabilice el pin
    EnviarMsj(etemp.fval,ehum.fval,lum,hum,id,r_ack);
    }
  }
delay(500);            //Recordad que solo lee una vez por segundo
}

 void LedOnOff(int pin){
  if(digitalRead(pin))digitalWrite(pin,LOW);
  else digitalWrite(pin,HIGH);
  }

  void EnviarMsj(float temp,float hum, int lum, int hum_s,int id, byte R_ACK){
    Cadena Enviar;
    Enviar.STX= STX;
    Enviar.ENQ = R_ACK;
    Enviar.Fun = 'R';
    Enviar.Direccion.ival = id;//direccion Maestro
    Enviar.dato0.fval = temp;
    Enviar.dato1.fval = hum; 
    Enviar.dato2.ival = lum;
    Enviar.dato3.ival = hum_s;
    
    for(int i=0;i<4;i++){
                         Enviar.dato0.b[i]+=0x30;
                         Enviar.dato1.b[i]+=0x30;
                         }
    for(int i=0;i<2;i++){Enviar.dato2.b[i]+=0x30;
                         Enviar.dato3.b[i]+=0x30;
                         Enviar.Direccion.b[i]+=0x30;
                         }
    Enviar.ETX = ETX;
    Enviar.CHK.val = CalculoCHK(Enviar);
    
    enviarEstructura((byte*)&Enviar, sizeof(Enviar));
    //  LedOnOff(13);// cambio estado del pin 
 }

 void MedirTemperatura(float &temp, float &hum)
 {
   int err;
  if((err = dht11.read(hum, temp)) == 0);   // Si devuelve 0 es que ha leido bien
  else{
       hum=-1;
       temp=-1;
      }
  
 }
void enviarEstructura(byte *structurePointer, int structureLength)
{
    Serial.write(structurePointer, structureLength);
    Serial.flush();// espero que la transmision se termine
    delay(30);
    digitalWrite(MTRX,LOW); // pongo MAESTRO DE NUEVO COMO EL QUE ENVIAR
}

void recibirEstructura(byte *structurePointer, int structureLength)
{
    Serial.readBytes(structurePointer, structureLength);
}

 unsigned long CalculoCHK(Cadena Enviar)
{ unsigned long Vr =0; 
  Vr =Enviar.ENQ + 
  (byte)Enviar.Fun+ (byte)Enviar.Direccion.ival+ (byte)Enviar.dato0.fval + 
  (byte)Enviar.dato1.fval + (byte)Enviar.dato2.ival + (byte)Enviar.dato3.ival+Enviar.ETX;
   return Vr;
  }
