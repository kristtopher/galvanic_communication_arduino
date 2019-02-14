const int GSR = A2;             // Fio de dados GSR (anarelo)
int lowThreshold = 0;
int rat = 300;
int sensorValue;
const int interruptPin = 2;     // pino de interrupção
int n = 500;                    // quantidade de amostras para média da condutância elétrica da pele
int frameSize = 32;             // tamanho do pacote (***a alterar para dinamico *** conforme necessidade)
int delayTime = 500;
const long interval = 140;      //150ms interval at which to stop PWM (milliseconds)
volatile bool flag = false;
volatile bool flag_start = false;
int overhead = 24;

String preamble,packet,data,data_,CRC, len;
char* dataLen, p;
char CRC_, preamble_; 

void setup() {
  long sum=0;
  Serial.begin(9600);
  pinMode(interruptPin, INPUT);
  delay(500);
  
  for(int i=0;i < n;i++){
    sensorValue = analogRead(GSR);
    sum += sensorValue;
    delay(5);
  }
  lowThreshold = sum / n;
  Serial.print(F("lowThreshold = "));
  Serial.println(lowThreshold);
  
  attachInterrupt(digitalPinToInterrupt(interruptPin), catch_signal, HIGH);
}

void loop() {
  if (flag_start){
    startbit();
    flag_start = false;
  }
}

void catch_signal() {
  flag_start = true;
}

void startbit(){
  long finish = 0;
  long i, f = 0;
  long initi = millis();
  while(digitalRead(interruptPin)){
    finish = millis() - initi;
  }
  if(finish >= 950){
    demodulation(); 
  }
}

void demodulation() {
  int count = 0;
  int analogValue = 0, data_length = 0;
  data_ = "";
  //delay(20);          //40                              //ajustar parametro
  while(count < frameSize){
  //for (int i = 0; i < frameSize; i++) {
    analogValue = analogRead(GSR);
    //Serial.println(analogValue); // debug d leitura do pino  
    if (analogValue > 750) {//lowThreshold + rat 600 //ajustar parametro média móvel 800
      packet = packet + '1';
      delay(interval-5);//-5
      count ++;
    }
    else if (analogValue < 400){//lowThreshold - rat 100 //ajustar parametro média móvel 400
      packet = packet + '0';
      delay(interval-5);//-5
      count ++;
    }

  }
  count = 0;
  //packet = 00100001000010000110000100111011       
  //         00100000000000000110000100111110

  //Serial.println(packet);
  Serial.println();
  preamble    = packet.substring(0,8);
  len         = packet.substring(8,16);
  dataLen     = const_cast<char*>(len.c_str());;
  data_length = strtol(dataLen, 0, 2);
  data        = packet.substring(16,16 + data_length);
  CRC         = packet.substring(16 + int(data_length),frameSize);
  preamble_   = binToASCII(preamble);
  CRC_        = binToASCII(CRC);
  
  for(int i = 0; i < (data_length/8); i++ )
    data_     = data_ + binToASCII(data.substring(i * 8, i * 8 + 8));
  
//  if(count > 16 && frameSize > 16){     // tamanho do pacote (***dinamico ***)
//    frameSize = int(data_length) + overhead;
//  }
  
  if (data != "" && checkCRC(String(data_), CRC) ) {
    if (data.indexOf('|') == -1) {
      Serial.print(F("Binari Packet Data: "));
      Serial.println(packet);
      Serial.print(F("ASCII Packet Data: "));
      Serial.print(preamble_);
      Serial.print(data_length);
      Serial.print(data_);
      Serial.println(CRC_);
      Serial.print(F("Received Data: "));
      Serial.println(data_);
    }
    else {
      Serial.println(F("ERROR: unsupported characters"));
    }
  }
  else if(data != "" && !checkCRC(String(data_), CRC))
    Serial.println(F("ERROR: CRC Invalid"));
  else{
    Serial.println(F("unknown error"));
    Serial.println(packet);
  }

  packet = "";
  interrupts();
}

String complete_zeros(int len){
  String zeros;
  for (int i = 0; i < len; i++)
    zeros = zeros + "0";
  return zeros;
}

