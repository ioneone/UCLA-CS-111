// Name  : Junhong Wang
// ID    : 504941113
// Email : oneone@g.ucla.edu

#include <termios.h>
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
#include <netdb.h>
#include <zlib.h>
#include <fcntl.h>
#include <stdio.h>

struct termios old_termios;

int port_flag     = 0;
int log_flag      = 0;
int compress_flag = 0;

int port   = -1;
int log_fd = -1;

z_stream deflate_stream;
z_stream inflate_stream;

void safe_tcgetattr(int fd, struct termios *termios_p) {
	if (tcgetattr(fd, termios_p) == -1) {
		fprintf(stderr, "Error getting terminal parameters: %s\n", strerror(errno));
		exit(1);
	}
}

void safe_tcsetattr(int fd, int action, struct termios *termios_p) {
	if (tcsetattr(fd, action, termios_p) == -1) {
		fprintf(stderr, "Error setting terminal parameters: %s\n", strerror(errno));
		exit(1);
	}
}

void save_terminal_params() {
	safe_tcgetattr(0, &old_termios);
}

void restore_terminal_params() {
	safe_tcsetattr(0, TCSANOW, &old_termios);
}

void verify_stdin_is_terminal() {
	if (!isatty(0)) {
		fprintf(stderr, "stdin is not associated with an open terminal device: %s\n", strerror(errno));
		exit(1);
	}
}

void switch_to_noncanonical_mode() {
	struct termios new_termios;

	safe_tcgetattr(0, &new_termios);

	new_termios.c_iflag = ISTRIP; /* only lower 7 bits	*/
	new_termios.c_oflag = 0;      /* no processing	    */
	new_termios.c_lflag = 0;      /* no processing      */

	safe_tcsetattr(0, TCSANOW, &new_termios);

}

int safe_read(int fd, void *buf, int size) {
	int ret = read(fd, buf, size);
	if (ret == -1) {
		fprintf(stderr, "Error reading stdin: %s\n", strerror(errno));
		exit(1);
	}

	return ret;
}

