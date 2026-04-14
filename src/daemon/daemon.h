#ifndef DAEMON_H
#define DAEMON_H

int daemon_start(const char *pid_file);
int daemon_stop(const char *pid_file);

#endif // DAEMON_H
