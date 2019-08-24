#include <ledpattern.h>
#include "NuMicro.h"
#include <math.h>
#include <APA102.h>


extern volatile uint32_t g_u32SysTick;
extern uint8_t led_position[24][2];

uint32_t HSV_Color_Calc(int angle){
    uint32_t red, green, blue;

    if (angle<60) {red = 255; green = round(angle*4.25-0.01); blue = 0;} else
    if (angle<120) {red = round((120-angle)*4.25-0.01); green = 255; blue = 0;} else
    if (angle<180) {red = 0, green = 255; blue = round((angle-120)*4.25-0.01);} else
    if (angle<240) {red = 0, green = round((240-angle)*4.25-0.01); blue = 255;} else
    if (angle<300) {red = round((angle-240)*4.25-0.01), green = 0; blue = 255;} else
                   {red = 255, green = 0; blue = round((360-angle)*4.25-0.01);}

    return red<<16|green<<8|blue;
}

void  patternError(){
	ClearPixels();
	int counter = g_u32SysTick/10;
	int r = counter%510;
	if(r>255){
		r = 510 - r;
	}
	//printf("%d\n",r);
	for(int i = 0 ;i<24;i++){
		SetPixelColorRGB(i,r,0,0);
	}
	PixelShow();
}

void function_pattern(int32_t controlA,int32_t controlB,int degree,float b,float x0,float y0,float height_amp){

	if(degree > 89) degree = 89;
	if(degree == 0) degree = 1;
	if(degree < -89) degree = -89;
	float a = tan(degree*3.14159/180.0);

	ClearPixels();

	for(int i=0;i<24;i++){

		float led_x = (float)led_position[i][0];
		float led_y = (float)led_position[i][1];

		float y_func =  ((led_x - x0) * a + b) + y0;
		float x_func =  (led_y - b - y0) / a + x0;

		float x_diff = x_func - led_x;
		float y_diff = led_y - y_func;

		if(x_diff < 0) x_diff = -x_diff;

		float height = y_diff*x_diff / sqrt(x_diff*x_diff + y_diff*y_diff);

		int height_diff = (int)(height*height_amp) %360;
		if(height_diff<0) height_diff += 360;
		SetPixelColor(i,HSV_Color_Calc(height_diff));
	}

	PixelShow();
}

void pattern1(int32_t controlA,int32_t controlB){
	ClearPixels();
	controlA = controlA/10;
	if(controlA < -9) controlA = -9;
	uint32_t speed = controlA + 10;
	uint32_t angle = controlB;


	int j = g_u32SysTick/speed;
	//printf("%d\n",j);
	for(int y0 = 0;y0<130;y0++){
		int color = (y0*3-j)%360;
		if(color<0) color +=360;
		drawLine( 0,  y0,  65,  y0-30, HSV_Color_Calc(color));
	}
	PixelShow();
}


uint32_t pixelEmittion[24] = {0};
int8_t pixelEmittion_color[24] = {0};
void pattern2(int32_t controlA,int32_t controlB){
	ClearPixels();
	for(int i=0;i<hardwareRandom() % 10;i++){
		uint8_t randomSelect = hardwareRandom() % 24;
		uint32_t randomValue = hardwareRandom() % 8192;
		uint8_t randomColor = hardwareRandom() % 40;

		if(randomValue > controlA+5000 && pixelEmittion[randomSelect]-g_u32SysTick>5000){
			//printf("======================>%d\n",randomSelect);
			pixelEmittion[randomSelect] = g_u32SysTick;
			pixelEmittion_color[randomSelect] = randomColor;
		}
	}
	//controlA = controlA/10;
	//if(controlA < -9) controlA = -9;
	//uint32_t speed = controlA + 10;

	for(int i = 0 ;i<24;i++){
		uint32_t timeLength = g_u32SysTick - pixelEmittion[i];
		uint8_t bright = 255*exp(-1 * (double)timeLength / 1000.0);
		//if(bright<30) bright = 30;
		uint32_t color = HSV_Color_Calc((g_u32SysTick/40+pixelEmittion_color[i]-20 + led_position[i][1]/2 ) % 360);//HSV_Color_Calc(pixelEmittion_color[i]);
		//printf("%d,%d,%d,%d\n",(g_u32SysTick/10+pixelEmittion_color[i]-20 + led_position[i][1]/2 ) % 360,pixelEmittion_color[i]-20,g_u32SysTick,led_position[i][1]/2);
		uint8_t r = (uint8_t)(color>>16);
		uint8_t g = (uint8_t)(color>>8);
		uint8_t b = (uint8_t)color;

		float bright_ratio = (float)bright/255.0;
		//printf("%d,%d,%d\n",i,timeLength,bright);
		SetPixelColorRGB(i,r*bright_ratio,g*bright_ratio,b*bright_ratio);
	}

	PixelShow();
}
void pattern3(int32_t controlA,int32_t controlB){
	ClearPixels();
	controlA = controlA/10;
	if(controlA < -9) controlA = -9;
	uint32_t speed = controlA + 10;
	uint32_t angle = controlB;
	int j = g_u32SysTick/speed;
	//printf("%d,%d\n",j,speed);
	for(int i = 0 ;i<24;i++){
		SetPixelColor(i,HSV_Color_Calc((i*15+j)%360));
	}
	PixelShow();
}

