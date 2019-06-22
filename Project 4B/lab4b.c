// Name  : Junhong Wang
// ID    : 504941113
// Email : oneone@g.ucla.edu

#include <mraa.h>
#include <mraa/aio.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include <math.h>
#include <errno.h>

int period = 1;
char scale = 'F';

int opt_log = 0;
int log_fd;

struct pollfd pfd;

mraa_aio_context sensor;
mraa_gpio_context button;

int is_time_to_report = 0;

char command[128];
int ptr = 0;

int is_report_stopped = 0;

time_t timer_start, timer_end;

char* get_usage() {
	return 
		"\
Usage: ./lab4b [OPTION]...\n\
uses the AIO functions of the MRAA library\n\
to get readings from your temperature sensor.\n\
\n\
  --period=NUMBER      specifies a sampling interval in seconds\n\
                       (defaulting to 1).\n\
  --scale=TYPE         converts the sensor value into a temperature\n\
                       (default F)\n\
                       F indicates Fahrenheit\n\
                       C indicates Celsius\n\
  --log=FILENAME       appends the report to a logfile\n\
\n\
During runtime, the program accepts the following commands from stdin:\n\
\n\
  SCALE=F              This command should cause all subsequent reports\n\
                       to be generated in degrees Fahrenheit.\n\
  SCALE=C              This command should cause all subsequent reports\n\
                       to be generated in degrees Centegrade\n\
  PERIOD=seconds       This command should change the number of seconds\n\
                       between reporting intervals. It is acceptable if\n\
                       this command does not take effect until after the\n\
                       next report.\n\
  STOP                 This command should cause the program to stop\n\
                       generating reports, but continue processing input\n\
                       commands. If the program is not generating reports,\n\
                       merely log receipt of the command.\n\
  START                This command should cause the program to, if\n\
                       stopped, resume generating reports. If the program\n\
                       is not stopped, merely log receipt of the command.\n\
  LOG line of text     This command requires no action beyond logging its\n\
                       receipt (the entire command, including the LOG).\n\
  OFF                  This command should, like pressing the button,\n\
                       output (and log) a time-stamped SHUTDOWN message\n\
                       and exit.\n\
		";
}

void handle_unrecognized_argument() {
	fprintf(stderr, "Unrecognized argument detected\n%s\n", get_usage());
	exit(1);
}

void handle_invalid_argument(char *message) {
	fprintf(stderr, "Invalid argument detected: %s\n", message);
	exit(1);
}

int safe_creat(char *pathname, int flags) {
	int fd = creat(pathname, flags);
	if (fd == -1) {
		fprintf(stderr, "Error creating a file: %s\n", strerror(errno));
		exit(1);
	}
	return fd;
}

int is_scale_valid() {
	return scale == 'F' || scale == 'C';
}

void handle_period_option() {
	period = atoi(optarg);
	if (period < 1) handle_invalid_argument("period cannot be less than 1");
}

void handle_scale_option() {
	scale = optarg[0];
	if (!is_scale_valid()) handle_invalid_argument("scale should be F or C");
}

void handle_log_option() {
	opt_log = 1;
	log_fd = safe_creat(optarg, 0644);
}

double to_fahrenheit(double celsius) {
	return celsius * 9/5 + 32;
}

double to_temperature(int raw_sensor_value) {
	int b = 4275;
	int r0 = 100000;
	double r = 1023 / (double) raw_sensor_value - 1.0;
	r *= r0;

	double temperature = 1.0/(log(r/r0)/b + 1/298.15) - 273.15;

	return (scale == 'F') ? to_fahrenheit(temperature) : temperature;
	
}

double get_temperature() {
	return to_temperature(mraa_aio_read(sensor));
}

void handle_options(int argc, char **argv) {
	int option;

	struct option options[] = {
		{"period", required_argument, NULL, 'p'},
		{"scale",  required_argument, NULL, 's'},
		{"log",    required_argument, NULL, 'l'},
		{0, 0, 0, 0}
	};

	while (1) {

		option = getopt_long(argc, argv, "", options, NULL);

		if (option == -1) break;

		switch (option) {
			case 'p':
				handle_period_option();
				break;
			case 's':
				handle_scale_option();
				break;
			case 'l':
				handle_log_option();
				break;
			default:
				handle_unrecognized_argument();

		}

	}
}

struct tm* get_current_localtime() {
	time_t raw_time;
	time(&raw_time);
	return localtime(&raw_time);
}

void initialize_timer() {
	time(&timer_start);
}

void update_timer() {
	time(&timer_end);
	if (difftime(timer_end, timer_start) >= period) {
		is_time_to_report = 1;
		time(&timer_start);
	} else {
		is_time_to_report = 0;
	}
}

