#include "request.h"
#include "server_thread.h"
#include "common.h"
#include "pthread.h"
#define MAX 570

struct server {
	int nr_threads;
	int max_requests;
	int max_cache_size;
	//Bounded buffer
	int *buffer;
	/* add any other parameters you need */
};

typedef struct cache {
	struct file_data *data;
	int counter; 
	int hash_value;
	struct cache *next;
}cache;

cache *cache_array[MAX];



typedef struct lru {
	struct cache* cp;
	struct lru *next;
	struct lru *prev;
}lru;

lru *start, *last;

//this variable must be updated atomically!
int cache_size = 0;


void fill_in(struct server *sv, int connfd);
int process(struct server *sv);
void worker_function (struct server *sv);
unsigned long hash(char *str);
struct cache* cache_lookup(char* file_name, unsigned long hv);
void cache_insert(struct file_data *data,unsigned long hv);
void cache_evict(int amount);
void lru_insert(struct cache* cache_block);
void lru_update(struct cache* cache_block);


int in=0;
int out=0;
/* static functions */

//mutex variable
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t lock2;



//pthread_cond_init
pthread_cond_t FULL;
pthread_cond_t EMPTY;
pthread_cond_t counter_not_0;

/* initialize file data */
static struct file_data *
file_data_init(void)
{
	struct file_data *data;

	data = Malloc(sizeof(struct file_data));
	data->file_name = NULL;
	data->file_buf = NULL;
	data->file_size = 0;
	return data;
}

/* free all file data */
static void
file_data_free(struct file_data *data)
{
	free(data->file_name);
	free(data->file_buf);
	free(data);
}

static void
do_server_request(struct server *sv, int connfd)
{
	//printf("Starting cache_size:%d\n",cache_size);
	int ret;
	struct request *rq;
	struct file_data *data;

	data = file_data_init();

	/* fills data->file_name with name of the file being requested */
	rq = request_init(connfd, data);
	if (!rq) {
		file_data_free(data);
		return;
	}

	cache* temp;
	int hash_index = hash(data->file_name);

	pthread_mutex_lock(&lock2);
	temp = cache_lookup(data->file_name, hash_index);
	pthread_mutex_unlock(&lock2);



	if (temp != NULL)
	{
		request_set_data(rq, temp->data);	

	}


	else
	{
		ret = request_readfile(rq);

		//printf("Request file size is %d\n", data->file_size);
		if ((data->file_size)>(sv->max_cache_size))
		{
			request_sendfile(rq);
			goto out;
		}

		pthread_mutex_lock(&lock2);
		if (cache_lookup(data->file_name, hash_index)==NULL) // if it won the race, then cache
		{
			if (((sv->max_cache_size)-cache_size) < (data->file_size)) // check available cache size
			{
				//printf("Evicting for file size(minimum) of : %d\n",data->file_size);
				int amount = (data->file_size)-((sv->max_cache_size)-cache_size);
				cache_evict(amount);
			}
			cache_insert(data, hash_index);
		}
		pthread_mutex_unlock(&lock2);
	}

	if (!ret)
		goto out;
	/* sends file to client */
	request_sendfile(rq);
out:
	//printf("cache_size:%d\n",cache_size);
	//printf("END OF REQUEST----------------\n");
	request_destroy(rq);
	//file_data_free(data);
}

void cache_evict(int amount)
{	
	int accumulator=0;
	//int z;
	int temp_hv;
	cache *current;
	lru *temp;
	while(accumulator < amount)
	{
		temp_hv = start->next->cp->hash_value;
		current = cache_array[temp_hv];
		//z=0;
		temp=start->next;

		if ((current->data->file_name)==(start->next->cp->data->file_name))
		{
			// while ((current->counter)!=0)
			// {
			// 	pthread_cond_wait (&counter_not_0,&lock4);
			// }

			accumulator = accumulator + (current->data->file_size);
			cache_size = cache_size-(current->data->file_size);
			cache_array[temp_hv]=current->next;
			free(current);
			
			start->next=temp->next;
			(temp->next)->prev=start;
			free(temp);
		}

		else 
		{

			while ((current->next->data->file_name)!=(start->next->cp->data->file_name))
			{
				current=current->next;
			}

			accumulator = accumulator + (current->next->data->file_size);
			cache_size = cache_size-(current->next->data->file_size);

			cache* temporary= current->next->next;
			free(current->next);
			current->next=temporary;
			
			start->next=temp->next;
			(temp->next)->prev=start;
			free(temp);
		}
	}
	return;
}