void pattern4(int32_t controlA,int32_t controlB){
	ClearPixels();
	controlA = controlA/10;
	if(controlA < -9) controlA = -9;
	uint32_t speed = controlA + 10;
	uint32_t angle = controlB;

	int j = g_u32SysTick/speed;
	//printf("%d\n",j);
	for(int x0 = 0;x0<85;x0++){
		int color = (x0*3-j)%360;
		if(color<0) color +=360;
		drawLine( x0,  0,  x0,  100, HSV_Color_Calc(color));
	}
	PixelShow();
}

void pattern5(int32_t controlA,int32_t controlB){

	ClearPixels();
	controlA = controlA/10;
	if(controlA < -90) controlA = -90;
	uint32_t speed = controlA + 100;
	uint32_t angle = controlB;

	int j = g_u32SysTick/speed;
	//printf("%d\n",j);
	uint32_t color1 = HSV_Color_Calc(j%360);
	uint8_t r1 = (uint8_t)(color1>>16);
	uint8_t g1 = (uint8_t)(color1>>8);
	uint8_t b1 = (uint8_t)color1;

	uint32_t color2 = HSV_Color_Calc((j+180)%360);
	uint8_t r2 = (uint8_t)(color2>>16);
	uint8_t g2 = (uint8_t)(color2>>8);
	uint8_t b2 = (uint8_t)color2;


	uint8_t startPoint1 = j%24;
	uint8_t startPoint2 = (startPoint1 + 12)%24;
	uint8_t totalLength = 12;

	for(int i = 0 ;i<totalLength;i++){
		uint8_t bright = 255*exp(-1 *  ((double)totalLength - (double)i) / 10.0);
		float bright_ratio = (float)bright/255.0;
		int k = (i + startPoint1)%24;
		//if(k>24) k = 24 - ( k - 24 );
		SetPixelColorRGB(k,r1*bright_ratio,g1*bright_ratio,b1*bright_ratio);
	}


	for(int i = 0 ;i<totalLength;i++){
		uint8_t bright = 255*exp(-1 *  ((double)totalLength - (double)i) / 10.0);
		float bright_ratio = (float)bright/255.0;
		int k = (i + startPoint2)%24;
		//if(k>24) k = 24 - ( k - 24 );
		SetPixelColorRGB(k,r2*bright_ratio,g2*bright_ratio,b2*bright_ratio);
	}
	SetPixelColor(16,0xFFFFFF);
	PixelShow();
}

void pattern9(int32_t controlA,int32_t controlB){
	ClearPixels();
	controlA = controlA/10;
	if(controlA < -90) controlA = -90;
	uint32_t speed = controlA + 100;
	uint32_t angle = controlB;

	int j = g_u32SysTick/speed;
	//printf("%d\n",j);
	//uint32_t color1 = HSV_Color_Calc(j%360);
	uint8_t r1 = 255;
	uint8_t g1 = 255;
	uint8_t b1 = 255;

	//uint32_t color2 = HSV_Color_Calc((j+180)%360);
	uint8_t r2 = 255;
	uint8_t g2 = 255;
	uint8_t b2 = 255;


	uint8_t startPoint1 = j%24;
	uint8_t startPoint2 = (startPoint1 + 12)%24;
	uint8_t totalLength = 12;

	for(int i = 0 ;i<totalLength;i++){
		uint8_t bright = 255*exp(-1 *  ((double)totalLength - (double)i) / 10.0);
		float bright_ratio = (float)bright/255.0;
		int k = (i + startPoint1)%24;
		//if(k>24) k = 24 - ( k - 24 );
		SetPixelColorRGB(k,r1*bright_ratio,g1*bright_ratio,b1*bright_ratio);
	}


	for(int i = 0 ;i<totalLength;i++){
		uint8_t bright = 255*exp(-1 *  ((double)totalLength - (double)i) / 10.0);
		float bright_ratio = (float)bright/255.0;
		int k = (i + startPoint2)%24;
		//if(k>24) k = 24 - ( k - 24 );
		SetPixelColorRGB(k,r2*bright_ratio,g2*bright_ratio,b2*bright_ratio);
	}
	SetPixelColor(16,0xFFFFFF);
	PixelShow();
}

