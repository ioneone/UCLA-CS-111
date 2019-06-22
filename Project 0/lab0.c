// Name  : Junhong Wang                                                         
// ID    : 504941113                                                            
// Email : oneone@g.ucla.edu

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>            
#include <errno.h>    
#include <getopt.h>   
#include <errno.h>                         

#include <string.h>
#include <signal.h>

char* getUsage() {
	return 
		"\
Usage: ./lab0 [OPTION]...\n\
write a program that copies its standard input to its standard output\n\
by reading from file descriptor 0 (until encountering an end of file)\n\
and writing to file descriptor 1\n\
\n\
  --input=filename   use the specified file as standard input\n\
  --output=filename  create the specified file and use it as standard output\n\
  --segfault         force a segmentation fault\n\
  --catch            catch the segmentation fault and log an error message\n\
		";
}

// segmentation fault handler
void handle_segfault() {
	// fprint or fprintf cause unspecified behavior
	// in signal handling
	write(2, "Segmentation fault detected\n", 28);
	exit(4);
}

void handle_input_option() {
	int fd;

	// open a file from --input
	fd = open(optarg, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Error opening %s: %s\n", optarg, strerror(errno));
		exit(2);
	}

	// redirect the opened file to file descriptor 0
	if (dup2(fd, 0) == -1) {
		fprintf(stderr, "Error redirecting stdin: %s\n", strerror(errno));
		exit(-1);
	}
	close(fd);
}

void handle_output_option() {
	int fd;

	// create a file from --output w/ rw-r--r-- permissions
	fd = creat(optarg, 0644);
	if (fd == -1) {
		fprintf(stderr, "Error creating %s: %s\n", optarg, strerror(errno));
		exit(3);
	}

	// redirect the created file to file descriptor 1
	if (dup2(fd, 1) == -1) {
		fprintf(stderr, "Error redirecting stdout: %s\n", strerror(errno));
		exit(-1);
	}
	close(fd);
}

void lazy_write() {

	char buf;
	int io_flag;

	// reading from file descriptor 0 (until encountering an end of file) 
	// and writing to file descriptor 1
	while (1) {

		io_flag = read(0, &buf, sizeof(char));

		// reaches EOF
		if (io_flag == 0) break;

		if (io_flag < 0) {
			fprintf(stderr, "Error reading stdin: %s\n", strerror(errno));
			exit(-1);
		}

		io_flag = write(1, &buf, sizeof(char));
		if (io_flag < 0) {
			fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
			exit(-1);
		}

	}
}

int main(int argc, char** argv) {

	int option;
	int segfault_flag = 0;
	int catch_flag = 0;

	struct option options[] = {
		{"input",    required_argument, NULL, 'i'},
		{"output",   required_argument, NULL, 'o'},
		{"segfault", no_argument,       NULL, 's'},
		{"catch",    no_argument,       NULL, 'c'},
		{0, 0, 0, 0}
	};

	while (1) {

		// iteratively parse option
		option = getopt_long(argc, argv, "", options, NULL);

		// no more options to parse
		if (option == -1) break;

		switch (option) {

			case 'i': // input
				handle_input_option();
				break;

			case 'o': // output
				handle_output_option();
				break;

			case 's': // segfault
				segfault_flag = 1;
				break;

			case 'c': // catch
				catch_flag = 1;
				break;

			default:
				fprintf(stderr, "Unrecognized argument detected\n%s\n", getUsage());
				exit(1);
				break;

		}
	}

	// catch segfault
	if (catch_flag) signal(SIGSEGV, handle_segfault);
	
	// cause segfault
	if (segfault_flag) {
		char *ptr = NULL;
		*ptr = 'a';
	}

	lazy_write();

	exit(0);
}
