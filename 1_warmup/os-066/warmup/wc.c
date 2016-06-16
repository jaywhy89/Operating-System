#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "wc.h"
#include <string.h>
#include <ctype.h>
#define MAX 1000000


unsigned long hash(char *str);

struct wc *create_hash_table();


//structure for linked list (separate chaining)
struct sc {
	char *str;
	int count;
	struct sc *next;
};

//structure for hash table
struct wc {
	int size;
	struct sc **table;
	/* you can define this struct to have whatever fields you want. */
};

//hash table 
struct wc *
wc_init(char *word_array, long size)
{
	// ht== my_hash_table
	struct wc *ht;
	ht = create_hash_table();

	//initialize the elements of the table
	int i;
	for (i=0; i<MAX; i++)
	{
		ht->table[i]=NULL;
	}


	struct sc *current, *temp;

	char *p = word_array;
	
	long lo=0;
	char temp_A[70];
	char *temp_B;
	long hv; //hash valu	

		
	//ptr = (struct wc *)malloc(sizeof(struct wc));
	//assert(wc);

	while(lo<size)
	{
		i=0;
		while (isspace(*p)) //loop until non-space is reached.
		{
			p++;
			lo++;
		}
		while (!(isspace(*p)) && (lo<size)) //extract the word and construct data structure
		{
			temp_A[i]=*p;
			i++;
			p++;
			lo++;
		}
		temp_A[i+1]='\0';
		//temp_B = malloc(sizeof(char)+strlen(temp_A));
		temp_B = malloc(strlen(temp_A)+sizeof(NULL));

		strcpy(temp_B,temp_A);
		//temp_B[sizeof(temp_A)]='\0';	
		memset (temp_A,0,sizeof temp_A); //reset a for next word use.

		hv = hash(temp_B);
		
		

		// put word in a data structure 
		if (ht->table[hv]!=NULL)  //if struct is not null, something exists already.
		{			
			//current = ptr[hv];
			current = ht->table[hv];
			if (strcmp(temp_B,current->str)==0)
			{
				current->count = current->count + 1;
			}
			else
			{
			
			int xyz = 1;	
			while (current != NULL)
			{
				if (strcmp(current->str,temp_B)==0)
				{
					current->count = current->count + 1;
					xyz = 0;
				}	
				else if ((current->next == NULL)&&(xyz==1))
				{
					temp=(struct sc*)malloc(sizeof(struct sc));
					if (temp == NULL)
						return 0;
					temp->str=(char*)malloc(strlen(temp_B)+sizeof(NULL));
					strcpy(temp->str,temp_B);
									
					temp->count=1;
					temp->next= NULL;
					current->next=temp;
					current=current->next;
				}
				current=current->next;
			}
			}
		}
		else if(ht->table[hv] == NULL)  //if struct is NULL, create the first node and store data
		{	
			ht->table[hv]=(struct sc *)malloc(sizeof(struct sc));
			if (ht->table[hv]==NULL)
				return 0;
			current= ht->table[hv];
			current->str=(char*)malloc(strlen(temp_B)+sizeof(NULL));
			strcpy(current->str,temp_B);
		
			current->count=1;
			current->next=NULL;
		}
		free(temp_B); 
		p++;
		lo++;
	
	}
	return ht;
	
}

	//printing output test
	/*
	for (i=0; i<MAX; i++)
	{
		if (ptr[i] != NULL)
		{	
			current = ptr[i];
			while (current != NULL)
			{
				printf("%s:%d\n",current->str,current->count);
				current=current->next;
			}
		}
	}
	*/





void
wc_output(struct wc *wc)
{
	struct sc *current;
	int i;
	for (i=0; i<MAX; i++)
	{
		if (wc->table[i] != NULL)
		{
		current = wc->table[i];
		while (current != NULL)
		{
			printf("%s:%d\n", current->str,current->count);
			current=current->next;
		}

		}
	}

	//int i;

	//struct wc *current[MAX]=wc;

}

struct wc *create_hash_table()
{
	struct wc *new_table;


	//allocate memory for table structure
	new_table = malloc(sizeof(struct wc));
	//if (new_table==NULL)
	//	return NULL;

	//allocate memory for the table itself
	new_table->table=malloc(sizeof(struct sc*) * MAX);
	if (new_table->table==NULL)
		return NULL;
	
	new_table->size=MAX;

	return new_table;
}

void
wc_destroy(struct wc *wc)
{
	
	struct sc *current, *temp;
	int i;
	for (i=0; i<MAX; i++)
	{
		if (wc->table[i] != NULL)
		{
		current = wc->table[i];

		while (current != NULL)
		{
			temp = current;
			current=current->next;
			free(temp->str);
			free(temp);
		}
		/*
		while (current->next != NULL)
		{
			temp= current->next->next;
			free(current->next->str);
			free(current->next);
			current->next=temp;
			
			//if next node is NULL, linked list won't increment
			if (current->next != NULL)
			{
				current=current->next;
			}
		}
		*/
	
		//free(wc->table[i]->str);
		//free(wc->table[i]);
	
		}
	}
	free(wc->table);
	free(wc);
	
}

unsigned long hash(char *str)
{
	unsigned long hash =5381;
	int c;

	while ((c=*str++)!=0)
		hash = ((hash<<5)+hash)+c;
	
	return hash % MAX;
}

