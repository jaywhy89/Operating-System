#include "request.h"
#include "server_thread.h"
#include "common.h"
#include "pthread.h"

struct server {
	int nr_threads;
	int max_requests;
	int max_cache_size;

	//Bounded buffer
	int *buffer;

	/* add any other parameters you need */
};

void fill_in(struct server *sv, int connfd);
int process(struct server *sv);
void worker_function (struct server *sv);

int in=0;
int out=0;
/* static functions */

//mutex variable
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//pthread_cond_init
pthread_cond_t FULL;
pthread_cond_t EMPTY;

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
	/* reads file, 
	 * fills data->file_buf with the file contents,
	 * data->file_size with file size. */
	ret = request_readfile(rq);
	if (!ret)
		goto out;
	/* sends file to client */
	request_sendfile(rq);
out:
	request_destroy(rq);
	file_data_free(data);
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
	
	
		pthread_t threads[nr_threads];

		for (a=0; a<nr_threads; a++){
			pthread_create (&threads[a], NULL, (void *)&worker_function, sv);
		}


		}
	}

	/* Lab 4: create queue of max_request size when max_requests > 0 */

	/* Lab 5: init server cache and limit its size to max_cache_size */

	/* Lab 4: create worker threads when nr_threads > 0 */

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

