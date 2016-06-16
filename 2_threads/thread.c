#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"

#define READY 0
#define RUNNING 1
#define EXIT 2
#define SLEEP 3

/* This is the thread control block */
typedef struct thread{
	Tid id;
	int state; // 0 ready, 1 running, 2 exit, 3 sleep
	ucontext_t *tcontext;
	struct thread *next;
	/* ... Fill this in ... */
}thread;

thread *first;  //declare global variable
thread *curr;
int aid[1023]={0}; // available ID finder function


void print_rdyq(void);
void thread_stub(void (*thread_main) (void *), void *arg);
int tcounter(void);

int id_calc(void); 
//returns availalbe id spot.

void
thread_init(void)
{
	//extern thread *first;
	first = (thread *)malloc(sizeof(thread));
	first->id=id_calc();
	first->state=RUNNING;  
	first->next=NULL;
	first->tcontext=(ucontext_t*)malloc(sizeof(ucontext_t));
	//getcontext(first->tcontext);
	curr=first;
	/* your optional code here */
	//register_interrupt_handler(1);
}

Tid
thread_id()
{
	return curr->id; 
}

Tid
thread_create(void (*fn) (void *), void *parg)
{
	int enabled = interrupts_set(0);
	thread *current;
	current=first;
	
	/*int tc=tcounter();
	
	if (tc+1>=THREAD_MAX_THREADS)
		return THREAD_NOMORE;
	*/

	while (current->next != NULL)
	{
		current=current->next;
	}
	current->next=malloc(sizeof(thread));
	if (current->next == NULL)
	{
		interrupts_set(enabled);
		return THREAD_NOMEMORY;
	}
	
	current=current->next;

	current->id=id_calc();
	current->state=READY;
	current->next=NULL;
	current->tcontext=(ucontext_t*)malloc(sizeof(ucontext_t));

	if (current->tcontext == NULL)
	{
		interrupts_set(enabled);
		return THREAD_NOMEMORY;
	}
	

	void *sp=(void*)malloc(THREAD_MIN_STACK);
	if (sp == NULL)
	{
		interrupts_set(enabled);
		return THREAD_NOMEMORY;
	}

	
    getcontext(current->tcontext);


	current->tcontext->uc_mcontext.gregs[REG_RSP]=(unsigned long)(sp+THREAD_MIN_STACK-8);
	current->tcontext->uc_mcontext.gregs[REG_RIP]=(unsigned long)thread_stub;
	current->tcontext->uc_mcontext.gregs[REG_RDI]=(unsigned long)fn;
	current->tcontext->uc_mcontext.gregs[REG_RSI]=(unsigned long)parg;
    //printf("current id is %d\n",current->id);

	interrupts_set(enabled);
	return current->id;	

}

Tid
thread_yield(Tid want_tid)
{
	
	int enabled = interrupts_set(0);
	int context_called=0;
	thread *current;
	current=curr;
	//int abc=1;
	//if (current->next==NULL)
	//	current=first;

	//printf("Yielding from %d to %d\n", curr->id, want_tid);

	if ((want_tid > (THREAD_MAX_THREADS-1))||(want_tid<-7))
	{
		interrupts_set(enabled);
		return THREAD_INVALID;
	}
	if (want_tid > tcounter())
	{
		interrupts_set(enabled);
		return THREAD_INVALID;
	}

	if (want_tid==THREAD_SELF) //no-op. return current id and continue the execution.
	{
		interrupts_set(enabled);
		return curr->id;
	}

	if (want_tid==THREAD_ANY)  //run any thread in the ready queue
	{
		//print_rdyq();
		if (first->next==NULL) //if there is only 1 thread running
		{
			interrupts_set(enabled);
			return THREAD_NONE;
		}

	
		while(current->state != READY) //find the first thread that is in ready state
		{
			current=current->next;
			if (current==NULL)
			{
				current=first;
				while(current->state != READY)
				{
					if(current==curr)
					{
						interrupts_set(enabled);
						return THREAD_NONE;	
					}
					current=current->next;
					if(current==curr)
					{
						interrupts_set(enabled);
						return THREAD_NONE;	
					}
				}
				
			}
		}
		//printf("Yielding from %d to %d \n", curr->id, current->id);

		curr->state=READY;

		getcontext(curr->tcontext);

		if (context_called==1)
		{
			interrupts_set(enabled);
			return curr->id;
		}

		context_called=1;
		curr=current;
		curr->state=RUNNING;

		setcontext(curr->tcontext);
		return curr->id;
	}

	current=first;
	while (current->id != want_tid)
	{
		current=current->next;
		if (current==NULL)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}
	}

	if (current->state == READY)
	{
		curr->state=READY;
		getcontext(curr->tcontext);

			if (context_called==1)
			{
				interrupts_set(enabled);
				return want_tid;
			}

		context_called=1;
		curr=current;
		curr->state=RUNNING;
		setcontext(curr->tcontext);
		
		return curr->id;
	}

	else if (current->state == RUNNING)
	{
		interrupts_set(enabled);			
		return current->id;
	}
	
	//printf("HI\n");
	return THREAD_FAILED;
}

