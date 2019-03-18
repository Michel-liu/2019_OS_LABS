#include<unistd.h>
#include<stdio.h>
int main(){
	int ret;
	ret = execl("/usr/bin/vi","vi","/home/liuhuan/Second/test.txt",NULL);
	if(ret == -1)
		perror("execl");
}
