#ifndef DAEMON_H
#define DAEMON_H

int start(char *pid_path,char *server_conf);
int stop(char *pid_path);
int restart(char *pid_path,char* server_conf);

#endif /* !DAEMON_H */
