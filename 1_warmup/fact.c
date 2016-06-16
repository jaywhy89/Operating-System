#include "common.h"
#include "string.h"

int factorial(int a);


int 
main(int argc,char *argv[])
{	
	char *s = argv[1];
	int i, l = strlen(argv[1]);
	int boo = 1;
	
	for (i=0; i <l; i++){
		if (!((s[i] >= '0') && (s[i] <='9')))
			boo = 0 ;
	}

	
	
	if (boo)
	{
		int var = atoi(s);
		if (var> 12) 
		printf ("Overflow\n");
		else if (var==0)
		printf ("Huh?\n");
		else
		{
			printf("%d\n",factorial(var));	
		}
	}

	else
		printf ("Huh?\n");



	return 0;
}

int factorial (int a)
{
	if (a==1)
	{
		return 1;
	}
	else 
	{
		a= a*factorial(a-1);
	}

	return a;
}