Tid
thread_exit(Tid tid)
{
	int enabled = interrupts_set(0);
	thread *current;
	//thread *temp;
	current = first;


	if (tid==THREAD_ANY)
	{
		while(current->state != READY)
		{
			current=current->next;
			if (current==NULL)
			{
				interrupts_set(enabled);
				return THREAD_NONE;
			}
		}
		current->state=EXIT;
		int x=current->id;
		aid[x]=0; //make ID available 
		current->id=-99;
		interrupts_set(enabled);
		return x;
	}

	if (first->next==NULL)
	{
		interrupts_set(enabled);
		return THREAD_NONE;
	}


	while ((current->id != tid))
	{
		current=current->next;
		if (current==NULL)
		{
			interrupts_set(enabled);
			return THREAD_INVALID;
		}
		
	}

 	current->state=EXIT;
 	int x=current->id;

	aid[x]=0;
	current->id=-99;

	interrupts_set(enabled);
	return x;

}

void print_rdyq(void)
{
	thread *current;
	current = first;
	while (current!=NULL)
	{
	printf("id:%d\n", current->id);
	printf("state:%d\n", current->state);
	current=current->next;
	}
}

void thread_stub(void (*thread_main) (void *), void *arg)
{
	interrupts_set(1);
	Tid ret;
	thread_main(arg);
	ret = thread_exit(THREAD_SELF);

	assert(ret==THREAD_NONE);
	exit(0);
}

int tcounter(void)
{
	thread *current;
	current=first;
	int i = 0;

	if(current==NULL)
		return -1;

	while (current->next != NULL)
	{
		current=current->next;
		i++;
	}
	return i;
}

int id_calc(void)
{
	int j=0;;
	while (aid[j]!=0) // aid[j]==0 means spot is available
	{
		j++;
	}

	aid[j]=1; //reserve the spot 
	return j;
}


/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* This is the wait queue structure */
typedef struct wait_queue {
	Tid id;
	struct wait_queue *next;
	int dummy;
}wait_queue;


struct wait_queue *
wait_queue_create()
{
	int enabled = interrupts_set(0);
	struct wait_queue *wq;

	wq = malloc(sizeof(struct wait_queue));
	assert(wq);
	wq->next=NULL;
	wq->dummy=0;

	interrupts_set(enabled);
	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	TBD();
	free(wq);
}

Tid
thread_sleep(struct wait_queue *queue)
{
	int enabled = interrupts_set(0);
	thread *current;
	current=first;

	wait_queue *q_current;
	q_current=queue;
	

	int context_called = 0;

	if (queue==NULL)
	{
		interrupts_set(enabled);
		return THREAD_INVALID;
	}	

	//find a thread that is ready, if none return THREAD_NONE
	while(current->state != READY)
	{
		current=current->next;
		if (current==NULL)
		{
			interrupts_set(enabled);
			return THREAD_NONE;
		}
	}

	//At this point, current is pointing at ready thread

	//set current running thread's state to sleep.
	curr->state=SLEEP;

	getcontext(curr->tcontext);

	if (context_called==1)
	{
		interrupts_set(enabled);
		return curr->id;
	}
	
	context_called=1;

	//only runs when there is 0 threads in the sleep queue. 
	//malloc not required
	if (queue->dummy == 0)
	{
		queue->dummy=1;
		queue->id=curr->id;
		queue->next=NULL;
	}

	//Add current running thread at the end.
	else
	{
		while(q_current->next != NULL)
		{
			q_current=q_current->next;
		}
	q_current->next=malloc(sizeof(struct wait_queue));
	q_current=q_current->next;
	q_current->id=curr->id;
	q_current->dummy=1;
	q_current->next=NULL;

	}
	
	curr=current;
	curr->state=RUNNING;
	setcontext(curr->tcontext);

	interrupts_set(enabled);
	return curr->id;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
	int enabled = interrupts_set(0);
	thread *current;
	current = first;

	wait_queue *q_current;
	q_current=queue;

	int counter=0;

	if (queue==NULL)
		interrupts_set(enabled);
		return counter;

	// dummy=0 means wait queue has been initialized but nothing has been added to wait queue.
	if (queue->dummy==0)
		interrupts_set(enabled);
		return counter;

	//wake up the the first thread(head) in the sleep queue.
	if (all==0)
	{
		//make current point to the corresponding id in main linked list
		while (current->id != q_current->id)
		{
			current=current->next;
			if (current==NULL)
			{
				interrupts_set(enabled);
				return counter;
			}
		}


		current->state=READY;
		queue=queue->next;
		free(q_current);

		interrupts_set(enabled);
		return 1;
	}

	if (all==1)
	{

		//wake up(put in ready state) for every thread in sleep queue.
		while (q_current != NULL)
		{
		q_current=queue;
		current=first;

			while(q_current->id != current->id)
			{
				current=current->next;
			}
		current->state=READY;
		queue=queue->next;
		free(q_current);
		q_current=queue;

		counter++;
		}

	interrupts_set(enabled);
	return counter;
	}
interrupts_set(enabled);
return 0;
}

struct lock {
	/* ... Fill this in ... */
};

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);

	TBD();

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	TBD();

	free(lock);
}

void
lock_acquire(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

void
lock_release(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

struct cv {
	/* ... Fill this in ... */
};

struct cv *
cv_create()
{
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);

	TBD();

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	TBD();

	free(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}
