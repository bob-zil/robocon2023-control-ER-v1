#include <Servo.h>
#include "sbus.h"

#define rs485 Serial2
#define rs232 Serial3
#define pc Serial
#define sen_homering 10
#define ring_limit 34

byte mreply[] = {};
Servo esc1, esc2, load, shoot;
int channels[16];  // Increased the size to 16 for 16 channels
int rotateChan[2];
boolean shooten = false;
boolean pickupen = false;
boolean pickup_en = false;
boolean lockgun1, lockgun2 = false;
int ring_count = 0;

/* SBUS object, reading SBUS */
bfs::SbusRx sbus_rx(&Serial1);
bfs::SbusData data;

void setup() {
  pc.begin(115200);
  pc.setTimeout(100);
  pc.flush();
  rs485.begin(115200);
  rs485.setTimeout(100);
  rs232.begin(19200);
  sbus_rx.Begin();
  esc1.attach(6, 1050, 1950);
  esc2.attach(7, 1050, 1950);
  load.attach(9, 900, 2100);
  shoot.attach(8);
  load_pro(false);
  shoot_pro(false);
  pinMode(sen_homering, INPUT);
  pinMode(13, OUTPUT);
  esc1.writeMicroseconds(1050);
  esc2.writeMicroseconds(1050);
  init_load_ring();
}

