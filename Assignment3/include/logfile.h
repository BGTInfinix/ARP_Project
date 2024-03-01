#ifndef LOGFILE_H
#define LOGFILE_H

void writeToLogFile(const char *logpath, const char *logMessage, const char *sender);
void clearLogFile(const char *logpath);

#endif /* LOGFILE_H */