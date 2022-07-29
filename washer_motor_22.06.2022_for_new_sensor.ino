
#include <Wire.h>
#include <avr/wdt.h>
#include <EEPROM.h>

const int pmot_en = 2;
const int pmot_stp = 3;
const int pmot_dir = 4;

const int mmot_en = 5;
const int mmot_stp = 6;
const int mmot_dir = 7;

const int p_sen = 12;
const int m_sen = 13;

const int rins = 9;
const int wash = 10;
const int waste = 11;

const int sol = 8;

int pinit_status = 0;
int minit_status = 0;

int a[8] = {2000, 2500, 3200, 3800, 4300, 4800, 5500, 6200};
int sp[10] = {90, 95, 100, 105, 110, 115, 120, 125, 130, 135};

String de;
String te;
int pos = 0; //positions
int vol = 0; //volume
int crs = 0; //cross wash
int spd = 0; //shake speed
int dur = 0; //shake speed

int wellbottom = 0;
int vpump = 0;

struct myobject {
  int val1;
};

void setup()
{
  Serial.begin(9600);
  pinMode(mmot_en, OUTPUT);
  pinMode(mmot_stp, OUTPUT);
  pinMode(mmot_dir, OUTPUT);
  pinMode(pmot_en, OUTPUT);
  pinMode(pmot_stp, OUTPUT);
  pinMode(pmot_dir, OUTPUT);
  pinMode(sol, OUTPUT);
  digitalWrite(mmot_en, HIGH);
  digitalWrite(pmot_en, HIGH);
  pinMode(p_sen, INPUT);
  pinMode(m_sen, INPUT);
  Wire.begin(0x07); //Set Arduino up as an I2C slave at address 0x07
  Wire.onRequest(requestEvent); //Prepare to send data
  Wire.onReceive(receiveEvent); //Prepare to recieve data
  analogWrite(rins, 255);
  analogWrite(wash, 255);
  analogWrite(waste, 0);
  digitalWrite(sol, LOW);
  delay(1000);
  get_motor_val();
  manifold_home();
  plate_home();
  move_manifold_bottom_prime();
  move_manifold_down(1200);
}

void loop()
{
  Wire.begin(0x07);
  delay(50);
  if (te.substring(0, 3) == "ini")
  {
    manifold_home();
    plate_home();
    delay(100);
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "rns")
  {
    rinse(spd, dur);
    delay(100);
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "prm")
  {
    prime(spd, dur);
    delay(100);
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "mov")
  {
    wash_plate(pos, vol, crs, spd);
    delay(100);
    if (te.substring(0, 3) == "ini")
    {
      manifold_home();
      plate_home();
    }
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "mof")
  {
    wash_plate_finalasp(pos, vol, crs);
    delay(100);
    if (te.substring(0, 3) == "ini")
    {
      manifold_home();
      plate_home();
    }
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "shk")
  {
    manifold_home();
    plate_home();
    shake(spd, dur);
    manifold_home();
    plate_home();
    move_manifold_bottom_prime();
    move_manifold_down(1200);
    delay(100);
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "sok")
  {
    idle();
    delay(100);
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "mhm")
  {
    manifold_home_count();
    delay(100);
    set_motor_val();
    delay(100);
    de = "caldone";
    te = "";
  }
  if (te.substring(0, 3) == "inf")
  {
    manifold_home();
    plate_home();
    analogWrite(waste, 0);

    move_manifold_bottom_prime();
    move_manifold_down(1200);
    delay(100);
    de = "done";
    te = "";
  }
}

void get_motor_val()
{
  myobject motorval;
  EEPROM.get(0, motorval);
  wellbottom = motorval.val1;
}

void set_motor_val()
{
  myobject motorval = {wellbottom};
  EEPROM.put(0, motorval);
}

void rinse(int spd, int dur)
{
  manifold_home();
  Serial.println(spd);
  Serial.println(dur);
  analogWrite(waste, 255);
  delay(500);
  move_manifold_bottom_prime();
  delay(1000);

  digitalWrite(sol, HIGH);
  delay(100);
  analogWrite(rins, spd);
  delay(dur * 1000);
  analogWrite(rins, 255);
  delay(100);
  digitalWrite(sol, LOW);
  analogWrite(waste, 0);
  delay(8000);

  move_manifold_down(1200);
}

