#ifndef LOGGER_H
#define LOGGER_H

void log_message(const char *message);

void close_log_file(void);

void setup_logging(int log_to_stdout, const char *log_file_path);
#endif /* !LOGGER_H*/
