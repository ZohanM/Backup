

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "enemy.h"



Enemy::Enemy(int x, int y, int w, int h, const unsigned short *image){
		this->x = x;
		this->y = y;
		this->xHitbox = w;
		this->yHitbox = x;
		this->image = image;
}