void prime(int spd, int dur)
{
  manifold_home();
  analogWrite(waste, 255);
  delay(500);
  move_manifold_bottom_prime();
  delay(1000);

  digitalWrite(sol, HIGH);
  delay(100);
  analogWrite(wash, spd);
  delay(dur * 1000);
  analogWrite(wash, 255);
  delay(100);
  digitalWrite(sol, LOW);
  analogWrite(waste, 0);
  delay(8000);
  //analogWrite(waste, 0);
  //move_manifold_up(7500);
  move_manifold_down(1200);
}

void plate_home()
{
  digitalWrite(pmot_en, LOW);
  int stat = digitalRead(p_sen);
  if (stat == 0)
  {
    digitalWrite(pmot_dir, HIGH);
    for (int i = 0; i < 500; i++)
    {
      digitalWrite(pmot_stp, HIGH);
      delayMicroseconds(200);
      digitalWrite(pmot_stp, LOW);
      delayMicroseconds(200);
    }
    digitalWrite(pmot_dir, LOW);
    for (int i = 0; i < 4000; i++)
    {

      stat = digitalRead(p_sen);
      if (stat == 1)
      {
        digitalWrite(pmot_stp, HIGH);
        delayMicroseconds(200);
        digitalWrite(pmot_stp, LOW);
        delayMicroseconds(200);
      }
      else
      {
        pinit_status = 1;
        break;
      }
    }
  }
  else
  {
    digitalWrite(pmot_dir, LOW);
    for (int i = 0; i < 30000; i++)
    {
      stat = digitalRead(p_sen);
      if (stat == 1)
      {
        digitalWrite(pmot_stp, HIGH);
        delayMicroseconds(200);
        digitalWrite(pmot_stp, LOW);
        delayMicroseconds(200);
      }
      else
      {
        pinit_status = 1;
        break;
      }
    }
  }
  stat = 1;
  digitalWrite(pmot_en, HIGH);
}


void manifold_home()
{
  digitalWrite(mmot_en, LOW);
  int stats = digitalRead(m_sen);
  if (stats == 0)
  {
    digitalWrite(mmot_dir, LOW);
    for (int i = 0; i < 500; i++)
    {
      digitalWrite(mmot_stp, HIGH);
      delayMicroseconds(100);
      digitalWrite(mmot_stp, LOW);
      delayMicroseconds(100);
    }
    digitalWrite(mmot_dir, HIGH);
    for (int i = 0; i < 4000; i++)
    {

      stats = digitalRead(m_sen);
      if (stats == 1)
      {
        digitalWrite(mmot_stp, HIGH);
        delayMicroseconds(100);
        digitalWrite(mmot_stp, LOW);
        delayMicroseconds(100);
      }
      else
      {
        minit_status = 1;
        break;
      }
    }
  }
  else
  {
    digitalWrite(mmot_dir, HIGH);
    for (int i = 0; i < 30000; i++)
    {
      stats = digitalRead(m_sen);
      if (stats == 1)
      {
        digitalWrite(mmot_stp, HIGH);
        delayMicroseconds(100);
        digitalWrite(mmot_stp, LOW);
        delayMicroseconds(100);
      }
      else
      {
        minit_status = 1;
        break;
      }
    }
  }
  stats = 1;
  digitalWrite(mmot_en, HIGH);
}

void manifold_home_count()
{
  int stats = digitalRead(m_sen);
  digitalWrite(mmot_en, LOW);
  digitalWrite(mmot_dir, HIGH);
  for (int i = 0; i < 30000; i++)
  {
    stats = digitalRead(m_sen);
    if (stats == 1)
    {
      digitalWrite(mmot_stp, HIGH);
      delayMicroseconds(100);
      digitalWrite(mmot_stp, LOW);
      delayMicroseconds(100);
    }
    else
    {
      wellbottom = i;

      break;
    }
  }
  digitalWrite(mmot_en, HIGH);
}
void move_plate_onepos()
{
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, HIGH);
  for (int i = 0; i < 480; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(200);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(200);
  }
  digitalWrite(pmot_en, HIGH);
}


