//Programa que comprueba la contraseña ingresada en el teclado matricial
#include <OnewireKeypad.h>
#include <Password.h>

char KEYS[] = {
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '*', '0', '#', 'D'
};

OnewireKeypad <Print, 16 > KP(Serial, KEYS, 4, 4, A0, 5600,1500);//
Password password=Password("1234");



void setup () {
  Serial.begin(9600);

  KP.SetHoldTime(1500);
  
  // This method is set in the constructor with a default value of 5.0
  // You only need to include this if your Arduino is not supplying 5v to
  // the keypad. ie. ~4.7v or even with 3.3v Arduino boards too.
  KP.SetKeypadVoltage(5.00);
  KP.ShowRange();
}

void loop() {
  char Key;
  byte KState = KP.Key_State();

  if (KState == PRESSED) {
    if ( Key = KP.Getkey() ) {
      Serial << "Pressed: " << Key << "\n";
      switch (Key) {
        case '*': checkPassword(); break;
        case '#': password.reset(); break;
        default: password.append(Key);
      }
    }
  } else if (KState == HELD) {
    Serial << "Key:" << KP.Getkey() << " being held\n";
  }
}

void checkPassword() {
  if (password.evaluate()) {
    Serial.println("Success");
    //Add code to run if it works
  } else {
    Serial.println("Wrong");
    //add code to run if it did not work
  }
}
