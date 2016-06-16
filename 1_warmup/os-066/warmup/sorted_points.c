#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "point.h"
#include "sorted_points.h"
#include "math.h"

struct sorted_points {
	double xpt;
	double ypt;
	double dist;
	struct sorted_points *next;
	/* you can define this struct to have whatever fields you want. */
};

struct sorted_points *sp_init()
{
	struct sorted_points *sp;
	
	sp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
	if (sp==NULL)
		return 0;
	assert(sp);
	
	sp->next=NULL;	
	return sp;
}

void
sp_destroy(struct sorted_points *sp)
{
	struct sorted_points *current,*temp;
	current=sp;

	while(current->next != NULL)
	{
		temp=current->next->next;
		free(current->next);
		current->next = temp;
	}

	free(sp);
}

int
sp_add_point(struct sorted_points *sp, double x, double y)
{
	double dt=sqrt((x*x)+(y*y));
	int retint=-1;

	struct sorted_points *current, *temp;
	current = sp;
	
	//if linked list is empty
	if (sp->next == NULL)
	{
		temp=(struct sorted_points *)malloc(sizeof(struct sorted_points));
		if (temp == NULL)
			return 0;
		//current=sp->next;
		temp->xpt=x;
		temp->ypt=y;
		temp->dist = dt;
		temp->next = NULL;
		sp->next=temp;
		return 1;
	}
	
	//inserting at the first node.
	else if ((dt < current->next->dist) || ( (dt == current->next->dist) && ((x <= current->next->xpt)&&(y <= current->next->ypt))))
	{
		temp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
		if (temp==NULL)
			return 0;
		temp->xpt=x;
		temp->ypt=y;
		temp->dist=dt;
		temp->next=sp->next;
		sp->next=temp;
		return 1;	
	}
	

	current=current->next;
	

	while (retint == -1)
	{
		//if last node is reached.
		if (current->next ==NULL)
		{
			temp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
			if (temp==NULL)
				return 0;
			temp->xpt=x;
			temp->ypt=y;
			temp->dist=dt;
			temp->next=NULL;
			current->next=temp;
			retint=1;
			return retint;
		}
		
		//if next element's dist is bigger than current dt
		else if ((dt < current->next->dist) || ( (dt == current->next->dist) && ((x <= current->next->xpt) && (y <= current->next->ypt))))
		{
				temp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
				if (temp==NULL)
					return 0;
				temp->xpt=x;
				temp->ypt=y;
				temp->dist=dt;
				temp->next=current->next;
				current->next=temp;
				retint=1;
				return retint;
		}

		//if dist is the same
		else if (dt == current->next->dist)
		{
			//dist is the same, but x point is larger.
			if (x > current->next->xpt)
			{
				//traverse until x is smaller than next node, while dist is the same
				while ((x>current->next->xpt) && (dt==current->next->dist))
				{
					current=current->next;

					if (current->next==NULL)
					{
						temp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
						if (temp==NULL)
							return 0;
						temp->xpt=x;
						temp->ypt=y;
						temp->dist=dt;
						temp->next=NULL;
						current->next=temp;
						retint=1;
						return retint;
					}

				}
			
								
				//correct x spot found. now check y condition... equal or smaller than next node ypt
				if (y <= current->next->ypt)
				{
					temp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
					if (temp==NULL)
						return 0;	
					temp->xpt=x;
					temp->ypt=y;
					temp->dist=dt;
					temp->next=current->next;
					current->next=temp;
					retint=1;
					return retint;
				}
			
				//if y is larger than next node ypt and dist is same 
				else if ((y > current->next->ypt) && (dt==current->next->dist))
				{
					while ((y > current->next->ypt) && (dt==current->next->dist))
					{
						current=current->next;
						
						if (current->next==NULL)
						{
							temp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
							if (temp==NULL)
								return 0;
							temp->xpt=x;
							temp->ypt=y;
							temp->dist=dt;
							temp->next=NULL;
							current->next=temp;
							retint=1;
							return retint;
						}
					}

					
				}

				
			//otherwise, this is the correct spot.
			temp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
			if (temp==NULL)
				return 0;
			temp->xpt=x;
			temp->ypt=y;
			temp->dist=dt;
			temp->next=current->next;
			current->next=temp;
			retint=1;
			return retint;
			}
		
			
			//if x is smaller or same but y is larger
			else if (y > current->next->ypt)
			{
				//find correct spot
				while ((y > current->next->ypt) && (dt==current->next->dist))
				{
								
					current = current->next;
					
					if (current->next==NULL)
					{
						temp=(struct sorted_points *)malloc(sizeof(struct sorted_points));
						if (temp==NULL)
							return 0;
						temp->xpt=x;
						temp->ypt=y;
						temp->dist=dt;
						temp->next=NULL;
						current->next=temp;
						retint =1;
						return retint;
					}

				}

								//correct y spot
				temp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
				if (temp==NULL)
					return 0;
				temp->xpt=x;
				temp->ypt=y;
				temp->dist=dt;
				temp->next=current->next;
				current->next=temp;
				retint=1;
				return retint;
			}
		}
			
	//keep traversing if dt is bigger than next->dist
	current = current->next;
	}

	return 0;
}




