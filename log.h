#ifndef H_LOG
#define H_LOG

#ifdef DEBUG
#define DLOG for(int _i=1; _i; _i=0, fflush(stdout))
#endif

#endif
