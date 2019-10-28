#ifdef printf
#undef printf
#endif
#define printf error_no_stdout_please

#ifdef puts
#undef puts
#endif
#define puts error_no_stdout_please

#ifdef putchar
#undef putchar
#endif
#define putchar error_no_stdout_please