int
sp_remove_first(struct sorted_points *sp, struct point *ret)
{
	struct sorted_points *temp;


	if (sp->next == NULL)
	{
		return 0;
	}

	ret->x=sp->next->xpt;
	ret->y=sp->next->ypt;
	
	temp= sp->next->next;
	free(sp->next);
	sp->next= temp;

	
	return 1;
}

int
sp_remove_last(struct sorted_points *sp, struct point *ret)
{
	struct sorted_points *current, *temp;
	current = sp;
		
	if (current->next == NULL)
	{	
		return 0;
	}
	
	/*else if (current->next->next==NULL)
	{
		printf("B\n");
		ret->x=current->next->xpt;
		ret->y=current->next->ypt;
		free(current->next);
		sp->next=NULL;
		return 1;
	}
	*/

	while (current->next != NULL)
	{
		temp=current;
		current=current->next;	
	}
		
	ret->x = current->xpt;
	ret->y = current->ypt;
	free(current);
	temp->next=NULL;
	return 1;
}

int
sp_remove_by_index(struct sorted_points *sp, int index, struct point *ret)
{
	//i is index of whole linked list
	int i=0;
	struct sorted_points *current, *temp;
	current = sp;
	
	if (sp->next == NULL)
	{
		return 0;
	}

	while (current->next != NULL)
	{
		temp=current;
		current=current->next;
		if (i==index)
		{
			ret->x=current->xpt;
			ret->y=current->ypt;
			temp->next=current->next;
			free(current);
			return 1;
		}
	i++;
	}
	return 0;


	/*
	while (current->next != NULL)
	{
		current=current->next;
		i++;
	}
	
	if (i==-1)
	{
		return 0;
	}

	if(index==0)
	{
		ret->x=sp->next->xpt;
		ret->y=sp->next->ypt;
		free(sp->next);
		sp->next=NULL;
		return 1;
	}

	else if(index>i)
	{
		return 0;
	}
	

	//we have total index number in i, now do the real traversing
   	//set current back to sp.
	current = sp;

	for(a=0; a<index; a++)
	{
		current=current->next;
	}
	
	temp = current->next->next;
	ret->x=current->next->xpt;
	ret->y=current->next->ypt;
	free(current->next);
	current->next=temp;

	//again,wrong code
	//current->next=temp->next->next;
	//free(temp->next);
	return 1;
	*/
}

int
sp_delete_duplicates(struct sorted_points *sp)
{
	struct sorted_points *current,*temp;
	current = sp;
	int count=0;
	
	if ((current->next->next == NULL)||(current->next==NULL))
	{
		return 0;
	}
	
	while (current->next != NULL)
	{
		if ((current->dist==current->next->dist) && (current->xpt==current->next->xpt) && (current->ypt==current->next->ypt))
		{
			temp=current->next->next;
			free(current->next);
			current->next=temp;
			count++;
		}
		else
		{
		current=current->next;
		}
	}
	
	return count;
}




