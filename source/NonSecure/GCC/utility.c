#include <math.h>
#include <utility.h>

/*--------------------------------------------------------------------------*/
/**
  * @brief  Calc RGB color on specific HSV angle
  * @param  angle
  * @retval uint32_t color.
  */
uint32_t HSV_Color_Calc(int angle){
	  uint32_t red, green, blue;

	  if (angle<60) {red = 255; green = round(angle*4.25-0.01); blue = 0;} else
	  if (angle<120) {red = round((120-angle)*4.25-0.01); green = 255; blue = 0;} else
	  if (angle<180) {red = 0, green = 255; blue = round((angle-120)*4.25-0.01);} else
	  if (angle<240) {red = 0, green = round((240-angle)*4.25-0.01); blue = 255;} else
	  if (angle<300) {red = round((angle-240)*4.25-0.01), green = 0; blue = 255;} else
	                 {red = 255, green = 0; blue = round((360-angle)*4.25-0.01);}

	  return red|green<<8|blue<<16;
}