void pattern7(int32_t controlA,int32_t controlB){


	int degree =  (g_u32SysTick/25) % 360;
	if(degree > 180) degree = 360 - degree;
	degree = degree - 90;


	if(degree > 89) degree = 89;
	if(degree == 0) degree = 1;
	if(degree < -89) degree = -89;

	function_pattern(controlA,controlB,degree,0,40,40,3);

}

void pattern8(int32_t controlA,int32_t controlB){


	int degree =  (g_u32SysTick/25) % 360;
	if(degree > 180) degree = 360 - degree;
	degree = degree - 90;

	float b = (g_u32SysTick/20) % 300;
	if(b > 150) b = 300 - b;

	if(degree > 89) degree = 89;
	if(degree == 0) degree = 1;
	if(degree < -89) degree = -89;

	function_pattern(controlA,controlB,degree,b,40,40,3);

}

int pattern9_x0 = -10;
int pattern9_y0 = 0;
uint32_t last_update_pattern9 = 0;
void pattern10(int32_t controlA,int32_t controlB){


	int degree =  (g_u32SysTick/10) % 360;
	if(degree > 180) degree = 360 - degree;
	degree = degree - 90;

	//float b = (g_u32SysTick/20) % 300;
	//if(b > 150) b = 300 - b;

	if(degree > 89) degree = 89;
	if(degree == 0) degree = 1;
	if(degree < -89) degree = -89;

	if( (g_u32SysTick-last_update_pattern9) > 30){
		int move_x =  hardwareRandom() % 5 - 2;
		int move_y =  ( ( hardwareRandom() & 0x7fffffff ) % 9 ) - 4;
		//pattern9_x0 = pattern9_x0 + move_x;
		//printf( "move_y = %d\n" , move_y );
		pattern9_y0 += move_y;
		last_update_pattern9 = g_u32SysTick;
	}
	//printf("%d,%d\n",pattern9_x0,pattern9_y0);

	function_pattern(controlA,controlB,degree,0,pattern9_x0,pattern9_y0,3);

}

void pattern6(int32_t controlA,int32_t controlB){


	int degree =  (g_u32SysTick/25) % 360;
	if(degree > 180) degree = 360 - degree;
	degree = degree - 90;

	float b = (g_u32SysTick/20) % 300;
	if(b > 150) b = 300 - b;

	if(degree > 89) degree = 89;
	if(degree == 0) degree = 1;
	if(degree < -89) degree = -89;

	function_pattern(controlA,controlB,degree,b,40,40,3);

}

uint32_t circleRadis[2] = {0};
int8_t circleStartPoint_x[2] = {-100};
int8_t circleStartPoint_y[2] = {-100};
uint8_t circle_count = 0;
void pattern11(int32_t controlA,int32_t controlB){

	ClearPixels();



	uint8_t randomSelectx = hardwareRandom() % 201;
	uint8_t randomSelecty = hardwareRandom() % 201;
	uint32_t randomValue = hardwareRandom() % 8192;
	uint8_t randomColor = hardwareRandom() % 80;

	//float randomdistance_x = circleStartPoint_x[circle_count%2] - randomSelectx;
	//float randomdistance_y = circleStartPoint_y[circle_count%2] - randomSelecty;
	//float randomdistance = sqrt(randomdistance_x*randomdistance_x + randomdistance_y*randomdistance_y);

	if(controlA < -300) controlA = -300+1;


	uint32_t timeLength1 = (g_u32SysTick - circleRadis[circle_count%2])/5;
	uint32_t timeLength2 = (g_u32SysTick - circleRadis[(circle_count+1)%2])/5;

	if(timeLength1 >= 300+controlA){
		//printf("======================>%d,%d\n",randomSelectx,randomSelecty);
		circle_count ++;
		circleRadis[circle_count%2] = g_u32SysTick;
		circleStartPoint_x[circle_count%2] += randomSelectx - 100;
		circleStartPoint_y[circle_count%2] += randomSelecty - 100;

	}

	float pixel_buffer[24] = {0};
	for(int k=0;k<2;k++){
		uint32_t timeLength = (g_u32SysTick - circleRadis[k])/5;
		if(circleStartPoint_x[k] != -100 && circleStartPoint_y[k] != -100){
			for(int i=0;i<24;i++){
				float distance_x = led_position[i][0] - circleStartPoint_x[k];
				float distance_y = led_position[i][1] - circleStartPoint_y[k];

				float distance = sqrt(distance_x*distance_x + distance_y*distance_y);

				//uint8_t bright = 255*exp(-1 * (double)distance / 1000.0);

				//printf("%f\n",distance);
				if(distance < timeLength){
					//float bright_ratio = distance/(float)timeLength;


					float bright_ratio = 255*exp(-1 *  ((double)timeLength - (double)distance) / 40.0);

					//if(bright_ratio < ) continue;
					//uint8_t r = bright_ratio;
					//uint8_t g = bright_ratio;
					//uint8_t b = bright_ratio;
					if(k == 0){
						pixel_buffer[i] = bright_ratio;//(uint32_t)r<<16 | (uint32_t)g<<16 | (uint32_t)b;
					}
					else{
						pixel_buffer[i] += bright_ratio;
					}
					//
				}
			}
		}
	}

	for(int i=0;i<24;i++){
		//printf("%f\n",pixel_buffer[i]);
		if(pixel_buffer[i]>255) pixel_buffer[i] = 255;
		//if(pixel_buffer[i] < 0.2) continue;
		uint8_t r = pixel_buffer[i];
		uint8_t g = pixel_buffer[i];
		uint8_t b = pixel_buffer[i];
		SetPixelColorRGB(i,r,g,b);
	}
	PixelShow();
}


