#include <Wire.h>

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

int pinit_status = 0;
int minit_status = 0;

int a[7] = {860, 1720, 2580, 3440, 4300, 5160, 6020};
int sp[10] = {70, 75, 80, 85, 90, 95, 100, 105, 110, 115};

String de;
String te;
int pos = 0; //positions
int vol = 0; //volume
int crs = 0; //cross wash
int spd = 0; //shake speed
int dur = 0; //shake speed



void setup()
{
  pinMode(mmot_en, OUTPUT);
  pinMode(mmot_stp, OUTPUT);
  pinMode(mmot_dir, OUTPUT);
  pinMode(pmot_en, OUTPUT);
  pinMode(pmot_stp, OUTPUT);
  pinMode(pmot_dir, OUTPUT);
  digitalWrite(mmot_en, HIGH);
  digitalWrite(pmot_en, HIGH);
  pinMode(p_sen, INPUT);
  pinMode(m_sen, INPUT);
  Wire.begin(0x07); //Set Arduino up as an I2C slave at address 0x07
  Wire.onRequest(requestEvent); //Prepare to send data
  Wire.onReceive(receiveEvent); //Prepare to recieve data
  analogWrite(rins, 255);
  analogWrite(wash, 255);
  analogWrite(waste, 255);


  delay(1000);
  manifold_home();
  plate_home();
}

void loop()
{
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
    rinse();
    delay(100);
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "prm")
  {
    prime();
    plate_home();
    delay(100);
    de = "done";
    te = "";
  }
  else if (te.substring(0, 3) == "mov")
  {
    wash_plate(pos, vol, crs);
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
    de = "done";
    manifold_home();
    plate_home();
    shake(spd, dur);
    manifold_home();
    plate_home();
    delay(100);
    de = "done";
    te = "";
  }
}

void rinse()
{
  analogWrite(waste, 0);
  delay(4000);
  move_manifold_bottom_prime();
  analogWrite(rins, 200);
  delay(5000);
  analogWrite(rins, 255);
  delay(2000);
  analogWrite(waste, 255);
  move_manifold_up(7500);
}

void prime()
{
  analogWrite(waste, 0);
  delay(4000);
  move_manifold_bottom_prime();
  analogWrite(wash, 220);
  delay(5000);
  analogWrite(wash, 255);
  delay(2000);
  analogWrite(waste, 255);
  move_manifold_up(7500);
}

