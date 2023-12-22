// Display LCD
#include <LiquidCrystal_I2C.h>
// Servo
#include <Servo.h>
//Libreria RFID master
#include <SPI.h>
#include <MFRC522.h>
// Libreria Keypad
#include <OnewireKeypad.h>
#include <Password.h>
Password password=Password("99999");
//Inicializamos el LCD
LiquidCrystal_I2C lcd(0x27, 16,2);
//Inicializamos el servo 
Servo servo;
int servoPos = 0;
//Definimos los pines
//Sensores infrarojos
#define sensorPin1 A3
#define sensorPin2 A2
int sensor_IR1 = 1;
int sensor_IR2 = 1;
//Modulo RFID
#define SS_PIN 10   //Slave Select Pin 
#define RST_PIN 9   //Reset Pin 

//Creamos una instancia de  MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);
//Creamos una instancia de  MIFARE_Key
MFRC522::MIFARE_Key key; 
MFRC522::StatusCode status;
//Bloques de memoria de la tarjeta donde se almacenan la contraseña y el balance de salso
int blockNum_Password = 2; 
int blockNum_Balance = 5;
// Crea otra matriz para leer datos del bloque 
// La longitud del búfer debe ser 2 bytes más que el tamaño del bloque (16 bytes). */
byte bufferLen = 18;
byte readBlockData[18];
String string="";
//Indicar recarga
int modo_recarga = 0;
int modo_pago = 0;
int pasar = 0;
int pasasensor2=0;
int countshow=0;
bool hasRun=false;
bool hasRun1=false;
float balance = 0.00;
//Configuramos el Keypad
char KEYS[] = {
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '*', '0', '#', 'D'
};

OnewireKeypad <Print, 16 > KP(Serial, KEYS, 4, 4, A0, 4900,1500 );//
char Key; //Tecla presionama
#define num 7 // Cantidad de datos del mondo a recargar
char Data[num]; // Cadena de teclas plesionadas
byte data_count = 0;  //Indice del la cadena Data
int monto_recarga = 0; //monto a recargar

void setup() {
  // Inicializar las comunicaciones seriales con la PC.*/
  Serial.begin(9600);
  // Servo
  servo.attach(8); 
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  //Sensores infrarrojos
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  //Voltaje del Teclado
  KP.SetKeypadVoltage(5.0);
  //Rfid
    // Inicializar el bus SPI */
  SPI.begin();
    // Inicializar el módulo MFRC522 */
  mfrc522.PCD_Init();
  // Mensaje Inicial
  lcd.init();
  lcd.backlight();
  lcd.begin (16, 2);
  lcd.setCursor(5, 0);
  lcd.print("Peaje ");
  lcd.setCursor(3, 1);
  lcd.print("automatico");
  delay(3000);
  lcd.clear();
}

