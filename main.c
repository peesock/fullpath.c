#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void * memcopy(char * dest, const char * src, size_t n){
	/* char * cdest = (char *)dest; */
	/* char * csrc = (char *)src; */
	while (n-- && (*dest++ = *src++)) ;
	return dest;
}

char * strcopy(char * dst, const char * src, size_t n){
		while (--n && (*dst++ = *src++))
		;

	if (!n) {
		*dst++ = '\0';
		return *src ? NULL : dst;
	} else {
		return dst;
	}
}

// usually faster than realpath, except with epically gigantic pathnames approaching the 4096 char limit on Linux.
char* fullpath(char* filename, char* cwd){
	int bufferlen;
	int filelen;
	int cwdlen;
	char* buffer;
	cwdlen = strlen(cwd);
	filelen = strlen(filename);
	bufferlen = cwdlen + filelen + 1;
	if (filename[0] != '/'){
		// combine cwd with relative path
		bufferlen += 1;
		buffer = malloc(bufferlen + 1);
		if (cwd[1] == '\0') {
			buffer[0] = '/';
			memcpy(buffer+1, filename, filelen);
			/* strcpy(buffer+1, filename); */
		} else {
			memcpy(buffer, cwd, cwdlen);
			/* strcpy(buffer, cwd); */
			buffer[cwdlen] = '/';
			memcpy(buffer+(cwdlen+1), filename, filelen);
			/* strcpy(buffer+(cwdlen+1), filename); */
		}
		// printf("concatted: %s\n", buffer);
	} else {
		buffer = malloc(bufferlen + 1);
		strcpy(buffer, filename);
	}

	// look for '.' and '/' chars and manipulate
	char c;
	for (int i = 0; i < bufferlen; i++){
		// printf("%c\n", buffer[i]);

		// remove duplicate '/'
		if (buffer[i] == '/' && buffer[i+1] == '/'){
			// printf("removing 1 slash.\n");
			bufferlen -= 1;
			for (int ii=i+1; ii <= bufferlen; ii++){
				// printf("hi\n");
				buffer[ii] = buffer[ii+1];
			}
			i--;
			continue;
		}

		if (buffer[i] == '/'){
			i++;
			// resolve ".." directories
			if (buffer[i] == '.'){
				// printf("i1: %c, i2: %c\n", buffer[i+1], buffer[i+2]);
				if (buffer[i+1] == '.' && ((c=buffer[i+2]) == '/' || c == '\0')){
					// printf("removing 2 dots.\n");
					// ii must start at the previous directory
					int p=i-2; // char before the last '/'
					for (; buffer[p] != '/'; p--){  }
					int delen = 3 + (i - p - 1);
					bufferlen -= delen;
					int ii=p;
					for (; ii <= bufferlen; ii++){
						// printf("Moving ii+delen=%d=%c to ii=%d=%c\n", ii + delen, buffer[ii + delen], ii, buffer[ii]);
						buffer[ii] = buffer[ii + delen];
					}
					i -= delen-1;
					continue;
				}

				// resolve "." directories
				if ((c=buffer[i+1]) == '/' || c == '\0'){
					// printf("removing 1 dot.\n");
					int delen = 2;
					bufferlen -= delen;
					for (int ii=i-1; ii <= bufferlen; ii++){
						// printf("Moving ii+delen=%d=%c to ii=%d=%c\n", ii + delen, buffer[ii + delen], ii, buffer[ii]);
						buffer[ii] = buffer[ii + delen];
					}
					i -= delen-1;
					continue;
				}
			}
		}
	}

	if (bufferlen == 0){
		buffer[0] = '/'; buffer[1] = '\0';
		bufferlen++;
	}
	else if (buffer[bufferlen-1] == '/'){
		buffer[bufferlen-1] = '\0';
	}
	return buffer;
}

char escaper(char*);

int main(int argc, char** argv){
	argc--;
	int i = 1;
	char d ='\n';
	if (strcmp(argv[1], "-d") == 0){
		d = escaper(argv[2]);
		i += 2;
	}
	if (argc == 0)
		exit(1);

	char* cwd = alloca(PATH_MAX);
	if ((cwd = getenv("PWD")) == NULL) {
		printf("%s\n", cwd);
		perror("getenv() error");
		return 1;
	}

	for (; i <= argc; i++){
		printf("%s%c", fullpath(argv[i], cwd), d);
	}
}

char escaper(char* escape){
	if (escape[0] == '\\' && escape[1] != '\0' && escape[2] == '\0'){
		char c1 = escape[1];
		switch (c1) {
			case '0':
				return '\0';
			case 'a':
				return '\a';
			case 'b':
				return '\b';
			case 'f':
				return '\f';
			case 'n':
				return '\n';
			case 'r':
				return '\r';
			case 't':
				return '\t';
			case 'v':
				return '\v';
			case '\\':
				return '\\';
		}
	}
	return escape[0];
}
