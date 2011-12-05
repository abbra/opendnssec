#ifndef _POLICY_POLICY_RESALT_CMD_H_
#define _POLICY_POLICY_RESALT_CMD_H_

#include "daemon/engine.h"

#ifdef __cplusplus
extern "C" {
#endif

void help_policy_resalt_cmd(int sockfd);

int handled_policy_resalt_cmd(int sockfd, engine_type* engine, const char *buf,
                              ssize_t n);

#ifdef __cplusplus
}
#endif

#endif