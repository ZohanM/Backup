
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

class Enemy{
	public:
	uint32_t x;
	uint32_t y;
	uint32_t xHitbox;
	uint32_t yHitbox;
	const unsigned short *image;
	
	Enemy(int x, int y, int w, int h, const unsigned short *image);
};
