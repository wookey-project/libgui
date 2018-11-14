#include "autoconf.h"
#include "libtft.h"
#include "libtouch.h"
#include "api/types.h"
#include "api/syscall.h"
#include "api/print.h"
#include "api/random.h"
#include "api/gui_pin.h"

#define HIGHLIGHT_COLOR WHITE
#define WHITE 255,255,255
#define BLACK 0,0,0
#define WOOKEY_BLUE 29,201,255
#define WOOKEY_RED 226,39,92 
#define WOOKEY_GREEN 9,250,57
#define WOOKEY_ORANGE 255,213,19
#define GRAY 226,226,226


extern const int font_width;
extern const int font_height;
extern const int font_blankskip;

static char *keys[12];

typedef enum {
    DRAW_MODE_PIN,
    DRAW_MODE_PETPIN
} t_draw_mode;



#define TXT_KEY_BASE   0x21
#define TXT_KEY_NUM    90
#define TXT_KEY_PAGES  9

static void dopermu(char **mykeys, const uint32_t mysize)
{
  uint32_t i;
  for(i=0;i<mysize-1;i++)
  {
    uint32_t rand;
    char *stock;
    get_random((unsigned char*)&rand,sizeof(uint32_t));
    //rand%=i; this will not work on cortex m4 gcc --nobuiltins
    for(rand&=0xff; rand>=(mysize-i);rand/=(mysize-i)); //Truncate then modulo
    stock=mykeys[rand+i];
    mykeys[rand+i]=mykeys[i];
    mykeys[i]=stock;
  }
}



static void draw_txt_pad(int x1,int x2, int y1, int y2, uint8_t offset)
{
  const int hspace=5, vspace=10, char_width=font_width/128;
  int hsize = (x2-x1-3*hspace)/3;
  int vsize = (y2-y1-4*vspace)/5;
  int i,j,k;

  //Draw rectangles
  for(i=1;i<4;i++)
  {
    for(j=0;j<3;j++)
    {
	tft_fill_rectangle(x1+hspace*j+hsize*j,x1+hsize*j+hspace*j+hsize,
		y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
		WHITE);
	tft_fill_rectangle(x1+hspace*j+hsize*j+2,x1+hsize*j+hspace*j+hsize-2,
		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
		WOOKEY_BLUE);
    }
  }
#if 1
	tft_fill_rectangle(x1,x1+hsize, y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
		WHITE);
	tft_fill_rectangle(x1+2,x1+hsize-2,
		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
		WOOKEY_RED);

	tft_fill_rectangle(x1+hspace+hsize,x1+hsize+hspace+hsize,
		y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
		WHITE);
	tft_fill_rectangle(x1+hspace+hsize+2,x1+hsize+hspace+hsize-2,
		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
		WOOKEY_BLUE);
	
	tft_fill_rectangle(x1+hspace*2+hsize*2,x1+hsize*2+hspace*2+hsize,
		y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
		WHITE);
	tft_fill_rectangle(x1+hspace*2+hsize*2+2,x1+hsize*2+hspace*2+hsize-2,
		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
		WOOKEY_GREEN);
//	tft_fill_rectangle(x1+hspace*2+hsize*2+2,x1+hsize*2+hspace*2+hsize-2,
//		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
//		WOOKEY_GREEN);
#endif
  //Draw strings
  tft_setfg(0,0,0);
  tft_setbg(WOOKEY_BLUE);
  //Bleu 29 201 255
  //Vert valid 9 250 57
  //Rouge correct 226 39 92
  //Gris 226 226 226
  k=0;
  for(i=1;i<4;i++)
  {
    for(j=0;j<3;j++)
    {
	tft_set_cursor_pos(x1+hspace*j+hsize*j+hsize/2-strlen(keys[k])*char_width/2,
		y1+i*vspace+(i)*vsize+vsize/2-font_height/4+font_blankskip/4);
    	//tft_puts(keys[k++]);
    	tft_putc(TXT_KEY_BASE + (offset * 10) + k++);	
    }
  }
#if 1 
  tft_setfg(0,0,0);
  tft_setbg(WOOKEY_RED);
  tft_set_cursor_pos(x1+hsize/2-char_width-char_width/2,
      y1+i*vspace+(i)*vsize+vsize/2-font_height/4+font_blankskip/4);
  tft_puts("<<");k++;
  //Dernier chiffre
  tft_setfg(0,0,0);
  tft_setbg(WOOKEY_BLUE);
  tft_set_cursor_pos(x1+hspace+hsize+hsize/2-char_width/2,
      y1+i*vspace+(i)*vsize+vsize/2-font_height/4+font_blankskip/4);
  tft_putc(TXT_KEY_BASE + (offset * 10) + k++);	
  //tft_puts(keys[k++]);	
  tft_setfg(0,0,0);
  tft_setbg(WOOKEY_GREEN);
  tft_set_cursor_pos(x1+hspace*2+hsize*2+hsize/2-char_width,
      y1+i*vspace+(i)*vsize+vsize/2-font_height/4+font_blankskip/4);
  tft_puts(">>");	
  i=4;
#endif
  // pin bar
  tft_fill_rectangle(x1,x2-hspace-60,
                y1, y1+vsize,
                WHITE);
  tft_fill_rectangle(x1+2,x2-hspace-62,
                y1+2, y1+vsize-2,
                226,226,226);
  // ok button
  tft_fill_rectangle(x2-hspace-58,x2-hspace,
                y1, y1+vsize,
                WHITE);
  tft_fill_rectangle(x2-hspace-56,x2-hspace-2,
                y1+2, y1+vsize-2,
                25,25,255);
  tft_setfg(0,0,0);
  tft_setbg(25,25,255);
  tft_set_cursor_pos(x2-hspace-58+29-char_width-char_width/2,
          y1+28-font_height/4+font_blankskip/4);
  tft_puts("Ok");	
}



