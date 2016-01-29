#include<stdio.h>
#include <qsim_magic.h>
int main(){
    qsim_magic_enable();
	printf ("Hello World!\n");
    qsim_magic_disable();
	return 0;
}
