#include "tm_frame.h"

TmFrame::TmFrame(){
    fid = 0xff;  // frame id
    pfid = 0xff; // previous frame id
    fbyte = 0;  // receive byte of a frame
}

byte TmFrame::parse(byte v){
  if(fid == 0xff){
    if(v > 15) return 1;  // Illegal ID
    fid = v;
    fbyte = 0;
  }else if(fid == 0x00 || fid == 0x01){
    freq[fbyte++] = v;
    if((v & 0xf0) == 0x80 || fbyte >= 8){ pfid = fid; fid = 0xff; }
  }else if(fid == 0x02){
    disp[fbyte++] = v;
    if((v & 0xf0) == 0x80 || fbyte >= 6){ pfid = fid; fid = 0xff; }
  }else if(fid == 0x05){
    smeter = v;
    pfid = fid; fid = 0xff;
  }else if(fid == 0x06){
    alt = v;
    pfid = fid; fid = 0xff;
  }else if(fid == 0x07){
    mr[fbyte++] = v;
    if((v & 0xf0) == 0x80 || fbyte >= 2){ pfid = fid; fid = 0xff; }
  }else if(fid == 0x08){
    fstat = v;
    pfid = fid; fid = 0xff;
  }else{
    pfid = fid; fid = 0xff;
    return 2;  // Unknown ID
  }
  return 0;
}

char * TmFrame::freq_s(){
  byte i, j;
  for(i = 0, j = 0;i < 8; i++){
      if((freq[i] & 0xf) < 10) str[j++] = (freq[i] & 0xf) + '0';
      else if((freq[i] & 0xf) == 10) str[j++] = '-';
      if((freq[i] & 0xf0) == 0x60) str[j++] = '.';
      if((freq[i] & 0xf0) == 0x80) break;
  }
  str[j] = '\0';
  return str;
}

char * TmFrame::freq_ss(){
  byte i, j, s;
  for(i = 0;i < 8; i++){
    if((freq[i] & 0xf0) == 0x60){
      s = i - 1; break;
    }
  }
  for(i = s, j = 0;i < 8; i++){
    if((freq[i] & 0xf) < 10) str[j++] = (freq[i] & 0xf) + '0';
    else if((freq[i] & 0xf) == 15) str[j++] = '0';
    else if((freq[i] & 0xf) == 10) str[j++] = '-';
    else str[j++] = ' ';
    
    if((freq[i] & 0xf0) == 0x60) str[j++] = '.';
    if((freq[i] & 0xf0) == 0x80) break;
  }
  str[j] = '\0';
  return str;
}

char * TmFrame::smeter_s(){
  str[0] = 'S';
  if(is_SBUSY()){
    str[2] = str[3] = ' ';
    switch(smeter & 0x7){
    case 0: str[1] = '0'; break;
    case 1: str[1] = '1'; break;
    case 2: str[1] = '3'; break;
    case 3: str[1] = '5'; break;
    case 4: str[1] = '7'; break;
    case 5: str[1] = '9'; break;
    case 6: str[1] = '9'; str[2] = '+'; break;
    case 7: str[1] = '9'; str[2] = str[3] = '+'; break;
    }
  }else{
    str[1] = str[2] = '_';
    str[3] = ' ';
  }
  str[4] = '\0';
  return str;
}

char * TmFrame::alt_s(){
  if((alt & 0x04) == 0){
    strcpy(str, "ALT OFF");
  }else{
    strcpy(str, " ALT ");
    if(alt & 0x01) str[4] = '>';
    if(alt & 0x02) str[0] = '<';
  }
  return str;
}

char * TmFrame::mr_s(){
  if((mr[0] & 0x03) < 3) str[0] = (mr[0] & 0x03) + '0';
  else str[0] = '0';
  str[1] = (mr[1] & 0x0f) + '0';
  str[2] = '\0';
  return str;
}

char * TmFrame::power_s(){
  switch(disp[4] & 0xc){
  case 0: str[0] = 'H'; str[1] = 'I'; str[2] = ' '; break;
  case 8: str[0] = 'M'; str[1] = 'I'; str[2] = 'D'; break;
  case 4: str[0] = 'L'; str[1] = 'O'; str[2] = 'W'; break;
  }
  str[3] = '\0';
  return str;
}

char TmFrame::CMV_s(){
  if(is_CALL()) return 'C';
  else if(is_MR()) return 'M';
  return 'V';
}

void TmFrame::print_fix16_col0(Print &p){
  p.print(freq_ss());

  if(is_STAR()) p.print('*'); // F(1 sec) + MR/M in MR mode
  else p.print(' ');

  if(fstat == 0x81) p.print('F');
  else if(fstat == 0x83) p.print('f');  // blinking
  else p.print(' ');

  p.print(CMV_s());
  
  p.print(mr_s());
  p.print(' ');
  p.print(power_s());
}

void TmFrame::print_fix16_col1(Print &p){
  p.print(smeter_s());

  if(is_REV()) p.print("REV");
  else         p.print("rev");

  switch(disp[0] & 0x3){  // SHIFT
  case 0: p.print("- "); break;
  case 1: p.print("+ "); break;
  case 3: p.print("--"); break;
  default:p.write(0xA5); p.print(" ");
  }

  if(is_AL()) p.print("AL ");
  else        p.print("al ");
  
  switch(disp[1] & 0xc){
  case 4:   p.print("cT "); break;
  case 0xc: p.print("CT "); break;
  default:  p.print("ct ");
  }
  
  if(is_BELL()) p.print('B');
  else          p.print('b');
}

void TmFrame::print_freq_frame(Print &p){
  p.print(freq_s());
  p.println(" MHz"); 
}

void TmFrame::print_disp_frame(Print &p){
  if(is_BUSY()) p.print("BUSY ");
  if(is_REV()) p.print("REV ");
  switch(disp[0] & 0x3){  // SHIFT
  case 0: p.print("- "); break;
  case 1: p.print("+ "); break;
  case 3: p.print("-- "); break;
  }
  switch(disp[1] & 0xc){
  case 4: p.print("T "); break;
  case 0xc: p.print("CTCSS "); break;
  }
  if(is_AL()) p.print("AL ");
  if(is_CALL()) p.print("CALL ");
  if(is_MR()) p.print("MR ");
  else        p.print("VFO ");
    
  if(is_BEEP_OFF()) p.print("BEEP_OFF "); // F(1 sec) + REV/STEP
  if(is_TONE_MODE()) p.print("TONE_MODE ");  // F(1 sec) + TONE/SHIFT
  if(is_STEP_MODE()) p.print("STEP_MODE ");
    
  // memory channel lock out
  if(is_STAR()) p.print("* "); // F(1 sec) + MR/M in MR mode
  if(is_SCAN_MODE()) p.print("SCAN_MODE ");  // VFO(1 sec) or MR(1 sec)

  p.print(power_s());
  p.print(" ");
    
  if(is_BELL()) p.print("BELL ");
  p.println(); 
}