static void draw_pin(int x1,int x2, int y1, int y2)
{
  const int hspace=5, vspace=10, char_width=font_width/128;
  int hsize = (x2-x1-3*hspace)/3;
  int vsize = (y2-y1-4*vspace)/5;
  int i,j,k;
  //Draw rectangles
  for(i=1;i<4;i++)
  {
    for(j=0;j<3;j++)
    {
	tft_fill_rectangle(x1+hspace*j+hsize*j,x1+hsize*j+hspace*j+hsize,
		y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
		WHITE);
	tft_fill_rectangle(x1+hspace*j+hsize*j+2,x1+hsize*j+hspace*j+hsize-2,
		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
		WOOKEY_BLUE);
    }
  }
#if 1
	tft_fill_rectangle(x1,x1+hsize, y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
		WHITE);
	tft_fill_rectangle(x1+2,x1+hsize-2,
		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
		WOOKEY_RED);

	tft_fill_rectangle(x1+hspace+hsize,x1+hsize+hspace+hsize,
		y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
		WHITE);
	tft_fill_rectangle(x1+hspace+hsize+2,x1+hsize+hspace+hsize-2,
		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
		WOOKEY_BLUE);
	
	tft_fill_rectangle(x1+hspace*2+hsize*2,x1+hsize*2+hspace*2+hsize,
		y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
		WHITE);
	tft_fill_rectangle(x1+hspace*2+hsize*2+2,x1+hsize*2+hspace*2+hsize-2,
		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
		WOOKEY_GREEN);
//	tft_fill_rectangle(x1+hspace*2+hsize*2+2,x1+hsize*2+hspace*2+hsize-2,
//		y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
//		WOOKEY_GREEN);
#endif
  //Draw strings
  tft_setfg(0,0,0);
  tft_setbg(WOOKEY_BLUE);
  //Bleu 29 201 255
  //Vert valid 9 250 57
  //Rouge correct 226 39 92
  //Gris 226 226 226
  k=0;
  for(i=1;i<4;i++)
  {
    for(j=0;j<3;j++)
    {
	tft_set_cursor_pos(x1+hspace*j+hsize*j+hsize/2-strlen(keys[k])*char_width/2,
		y1+i*vspace+(i)*vsize+vsize/2-font_height/4+font_blankskip/4);
    	tft_puts(keys[k++]);	
    }
  }
#if 1 
  tft_setfg(0,0,0);
  tft_setbg(WOOKEY_RED);
  tft_set_cursor_pos(x1+hsize/2-char_width-char_width/2,
      y1+i*vspace+(i)*vsize+vsize/2-font_height/4+font_blankskip/4);
  tft_puts("Cor");k++;
  //Dernier chiffre
  tft_setfg(0,0,0);
  tft_setbg(WOOKEY_BLUE);
  tft_set_cursor_pos(x1+hspace+hsize+hsize/2-char_width/2,
      y1+i*vspace+(i)*vsize+vsize/2-font_height/4+font_blankskip/4);
  tft_puts(keys[k++]);	
  tft_setfg(0,0,0);
  tft_setbg(WOOKEY_GREEN);
  tft_set_cursor_pos(x1+hspace*2+hsize*2+hsize/2-char_width,
      y1+i*vspace+(i)*vsize+vsize/2-font_height/4+font_blankskip/4);
  tft_puts("Ok");	
  i=4;
#endif
#if 0
  //Draw last line
  tft_fill_rectangle(x1,x2-hspace,
                y1+i*vspace+i*vsize, y1+i*vspace+i*vsize+vsize,
                WHITE);
  tft_fill_rectangle(x1+2,x2-hspace-2,
                y1+i*vspace+i*vsize+2, y1+i*vspace+i*vsize+vsize-2,
                0,0,0);
#else
  tft_fill_rectangle(x1,x2-hspace,
                y1, y1+vsize,
                WHITE);
  tft_fill_rectangle(x1+2,x2-hspace-2,
                y1+2, y1+vsize-2,
                226,226,226);

#endif
}


