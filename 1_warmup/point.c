#include <assert.h>
#include "common.h"
#include "point.h"
#include "math.h"


void
point_translate(struct point *p, double x, double y)
{
	p->x = p->x + x;
	p->y = p->y + y;
	return;	
	//TBD();
}

double
point_distance(const struct point *p1, const struct point *p2)
{
	double dist_x, dist_y, dist;
	dist_x = (p2->x)-(p1->x);
	dist_y = (p2->y)-(p1->y);
	dist_x = dist_x * dist_x;
	dist_y = dist_y * dist_y;
	dist = sqrt(dist_x + dist_y);
	//TBD();
	return dist;
}

int
point_compare(const struct point *p1, const struct point *p2)
{
	int result;
	double p1x, p1y, p2x, p2y, p1_dist, p2_dist;	
	p1x = (p1->x)*(p1->x);
	p1y = (p1->y)*(p1->y);
	p2x = (p2->x)*(p2->x);
	p2y = (p2->y)*(p2->y);
	p1_dist = sqrt(p1x+p1y);
	p2_dist = sqrt(p2x+p2y);	
	if (p1_dist>p2_dist)
		result=1;
	else if (p2_dist>p1_dist)
		result=-1;
	else if(p1_dist==p2_dist)
		result=0;
	
	//TBD();
	return result;
}
