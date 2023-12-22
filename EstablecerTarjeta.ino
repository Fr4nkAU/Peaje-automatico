//Libreria RFID master
#include <SPI.h>
#include <MFRC522.h>

//Modulo RFID
#define SS_PIN 10   //Slave Select Pin 
#define RST_PIN 9   //Reset Pin 


//Creamos una instancia de  MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);
//Creamos una instancia de  MIFARE_Key
MFRC522::MIFARE_Key key; 
MFRC522::StatusCode status;

int blockNum_Password = 2; 
int blockNum_Balance = 5;

//Editar la contraseña a escribir
byte PasswordInicial[16] = {"4461"}; //7819 //
//Editar el balance inicial
byte BalanceInicial[16] = "150.00";


// Crea otra matriz para leer datos del bloque 
// La longitud del búfer debe ser 2 bytes más que el tamaño del bloque (16 bytes). */
byte bufferLen = 18;
byte readBlockData[18];
String string="";



void setup() {
  // Inicializar las comunicaciones seriales con la PC.
  Serial.begin(9600);
  // Inicializar el bus SPI 
  SPI.begin();
  // Inicializar el módulo MFRC522
  mfrc522.PCD_Init();
  Serial.println("Escaneando Tarjeta...");

}

void loop() {
  // Prepare la clave para la autenticación. 
  // Todas las claves están configuradas en FFFFFFFFFFFFh en el momento de la entrega del chip desde la fábrica. 
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }
  // Buscar nuevas tarjetas 
  // Restablezca el bucle si no hay ninguna tarjeta nueva en el lector RC522 */
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Selecciona una de las tarjetas 
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  Serial.print("\n");
  Serial.println("**Tarjeta Detectada*");
  // Imprime el UID de la Tarjeta 
  Serial.print(F("UID de la Tarjeta:"));
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print("\n");



  // Establecemos la contraseña
  
  Serial.println("Escrebiendo la nueva contraseña..");
  WriteDataToBlock(blockNum_Password, PasswordInicial);
  Serial.print("\n");
  // Establecemosa el saldo
  Serial.println("Escrbiendo el nuevo saldo..");
  WriteDataToBlock(blockNum_Balance, BalanceInicial);
  


  // Leemos e imprimimos la contraseña y el balance de la tarjeta
  Serial.print("\n");
  Serial.println("Leyendo datos de la tarjeta...");


  ReadDataFromBlock(blockNum_Password, readBlockData);
  string = String((char*)readBlockData);
  Serial.print("Contraseña:  ");
  Serial.println(string);

  ReadDataFromBlock(blockNum_Balance, readBlockData);
  string = String((char*)readBlockData);
  Serial.print("Balance:  ");
  Serial.println(string);

  
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

}

//Borrar los textor despues de verificar

void WriteDataToBlock(int blockNum, byte blockData[]) 
{
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