/*
void colorholeled(){
    int      head  = 0, tail = -10; // Index of first 'on' and 'off' pixels
    uint32_t color = 0;

    while(1){
        SetPixelColor(head, color);
        SetPixelColor(tail, 0);
        PixelShow();

          if(++head >= 22) {         // Increment head index.  Off end of strip?
            head = 0;                       //  Yes, reset head index to start
            if((color >>= 8) == 0)          //  Next color (R->G->B) ... past blue now?
              color = 0xFF0000;             //   Yes, reset to red
          }
          if(++tail >= 30) tail = 0; // Increment, reset tail index
          Delayms(50);

          if(PB2_NS==0){
              return;
          }
    }
}


void rainbowCycle(){
  uint16_t i, j;
  while(1) { // 5 cycles of all colors on wheel
    for(i=0; i< 24; i++) {
      SetPixelColor(i, HSV_Color_Calc(((i * 256 / 30) + j) & 255));
    }
    PixelShow();
    Delayms(4);
    j++;
    if(PB2_NS==0){
          return;
      }
  }
}

	uint32_t circleRadis = 0;
	int8_t circleStartPoint = 0;


	uint8_t randomSelect = hardwareRandom() % 20;
	uint32_t randomValue = hardwareRandom() % 8192;
	uint8_t randomLength = hardwareRandom() % 50;

	if(randomValue > controlA+7000 && (pattern4Start[randomSelect] > 300 || pattern4Length[randomSelect] == 0)){
		pattern4Start[randomSelect] = -randomLength-30;
		pattern4Length[randomSelect] = randomLength+30;
		printf("====>%d,%d\n",randomSelect,randomLength+30);
	}


	for(int x0=0;x0<100;x0+=5){
		printf("%d,%d\n",pattern4Start[x0/5],pattern4Length[x0/5]);
		if(pattern4Start[x0/5] < 100 && pattern4Length[randomSelect] > 0){
			for(int k=0;k<5;k++){
				drawLine( x0+k+3, pattern4Start[x0/5], x0+k+3, pattern4Start[x0/5] + pattern4Length[x0/5], 0x7F7F7F);
				drawLine( x0+k+3, pattern4Start[x0/5]+ pattern4Length[x0/5]-5, x0+k+3, pattern4Start[x0/5] + pattern4Length[x0/5], 0x00FF00);
			}

		}
		pattern4Start[x0/5] +=3;
		if(pattern4Start[x0/5] >= 200)
			pattern4Start[x0/5] = 0;
	}

	int16_t pattern4Start[100/5] = {0};
	int16_t pattern4Length[100/5] = {0};
	ClearPixels();

	uint32_t timeLength = (g_u32SysTick - circleRadis)/50;

	uint8_t randomSelect = hardwareRandom() % 24;
	uint32_t randomValue = hardwareRandom() % 8192;
	uint8_t randomColor = hardwareRandom() % 80;

	if(randomValue > controlA+8100 && timeLength >= 30){
		printf("======================>%d\n",randomSelect);
		circleRadis = g_u32SysTick;
		circleStartPoint = randomSelect;
	}


	if(timeLength < 30 && circleStartPoint != -1){
		for(int k=20;k>0;k-=2){
			uint8_t b = 255.0 * (float)k/20.0;
			uint32_t c = (uint32_t) b << 16 | (uint32_t) b << 8 | (uint32_t) b << 0;
			fillCircle(led_position[circleStartPoint][0],led_position[circleStartPoint][1],timeLength+k-20,c);
		}
	}
	drawPixel(led_position[circleStartPoint][0],led_position[circleStartPoint][1],0xFF0000);
	PixelShow();



*/
