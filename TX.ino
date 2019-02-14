int delayTime = 500;
const long interval = 140; // interval at which to stop PWM (milliseconds)
unsigned long previousMillis = 0;
int A_3 = 3;              // saída do PWM (eletrodo)
int interruptPin = 9;     // sinal de interrupção de envio de mensagem

// Serial variables
char input = 'a';         // payload fixo enviado

void setup() {
  Serial.begin(9600);
  pinMode(A_3,OUTPUT);
  pinMode(interruptPin,OUTPUT);
}

void loop() {
  String message, CRC, data, packet, preamble, dataLen;
  char crc;
  boolean shouldSend;
  long init;
    if ((data = ASCII2bin(input)) != "") { // montagem do pacote   
      preamble = "00100001";   // - Change preamble later = !
      dataLen = complete_zeros(8 - String(data.length(), BIN).length()) + String(data.length(), BIN);//converting the char data type representing the length of the data segment to a binary representation in string format
      CRC = calcCRC(String(input));
      packet = preamble + dataLen + data + CRC ; //packet/frame to be sent (preamble, size of data, and data,crc)
      shouldSend = true;
    }

    if (shouldSend) {
      digitalWrite(A_3, HIGH);digitalWrite(interruptPin, HIGH);  // interrupção início de envio
      delay(1000);
      modulation(packet);                // envio do pacote
      //digitalWrite(interruptPin, LOW);   // fim do envio
      digitalWrite(A_3, LOW);
      delay(1000);
    }
    
    // Reset variables
    data = "";
    packet = "";
    
    shouldSend = false;
}

void _2mod(int state) {
  unsigned long currentMillis = millis();
  while (currentMillis + interval > millis())  //"espera ocupada"
    digitalWrite(A_3,state);//analogWrite(A_3, 127);
}

void modulation(String packet){
  for (int i = 0; i < packet.length(); i++) {
    //Serial.print(packet.charAt(i));
    if (packet.charAt(i) == '0'){
      _2mod(0);
    }
    else if (packet.charAt(i) == '1')
      _2mod(1);
  }
  //Serial.println();
}

String complete_zeros(int len){
  String zeros;
  for (int i = 0; i < len; i++)
    zeros = zeros + "0";
  return zeros;
}

String ASCII2bin(char value){ // Convert ASCII to binary
  String val;
  val = val + complete_zeros (8 - String(value, BIN).length()) + String(value, BIN);
  return val;
}

byte CRC8(const byte *data, byte len) {
  byte crc = 0x00;
  while (len--) {
    byte extract = *data++;
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum)
        crc ^= 0x8C;
      extract >>= 1;
    }
  }
  return crc;
}

String calcCRC(String message){
  byte ValB[message.length()];  //byte array allocation for data type conversion//length of the message in uint8 to match the function input data type below
  message.getBytes(ValB, message.length() + 1); //converting the string to a byte array
  char crc = char(CRC8(ValB, sizeof(ValB)));
  return complete_zeros(8 - String(crc, BIN).length()) + String(crc, BIN);//converting the char data type representing the CRC check value to a binary representation in string format    
}