static void pin_draw_case(int x1,int x2, int y1, int y2, const char *c, 
			uint8_t r, uint8_t g, uint8_t b);

uint8_t pin_request_string_validation(const char *msg, const char *string, uint8_t string_len)
{
  string_len = string_len;
  tft_setfg(200,200,200);
  tft_setbg(5,0,5);
  tft_set_cursor_pos(0,29);

  tft_fill_rectangle(0,240,0,320,0,0,0);
  tft_setfg(200,200,200);
  tft_setbg(5,0,5);
  tft_set_cursor_pos(0,0);
  tft_puts("Please validate");
  tft_set_cursor_pos(0,29);
  tft_puts((char*)msg);

  pin_draw_case(10, 230, 70, 230, string, 245, 245, 245);

  tft_fill_rectangle(8, 82, 248, 302, 240,240,240);
  pin_draw_case(10, 80, 250, 300, "OK", WOOKEY_GREEN);

  tft_fill_rectangle(158, 232, 248, 302, 240,240,240);
  pin_draw_case(160, 230, 250, 300, "KO", WOOKEY_RED);

  while (1) {
    touch_read_X_DFR();//Ensures that PenIRQ is enabled
    /*
     * Between touch_read_X_DFR and touch_is_touched, we need to wait a little
     * or touch_is_touched() will return an invalid value
     */
    sys_sleep(10, SLEEP_MODE_INTERRUPTIBLE);

    while(!(touch_is_touched())) {
        touch_enable_exti();
        sys_yield();
    }
    //Refresh the actual positions
    touch_refresh_pos();
    //Follow the motion on the screen
    while(touch_refresh_pos(),touch_is_touched())
    {
      int posx,posy;
      //touch_refresh_pos();
      posy=touch_getx();
      posx=touch_gety();

      if (posx > 10 && posx < 80 && posy > 250 && posy < 300) {

          /* OK pushed */
          return 0;
      }

      if (posx > 160 && posx < 230 && posy > 250 && posy < 300) {
          /* Invalid pushed */
          return 1;
      }
    }
  }
  return 0;
}


static void pin_redraw_text_footer(const char nb_given, const char *str, t_draw_mode mode, 
			int x1, int x2, int y1, int y2)
{
  int posx,posy,i;
  const int char_width=font_width/128;
  tft_setbg(226,226,226);
  tft_setfg(0,0,0);
  //tft_fill_rectangle(x1,x2,y1, y2, 10+10*j,10+10*j,10+10*j);
  tft_fill_rectangle(x1,x2,y1, y2, 226,226,226);
  if (mode == DRAW_MODE_PIN) {
      posx=(x2-x1-nb_given*char_width)/2;
  } else {
      posx=(x1+ 15); // left align
  }
  posy=(y2-y1-font_height/2)/2;
  tft_set_cursor_pos(posx,posy+y1);
  if (nb_given > 10) {
      i = nb_given - 10 + 1;
      tft_putc('#');
  } else {
      i = 0;
  }

  for(;i<nb_given;i++) {
      if (mode == DRAW_MODE_PIN) {
          tft_putc('*');
      } else {
          if (nb_given > 1 && i < nb_given - 1) {
              tft_putc('*');
          } else {
              tft_putc(str[i]);
          }
      }
  }
}

