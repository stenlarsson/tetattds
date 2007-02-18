#ifdef _DEBUG
#define ASSERT(x) if(!(x)) {\
	printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
	exit(1);\
}
#else
#define ASSERT(x) if(!(x)) {\
	printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
}
#endif