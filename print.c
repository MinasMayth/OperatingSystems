#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	char *line = NULL;
	size_t len = 0;
	getline(&line, &len, stdin);
	printf("%s\n", line);
	free(line);
}
