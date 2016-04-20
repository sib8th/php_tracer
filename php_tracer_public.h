#ifndef PHP_TRACER_PUBLIC_H
#define PHP_TRACER_PUBLIC_H

#include "ext/standard/php_smart_str.h"	


void smart_str_wrap_quotes_s(smart_str *str,const char* content);
void smart_str_wrap_quotes_l(smart_str *str,long content);
void smart_str_wrap_quotes_sc(smart_str *str,const char* content);
void smart_str_wrap_quotes_lc(smart_str *str,long content);


#endif	/* PHP_TRACER_PUBLIC_H */