static void pin_draw_case(int x1,int x2, int y1, int y2, const char *c, 
			uint8_t r, uint8_t g, uint8_t b)
{
  const int char_width=font_width/128;
  int posx,posy;
  tft_setbg(r,g,b);
  uint8_t chars_per_line;
  uint8_t numlines;
  //tft_setfg(0,0,0);
  tft_fill_rectangle(x1,x2,y1,y2,r,g,b);
  // 1) calculate the number of char per line in the case
  chars_per_line=((x2 - 5 - x1 + 5)/char_width);
  // 2) calculate the number of lines
  numlines=(strlen(c)/chars_per_line) + 1;
  // 3) calculate the position of the first line depending on
  // the number of lines
  //  ... the middle line...
  posy=(y2-y1-font_height/2)/2;
  for (uint8_t i = 1; i < numlines; ++i) {
      // go up of 1 line
      posy -= font_height/2;
  }
  uint8_t missing_chars = strlen(c);
  uint8_t i = 0;
  char line[chars_per_line + 1];
  memset(line, 0x0, chars_per_line + 1);
  do {
      if (missing_chars > chars_per_line) {
          memcpy(line, &(c[i]), chars_per_line);
          posx=(x2-x1-chars_per_line*char_width)/2;
          tft_set_cursor_pos(x1+posx,posy+y1);
          tft_puts((char*)line);
          // print only a part of the string
          missing_chars -= chars_per_line;
          i += chars_per_line;
      } else {
          posx=(x2-x1-strlen(&(c[i]))*char_width)/2;
          tft_set_cursor_pos(x1+posx,posy+y1);
          tft_puts((char*)&(c[i]));
          missing_chars = 0;
      }
      posy += font_height/2;
  } while (missing_chars > 0);
//  posx=(x2-x1-strlen(c)*char_width)/2;
}

static void pin_normal_case(int x1,int x2, int y1, int y2, char *c)
{
   pin_draw_case(x1,x2,y1,y2,c,WOOKEY_BLUE);
}
static void pin_highlight_case(int x1,int x2, int y1, int y2, char *c)
{
   pin_draw_case(x1,x2,y1,y2,c,HIGHLIGHT_COLOR);
}