void loop() {
  if (sbus_rx.Read()) {  // get sbus channels from receiver
    data = sbus_rx.data();

    // Store SBUS channels in the channels array
    for (int8_t i = 0; i < data.NUM_CH; i++) {
      channels[i] = data.ch[i];
    }

    // Add additional data to the channels array
    channels[14] = rotateChan[0];
    channels[15] = rotateChan[1];

    // Send the channels array as binary data
    Serial.write((uint8_t*)channels, sizeof(channels));
  }

  if (data.ch[6] == 1002) {  //armed the remote control with button 'A'

    // pc.println("armed");
    if (data.ch[4] > 1200) { // bring container to the ground --> 'B' (High)
      if (pickupen == false) {
        pickup_ring();
        pickupen = true;
      }
    } else {
      pickupen = false;
      // pc.print(channels);
    }
    if (data.ch[7] == 1722) { // pickup ring to the top --> 'B' (hold)
      if (pickup_en == false) {
        pickupRing();
        // pickup_en = true;
      }
    } else {
      pickup_en = false;
    }

    if (data.ch[3] < 1012 && data.ch[3] > 992) {//left joystick cannon --> right joystick
      if (lockgun1 == false) {
        stopmotor(1);
        runInc_speed(1, 0.1, 180);
        lockgun1 = true;
      }
    } else if (data.ch[3] > 1012) {
      float velo = map(data.ch[3], 1012, 1722, 0, -20);
      run_speed(1, velo);
      lockgun1 = false;
    } else {
      float velo = map(data.ch[3], 992, 282, 0, 20);
      run_speed(1, velo);
      lockgun1 = false;
    }

    if (data.ch[2] < 1012 && data.ch[2] > 992) {//left joystick cannon --> right joystick
      if (lockgun2 == false) {
        stopmotor(2);
        runInc_speed(2, 0.1, 180);
        lockgun2 = true;
      }
    } else if (data.ch[2] > 1012) {
      float velo = map(data.ch[2], 1012, 1722, 0, -20);
      run_speed(2, velo);
      lockgun2 = false;
    } else {
      float velo = map(data.ch[2], 992, 282, 0, 20);
      run_speed(2, velo);
      lockgun2 = false;
    }
    if (data.ch[8] == 1722) { // shoot 'C' --> 'C'
      // if (data.ch[11] == 282) {
      //   armesc();
      // }  //test
      shoot_pro(true);
      shooten = true;
    } else {
      shoot_pro(false);
      load_ring(); //cmt this out
    if (data.ch[9] == 1722) { // enable servo shoot motor 'D'
      int offset = map(data.ch[11], 282, 1722, 1050, 1950);
      esc1.writeMicroseconds(offset);
      esc2.writeMicroseconds(offset);
    }

    if (data.ch[0] != 1002 || data.ch[1] != 1002) { // movement --> left joystick
      unsigned long t1 = millis();
      while (true) {
        channels = "";
        if (sbus_rx.Read()) {
          data = sbus_rx.data();
          for (int8_t i = 0; i < data.NUM_CH; i++) {
            channels = channels + String(data.ch[i]) + ",";
          }
          if (data.ch[5] == 1722) {
            //manual = 3;  // true
            delay(100);
            pc.println(channels);
          }
        }
        unsigned long t2 = millis();
        if ((t2 - t1) > 5000) {
          break;
        }
      }
    }

    if (rotateChan[0] != rotateChan[1]) { //rotate
      unsigned long t1 = millis();

      while (true) {
        pc.println(rotateChan[0]);
        pc.println(rotateChan[1]);
        if (rotateChan[0] < rotateChan[1]) {
          turning = false;
        } else if (rotateChan[0] > rotateChan[1]) {
          turning = true;
        } else if (rotateChan[0] == rotateChan[1]) {
          break;
        }
        channels = "";
        if (sbus_rx.Read()) {
          data = sbus_rx.data();
          rotateChan[0] = rotateChan[1];
          rotateChan[1] = data.ch[10];
          if (turning) {
            data.ch[10] = -1;
          } else {
            data.ch[10] = 1;
          }
          for (int8_t i = 0; i < data.NUM_CH; i++) {
            channels = channels + String(data.ch[i]) + ",";
          }
          if (data.ch[5] == 1722) {
            //manual = 3;  // true
            delay(100);
            pc.println(channels);
          }
        }
        unsigned long t2 = millis();
        if ((t2 - t1) > 5000) {
          break;
        }
      }
    }
  }
}

void shoot_pro(boolean sh) {  // true = shoot
  if (sh == true) {
    shoot.write(145);
  } else {
    shoot.write(47);
  }
}

void load_pro(boolean ld) {  // true = load
  if (ld == true) {
    load.writeMicroseconds(900);  //48
  } else {
    load.writeMicroseconds(2100);
  }
}

void init_load_ring() {  //find zero of the container
  runMulti_Angle_speed(1, 0, 40);

  if (digitalRead(sen_homering) != 0) {
    run_speed(3, -3600);
  }
  while (digitalRead(sen_homering) == 1) {
    delay(10);
  }
  for (int i = 0; i < 5; i++) {
    stopmotor(3);
    delay(10);
  }
  runInc_speed(3, 540, 3600);
}

void load_ring() {  //bring the ring up one by one
  if (shooten == true) {
    shooten = false;
    ring_count++;
    if (ring_count < 11) {
      load_pro(true);
      delay(1000);
      load_pro(false);
      delay(1000);
      runInc_speed(3, 630, 3600);
    } else {
      ring_count = 0;
      init_load_ring();
    }
  }
}

void pickup_ring() {           //drop the container to the bottom
  runInc_speed(3, -660, 720);  //-540
}

void shoot_speed() {
  int shoot_sp;
  shoot_sp = map(data.ch[11], 282, 1722, 1050, 1950);
  esc1.writeMicroseconds(shoot_sp);
  esc2.writeMicroseconds(shoot_sp);
}

void pickupRing() {  //pickup the rings to the loading area
  if (digitalRead(ring_limit) != 1) {
    run_speed(3, 3600);
  }

  while (digitalRead(ring_limit) == 0) {
    delay(10);
  }

  for (int i = 0; i < 5; i++) {
    stopmotor(3);
    delay(10);
  }
  runInc_speed(3, -630, 3600);

  pickup_en = true;
  shooten = true;
  load_pro(false);
}

void blink() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);
}
