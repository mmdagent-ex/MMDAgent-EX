#ifndef __MMDAGENT_PLUGIN_REMOTE_ADIN_FILE_H__

#define __MMDAGENT_PLUGIN_REMOTE_ADIN_FILE_H__

#include <sent/stddefs.h>
#include <sent/speech.h>
#include <sent/adin.h>

FILE *adin_file_open(const char *filename);
int adin_file_read(FILE *fp, SP16 *buf, int sampnum);
boolean adin_file_close(FILE *fp);

#endif /* __MMDAGENT_PLUGIN_REMOTE_ADIN_FILE_H__ */
