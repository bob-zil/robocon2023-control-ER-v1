void trans(byte data[], byte sizedata, int sizereply) {
  //byte respon[sizereply];
  // for (int i = 0; i < 49; i++) {
  //   mreply[i] = 0;
  // }
  if (rs485.availableForWrite() >= sizedata) {
    rs485.write(data, sizedata);
  }
  rs485.flush();
  // while (rs485.available() == 0) {
  //   delay(1);
  //   pc.println("here");
  // }
  // if (rs485.available() > 0) {
  //   rs485.readBytes(mreply, sizereply);
  // }
  // for (int i = 0; i < sizereply; i++) {
  //   Serial.print(mreply[i], HEX);
  //   Serial.print(" ");
  // }
  // Serial.println("");
  delay(5);
}

void stopmotor(byte idm) {
  byte mstop[5] = { 0x3e, 0x81, idm, 0x00 };
  mstop[4] = 0x3e + 0x81 + idm;
  trans(mstop, 5, 5);
}

void opermotor(byte idm) {
  byte moper[5] = { 0x3e, 0x88, idm, 0x00 };
  moper[4] = 0x3e + 0x88 + idm;
  trans(moper, 5, 5);
}

void clearError(byte idm) {
  byte mstop[5] = { 0x3e, 0x9b, idm, 0x00 };
  mstop[4] = 0x3e + 0x9b + idm;
  trans(mstop, 5, 13);
}

//void writeMotor0(byte idm) {
//  readEncoder(idm);
//  byte mWriteM0[8] = {0x3e, 0x91, idm, 0x02};
//  mWriteM0[4] = 0x3e + 0x91 + idm + 0x02;
//  mWriteM0[5] = mreply[7];
//  mWriteM0[6] = mreply[8];
//  mWriteM0[7] = mWriteM0[5] + mWriteM0[6];
//  trans(mWriteM0, 8, 8);
//}
void runMulti_Angle_speed(byte mId, float angle, float velo) {
  if (mId == 1 || mId == 2) {
    angle = angle * 9;
    velo = velo * 9;
  }
  long sum = 0;
  int64_t degree = angle * 100;
  uint32_t speed = velo * 100;
  byte mpos[18] = { 0x3e, 0xa4, mId, 0x0c };
  mpos[4] = 0x3e + 0xa4 + mId + 0x0c;
  for (int i = 5; i < 13; i++) {
    mpos[i] = degree >> 8 * (i - 5);
    sum += mpos[i];
  }
  for (int i = 13; i < 17; i++) {
    mpos[i] = speed >> 8 * (i - 13);
    sum += mpos[i];
  }
  mpos[17] = sum;
  trans(mpos, 18, 13);
}

void runInc_speed(byte mId, float angle, float velo) {
  if ((mId == 1) || (mId == 2)) {
    angle = angle * 9;
    velo = velo * 9;
  }
  long sum = 0;
  int64_t degree = angle * 100;
  uint32_t speed = velo * 100;
  byte minc[14] = { 0x3e, 0xa8, mId, 0x08 };
  minc[4] = 0x3e + 0xa8 + mId + 0x08;
  for (int i = 5; i < 9; i++) {
    minc[i] = degree >> 8 * (i - 5);
    sum += minc[i];
  }
  for (int i = 9; i < 13; i++) {
    minc[i] = speed >> 8 * (i - 9);
    sum += minc[i];
  }
  minc[13] = sum;
  trans(minc, 14, 13);
}

void run_speed(byte mId, float velo) {
  if ((mId == 1) || (mId == 2)) {
    velo = velo * 9;
  }
  long sum = 0;
  int32_t speed = velo * 100;
  byte mspeed[10] = { 0x3e, 0xa2, mId, 0x04 };
  mspeed[4] = 0x3e + 0xa2 + mId + 0x04;
  for (int i = 5; i < 9; i++) {
    mspeed[i] = speed >> 8 * (i - 5);
    sum += mspeed[i];
  }
  mspeed[9] = sum;
  trans(mspeed, 10, 13);
}

void trans_rs232(byte data[], byte sizedata, int sizereply) {
  //byte respon[sizereply];
  // for (int i = 0; i < 49; i++) {
  //   mreply[i] = 0;
  // }
  if (rs232.availableForWrite() >= sizedata) {
    rs232.write(data, sizedata);
  }
  rs232.flush();
  // while (rs485.available() == 0) {
  //   delay(1);
  //   pc.println("here");
  // }
  if (rs232.available() > 0) {
    rs232.readBytes(mreply, sizereply);
  }
  for (int i = 0; i < sizereply; i++) {
    Serial.print(mreply[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
  delay(5);
}

void open_laser(){
  byte open[1] = {0X4F};
  trans_rs232(open,1,6);
}

void Get_distance_laser(){
  byte get[1] = {0x44};
  trans_rs232(get,1,6);
}

void close_laser(){
  byte close[1] = {0x43};
  trans_rs232(close,1,6);
}