void plate_home()
{
  digitalWrite(pmot_en, LOW);
  int stat = digitalRead(p_sen);
  if (stat == 1)
  {
    digitalWrite(pmot_dir, LOW);
    for (int i = 0; i < 2000; i++)
    {
      digitalWrite(pmot_stp, HIGH);
      delayMicroseconds(100);
      digitalWrite(pmot_stp, LOW);
      delayMicroseconds(100);
    }
    digitalWrite(pmot_dir, HIGH);
    for (int i = 0; i < 4000; i++)
    {

      stat = digitalRead(p_sen);
      if (stat == 0)
      {
        digitalWrite(pmot_stp, HIGH);
        delayMicroseconds(100);
        digitalWrite(pmot_stp, LOW);
        delayMicroseconds(100);
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
    digitalWrite(pmot_dir, HIGH);
    for (int i = 0; i < 30000; i++)
    {
      stat = digitalRead(p_sen);
      if (stat == 0)
      {
        digitalWrite(pmot_stp, HIGH);
        delayMicroseconds(100);
        digitalWrite(pmot_stp, LOW);
        delayMicroseconds(100);
      }
      else
      {
        pinit_status = 1;
        break;
      }
    }
  }
  stat = 0;
  digitalWrite(pmot_en, HIGH);
}


void manifold_home()
{
  digitalWrite(mmot_en, LOW);
  int stats = digitalRead(m_sen);
  if (stats == 1)
  {
    digitalWrite(mmot_dir, HIGH);
    for (int i = 0; i < 2000; i++)
    {
      digitalWrite(mmot_stp, HIGH);
      delayMicroseconds(100);
      digitalWrite(mmot_stp, LOW);
      delayMicroseconds(100);
    }
    digitalWrite(mmot_dir, LOW);
    for (int i = 0; i < 4000; i++)
    {

      stats = digitalRead(m_sen);
      if (stats == 0)
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
    digitalWrite(mmot_dir, LOW);
    for (int i = 0; i < 30000; i++)
    {
      stats = digitalRead(m_sen);
      if (stats == 0)
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
  stats = 0;
  digitalWrite(mmot_en, HIGH);
}


void move_plate_onepos()
{
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, LOW);
  for (int i = 0; i < 600; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(pmot_en, HIGH);
}


void move_plate_onepos_cross()
{
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, LOW);
  for (int i = 0; i < 500; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(pmot_en, HIGH);
}


void move_plate_pos(int val)
{
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, LOW);
  for (int i = 0; i < val; i++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(pmot_en, HIGH);
}



void move_manifold_bottom()
{
  delay(100);
  digitalWrite(mmot_en, LOW);
  digitalWrite(mmot_dir, HIGH);
  for (int k = 0; k < 6500; k++)
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
  digitalWrite(mmot_dir, HIGH);
  for (int k = 0; k < 7500; k++)
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

void move_manifold_down(int pos)//pos=6000
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

void init_position()
{
  delay(200);
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, LOW);
  for (int j = 0; j < 800; j++)
  {
    digitalWrite(pmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(pmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(pmot_en, HIGH);
  delay(200);
  digitalWrite(mmot_en, LOW);
  digitalWrite(mmot_dir, HIGH);
  for (int k = 0; k < 4000; k++)
  {
    digitalWrite(mmot_stp, HIGH);
    delayMicroseconds(100);
    digitalWrite(mmot_stp, LOW);
    delayMicroseconds(100);
  }
  digitalWrite(mmot_en, HIGH);
  pinit_status = 0;
}

void wash_plate(int movpos, int vol, int cross)
{
  analogWrite(waste, 0);
  if (pinit_status == 1)
  {
    init_position();
  }
  for (int i = 0; i < movpos; i++)
  {
    move_plate_onepos();
  }
  move_manifold_bottom();
  delay(3000);
  move_manifold_up(a[vol]);
  analogWrite(wash, 160);
  delay(1000);
  analogWrite(wash, 255);
  delay(2000);
  move_manifold_up(6500 - a[vol]);
  analogWrite(waste, 255);
}

void wash_plate_finalasp(int movpos, int vol, int cross)
{
  analogWrite(waste, 0);
  if (pinit_status == 1)
  {
    init_position();
    move_plate_pos(100);
  }
  for (int i = 0; i < movpos; i++)
  {
    move_plate_onepos_cross();
  }
  move_manifold_bottom();
  delay(3000);
  if (cross == 1)
  {
    move_manifold_up(2000);
    delay(500);
    move_plate_pos(100);
    move_manifold_down(2000);
    delay(2000);
  }
  move_manifold_up(6500);
  analogWrite(waste, 255);
  delay(500);
}

void shake(int spd, int dur)
{
  uint32_t period = dur * 1000L;
  digitalWrite(pmot_en, LOW);
  digitalWrite(pmot_dir, LOW);
  for (int i = 0; i < 1000; i++)
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
      delayMicroseconds(sp[spd]);
      digitalWrite(pmot_stp, LOW);
      delayMicroseconds(sp[spd]);
    }
    if (te.substring(0, 3) == "ini")
    {
      break;
    }
    digitalWrite(pmot_dir, HIGH);
    for (int i = 0; i < 500; i++)
    {
      digitalWrite(pmot_stp, HIGH);
      delayMicroseconds(sp[spd]);
      digitalWrite(pmot_stp, LOW);
      delayMicroseconds(sp[spd]);
    }
  }
  digitalWrite(pmot_en, HIGH);
}

//void(* resetFunc) (void) = 0;

void requestEvent()
{
  char buf[30];
  de.toCharArray(buf, 30);
  Wire.write(buf, 30); //Write String to Pi.
  delay(100);
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
  if (te.substring(0, 3) == "mov" || te.substring(0, 3) == "mvf")
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
      i++;
    }
  }
  else if (te.substring(0, 3) == "shk")
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
  //  else if (te.substring(0, 3) == "ini")
  //  {
  //    resetFunc();
  //  }
}
