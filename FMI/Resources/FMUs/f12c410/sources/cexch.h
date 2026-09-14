#ifndef EXCEPTION_HANDLING
#define EXCEPTION_HANDLING

#if defined(__cplusplus)
extern "C" {
#endif

#include <setjmp.h>
#define ERR_RECURSIVE -123456

#if !defined(DYMOLA_STATIC)
#if defined(__cplusplus)
#define DYMOLA_STATIC extern
#elif defined(RT) && !defined(DYM2DS)
#define DYMOLA_STATIC static
#else
#define DYMOLA_STATIC
#endif
#endif

DYMOLA_STATIC int* getExceptionCounter();
DYMOLA_STATIC jmp_buf* getExceptionBuffer();

#define EXCEPTION_TRY do {int* exception_count =getExceptionCounter(); jmp_buf* exception_buffer = getExceptionBuffer();  int exception_code = 0; if(exception_count && exception_buffer){ if(*exception_count == 10 ) longjmp(exception_buffer[*exception_count-1], ERR_RECURSIVE ); if ((exception_code = setjmp(exception_buffer[(*exception_count)++])) == 0) {
#define EXCEPTION_THROW(code) {int* exception_count =getExceptionCounter(); jmp_buf* exception_buffer = getExceptionBuffer(); if(exception_count && exception_buffer && *exception_count)longjmp(exception_buffer[*exception_count-1], code ); }
#define EXCEPTION_CATCH(code) } else if (code == exception_code) {
#define EXCEPTION_CATCH_ALL } else {
#define EXCEPTION_END } --(*exception_count);} } while(0)

#if defined __cplusplus
}
#endif

#endif