bool checkCRC(String message, String CRC){
  byte ValB[message.length()];  //byte array allocation for data type conversion//length of the message in uint8 to match the function input data type below
  message.getBytes(ValB, message.length() + 1); //converting the string to a byte array
  char crc = char(CRC8(ValB, sizeof(ValB)));
  String valCRC = complete_zeros(8 - String(crc, BIN).length()) + String(crc, BIN);//converting the char data type representing the CRC check value to a binary representation in string format
  if(valCRC == CRC){
    return true;
  }
  else if(valCRC.length() > 8) // para pacote maiores (para validar CRC completo alterar tamanho do frame)
    if(valCRC.substring(0,8)  == CRC)
      return true;
  else{
    Serial.println(message);
    Serial.println(valCRC);
    Serial.println(F("CRC check is NOT valid"));
    return false;
  }
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

char binToASCII(String binString) {
  // Symbols
  if      (binString == "00100000") return ' ';
  else if (binString == "01100110") return 'f';
  else if (binString == "00100001") return '!';
  else if (binString == "00100010") return '"';
  else if (binString == "00100011") return '#';
  else if (binString == "00100100") return '$';
  else if (binString == "00100101") return '%';
  else if (binString == "00100110") return '&';
  else if (binString == "00100111") return '\'';
  else if (binString == "00101000") return '(';
  else if (binString == "00101001") return ')';
  else if (binString == "00101010") return '*';
  else if (binString == "00101011") return '+';
  else if (binString == "00101100") return ',';
  else if (binString == "00101101") return '-';
  else if (binString == "00101110") return '.';
  else if (binString == "00101111") return '/';
  else if (binString == "00111010") return ':';
  else if (binString == "00111011") return ';';
  else if (binString == "00111100") return '<';
  else if (binString == "00111101") return '=';
  else if (binString == "00111110") return '>';
  else if (binString == "00111111") return '?';
  else if (binString == "01000000") return '@';
  
  // Lowercase
  else if (binString == "01100001") return 'a';
  else if (binString == "01100010") return 'b';
  else if (binString == "01100011") return 'c';
  else if (binString == "01100100") return 'd';
  else if (binString == "01100101") return 'e';
  else if (binString == "01100110") return 'f';
  else if (binString == "01100111") return 'g';
  else if (binString == "01101000") return 'h';
  else if (binString == "01101001") return 'i';
  else if (binString == "01101010") return 'j';
  else if (binString == "01101011") return 'k';
  else if (binString == "01101100") return 'l';
  else if (binString == "01101101") return 'm';
  else if (binString == "01101110") return 'n';
  else if (binString == "01101111") return 'o';
  else if (binString == "01110000") return 'p';
  else if (binString == "01110001") return 'q';
  else if (binString == "01110010") return 'r';
  else if (binString == "01110011") return 's';
  else if (binString == "01110100") return 't';
  else if (binString == "01110101") return 'u';
  else if (binString == "01110110") return 'v';
  else if (binString == "01110111") return 'w';
  else if (binString == "01111000") return 'x';
  else if (binString == "01111001") return 'y';
  else if (binString == "01111010") return 'z';

  // Capital
  else if (binString == "01000001") return 'A';
  else if (binString == "01000010") return 'B';
  else if (binString == "01000011") return 'C';
  else if (binString == "01000100") return 'D';
  else if (binString == "01000101") return 'E';
  else if (binString == "01000110") return 'F';
  else if (binString == "01000111") return 'G';
  else if (binString == "01001000") return 'H';
  else if (binString == "01001001") return 'I';
  else if (binString == "01001010") return 'J';
  else if (binString == "01001011") return 'K';
  else if (binString == "01001100") return 'L';
  else if (binString == "01001101") return 'M';
  else if (binString == "01001110") return 'N';
  else if (binString == "01001111") return 'O';
  else if (binString == "01010000") return 'P';
  else if (binString == "01010001") return 'Q';
  else if (binString == "01010010") return 'R';
  else if (binString == "01010011") return 'S';
  else if (binString == "01010100") return 'T';
  else if (binString == "01010101") return 'U';
  else if (binString == "01010110") return 'V';
  else if (binString == "01010111") return 'W';
  else if (binString == "01011000") return 'X';
  else if (binString == "01011001") return 'Y';
  else if (binString == "01011010") return 'Z';

  // Numbers
  else if (binString == "00110000") return '0';
  else if (binString == "00110001") return '1';
  else if (binString == "00110010") return '2';
  else if (binString == "00110011") return '3';
  else if (binString == "00110100") return '4';
  else if (binString == "00110101") return '5';
  else if (binString == "00110110") return '6';
  else if (binString == "00110111") return '7';
  else if (binString == "00111000") return '8';
  else if (binString == "00111001") return '9';
  else if (binString == "00100001") return '!';

  // Error
  else return '|';
}