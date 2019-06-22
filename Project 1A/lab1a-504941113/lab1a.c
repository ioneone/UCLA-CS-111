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

struct termios old_termios;
int child_pid;
int shell_flag = 0;
int debug_flag = 0;

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

void lazy_write() {

	char buf;

	while (1) {

		safe_read(0, &buf, 1);

		if (buf == '\004') break; /* C-d (EOF) */

		if (buf == '\r' || buf == '\n') {
			safe_write(1, "\r\n", 2);
			continue;
		}
		
		safe_write(1, &buf, 1);

	}

}

char* getUsage() {
	return 
		"\
Usage: ./lab1a [OPTION]...\n\
put the keyboard (the file open on file descriptor 0)\n\
into character-at-a-time, no-echo mode\n\
(also known as non-canonical input mode with no echo)\n\
\n\
  --shell  pass input/output between the terminal and a shell\n\
  --debug  run program in debug mode\n\
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

void handle_read_from_keyboard(int pfd_forward[]) {
	char buf[256];
	int ret = safe_read(0, buf, 256);
	int i;

	for (i = 0; i < ret; i++) {

		if (buf[i] == '\004') {
			if (debug_flag) safe_write(1, "^D", 2);
			close(pfd_forward[1]);
			continue;
		}

		if (buf[i] == '\003') {
			if (debug_flag) safe_write(1, "^C", 2);
			safe_kill(child_pid, SIGINT);
			continue;
		}

		if (buf[i] == '\r' || buf[i] == '\n') {
			safe_write(1, "\r\n", 2);
			safe_write(pfd_forward[1], "\n", 1);
			continue;
		}

		safe_write(1, &buf[i], 1);
		safe_write(pfd_forward[1], &buf[i], 1);

	}
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

void handle_parent_process(int pfd_forward[], int pfd_backward[]) {
	safe_close(pfd_forward[0]);
	safe_close(pfd_backward[1]);

	safe_signal(SIGPIPE, handle_sigpipe);

	// pfds[0] listens for stdin
	// pfds[1] listens for pfd_backward[0]
	struct pollfd pfds[2];
	add_observer(&pfds[0], 0,               POLLIN | POLLHUP | POLLERR);
	add_observer(&pfds[1], pfd_backward[0], POLLIN | POLLHUP | POLLERR);

	while (1) {

		safe_poll(pfds, 2, -1);

		// read from keyboard (stdin)
		if (pfds[0].revents & POLLIN) handle_read_from_keyboard(pfd_forward);

		// read from pfd_backward[0]
		if (pfds[1].revents & POLLIN) handle_read_from_child(pfd_backward);

		// stdin disconnected || backward pipe disconnected
		if ((pfds[0].revents & POLLHUP) || (pfds[1].revents & POLLHUP)) {
			read_leftover_from_child(pfd_backward);
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

void handle_shell_option() {
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

void support_options(int argc, char **argv) {
	int option;

	struct option options[] = {
		{"shell", no_argument, NULL, 's'},
		{"debug", no_argument, NULL, 'd'},
		{0, 0, 0, 0}
	};

	while (1) {
		option = getopt_long(argc, argv, "", options, NULL);

		if (option == -1) break;

		switch (option) {
			case 's':  // --shell
				shell_flag = 1;
				break;
			case 'd':
				debug_flag = 1;
				break;
			default:
				fprintf(stderr, "Unrecognized argument detected\n%s\n", getUsage());
				exit(1);
		}
	}

}

void report_child_exit_status() {
	if (!shell_flag) return;
	int status;
	safe_waitpid(child_pid, &status, 0);

	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(status), WEXITSTATUS(status));
}

void handle_exit() {
	restore_terminal_params();
	report_child_exit_status();
}

int main(int argc, char **argv) {

	support_options(argc, argv);
	
	save_terminal_params();
	atexit(handle_exit);

	verify_stdin_is_terminal();

	switch_to_noncanonical_mode();

	if (shell_flag) {
		handle_shell_option();
		exit(0);
	}

	lazy_write();
	exit(0);


}
