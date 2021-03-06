#ifndef PHP_TRACER_PUBLIC_H
#define PHP_TRACER_PUBLIC_H

#include "ext/standard/php_smart_str.h"	
#include <uuid/uuid.h>

void smart_str_wrap_quotes_s(smart_str *str,const char* content);
//void smart_str_wrap_quotes_l(smart_str *str,long content);
void smart_str_wrap_quotes_sc(smart_str *str,const char* content);
void smart_str_wrap_quotes_lc(smart_str *str,long content);
void generate_uuid(char* str);
int get_time_zone();
void  convert_ts_tz(smart_str *str,struct timeval in_time);
char* convert_str_pp(zval ** val);
ulong convert_l_pp(zval ** val);
//zval *debug_backtrace_get_args(void ***curpos TSRMLS_DC);
//void debug_print_backtrace_args(zval *arg_array TSRMLS_DC, smart_str *trace_str);
#endif	/* PHP_TRACER_PUBLIC_H */
