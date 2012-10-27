/*
  rcx0_clone - The clone of a kenwood RC-20/10 for the Arduino.
  Copyright (c) 2012 Jun
*/
#include <LiquidCrystal.h>
#include "tm_frame.h"
#include "tm_spi.h"

//#define DEBUG  // for debug mode

#define RS  5
#define RW  6
#define EN  7
#define D4  A2
#define D5  A3
#define D6  A4
#define D7  A5
LiquidCrystal lcd(RS,RW,EN,D4,D5,D6,D7);

TmFrame tmf;
TmSpi& tmspi = TmSpi::get_instance();

// for command
byte cmd[2];
byte cmd_cnt = 0;

volatile void call_back(byte v) {
  byte ret;
  ret = tmf.parse(v);
  if(ret == 1){
    Serial.print("Illegal ID ");
    Serial.println(v, HEX);
  }else if(ret == 2){
    Serial.print("Unknown ID ");
    Serial.print(tmf.pfid, HEX);
    Serial.print(" ");
    Serial.println(v, HEX);
  }
  if(tmf.fid == 0xff && tmf.pfid != 0xff){
    print_frame(tmf.pfid);
    print_lcd();
  }
}

volatile void call_back_debug(byte v) {
  Serial.print(v, HEX);
  Serial.print(" ");
  if((v & 0xF0) == 0x80) Serial.println();
}

void setup() {
  Serial.begin(9600);
  
  lcd.begin(16, 2);
  lcd.clear();
#ifdef DEBUG
  Serial.println("START DEBUG MODE");
  tmspi.setup(call_back_debug);
#else
  Serial.println("START");
  tmspi.setup(call_back);
#endif
}

void loop() {
  byte buf;

  switch(tmspi.loop()){
  case -1: Serial.println("time out in RWAIT"); break;
  case -2: Serial.println("time out in RPREP"); break;
  case -3: Serial.println("time out in RRCV"); break;
  case -4: Serial.println("time out in SPREP"); break;
  case -5: Serial.println("time out in SENDING"); break;
  }

  if (tmspi.is_waiting() && Serial.available() > 0) {
    buf = Serial.read();
    if(('0' <= buf && buf <='9') ||
      ('a' <= buf && buf <='f') || 
      ('A' <= buf && buf <='F')){
      cmd[cmd_cnt++] = buf;
    }
    if(cmd_cnt >= 2){
      delay(50);
      buf = htoi(cmd[0]) * 16 + htoi(cmd[1]);
      Serial.println(buf, HEX);
      if(tmspi.send_cmd(buf) < 0) Serial.println("SEND ERR");
      cmd_cnt = 0;
    }
  }
}

byte htoi(byte c) {
  if('0' <= c && c <='9') return(c - '0');
  if('a' <= c && c <='f') return(c - 'a' + 10);
  if('A' <= c && c <='F') return(c - 'A' + 10);
  return 0;
}

void print_lcd() {
  lcd.setCursor(0, 0);
  tmf.print_fix16_col0(lcd);
  lcd.setCursor(0, 1);
  tmf.print_fix16_col1(lcd);
}

void print_frame(byte fid) {
  switch(fid){
  case 0x00:  // receiving freq.
  case 0x01:  // on air freq.
    tmf.print_freq_frame(Serial);
    break;
  case 0x02:  // display status
    tmf.print_disp_frame(Serial);
    break;
  case 0x05:  // s-meter
    Serial.println(tmf.smeter_s());
    break;
  case 0x06:  // ALT
    Serial.println(tmf.alt_s());
    break;
  case 0x07:  // memory channel status
    if(tmf.isnt_MR()) break;  // in VFO mode
    Serial.print("M");
    Serial.print(tmf.mr_s());
    Serial.println(); 
    break;
  case 0x08:  // function key status
    if(tmf.fstat == 0x81) Serial.println("F");
    else if(tmf.fstat == 0x83) Serial.println("F blinking");
    break;
  }
}

