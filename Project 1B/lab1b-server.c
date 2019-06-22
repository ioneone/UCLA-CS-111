// Name  : Junhong Wang
// ID    : 504941113
// Email : oneone@g.ucla.edu

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <zlib.h>
#include <netinet/in.h>

int child_pid;

int port_flag     = 0;
int compress_flag = 0;

int port = -1;

int new_sfd;

z_stream deflate_stream;
z_stream inflate_stream;

int safe_read(int fd, char *buf, int size) {
	int ret = read(fd, buf, size);
	if (ret == -1) {
		fprintf(stderr, "Error reading stdin: %s\n", strerror(errno));
		exit(1);
	}

	return ret;
}

int safe_write(int fd, char *buf, int size) {
	int ret = write(fd, buf, size);
	if (ret == -1) {
		fprintf(stderr, "Error writing to stdout: %s\n", strerror(errno));
		exit(1);
	}

	return ret;
}

char* getUsage() {
	return 
		"\
Usage: ./lab1b-server --port=number [OPTION]...\n\
The server program will connect with the client,\n\
receive the client's commands and send them to the shell, \n\
and will \"serve\" the client the outputs of those commands.\n\
\n\
	--port=number   specify the port number for connection\n\
	--compress      compress the communication\n\
		";
}

void safe_dup2(int oldfd, int newfd) {
	if (dup2(oldfd, newfd) == -1) {
		fprintf(stderr, "Erorr during dup2: %s\n", strerror(errno));
		exit(1);
	}
}

void safe_close(int fd) {
	if (close(fd) == -1) {
		fprintf(stderr, "Erorr closing fd: %s\n", strerror(errno));
		exit(1);
	}
}

void safe_execvp(char *file, char **argv) {
	if (execvp(file, argv) == -1) {
		fprintf(stderr, "Error executing %s: %s\n", file, strerror(errno));
		exit(1);
	}
}

void execute_shell() {
	char *file = "/bin/bash";
	char *argv[2];
	argv[0] = file;
	argv[1] = NULL;

	safe_execvp(file, argv);
	
}

void handle_child_process(int pfd_forward[], int pfd_backward[]) {

	// file descriptor redirection
	safe_dup2(pfd_forward[0], 0);
	safe_dup2(pfd_backward[1], 1);
	safe_dup2(pfd_backward[1], 2);

	// close the other ends of pipes
	safe_close(pfd_forward[0]);
	safe_close(pfd_forward[1]);
	safe_close(pfd_backward[0]);
	safe_close(pfd_backward[1]);

	execute_shell();
}

void safe_kill(int pid, int sig) {
	if (kill(pid, sig) == -1) {
		fprintf(stderr, "Error killing a process: %s\n", strerror(errno));
		exit(1);
	}
}

void safe_poll(struct pollfd fds[], int nfds, int timeout) {
	if (poll(fds, nfds, timeout) == -1) {
		fprintf(stderr, "Error while polling: %s\n", strerror(errno));
		exit(1);
	}
}

void safe_waitpid(int pid, int *status, int options) {
	if (waitpid(pid, status, options) == -1) {
		fprintf(stderr, "Error waiting a process: %s\n", strerror(errno));
		exit(1);
	}
}

void handle_sigpipe() {
	// It is not safe to call fprintf in signal handler
	write(2, "SIGPIPE detected\n", 17);
	exit(0);
}

void safe_deflate(z_stream *strm, int flush) {
	if (deflate(strm, flush) != Z_OK) {
		fprintf(stderr, "Error deflating data\n");
		exit(1);
	}
}

void simple_deflate(char *buf, int *ret) {
	char newBuf[256];
	int i;

	deflate_stream.avail_in  = *ret;                    /* number of bytes available at next_in */
	deflate_stream.next_in   = (unsigned char*) buf;    /* next input byte */
	deflate_stream.avail_out = 256;                     /* remaining free space at next_out */
	deflate_stream.next_out  = (unsigned char*) newBuf; /* next output byte will go here */

	do {

		safe_deflate(&deflate_stream, Z_SYNC_FLUSH);
		
	} while (deflate_stream.avail_in > 0);

	*ret = 256 - deflate_stream.avail_out; /* size of deflated data */

	for (i = 0; i < *ret; i++)
		buf[i] = newBuf[i];
}