void safe_poll(struct pollfd fds[], int nfds, int timeout) {
	if (poll(fds, nfds, timeout) == -1) {
		fprintf(stderr, "Error while polling: %s\n", strerror(errno));
		exit(1);
	}
}

int safe_read(int fd, void *buf, int size) {
	int ret = read(fd, buf, size);
	if (ret == -1) {
		fprintf(stderr, "Error reading stdin: %s\n", strerror(errno));
		exit(1);
	}

	return ret;
}

void handle_report() {
	double temperature = get_temperature();
	struct tm *current_localtime = get_current_localtime();
	char *format = "%02d:%02d:%02d %.1f\n";
	
	fprintf(stdout, format, current_localtime->tm_hour, current_localtime->tm_min, current_localtime->tm_sec, temperature);
	if (opt_log) dprintf(log_fd, format, current_localtime->tm_hour, current_localtime->tm_min, current_localtime->tm_sec, temperature);
}

void handle_shutdown() {
	struct tm *current_localtime = get_current_localtime();
	char *format = "%02d:%02d:%02d SHUTDOWN\n";
	
	fprintf(stdout, format, current_localtime->tm_hour, current_localtime->tm_min, current_localtime->tm_sec);
	if (opt_log) dprintf(log_fd, format, current_localtime->tm_hour, current_localtime->tm_min, current_localtime->tm_sec);
}

int is_button_pressed() {
	int ret = mraa_gpio_read(button);
	if (ret == -1) {
		fprintf(stderr, "There was a fatal error reading the button's GPIO value\n");
		exit(1);
	}
	return ret;
}

void initialize_poll() {
	pfd.fd = 0;
	pfd.events = POLLIN | POLLHUP | POLLERR;
}

void clear_command() {
	memset(command, 0, 128);
	ptr = 0;
}

int is_command_equal_to(char *str) {
	return strcmp(command, str) == 0;
}

int is_command_start_with(char *str) {
	return strncmp(command, str, strlen(str)) == 0;
}

void process_command() {

	if (opt_log) dprintf(log_fd, "%s\n", command);

	if (is_command_equal_to("SCALE=F")) {
		scale = 'F';
	}
	else if (is_command_equal_to("SCALE=C")) {
		scale = 'C';
	}
	else if (is_command_start_with("PERIOD=")) {
		char *seconds = command + strlen("PERIOD=");
		period = atoi(seconds);
	}
	else if (is_command_equal_to("STOP")) {
		is_report_stopped = 1;
	}
	else if (is_command_equal_to("START")) {
		is_report_stopped = 0;
	}
	else if (is_command_start_with("LOG ")) {
		// do nothing
	}
	else if (is_command_equal_to("OFF")) {
		handle_shutdown();
		exit(0);
	}

	clear_command();

}

void extract_commands(char *buf, int size) {
	int i;

	for (i = 0; i < size; i++) {
		if (buf[i] == '\n') 
			process_command();
		else
			command[ptr++] = buf[i];
	}
}

void handle_poll() {
	char buf[128];

	safe_poll(&pfd, 1, 0);

	if (pfd.revents & POLLIN) {		
		int ret = safe_read(0, buf, 128);
		// do not assume that one call to read(2) will return one command.
		extract_commands(buf, ret);
	}

	if (pfd.revents & POLLHUP) exit(0);

	if (pfd.revents & POLLERR) {
		fprintf(stderr, "stdin POLLERR detected: %s\n", strerror(errno));
		exit(1);
	}
}

void clean_up() {
	if (opt_log) close(log_fd);
	mraa_aio_close(sensor);
	mraa_gpio_close(button);
}

void initialize_button() {
	// initialize MRAA pin 60 (GPIO_50) for button
	button = mraa_gpio_init(60);

	// configure GPIO interface to be an input pin
	mraa_gpio_dir(button, MRAA_GPIO_IN);
}

void initialize_sensor() {
	// initialize MRAA pin 1 (Analog A0/A1) for sensor
	sensor = mraa_aio_init(1);
}

void run() {

	while (1) {

		update_timer();

		if (is_time_to_report && !is_report_stopped) handle_report();

		if (is_button_pressed()) {
			handle_shutdown();
			break;
		}

		handle_poll();

	}
}

void initialize() {
	initialize_sensor();

	initialize_button();

	initialize_poll();
	
	initialize_timer();

	atexit(clean_up);
}

void prepare(int argc, char **argv) {
	handle_options(argc, argv);

	initialize();
}

int main(int argc, char **argv) {

	prepare(argc, argv);
	
	run();

	exit(0);

}