void loop() {
  if (modo_recarga==1&&password.evaluate())//?????
  {
    if(hasRun==0){
      lcd.clear();
      hasRun=1;
    }
    recarga();
  }
  else if (modo_pago==1&&password.evaluate())
  {//¿podemos cambiarlo por switch?
  //queremsos A modo pago, B modo recarga
   if(balance>=100){
    pago();
   }
   else{
    exigirRecarga();
   }
  }
  else
  {
    lcd.setCursor(0, 0);
    leerSersoresIR(); //Lee sensores
   // Lee tarjeta , continua el codigo si no hay tarjeta//obtiene datos //falta guardar el balance
    if(password.evaluate()){
      while (countshow<1){
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("Seleccione una");
      lcd.setCursor(3,1);
      lcd.print("opcion");
      lcd.clear();
      countshow++;
      }
      hasRun=0;
      lcd.setCursor(3,0);
      lcd.print("A. Recarga");
      lcd.setCursor(3,1);
      lcd.print("B. Pago");
       byte KState = KP.Key_State();
        if (KState == PRESSED) {
          if ( Key = KP.Getkey() ) {
            Serial << "Pressed: " << Key << "\n";
            switch (Key) {
              case 'A': modo_recarga=1; modo_pago=0;break;
              case 'B': modo_pago=1;modo_recarga=0; break;
              default: break;
            }
          }
        }
    }
    if(sensor_IR2==1&&pasar==1&&pasasensor2==0){
      imprimirPago();
    }
    if(sensor_IR2==1&&pasar==1&&pasasensor2==1){
      if(hasRun1==0){
        lcd.clear();
        hasRun1=1;
      }
      lcd.setCursor(0, 0);
      lcd.print("Tenga un buen");
      lcd.setCursor(0, 1);
      lcd.print("viaje");
    }
    // Presiona el boton "A" para iniciar el modo recarga, sino , continua el codigo
    if (sensor_IR2 == 0 && pasar == 1)
    {
      servo.write(90);
      pasasensor2=1;
    }  
    else if (sensor_IR1 == 0)//Lo reinicia todo
    {
      lcd.clear();
      lcd.print("Bienvenido!!!");
      servo.write(10);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Vehiculo Detectado");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Coloque su tarjeta");
      lcd.setCursor(0, 1);
      lcd.print("en el lector....");
      delay(2000);
      lcd.clear();
      password.reset();
      modo_recarga = 0;
      modo_pago = 0;
      pasar = 0;
      pasasensor2=0;
      countshow=0;
      hasRun=false;
      hasRun1=false;//ESTO CAMBIé para que el servo siga arriba hasta que pase un nuevo carro
    }
    ObtieneyVerificaPassword();
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}




void pago()
{ 
  // Leemos el balance de la tarjeta
  Serial.print("Realizando Pago...");
  lcd.clear();
  lcd.print("Realizando Pago...");
  delay(500);

  balance = balance - 100; // String a float
  char balance_char[] = "";
  dtostrf(balance,2, 2, balance_char);
  byte Balance[16];  
  String(balance_char).getBytes(Balance,16);
  Serial.println(String((char*)Balance));
  WriteDataToBlock(blockNum_Balance, Balance);//siempre va a estar leyendo datos podemos añadir un flag de que si ya leyo o no los  datos
  modo_pago=0;
  pasar = 1;
  password.reset();
}




void leerSersoresIR()
{
  sensor_IR1 = digitalRead(sensorPin1);
  sensor_IR2 = digitalRead(sensorPin2);
}

//creo que se puede borrar
void servoDown()
{
  servo.attach(9);
  for (servoPos = 30; servoPos <= 120; servoPos += 1)
  {
    servo.write(servoPos);
    delay(5);
  }
}


void ObtieneyVerificaPassword(){
  

  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    Serial.println("no nueva tarjeta");
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    Serial.println("no lee");
    return;
  }

  // Leemos el balance de la tarjeta
  Serial.print("\n");
  Serial.println("Leyendo datos de la tarjeta...");

  ReadDataFromBlock(blockNum_Balance, readBlockData);
  balance = atof((char*)readBlockData); // String a float
  Serial.print("Balance");
  Serial.println((char*)readBlockData); 
  ReadDataFromBlock(blockNum_Password, readBlockData);
  password.set((char*)readBlockData);//siempre va a estar leyendo datos podemos añadir un flag de que si ya leyo o no los  datos
  Serial.print("contraseña");
  Serial.println((char*)readBlockData);

  int count=0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inserte su clave");
  delay(500);
  while(count<4){//Podemos contar hasta cuatro/
    char Key;
    byte KState = KP.Key_State();
    if (KState == PRESSED) { 
      if ( Key = KP.Getkey() ) {
        count++;
        lcd.setCursor(count, 1);
        lcd.print("*");//solo para mostraar la contraseña como asteriscos
        delay(100);
        password.append(Key);
        if(Key=='*'){
          password.reset();
          lcd.clear();
          count=0;
        } 
      }
    }
  }
  checkPassword();
}


void checkPassword() {
  if (password.evaluate()) {
    lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Clave correcta");
      delay(1000);
    //Add code to run if it works
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);        
    lcd.print("Clave incorrecta");
    delay(1500);
    lcd.clear();
    lcd.setCursor(3, 0);        
    lcd.print("Reingrese su");
    lcd.setCursor(5, 1);        
    lcd.print("tarjeta");
    password.reset();
    //add code to run if it did not work
  }
}

