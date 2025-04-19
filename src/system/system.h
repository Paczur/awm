#ifndef H_AWM_SYSTEM
#define H_AWM_SYSTEM

int system_run(const char *cmd);
int system_run_thread(const char *cmd);
void system_run_bg(const char *cmd);
void system_run_bg_raw(const char *cmd);
#define SYSTEM_RUN_BG(cmd) system_run_bg_raw(cmd " >/dev/null 2>&1");

#endif
