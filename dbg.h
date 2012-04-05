//#define DEBUG

#ifdef DEBUG 
#define DBG(...) printf("" __VA_ARGS__) 
#else 
#define DBG(...) 
#endif 