int safe_write(int fd, void *buf, int size) {
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
Usage: ./lab1b-client --port=number [OPTION]...\n\
The client program will open a connection to a server\n\
(port specified with the mandatory --port=command line parameter)\n\
rather than sending it directly to a shell. The client should then \n\
send input from the keyboard to the socket (while echoing to the display),\n\
and input from the socket to the display.\n\
\n\
	--port=number   specify the port number for connection\n\
	--log=filename  maintain a record of data sent over the socket\n\
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


void handle_read_from_child(int pfd_backward[]) {
	char buf[256];
	int ret = safe_read(pfd_backward[0], buf, 256);
	int i;
	for (i = 0; i < ret; i++) {

		if (buf[i] == '\n') {
			safe_write(1, "\r\n", 2);
			continue;
		}

		safe_write(1, &buf[i], 1);

	}
}

void read_leftover_from_child(int pfd_backward[]) {
	char buf[256];
	int ret;
	while ((ret = safe_read(pfd_backward[0], buf, 256)) != 0) {
		int i;
		for (i = 0; i < ret; i++) {

			if (buf[i] == '\n') {
				safe_write(1, "\r\n", 2);
				continue;
			}

			safe_write(1, &buf[i], 1);

		}
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

int safe_socket(int domain, int type, int protocol) {
	int sfd = socket(domain, type, protocol);
	if (sfd == -1) {
		fprintf(stderr, "Error opening socket: %s\n", strerror(errno));
		exit(1);
	}
	return sfd;
}

struct hostent* safe_gethostbyname(char *name) {
	struct hostent *server = gethostbyname(name);
	if (server == NULL) {
		fprintf(stderr, "Error finding the host: %s\n", strerror(h_errno));
		exit(1);
	}
	return server; 
}

void safe_connect(int sockfd, struct sockaddr *addr, socklen_t addrlen) {
	if (connect(sockfd, addr, addrlen) == -1) {
		fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
		exit(1);
	}
}

int handle_socket() {
	int sfd;
	struct hostent *server;
	struct sockaddr_in server_address;

	sfd = safe_socket(AF_INET, SOCK_STREAM, 0);

	server = safe_gethostbyname("localhost");

	memset((char*) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	memcpy((char*) &server_address.sin_addr.s_addr, (char*) server->h_addr, server->h_length);
	server_address.sin_port = htons(port);

	safe_connect(sfd, (struct sockaddr*) &server_address, sizeof(server_address));

	return sfd;
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

void simple_dprintf(int fd, char *format, int n) {
	if (dprintf(fd, format, n) == -1) {
		fprintf(stderr, "Error using dprintf: %s\n", strerror(errno));
		exit(1);
	}
}

void log_bytes_sent(char *buf, int ret) {
	simple_dprintf(log_fd, "SENT %d bytes: ", ret);
	safe_write(log_fd, buf, ret);
	safe_write(log_fd, "\n", 1);
}

void log_bytes_received(char *buf, int ret) {
	simple_dprintf(log_fd, "RECEIVED %d bytes: ", ret);
	safe_write(log_fd, buf, ret);
	safe_write(log_fd, "\n", 1);
}

// handles <cr><lf> translation
void safe_terminal_write(int fd, char *buf, int ret) {
	int i;

	for (i = 0; i < ret; i++) {

		if (buf[i] == '\r' || buf[i] == '\n')
			safe_write(fd, "\r\n", 2);
		else
			safe_write(fd, &buf[i], 1);

	}
}

void handle_read_from_keyboard(int sfd) {
	char buf[256];
	int ret;

	ret = safe_read(0, buf, 256);
			
	safe_terminal_write(1, buf, ret);

	if (compress_flag) simple_deflate(buf, &ret);

	if (log_flag) log_bytes_sent(buf, ret);

	safe_write(sfd, buf, ret);
}

int handle_read_from_socket(int sfd) {
	char buf[2048]; /* reserve enough space for decompression */
	int ret;

	ret = safe_read(sfd, buf, 256);

	// socked is closed
	if (ret == 0) return ret;

	if (log_flag) log_bytes_received(buf, ret);

	if (compress_flag) simple_inflate(buf, &ret);

	safe_terminal_write(1, buf, ret);

	return ret;
}

void read_leftover(int sfd) {

	char buf[2048]; /* reserve enough space for decompression */
	int ret;

	while ((ret = safe_read(sfd, buf, 256)) != 0) {

		if (log_flag) log_bytes_received(buf, ret);

		if (compress_flag) simple_inflate(buf, &ret);
		
		safe_terminal_write(1, buf, ret);

	}

}

void handle_port_option() {

	int sfd;
	struct pollfd pfds[2];

	sfd = handle_socket();
	
	add_observer(&pfds[0], 0,   POLLIN | POLLHUP | POLLERR);
	add_observer(&pfds[1], sfd, POLLIN | POLLHUP | POLLERR);
	
    
	while (1) {

		safe_poll(pfds, 2, -1);

		// read from keyboard (stdin)
		if (pfds[0].revents & POLLIN) handle_read_from_keyboard(sfd);

		// read from socket
		if ((pfds[1].revents & POLLIN)) {

			int ret = handle_read_from_socket(sfd);

			// socket is closed
			if (ret == 0) return;
		}

		// stdin disconnected || socket disconnected
		if ((pfds[0].revents & POLLHUP) || (pfds[1].revents & POLLHUP)) {
			read_leftover(sfd);
			return;
		}

		// stdin error
		if (pfds[0].revents & POLLERR) {
			fprintf(stderr, "stdin POLLERR detected: %s\n", strerror(errno));
			return;
		}

		// socket error
		if (pfds[1].revents & POLLERR) {
			fprintf(stderr, "socket POLLERR detected: %s\n", strerror(errno));
			return;
		}

	}

}

int safe_creat(char *pathname, int flags) {
	int fd = creat(pathname, flags);
	if (fd == -1) {
		fprintf(stderr, "Error creating a file: %s\n", strerror(errno));
		exit(1);
	}
	return fd;
}

void support_options(int argc, char **argv) {
	int option;

	struct option options[] = {
		{"port",     required_argument, NULL, 'p'},
		{"log",      required_argument, NULL, 'l'},
		{"compress", no_argument      , NULL, 'c'},
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
			case 'l':
				log_flag = 1;
				log_fd = safe_creat(optarg, 0644);
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

int main(int argc, char **argv) {

	support_options(argc, argv);
	
	save_terminal_params();
	atexit(restore_terminal_params);

	verify_stdin_is_terminal();

	switch_to_noncanonical_mode();

	handle_port_option();

	exit(0);


}