void cache_insert(struct file_data *data,unsigned long hv)
{
	cache* temp = (cache *)malloc(sizeof(cache));
	temp->data = data;
	temp->counter=0;
	temp->hash_value=hv;
	temp->next = cache_array[hv];
	cache_array[hv]=temp;
	cache_size= cache_size + (temp->data->file_size);
	lru_insert(temp);
	return;
}

struct cache* cache_lookup(char* file_name, unsigned long hv)
{
	if (cache_array[hv]!=NULL)
	{
		cache* current= cache_array[hv];
		while(current != NULL)
		{
			if (strcmp(current->data->file_name, file_name)==0)
			{
				//printf("CACHE HIT\n");
				lru_update(current);
				current->counter= (current->counter)+1;
				return current;
			}

		current=current->next;
		}
	}
	return NULL;
}

void lru_insert(struct cache* cache_block)
{
	//lru* temp=last;
	last->cp=cache_block;
	last->next=(lru*)malloc(sizeof(lru));
	(last->next)->prev=last;
	(last->next)->next=NULL;
	last = last->next;
}

void lru_update(struct cache* cache_block)
{
	lru* temp;
	temp=start->next;
	int z=0;
	while ((temp->next!=NULL)||(z==0))
	{
		if ((temp->cp->data->file_name)==(cache_block->data->file_name))
		{
			(temp->prev)->next=temp->next;
			(temp->next)->prev=temp->prev;

			temp->next=last;
			temp->prev=last->prev;
			
			(last->prev)->next=temp;
			last->prev=temp;
			z=1;
		}
		temp=temp->next;
	}
}
/* entry point functions */

struct server *
server_init(int nr_threads, int max_requests, int max_cache_size)
{
	struct server *sv;
	int a;
	sv = Malloc(sizeof(struct server));
	sv->nr_threads = nr_threads;
	sv->max_requests = max_requests+1;
	sv->max_cache_size = max_cache_size;


	if (nr_threads > 0 || max_requests > 0 || max_cache_size > 0) {
		
		//create bounded buffer of size, # of  requests+1 
		sv->buffer = malloc((max_requests+1)*sizeof(int));

		if (nr_threads>0)
		{
	
		if (max_cache_size > 0)
		{
			int i;  
			//MAX = 1000  ->   Hash table size of 1000
			for (i=0; i<MAX; i++)
			{
				cache_array[i]=NULL;			
			}
		}

		// LRU double linked list
		start=(lru*)malloc(sizeof(lru));
		last=(lru*)malloc(sizeof(lru));
		start->prev=NULL;
		start->next=last;
		last->prev=start;
		last->next=NULL;


		pthread_t threads[nr_threads];

		for (a=0; a<nr_threads; a++){
			pthread_create (&threads[a], NULL, (void *)&worker_function, sv);
		}


		}
	}
	return sv;
}

void
server_request(struct server *sv, int connfd)
{
	if (sv->nr_threads == 0) { /* no worker threads */
		do_server_request(sv, connfd);
	} 
	else {

		fill_in(sv, connfd);
		//pthread_cond_signal(&BLOCK);

		/*  Save the relevant info in a buffer and have one of the
		 *  worker threads do the work. */
	}
}

void fill_in(struct server *sv, int connfd)
{
	pthread_mutex_lock(&lock);

	//if buffer is full, then wait
	while ((in-out+sv->max_requests)%(sv->max_requests) == (sv->max_requests-1)) {
		//printf("buffer is full\n");
		pthread_cond_wait (&FULL,&lock);
	}
	sv->buffer[in]=connfd;
	//printf("buffing in\n");
	if (in==out){
		pthread_cond_signal(&EMPTY);
	}
	in= (in+1)%(sv->max_requests);
	//printf("in ++;\n");
	pthread_mutex_unlock(&lock);
}

int process(struct server *sv)
{
	pthread_mutex_lock(&lock);
	int msg;
	while (in==out){
		//printf("I'm in and going to sleep\n");
		//printf("buffer is empty\n");
		pthread_cond_wait(&EMPTY,&lock);
	}
	msg = sv->buffer[out];
	//printf("Retrieving msg\n");
	if ((in-out+sv->max_requests)%sv->max_requests == sv->max_requests-1){
		pthread_cond_signal(&FULL);
	}
	out = (out+1)%sv->max_requests;
	//printf("out ++\n");
	pthread_mutex_unlock(&lock);
	return msg;
}

void worker_function (struct server *sv)
{

while(1)
{
	
	//pthread_cond_wait(&BLOCK,&lock);
	int connfd2 = process(sv);
	do_server_request(sv,connfd2);
}

}

unsigned long hash(char *str)
{
	unsigned long hash =5381;
	int c;

	while ((c=*str++)!=0)
		hash = ((hash<<5)+hash)+c;
	
	return hash % MAX;
}