void safe_inflate(z_stream *strm, int flush) {
	if (inflate(strm, flush) != Z_OK) {
		fprintf(stderr, "Error inflating data\n");
		exit(1);
	}
}

void simple_inflate(char *buf, int *ret) {
	char newBuf[2048]; /* reserve enough space for decompression */
	int i;

	inflate_stream.avail_in  = *ret;
	inflate_stream.next_in   = (unsigned char*) buf;
	inflate_stream.avail_out = 2048;
	inflate_stream.next_out  = (unsigned char*) newBuf;

	do {

		safe_inflate(&inflate_stream, Z_SYNC_FLUSH);
		
	} while (inflate_stream.avail_in > 0);

	*ret = 2048 - inflate_stream.avail_out; /* size of inflated data */
	
	for (i = 0; i < *ret; i++)
		buf[i] = newBuf[i];
}

void safe_shell_write(int pfd_forward[], char *buf, int ret) {
	int i;

	for (i = 0; i < ret; i++) {

		if (buf[i] == '\004') {
			safe_close(pfd_forward[1]);
			continue;
		}

		if (buf[i] == '\003') {
			safe_kill(child_pid, SIGINT);
			continue;
		}

		if (buf[i] == '\r' || buf[i] == '\n') {
			safe_write(pfd_forward[1], "\n", 1);
			continue;
		}

		safe_write(pfd_forward[1], &buf[i], 1);

	}
}

void handle_read_from_socket(int pfd_forward[]) {
	char buf[2048]; /* reserve enough space for decompression */
	int ret = safe_read(new_sfd, buf, 256);
	
	if (compress_flag) simple_inflate(buf, &ret);

	safe_shell_write(pfd_forward, buf, ret);
	
}

void handle_read_from_child(int pfd_backward[]) {
	char buf[256];
	int ret = safe_read(pfd_backward[0], buf, 256);

	if (compress_flag) simple_deflate(buf, &ret);

	safe_write(new_sfd, buf, ret);
}

void read_leftover(int pfd_backward[]) {
	char buf[256];
	int ret;
	while ((ret = safe_read(pfd_backward[0], buf, 256)) != 0) {

		if (compress_flag) simple_deflate(buf, &ret);

		safe_write(new_sfd, buf, ret);
	}
}

void add_observer(struct pollfd *pfd, int target_fd, short events) {
	pfd->fd = target_fd;
	pfd->events = events;
}

void safe_signal(int signum, void (*handler)()) {
	if (signal(signum, handler) == SIG_ERR) {
		fprintf(stderr, "Error setting signal: %s\n", strerror(errno));
		exit(1);
	}
}

void report_child_exit_status() {
	int status;
	safe_waitpid(child_pid, &status, 0);

	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(status), WEXITSTATUS(status));
}

void handle_parent_process(int pfd_forward[], int pfd_backward[]) {

	atexit(report_child_exit_status);
	safe_signal(SIGPIPE, handle_sigpipe);

	safe_close(pfd_forward[0]);
	safe_close(pfd_backward[1]);
	
	// pfds[0] listens for socket
	// pfds[1] listens for pfd_backward[0]
	struct pollfd pfds[2];
	add_observer(&pfds[0], new_sfd,         POLLIN | POLLHUP | POLLERR);
	add_observer(&pfds[1], pfd_backward[0], POLLIN | POLLHUP | POLLERR);

	while (1) {

		safe_poll(pfds, 2, -1);

		// read from socket
		if (pfds[0].revents & POLLIN) handle_read_from_socket(pfd_forward);

		// read from pfd_backward[0]
		if (pfds[1].revents & POLLIN) handle_read_from_child(pfd_backward);

		// stdin disconnected || backward pipe disconnected
		if ((pfds[0].revents & POLLHUP) || (pfds[1].revents & POLLHUP)) {
			read_leftover(pfd_backward);
			return;
		}

		// stdin error
		if (pfds[0].revents & POLLERR) {
			fprintf(stderr, "stdin POLLERR detected: %s\n", strerror(errno));
			return;
		}

		// backward pipe error
		if (pfds[1].revents & POLLERR) {
			fprintf(stderr, "backward pipe POLLERR detected: %s\n", strerror(errno));
			return;
		}

	}

}

int safe_fork() {
	int pid = fork();
	if (pid == -1) {
		fprintf(stderr, "Error forking the process: %s\n", strerror(errno));
		exit(1);
	}
	return pid;
}

