#include "tm_spi.h"

TmSpi::TmSpi(){
  bit_stat = TMSPI_RWAIT;
  bit_data = 0;
  bit_cnt = 0;
  time = 0;
  retry_cnt = 0;
}

void TmSpi::setup(volatile void (*func)(byte)){
  cb_func = func;
  
  pinMode(TMSPI_P_READ, INPUT);   // PTT/SS
  pinMode(TMSPI_P_CLK, INPUT);    // UP
  pinMode(TMSPI_P_WRITE, OUTPUT); // DN
  digitalWrite(TMSPI_P_WRITE, HIGH);
  
  bit_stat == TMSPI_RWAIT;
  send_cmd(0);
}

char TmSpi::loop(){
  byte buf;
  
  if(bit_stat == TMSPI_RWAIT && digitalRead(TMSPI_P_READ) == 0){
    digitalWrite(TMSPI_P_WRITE, LOW);
    delayMicroseconds(200);
    if(digitalRead(TMSPI_P_READ) == 0){
      bit_stat = TMSPI_RPREP;
      bit_data = bit_cnt = 0;
      digitalWrite(TMSPI_P_LED, HIGH);  // indicate in receiving
      time = micros();
      attachInterrupt(TMSPI_INT_CLK, TmSpi::int_rcv_clk, RISING);
    }else{
      digitalWrite(TMSPI_P_WRITE, HIGH); // time out
      return -1; // time out in RWAIT
    }
  }else if(bit_stat == TMSPI_RPREP){
    if(micros() - time > 2000){  // time out
      detachInterrupt(TMSPI_INT_CLK);
      bit_stat = TMSPI_RWAIT;
      digitalWrite(TMSPI_P_LED, LOW);
      digitalWrite(TMSPI_P_WRITE, HIGH);
      delay(100);
      return -2; // time out in RPREP
    }
  }else if(bit_stat == TMSPI_RRCV){
    if(micros() - time > 10000){  // time out
      detachInterrupt(TMSPI_INT_CLK);
      bit_stat = TMSPI_RWAIT;
      digitalWrite(TMSPI_P_LED, LOW);
      digitalWrite(TMSPI_P_WRITE, HIGH);
      delay(100);
      return -3; // time out in RRCV
    }
  }else if(bit_stat == TMSPI_SENT && digitalRead(TMSPI_P_CLK)){
    delayMicroseconds(100);
    digitalWrite(TMSPI_P_WRITE, HIGH);
    digitalWrite(TMSPI_P_LED, LOW);
    bit_stat = TMSPI_RWAIT;
  }else if(bit_stat == TMSPI_SPREP){
    if(micros() - time > 2000){  // time out
      detachInterrupt(TMSPI_INT_CLK);
      bit_stat = TMSPI_RWAIT;
      digitalWrite(TMSPI_P_LED, LOW);
      digitalWrite(TMSPI_P_WRITE, HIGH);
      delay(50);
      if(retry_cnt++ < 5) send_cmd(bit_data);
      else return -4;  // time out in SPREP
    }
  }else if(bit_stat == TMSPI_SENDING){
    if(micros() - time > 10000){  // time out
      detachInterrupt(TMSPI_INT_CLK);
      bit_stat = TMSPI_RWAIT;
      digitalWrite(TMSPI_P_LED, LOW);
      digitalWrite(TMSPI_P_WRITE, HIGH);
      delay(50);
      return -5; // time out in SENDING
    }
  }
  return 0;
}

char TmSpi::send_cmd(byte c) {
  if(bit_stat != TMSPI_RWAIT) return -1;  // error
  
  retry_cnt = 0;
  bit_stat = TMSPI_SPREP;
  bit_data = c;
  bit_cnt = 0x80;
  digitalWrite(TMSPI_P_LED, HIGH);
  digitalWrite(TMSPI_P_WRITE, LOW);
  time = micros();
  delayMicroseconds(200);
  attachInterrupt(TMSPI_INT_CLK, TmSpi::int_snd_clk, FALLING);
  
  return 0;
}

void TmSpi::int_rcv_clk() {
  TmSpi & obj = get_instance();
  byte ret;
  noInterrupts();
  if(obj.bit_stat == TMSPI_RPREP && digitalRead(TMSPI_P_CLK)) {
    digitalWrite(TMSPI_P_WRITE, HIGH);
    obj.bit_stat = TMSPI_RRCV;
  }else if(obj.bit_stat == TMSPI_RRCV && digitalRead(TMSPI_P_CLK)){
    obj.bit_data = (obj.bit_data << 1) | digitalRead(TMSPI_P_READ);
    if( (++obj.bit_cnt) >= 8){
      obj.cb_func(obj.bit_data);
      ret = 0;
      while(digitalRead(TMSPI_P_READ) == 0 && ret++ < 100) delayMicroseconds(10);
      obj.bit_stat = TMSPI_RWAIT;
      digitalWrite(TMSPI_P_LED, LOW);
      detachInterrupt(TMSPI_INT_CLK);
    }
  }
  interrupts();
}

void TmSpi::int_snd_clk() {
  TmSpi & obj = get_instance();
  noInterrupts();
  if(obj.bit_stat == TMSPI_SPREP && digitalRead(TMSPI_P_CLK) == 0 && digitalRead(TMSPI_P_READ) == 0){
    obj.bit_stat = TMSPI_SENDING;
    obj.time = micros();
  }else if(obj.bit_stat == TMSPI_SENDING && digitalRead(TMSPI_P_CLK) == 0 && digitalRead(TMSPI_P_READ)){
    if(obj.bit_data & obj.bit_cnt){
      digitalWrite(TMSPI_P_WRITE, HIGH);
    }else{
      digitalWrite(TMSPI_P_WRITE, LOW);
    }
    obj.bit_cnt >>= 1;
    if(obj.bit_cnt == 0){
      obj.bit_stat = TMSPI_SENT;
      detachInterrupt(TMSPI_INT_CLK);
    }
  }
  interrupts();
}
