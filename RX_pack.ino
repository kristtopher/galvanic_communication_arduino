const int GSR = A2;             // Fio de dados GSR (anarelo)
int lowThreshold = 0;
int sensorValue;
const int interruptPin = 2;     // pino de interrupção
int n = 500;                    // quantidade de amostras para média da condutância elétrica da pele
int frameSize = 36;             // tamanho do pacote (***a alterar para dinamico *** conforme necessidade)
int delayTime = 500;
const long interval = 125;      //150ms interval at which to stop PWM (milliseconds)
volatile bool flag_start = false;

String packet,data, parity;
uint32_t skey[4];
int id;

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
  Serial.print(F("lowThreshold = ")); // calcular max e min para média móvel
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
  int analogValue = 0;

  while(count < frameSize){
    analogValue = analogRead(GSR);
    //Serial.println(analogValue); // debug d leitura do pino  
    if (analogValue > 700) {//lowThreshold + rat 600 //ajustar parametro média móvel 800
      packet = packet + '1';
      delay(interval);
      count ++;
    }
    else if (analogValue < 500){//lowThreshold - rat 100 //ajustar parametro média móvel 400
      packet = packet + '0';
      delay(interval);
      count ++;
    }

  }
  count = 0;
  
  parity = packet[frameSize - 1];

  if(parity == checkParity(packet.substring(0 , 35))){
    id   = binToInt(packet.substring(0 ,  3));
    data = packet.substring(3 , 35);
    skey[id] = bitArrayToInt32(data, data.length());
    Serial.println(id);
    //Serial.println(data);
    Serial.println(skey[id],HEX);
  }
  else{
    Serial.println(F("ERROR"));
    Serial.println(packet);
  }
 
  packet = "";
  interrupts();
}

String checkParity(String data){
  int one = 0;
  for(int i = 0; i < data.length(); i++){
    if(data[i] == '1' )
      one++;
  }
  if(one%2)
    return "1";
  else
    return "0";
}

int binToInt(String binString){
  if      (binString == "000") return 0;
  else if (binString == "001") return 1;
  else if (binString == "010") return 2;
  else if (binString == "011") return 3;
  else if (binString == "100") return 4;
  else if (binString == "101") return 5;
  else if (binString == "110") return 6;
  else if (binString == "111") return 7;
  // Error
  else return -1;
}

uint32_t bitArrayToInt32(String arr, int count){
    uint32_t ret = 0x0000;
    unsigned int p1 = 0x0000;
    unsigned int p2 = 0x0000;
    for (int i = 0; i < count/2; i++) {
        if(arr[i] == '1'){
          p1 |= 0x01 << (count/2 - i - 1);
        }
       else{
          p1 |= 0x00 << (count/2 - i - 1);
        }
    }
    for (int i = count/2; i < count; i++) {
      if(arr[i] == '1'){
        p2 |= 0x01 << (count - i - 1);
      }
     else{
        p2 |= 0x00 << (count - i - 1);
      }
    }
    ret |= p1;
    ret = ret << 16;
    ret |= p2;
    return ret;
}