void pin_request_string(const char *title,
                        uint32_t    title_len,
                        int x1,int x2, int y1, int y2,
                        char *string, uint8_t maxlen)
{
  const int hspace=5, vspace=10;
  uint8_t nb_given=0;
  int hsize = (x2-x1-3*hspace)/3;
  int vsize = (y2-y1-4*vspace)/5;
  uint8_t offset = 0;
  title_len = title_len;

  tft_setfg(200,200,200);
  tft_setbg(5,0,5);
  tft_set_cursor_pos(0,29);

  tft_fill_rectangle(0,240,0,320,0,0,0);
  tft_setfg(200,200,200);
  tft_setbg(5,0,5);
  tft_set_cursor_pos(0,0);
  tft_puts("  Please enter ");
  tft_set_cursor_pos(0,29);
  tft_puts((char*)title);
  draw_txt_pad(x1,x2,y1,y2, 0);

//Main interaction loop
  while(1) // wait for event
  {
      int mycase=0;
      int lastcase=-1;
    int lastx=0,lasty=0;
    char key[2] = { 0 };
    //Wait for touchscreen to be touched
    touch_read_X_DFR();//Ensures that PenIRQ is enabled
    /*
     * Between touch_read_X_DFR and touch_is_touched, we need to wait a little
     * or touch_is_touched() will return an invalid value
     */
    sys_sleep(10, SLEEP_MODE_INTERRUPTIBLE);

    while(!(touch_is_touched())) {
        touch_enable_exti();
        sys_yield();
    }
    //Refresh the actual positions
    touch_refresh_pos();
    //Follow the motion on the screen
    while(touch_refresh_pos(),touch_is_touched())
    {
      int colx,coly;
      int posx,posy;
      //touch_refresh_pos();
      posy=touch_getx();
      posx=touch_gety();
#if PIN_DEBUG
      printf("posx %d posy %d\n",posx,posy);
#endif
      //Locate the column
      if(posx>=x1 && posx<=(x1+hsize))
	colx=0;
      else if(posx>=(x1+hspace+hsize) && posx<=(x1+hspace+2*hsize))
	colx=1;
      else if(posx>=(x1+2*hspace+2*hsize) && posx<=(x1+2*hspace+3*hsize))
	colx=2;
      else
	colx=-1;
      //Locate the line
      if(posy>=y1 && posy<=(y1+vsize))
	coly=-1;
      else if(posy>=(y1+vspace+vsize) && posy<=(y1+vspace+2*vsize))
	coly=1;
      else if(posy>=(y1+2*vspace+2*vsize) && posy<=(y1+2*vspace+3*vsize))
	coly=2;
      else if(posy>=(y1+3*vspace+3*vsize) && posy<=(y1+3*vspace+4*vsize))
	coly=3;
      else if(posy>=(y1+4*vspace+4*vsize) && posy<=(y1+4*vspace+5*vsize))
	coly=4;
      else
	coly=-1;

      //Recolor the lastcase rectangle if it needs to
      mycase=3*(coly-1)+colx;

      key[0] = TXT_KEY_BASE + (offset * 10) + (lasty - 1) * 3 + lastx;	
      if(lastcase>=0 && mycase!=lastcase)
      {
        if ((lasty==4) && (lastx==0))
        {
          pin_draw_case(x1+lastx*hspace+lastx*hsize+2,
              x1+lastx*hspace+lastx*hsize+hsize-2,
              y1+lasty*vspace+lasty*vsize+2,
              y1+lasty*vspace+lasty*vsize+vsize-2, key,WOOKEY_ORANGE);
        } 
        else if ((lasty==4) && (lastx==2))
        {
          pin_draw_case(x1+lastx*hspace+lastx*hsize+2,
              x1+lastx*hspace+lastx*hsize+hsize-2,
              y1+lasty*vspace+lasty*vsize+2,
              y1+lasty*vspace+lasty*vsize+vsize-2, key,WOOKEY_ORANGE);
        }
        else
        {
          pin_normal_case(x1+lastx*hspace+lastx*hsize+2,
              x1+lastx*hspace+lastx*hsize+hsize-2,
              y1+lasty*vspace+lasty*vsize+2,
              y1+lasty*vspace+lasty*vsize+vsize-2,key);
        }
      }
      if (posx > x2 - 60 && posx < x2 &&
              posy > y1 && posy < y1 + 58) { // is okay button ?
          return;
      }
      //Is touch currently out of range
      if( colx==-1 || coly==-1)
      {	
        lastcase=-1;
        continue;
      }
      //Then invert the new location
      pin_highlight_case(x1+colx*hspace+colx*hsize+2,
          x1+colx*hspace+colx*hsize+hsize-2,
          y1+coly*vspace+coly*vsize+2,
          y1+coly*vspace+coly*vsize+vsize-2,key);
#if PIN_DEBUG
      printf("changement de case lastcase %d mycase %d\n",lastcase,mycase);
#endif
      lastcase=mycase;
      lastx=colx;
      lasty=coly;
    }
    //Validation at th last position
    //Were we out of the scope ? Then do nothing
    if(lastcase<0) {
        continue;
    }
    //Otherwise redraw last case as normal
    pin_normal_case(x1+lastx*hspace+lastx*hsize+2,
	x1+lastx*hspace+lastx*hsize+hsize-2,
	y1+lasty*vspace+lasty*vsize+2,
	y1+lasty*vspace+lasty*vsize+vsize-2, key);
    //Check for 'Cor' or 'Ok'
#if PIN_DEBUG
    printf("lastx %d lasty %d\n",lastx,lasty);
#endif
    if ((lasty==4) && (lastx==0))
    {
        if (offset == 0) {
            offset = 8;
        } else {
          offset--;
        }
	 //tft_setbg(WOOKEY_RED);  
	 pin_draw_case(x1+lastx*hspace+lastx*hsize+2,
	x1+lastx*hspace+lastx*hsize+hsize-2,
	y1+lasty*vspace+lasty*vsize+2,
	y1+lasty*vspace+lasty*vsize+vsize-2, "<<",WOOKEY_ORANGE);
      //Last touch was for correcting
      draw_txt_pad(x1,x2,y1,y2, offset);
    }
    else if ((lasty==4) && (lastx==2))
    {
        if (offset == 8) {
            offset = 0;
        } else {
            offset++;
        }
        //tft_setbg(WOOKEY_GREEN);  
        pin_draw_case(x1+lastx*hspace+lastx*hsize+2,
                x1+lastx*hspace+lastx*hsize+hsize-2,
                y1+lasty*vspace+lasty*vsize+2,
                y1+lasty*vspace+lasty*vsize+vsize-2, ">>",WOOKEY_ORANGE);
        //Last touch was Ok 
#if PIN_DEBUG
        printf("nb_given %d nb_pin %d\n",nb_given,maxlen);
#endif
      draw_txt_pad(x1,x2,y1,y2, offset);
    } else {
        if (nb_given < maxlen) {
          string[nb_given++]=key[0];
        }
    }
    //Redraw text footer
    pin_redraw_text_footer(nb_given, string, DRAW_MODE_PETPIN, x1+2, x2-hspace-62,
	    y1+2, y1+vsize-2);  
  }
}


