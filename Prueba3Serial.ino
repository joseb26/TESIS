// FUNCIONA BELLOOO

const int MTRX =  22;  // HIGH = Driver / LOW = Receptor
const byte STX = 0X02,
           ETX = 0X03,
           ENQ = 0X05,
           ACK = 0X06,
           NAK = 0X15,
           F_C1 = 0X00,
           F_C2 = 0X00; // F_C1 Y FC2 Son los de funcion de caracteres, por ahora no se usara pero para futuro se dejara en la trama

boolean state = HIGH;
union miDato{
   float fval;
   byte  b[4];     
}etemp,ehum;

union inDato{
   int ival;
   byte  b[2];     
};

union ulDato{
   unsigned long val;
   byte b[4];     
};
struct Cadena
  {byte STX;
   inDato Direccion;
   byte ENQ;
   byte Fun;
   miDato dato0;
   miDato dato1;
   inDato dato2;
   inDato dato3;
   byte ETX;
   ulDato CHK;  
  } Recibir;
//Enviar.CHK.val;
int lum=0,hum_s=0,
    slave = 10;
unsigned long CHK=0;
void setup() {
Serial1.begin(9600); // send and receive at 9600 baud
Serial.begin(9600); 
pinMode(13,OUTPUT);
pinMode(12,OUTPUT);
pinMode(MTRX,OUTPUT);
digitalWrite(MTRX,HIGH);// ESCLAVO
}

void loop() {
  delay(500);
  if(slave==13)slave=10; 
  
  if(digitalRead(MTRX)){ delay(100);EnviarMsj(slave,'R');}
  EsperarACK(50,slave);
  if(Serial1.available() >= (sizeof(Recibir))) //esperamos el inicio de trama
  { recibirEstructura((byte*)&Recibir,sizeof(Recibir));
    CHK = CalculoCHK(Recibir);
    
    
    if(CHK == Recibir.CHK.val){
      if(Recibir.Fun=='R')
        { 
          LedOnOff(12);// cambio estado del pin
          for(int i=0;i<2;i++)Recibir.Direccion.b[i]-=0x30;
          int direccion = Recibir.Direccion.ival;
          for(int i=0;i<4;i++)Recibir.dato0.b[i]-=0x30;
          etemp.fval = Recibir.dato0.fval;
          for(int i=0;i<4;i++)Recibir.dato1.b[i]-=0x30;
          ehum.fval = Recibir.dato1.fval;
          for(int i=0;i<2;i++)Recibir.dato2.b[i]-=0x30;
          lum=Recibir.dato2.ival;
          for(int i=0;i<2;i++)Recibir.dato3.b[i]-=0x30;
          hum_s=Recibir.dato3.ival;
       
          Serial.print("ENQ: ");
          Serial.print(Recibir.ENQ);
          Serial.print(" Esclavo: ");
          Serial.print(Recibir.Direccion.ival);
          Serial.print(" temp: ");
          Serial.print(etemp.fval);
          Serial.print(" hum: ");
          Serial.print(ehum.fval);
          Serial.print(" lum: ");
          Serial.print(lum);
           Serial.print(" hum_suelo: ");
          Serial.println(hum_s);
          digitalWrite(MTRX,HIGH); // pongo MAESTRO DE NUEVO COMO EL QUE ENVIAR
          slave++;
          delay(100);
          }
     }
     else {
          Serial.print("Error de recepcion");
          digitalWrite(MTRX,HIGH); // pongo MAESTRO DE NUEVO COMO EL QUE ENVIAR
          slave++;
          delay(100);
          }
      
  }
 
  
  
}
void EnviarMsj(int direccion, byte Funcion){
    Cadena Enviar;
    Enviar.STX= STX;
    Enviar.ENQ = ENQ;
    Enviar.Fun = Funcion;
    Enviar.Direccion.ival = direccion;// Direccion al esclavo a enviar
    for(int i=0;i<4;i++){
                        Enviar.dato0.b[i]=0x30;
                        Enviar.dato1.b[i]=0x30; 
                        }
    for(int i=0;i<2;i++){
                        Enviar.Direccion.b[i]+=0x30;
                         Enviar.dato2.b[i]=0x30;
                         Enviar.dato3.b[i]=0x30;
                         }
    Enviar.ETX = ETX;
    Enviar.CHK.val = CalculoCHK(Enviar);
    for(int i=0;i<4;i++)Enviar.CHK.b[i]+=0x30;
    LedOnOff(13);// cambio estado del pin
    enviarEstructura((byte*)&Enviar, sizeof(Enviar));
    
 }
     void LedOnOff(int pin){
    if(digitalRead(pin))digitalWrite(pin,LOW);
    else digitalWrite(pin,HIGH);
     }


void enviarEstructura(byte *structurePointer, int structureLength)
{   Serial.write(structurePointer, structureLength); //debug para saber que se envia
    Serial1.write(structurePointer, structureLength);
    Serial1.flush();// espero que la transmision se termine
    delay(30);
    digitalWrite(MTRX,LOW); // pongo MAESTRO DE NUEVO COMO EL QUE ENVIAR
}

void recibirEstructura(byte *structurePointer, int structureLength)
{
    Serial1.readBytes(structurePointer, structureLength);
}

void EsperarACK(unsigned long TiempoEspera, int &slave){
 unsigned long Tiempo =millis();
 while (!Serial.available() && (millis() - Tiempo) < TiempoEspera);
 if((millis() - Tiempo) > TiempoEspera)
    {
      digitalWrite(MTRX,HIGH); // pongo MAESTRO DE NUEVO COMO EL QUE ENVIAR
      slave++;
      
      }
 }

 unsigned long CalculoCHK(Cadena Enviar)
{ unsigned long Vr =0; 
  Vr =Enviar.ENQ + 
  (byte)Enviar.Fun+ (byte)Enviar.Direccion.ival+ (byte)Enviar.dato0.fval + 
  (byte)Enviar.dato1.fval + (byte)Enviar.dato2.ival + (byte)Enviar.dato3.ival+Enviar.ETX;
   return Vr;
 }