void safe_pipe(int pipefd[2]) {
	if (pipe(pipefd) == -1) {
		fprintf(stderr, "Error creating pipe: %s\n", strerror(errno));
		exit(1);
	}
}

void handle_shell() {
	int pfd_forward[2];
	int pfd_backward[2];

	safe_pipe(pfd_forward);
	safe_pipe(pfd_backward);

	child_pid = safe_fork();

	// child process goes here
	if (child_pid == 0)
		handle_child_process(pfd_forward, pfd_backward);
	// parent process goes here
	else
		handle_parent_process(pfd_forward, pfd_backward);

}

void cleanup_deflate_stream() {
	// clean up dynaimcally allocated memory
	deflateEnd(&deflate_stream);
}

void safe_delfateInit(z_stream *strm, int level) {
	if (deflateInit(strm, level) != Z_OK) {
		fprintf(stderr, "Error initializing deflate stream\n");
		exit(1);
	}
}

void init_deflate_stream() {
	deflate_stream.zalloc = Z_NULL;
	deflate_stream.zfree  = Z_NULL;
	deflate_stream.opaque = Z_NULL;

	safe_delfateInit(&deflate_stream, Z_DEFAULT_COMPRESSION);
	
	atexit(cleanup_deflate_stream);
}

void cleanup_inflate_stream() {
	// clean up dynaimcally allocated memory
	inflateEnd(&inflate_stream);
}

void safe_inflateInit(z_stream *strm) {
	if (inflateInit(strm) != Z_OK) {
    	fprintf(stderr, "Error initializing inflate stream\n");
    	exit(1);
    }
}

void init_inflate_stream() {
	inflate_stream.zalloc = Z_NULL;
	inflate_stream.zfree  = Z_NULL;
	inflate_stream.opaque = Z_NULL;

	safe_inflateInit(&inflate_stream);

    atexit(cleanup_inflate_stream);
}

void init_compression_streams() {
	init_deflate_stream();
	init_inflate_stream();
}

void support_options(int argc, char **argv) {
	int option;

	struct option options[] = {
		{"port",     required_argument, NULL, 'p'},
		{"compress", no_argument,       NULL, 'c'},
		{0, 0, 0, 0}
	};

	while (1) {
		option = getopt_long(argc, argv, "", options, NULL);

		if (option == -1) break;

		switch (option) {
			case 'p':
				port_flag = 1;
				port = atoi(optarg);
				break;
			case 'c':
				compress_flag = 1;
				init_compression_streams();
				break;
			default:
				fprintf(stderr, "Unrecognized argument detected\n%s\n", getUsage());
				exit(1);
		}
	}

	if (!port_flag) {
		fprintf(stderr, "--port option is mandatory\n");
		exit(1);
	}
}



int safe_socket(int domain, int type, int protocol) {
	int sfd = socket(domain, type, protocol);
	if (sfd == -1) {
		fprintf(stderr, "Error opening socket: %s\n", strerror(errno));
		exit(1);
	}
	return sfd;
}

void safe_bind(int sfd, struct sockaddr *addr, int addrlen) {
	if (bind(sfd, addr, addrlen) == -1) {
		fprintf(stderr, "Error binding a name to a socket: %s\n", strerror(errno));
		exit(1);
	}
}

int safe_accept(int sfd, struct sockaddr *addr, unsigned int *addrlen) {
	int new_sfd = accept(sfd, addr, addrlen);
	if (new_sfd < 0) {
		fprintf(stderr, "Error accepting a connection on a socket: %s\n", strerror(errno));
		exit(1);
	}
	return new_sfd;
}


void handle_socket() {

	int sfd;
	int client_length; /* size of the address of the client */
	
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	sfd = safe_socket(AF_INET, SOCK_STREAM, 0);

	memset((char*) &server_address, 0, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port        = htons(port);

	safe_bind(sfd, (struct sockaddr*) &server_address, sizeof(server_address));

	listen(sfd, 1);

	client_length = sizeof(client_address);
	new_sfd = safe_accept(sfd, (struct sockaddr*) &client_address, (unsigned int*) &client_length);

}

int main(int argc, char **argv) {

	support_options(argc, argv);
	
	handle_socket();

	handle_shell();

	exit(0);


}