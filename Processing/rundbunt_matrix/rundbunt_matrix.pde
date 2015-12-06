// -------------------------------------------------------------------------------
// Rundbunt Mini Emulator
// -------------------------------------------------------------------------------
// 005 - 2015-10-07 - corrected hue calculation with modulo
// -------------------------------------------------------------------------------


HsbColor[] hsvbuf1 = new HsbColor[64];

int nbrPara = 4;
int[] para = new int[nbrPara];
int heightSlider = 120/nbrPara;

HsbColor[][] leds = new HsbColor[8][8];

class HsbColor {
  float h, s, v;

  HsbColor(float ih, float is, float iv) {
     h = ih;
     s = is;
     v = iv;
  }
 
  void set_HSV(float ih, float is, float iv) {
     h = ih;
     s = is;
     v = iv;
  }
}

void setup() {
  size(500, 500);
  frameRate(10);
  background(32);
  
  colorMode(HSB, 1);
    
  //for (int i = 0; i < 64; i ++) {
  //  hsvbuf1[i] = new HsbColor(i * 4, 255, 255);
  //}
  for(int x=0; x<8; x++) {
     for(int y=0; y<8; y++) {
       leds[x][y] = new HsbColor(x * 4, 255, 255);
     }
  }

  for(int i=0; i<nbrPara; i++) {
    para[i] = 512;
  }
  for(int i=0; i<nbrPara; i++) {
    draw_para(i);
  }
}

int of = 0;
void drawPlasma01() {
  of++;
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      int col = int(128 + (256 * sin(x+y+of)));
      leds[x][y] = new HsbColor(col, col, col);
    }
  } 
  render_lamp();
}

void drawPlasma02() {
  of++;
  if(of>16) of=1;
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      int col = int(128.0 + (128.0 * sin(sqrt((x - 8 / 2.0) * (x - 8 / 2.0) + (y - 8 / 2.0) * (y - 8 / 2.0)) / of)));
      leds[x][y] = new HsbColor(col, 255, 255);
    }
  } 
  render_lamp();
}

float ofs = 1;
void drawPlasma03() {
  ofs += 0.3;
  //if(ofs>16) ofs=1;
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      int col = int
        (
              128.0 + (108.0 * sin(x+ofs / 8.0))
            + 128.0 + (108.0 * sin(y+ofs / 8.0))
        );

      leds[x][y] = new HsbColor(col, 255, 255);
    }
  }
  render_lamp();
}

float ofx = 1;
float ofy = 1;
void drawPlasma04() {
  ofx += 0.6;
  ofy += 0.3;
  //if(ofs>16) ofs=1;
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      int col = int
         (
               128.0 + (128.0 * sin(x+ofx / 16.0))
             + 128.0 + (128.0 * sin(y+ ofy / 8.0))
             + 128.0 + (128.0 * sin((x + y) / 16.0))
             + 128.0 + (128.0 * sin(sqrt(float(x * x + y * y)) / 8.0))
         ) / 3;

      leds[x][y] = new HsbColor(col, 255, 255);
    }
  } 
  render_lamp();
}

float time = 0;
void drawPlasma05() {
  time += 0.1;
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      float cx = sin(float(x)/6+time/5);
      float cy = cos(float(y)/5+time/3);
      float col = pow((sin( sqrt( cx*cx+cy*cy ) )),2);
      println(col+"  cx:"+cx+"  cy:"+cy);
      leds[x][y] = new HsbColor(col, 1, 0.8);
    }
  } 
  render_lamp();
}

float t = 0;
void drawPlasma06() {
  float a = map(para[0],0,1024,0,1);
  float b = map(para[1],0,1024,0,1);
  float c = map(para[2],0,1024,0,1);
  float d = map(para[2],0,1024,0,1);
  t += 0.1;
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      
      float col = sin(x*c+t);
            col += sin(a*(x*sin(t/2)+y*cos(t/3))+t);
       float cx = x - 3.5 + 5.0*sin(t/3.0);
       float cy = y - 3.5 + 5.0*cos(t/3.0);
            col += sin(sqrt(b*(cx*cx+cy*cy)+0.0)+t);
            col = (2.5+col)/5;
      //println(col+"  cx:"+cx+"  cy:"+cy);
      leds[x][y] = new HsbColor(col, 1, 0.8);
    }
  } 
  //println(a+"  "+b);
  render_lamp();
}

HsbColor green = new HsbColor(0.40,1,0.6);
int[][] l = new int[][] { {1,1},{3,1},{5,1},{7,1},  
                          {0,3},{2,3},{4,3},{6,3},
                          {1,5},{3,5},{5,5},{7,5} };
void drawChristmas() {
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      leds[x][y] = green;
     }
  } 
  for(int x=0; x<l.length; x++) {  
      leds[ l[x][0] ][ l[x][1] ] = new HsbColor(0.2, 1.0, random(70, 80)/100);
  } 
  render_lamp();
}

void draw() {
  
  drawPlasma06();
  //drawChristmas();
}

void render_lamp() {
  noStroke();
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      int x0 = 40  + 40 * x;
      int y0 = (8 * 40) - 40 * y;
      float h = leds[x][y].h % 256;
      fill(color(h, leds[x][y].s, leds[x][y].v));
      rect (x0, y0, 38, 38);
    }
  }
}

void mouseDragged() {
  int mx = mouseX;
  int my = height - mouseY;
  for(int i=0; i<nbrPara; i++) {  
    if(mouseY > height-heightSlider*(i+1) && mouseY < height-heightSlider*i) {
      para[i] = min(1023, max(0, round(map(mx, 0, width, 0, 1023))));
      draw_para(i);
    }
  }
}

void draw_para(int n) {
  int i = n+1;
  noStroke();
  fill(0);
  rect(0, height - heightSlider*i, width, heightSlider);
  fill(#3E484F);
  rect(0, height - heightSlider*i, map(para[n], 0, 1023, 0, width), heightSlider);
  fill(255);
}