void move_plate_onepos_cross()
{
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, HIGH);
  for (int i = 0; i < 350; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(200);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(200);
  }
  digitalWrite(pmot_en, HIGH);
}


void move_plate_pos(int val)
{
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, HIGH);
  for (int i = 0; i < val; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(200);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(200);
  }
  digitalWrite(pmot_en, HIGH);
}

void move_plate_pos_slow(int val)
{
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, HIGH);
  for (int i = 0; i < val; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(400);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(400);
  }
  digitalWrite(pmot_en, HIGH);
}

void move_plate_pos_rev_slow(int val)
{
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, LOW);
  for (int i = 0; i < val; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(400);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(400);
  }
  digitalWrite(pmot_en, HIGH);
}




void move_manifold_bottom()
{
  delay(100);
  digitalWrite(mmot_en, LOW);
  digitalWrite(mmot_dir, LOW);
  for (int k = 0; k < wellbottom + 500; k++)
  {
    digitalWrite(mmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(mmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(mmot_en, HIGH);
}

void move_manifold_bottom_prime()
{
  delay(100);
  digitalWrite(mmot_en, LOW);
  digitalWrite(mmot_dir, LOW);
  for (int k = 0; k < wellbottom - 1500; k++)
  {
    digitalWrite(mmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(mmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(mmot_en, HIGH);
}

void move_manifold_up(int pos)//pos=6000
{
  delay(100);
  digitalWrite(mmot_en, LOW);
  digitalWrite(mmot_dir, HIGH);
  for (int k = 0; k < pos; k++)
  {
    digitalWrite(mmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(mmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(mmot_en, HIGH);
}

void move_manifold_down(int pos)//pos=6000
{
  delay(100);
  digitalWrite(mmot_en, LOW);
  digitalWrite(mmot_dir, LOW);
  for (int k = 0; k < pos; k++)
  {
    digitalWrite(mmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(mmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(mmot_en, HIGH);
}

void init_position()
{
  delay(200);
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, HIGH);
  for (int j = 0; j < 600; j++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(pmot_en, HIGH);
  //  delay(200);
  //  digitalWrite(mmot_en, LOW);
  //  digitalWrite(mmot_dir, LOW);
  //  for (int k = 0; k < 1500; k++)
  //  {
  //    digitalWrite(mmot_stp, HIGH);
  //    delayMicroseconds(100);
  //    digitalWrite(mmot_stp, LOW);
  //    delayMicroseconds(100);
  //  }
  //  digitalWrite(mmot_en, HIGH);
  pinit_status = 0;
}

void init_position_finalasp()
{
  delay(200);
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, HIGH);
  for (int j = 0; j < 600; j++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(pmot_en, HIGH);
  //  delay(200);
  //  digitalWrite(mmot_en, LOW);
  //  digitalWrite(mmot_dir, LOW);
  //  for (int k = 0; k < 1500; k++)
  //  {
  //    digitalWrite(mmot_stp, HIGH);
  //    delayMicroseconds(100);
  //    digitalWrite(mmot_stp, LOW);
  //    delayMicroseconds(100);
  //  }
  //  digitalWrite(mmot_en, HIGH);
  pinit_status = 0;
}

void wash_plate(int movpos, int vol, int cross, int spd)
{
  Serial.println(vol);
  analogWrite(waste, 255);
  if (pinit_status == 1)
  {
    init_position();
  }
  for (int i = 0; i < movpos; i++)
  {
    move_plate_onepos();
  }
  move_manifold_bottom();
  delay(1000);
  move_manifold_up(a[vol]);
  digitalWrite(sol, HIGH);
  delay(100);
  analogWrite(wash, spd);
  delay(500 + (vol * 200));
  analogWrite(wash, 255);
  delay(100);
  digitalWrite(sol, LOW);
  delay(1000);
  if (vpump == 0)
    analogWrite(waste, 0);
  else
    analogWrite(waste, 255);

  //move_manifold_up(7500 - a[vol]);
  manifold_home();
}

void wash_plate_finalasp(int movpos, int vol, int cross)
{
  Serial.println(movpos);
  analogWrite(waste, 255);
  if (pinit_status == 1)
  {
    init_position_finalasp();
    //move_plate_pos(77);
  }
  for (int i = 0; i < movpos; i++)
  {
    move_plate_onepos();
  }
  move_manifold_bottom();
  delay(3000);
  if (cross == 1)
  {
    move_manifold_up(2000);
    delay(100);
    move_plate_pos_slow(100);
    delay(100);
    move_manifold_down(2000);
    delay(100);
    move_manifold_up(2000);
    delay(100);
    move_plate_pos_rev_slow(100);
    delay(2000);
  }
  manifold_home();
  analogWrite(waste, 0);
  delay(500);
}

void shake(int spd, int dur)
{
  manifold_home();
  uint32_t period = dur * 1000L;
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, HIGH);
  for (int i = 0; i < 500; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(100);
  }
  for ( uint32_t tStart = millis();  (millis() - tStart) < period;  )
  {
    digitalWrite(pmot_dir, LOW);
    for (int i = 0; i < 500; i++)
    {
      digitalWrite(pmot_stp, HIGH);
      delayMicroseconds(sp[spd - 1]);
      digitalWrite(pmot_stp, LOW);
      delayMicroseconds(sp[spd - 1]);
    }
    if (te.substring(0, 3) == "ini")
    {
      break;
    }
    digitalWrite(pmot_dir, HIGH);
    for (int i = 0; i < 500; i++)
    {
      digitalWrite(pmot_stp, HIGH);
      delayMicroseconds(sp[spd - 1]);
      digitalWrite(pmot_stp, LOW);
      delayMicroseconds(sp[spd - 1]);
    }
  }
  digitalWrite(pmot_en, HIGH);
  //move_manifold_bottom_prime();

}

void idle()
{
  uint32_t period = dur * 1000L;       // 5 minutes
  for ( uint32_t tStart = millis();  (millis() - tStart) < period;  )
  {

  }
}


void requestEvent()
{
  if (de.substring(0, 7) == "caldone")
  {
    String s1 = String(wellbottom);
    String all = de + " " + s1;
    char buf[30];
    all.toCharArray(buf, 30);
    Wire.write(buf, 30);
  }
  else
  {
    char buf[30];
    de.toCharArray(buf, 30);
    Wire.write(buf, 30);
  }
}

void receiveEvent(int numBytes)
{
  char rc[30] = "";
  int count = 0;
  while (Wire.available())
  {
    char c = Wire.read();
    rc[count] = c;
    count++;
    delay(10);
  }
  de = rc;
  te = de;
  Serial.println(de);
  if (te.substring(0, 3) == "mov" || te.substring(0, 3) == "mof")
  {
    char buf[30];
    te.toCharArray(buf, 30);
    char *p = buf;
    char *str;
    int i = 0;
    while ((str = strtok_r(p, " ", &p)) != NULL) // delimiter is the space
    {
      if (i == 1)
      {
        pos = atoi(str);
      }
      if (i == 2)
      {
        vol = atoi(str);
      }
      if (i == 3)
      {
        crs = atoi(str);
      }
      if (i == 4)
      {
        spd = atoi(str);
      }
      if (i == 5)
      {
        vpump = atoi(str);
      }
      i++;
    }
  }
  else if (te.substring(0, 3) == "shk" || te.substring(0, 3) == "rns" || te.substring(0, 3) == "prm")
  {
    char buf[30];
    te.toCharArray(buf, 30);
    char *p = buf;
    char *str;
    int i = 0;
    while ((str = strtok_r(p, " ", &p)) != NULL) // delimiter is the space
    {
      if (i == 1)
      {
        spd = atoi(str);
      }
      if (i == 2)
      {
        dur = atoi(str);
      }
      i++;
    }
  }
  else if (te.substring(0, 3) == "sok")
  {
    char buf[30];
    te.toCharArray(buf, 30);
    char *p = buf;
    char *str;
    int i = 0;
    while ((str = strtok_r(p, " ", &p)) != NULL) // delimiter is the space
    {
      if (i == 1)
      {
        dur = atoi(str);
      }
      i++;
    }
  }
  else if (te.substring(0, 3) == "stp")
  {
    de = "done";
    wdt_enable(WDTO_60MS);
  }
}