uint8_t pin_request_digits(const char *title,
             uint32_t    title_len __attribute__((unused)),
             int x1,int x2, int y1, int y2,
             char *mypin, uint8_t max_pin_len)
{
  const int hspace=5, vspace=10;
  char *tmp;
  char *tmpkeys[12]={"1","2","3","4","5","6","7","8","9","0","Cor","Ok"};
  uint8_t nb_given=0;
  int hsize = (x2-x1-3*hspace)/3;
  int vsize = (y2-y1-4*vspace)/5;
  int i;
  for (i=0;i<12;i++)
    keys[i]=tmpkeys[i];
  dopermu(keys,10);
  tmp=keys[9];
  keys[9]=keys[10];
  keys[10]=tmp;

  tft_setfg(200,200,200);
  tft_setbg(5,0,5);
  tft_set_cursor_pos(0,29);

  tft_fill_rectangle(0,240,0,320,0,0,0);
  tft_setfg(200,200,200);
  tft_setbg(5,0,5);
  tft_set_cursor_pos(0,0);
  tft_puts("  Please enter ");
  tft_set_cursor_pos(0,29);
  tft_puts((char*)title);

  draw_pin(x1,x2,y1,y2);
//Main interaction loop
  while(1)
  {
    int mycase=0;
    int lastcase=-1;
    int lastx=0,lasty=0;
    //Wait for touchscreen to be touched
    touch_read_X_DFR();//Ensures that PenIRQ is enabled
    while(!(touch_is_touched())) {
        touch_enable_exti();
        sys_yield();
    }
    //Refresh the actual positions
    touch_refresh_pos();
    //Follow the motion on the screen
    while(touch_refresh_pos(),touch_is_touched())
    {
      int colx,coly;
      int posx,posy;
      //touch_refresh_pos();
      posy=touch_getx();
      posx=touch_gety();
#if PIN_DEBUG
      printf("posx %d posy %d\n",posx,posy);
#endif
      //Locate the column
      if(posx>=x1 && posx<=(x1+hsize))
	colx=0;
      else if(posx>=(x1+hspace+hsize) && posx<=(x1+hspace+2*hsize))
	colx=1;
      else if(posx>=(x1+2*hspace+2*hsize) && posx<=(x1+2*hspace+3*hsize))
	colx=2;
      else
	colx=-1;
      //Locate the line
      if(posy>=y1 && posy<=(y1+vsize))
	coly=-1;
      else if(posy>=(y1+vspace+vsize) && posy<=(y1+vspace+2*vsize))
	coly=1;
      else if(posy>=(y1+2*vspace+2*vsize) && posy<=(y1+2*vspace+3*vsize))
	coly=2;
      else if(posy>=(y1+3*vspace+3*vsize) && posy<=(y1+3*vspace+4*vsize))
	coly=3;
      else if(posy>=(y1+4*vspace+4*vsize) && posy<=(y1+4*vspace+5*vsize))
	coly=4;
      else
	coly=-1;

      //Recolor the lastcase rectangle if it needs to
      mycase=3*(coly-1)+colx;
      if(lastcase>=0 && mycase!=lastcase)
      {
        if ((lasty==4) && (lastx==0))
        {
          pin_draw_case(x1+lastx*hspace+lastx*hsize+2,
              x1+lastx*hspace+lastx*hsize+hsize-2,
              y1+lasty*vspace+lasty*vsize+2,
              y1+lasty*vspace+lasty*vsize+vsize-2, keys[lastcase],WOOKEY_RED);
        } 
        else if ((lasty==4) && (lastx==2))
        {
          pin_draw_case(x1+lastx*hspace+lastx*hsize+2,
              x1+lastx*hspace+lastx*hsize+hsize-2,
              y1+lasty*vspace+lasty*vsize+2,
              y1+lasty*vspace+lasty*vsize+vsize-2, keys[lastcase],WOOKEY_GREEN);
        }
        else
        {
          pin_normal_case(x1+lastx*hspace+lastx*hsize+2,
              x1+lastx*hspace+lastx*hsize+hsize-2,
              y1+lasty*vspace+lasty*vsize+2,
              y1+lasty*vspace+lasty*vsize+vsize-2,keys[lastcase]);
        }
      }
      //Is touch currently out of range
      if( colx==-1 || coly==-1)
      {	
        lastcase=-1;
        continue;
      }
      //Then invert the new location
      pin_highlight_case(x1+colx*hspace+colx*hsize+2,
          x1+colx*hspace+colx*hsize+hsize-2,
          y1+coly*vspace+coly*vsize+2,
          y1+coly*vspace+coly*vsize+vsize-2,keys[mycase]);
#if PIN_DEBUG
      printf("changement de case lastcase %d mycase %d\n",lastcase,mycase);
#endif
      lastcase=mycase;
      lastx=colx;
      lasty=coly;
    }
    //Validation at th last position
    //Were we out of the scope ? Then do nothing
    if(lastcase<0) 
      continue;
    //Otherwise redraw last case as normal
    pin_normal_case(x1+lastx*hspace+lastx*hsize+2,
	x1+lastx*hspace+lastx*hsize+hsize-2,
	y1+lasty*vspace+lasty*vsize+2,
	y1+lasty*vspace+lasty*vsize+vsize-2, keys[lastcase]);
    //Check for 'Cor' or 'Ok'
#if PIN_DEBUG
    printf("lastx %d lasty %d\n",lastx,lasty);
#endif
    if ((lasty==4) && (lastx==0))
    {
	 //tft_setbg(WOOKEY_RED);  
	 pin_draw_case(x1+lastx*hspace+lastx*hsize+2,
	x1+lastx*hspace+lastx*hsize+hsize-2,
	y1+lasty*vspace+lasty*vsize+2,
	y1+lasty*vspace+lasty*vsize+vsize-2, keys[lastcase],WOOKEY_RED);
      //Last touch was for correcting
      if(nb_given>0)
      {
	mypin[nb_given]=0;
	nb_given--;//just to catch the ++ of pin_redraw_text_footer
#if PIN_DEBUG
      printf("nb_given %d max_pin_len %d\n",nb_given,nb_pin);
#endif
      }
    }
    else if ((lasty==4) && (lastx==2))
    {
        //tft_setbg(WOOKEY_GREEN);  
        pin_draw_case(x1+lastx*hspace+lastx*hsize+2,
                x1+lastx*hspace+lastx*hsize+hsize-2,
                y1+lasty*vspace+lasty*vsize+2,
                y1+lasty*vspace+lasty*vsize+vsize-2, keys[lastcase],WOOKEY_GREEN);
        //Last touch was Ok 
#if PIN_DEBUG
        printf("nb_given %d nb_pin %d\n",nb_given,max_pin_len);
#endif
        //if(nb_given == nb_pin), always return when ok is pushed
            return nb_given;	
    }
    else
    {
        if(nb_given<max_pin_len)
            mypin[nb_given++]=keys[lastcase][0];
    }
    //Redraw text footer
    pin_redraw_text_footer(nb_given, 0, DRAW_MODE_PIN, x1+2, x2-hspace-2,
	    y1+2, y1+vsize-2);  
  }
}
