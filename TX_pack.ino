int delayTime = 500;
const long interval = 125; // interval at which to stop PWM (milliseconds)
unsigned long previousMillis = 0;
int A_3 = 3;              // saída do PWM (eletrodo)
String control = "00";

void setup() {
  Serial.begin(9600);
  pinMode(A_3,OUTPUT);
}

void loop() {
  String data, packet;
  boolean shouldSend;
  uint32_t skey[4] = {0x9474B8E8, 0xC73BCA7D, 0x53239142, 0xf3c3121a};
  
  for(int i = 0; i < 4; i++){
    //control = "00";               // pacote de dados
    data = String(skey[i], BIN);
    if(32 - data.length())
      data = complete_zeros(32 - data.length()) + data;
    
    if (data != "") { // montagem do pacote   
      packet = complete_zeros(3 - String(i, BIN).length()) + String(i, BIN);   
      //packet += control ;
      packet += data;
      packet += checkParity(packet);
      shouldSend = true;
    }
    
    if (shouldSend) {
      digitalWrite(A_3, HIGH);
      delay(1000);
      modulation(packet);                // envio do pacote
      digitalWrite(A_3, LOW);
      delay(1000);
    }
    // Reset variables
    data = "";
    packet = "";
    shouldSend = false;
  }

}

void OOK_2mod(int state) {
  unsigned long currentMillis = millis();
  while (currentMillis + interval > millis())  //"espera ocupada"
    digitalWrite(A_3,state);//analogWrite(A_3, 127);
}

void modulation(String packet){
  for (int i = 0; i < packet.length(); i++) {
    Serial.print(packet.charAt(i));
    if (packet.charAt(i) == '0'){
      OOK_2mod(0);
    }
    else if (packet.charAt(i) == '1')
      OOK_2mod(1);
  }
  Serial.println();
}

String complete_zeros(int len){
  String zeros;
  for (int i = 0; i < len; i++)
    zeros = zeros + "0";
  return zeros;
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