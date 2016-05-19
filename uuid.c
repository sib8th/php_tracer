#include <stdio.h>
#include <uuid/uuid.h>
int main(int argc, char **argv){

	uuid_t uuid;
	char str[37];
	uuid_generate(uuid);
	uuid_unparse(uuid,str);
	printf("%s\n",str);
	return 0;

}