//


void recarga()
{
  int count=0;
  lcd.setCursor(0, 0);
  lcd.print ("Ingrese el monto");
  while(count<3)
  {
    count++;
    byte KState = KP.Key_State();
    if (KState == PRESSED)
    {
      Key = KP.Getkey();
      if (Key == '*')
      {
        monto_recarga = atoi(Data);
        balance = monto_recarga + balance ;
        char balance_char[] = "";
        dtostrf(balance,2, 2, balance_char);
        byte Balance[16];  
        String(balance_char).getBytes(Balance,16);

        // reescribir tarjeta
        WriteDataToBlock(blockNum_Balance,Balance);


        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tu balance");
        lcd.setCursor(0, 1);
        lcd.print("actual: ");
        lcd.setCursor(9, 1);
        lcd.print (balance);
        delay(3000);
        clearData();
        lcd.clear();
        modo_recarga = 0;
      }
      else if(Key == 'A'||Key == 'B'||Key == 'C'||Key == 'D'||Key == '#'){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tecla");
        lcd.setCursor(0, 1);
        lcd.print("incorrecta");
        delay(1500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Reingrese su");
        lcd.setCursor(0, 1);
        lcd.print("monto");
        delay(1500);
        lcd.clear();
        clearData();
        count=0;
      }
      else{
        Data[data_count] = Key;
        lcd.setCursor(data_count, 1);
        lcd.print(Data[data_count]);
        data_count++;
      }
    }
  }
}


void clearData()
{
  while (data_count != 0)
  {
    Data[data_count--] = 0;
  }
  return;
}


void KeyPadA()
{
  byte KState = KP.Key_State();

  if (KState == PRESSED)
  {
    Key = KP.Getkey();
    if (Key)
    {
      if (Key == 'A')
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Modo Recarga");
        lcd.setCursor(0, 1);
        lcd.print("................");
        delay(1500);
        lcd.clear();
        modo_recarga = 1;
      }
    }
  }
}

void imprimirPago()
{
  if(hasRun==0){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Pago la boleta");
    lcd.setCursor(0, 1);
    lcd.print(" exitosamente");
    delay(1500);
    lcd.clear();
    hasRun=1;
  }
  lcd.setCursor(0, 0);
  lcd.print(String("Su balance actual"));
  lcd.setCursor(0, 1);
  lcd.print("Monto: "+String(balance));
}

void exigirRecarga()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  Su balance");
  lcd.setCursor(0, 1);
  lcd.print(" es insuficiente");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Su balance actual");
  lcd.setCursor(0, 1);
  lcd.print("Monto: "+String(balance));
  delay(1500);
  lcd.clear();
  modo_pago=0;
}

void WriteDataToBlock(int blockNum, byte blockData[]) 
{
  byte s;
  byte req_buff[2];
  byte req_buff_size=2;
  mfrc522.PCD_StopCrypto1();
  s = mfrc522.PICC_HaltA();
  Serial.print("Halt Status: ");
  Serial.println(mfrc522.GetStatusCodeName((MFRC522::StatusCode)s));
  delay(100);
  s = mfrc522.PICC_WakeupA(req_buff,&req_buff_size);
  Serial.print("Request: ");
  Serial.println(mfrc522.GetStatusCodeName((MFRC522::StatusCode)s));
  delay(100);
  s = mfrc522.PICC_Select( &(mfrc522.uid),0);
  Serial.print("Selected : ");
  Serial.println(mfrc522.GetStatusCodeName((MFRC522::StatusCode)s));
  // Autentificando el Bloque de datos deseado para la escritura usando la Key A 
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("No se pudo escribir en la tarjeta: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  
  // Escribiendo en el Bloque de datos
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Escritura del Bloque Fallida: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
}


void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{
  /* Authenticating the desired data block for Read access using Key A */
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK)
  {
     Serial.print("Autenticacion fallo para lectura: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }

  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("La lectura fallo: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
}