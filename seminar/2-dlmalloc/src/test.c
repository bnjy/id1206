#include <stdlib.h>
#include <stdio.h>
#include "dlmall.h"

void testFunction(){
	init();
	
	struct head *var1 = dalloc(500);
	struct head *var2 = dalloc(200);
	struct head *var3 = dalloc(1000);
	struct head *var4 = dalloc(819);
	dfree(var1);
	dfree(var2);
	dfree(var3);
	dfree(var4);

	traverseWithArena();
	//traverse(first);
	sanity();
}

int main(){
	testFunction();
}
