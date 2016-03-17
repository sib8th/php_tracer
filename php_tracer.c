/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_php_tracer.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "slog.h"
/* If you declare any globals in php_php_tracer.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(php_tracer)
*/

/* True global resources - no need for thread safety here */
static int le_php_tracer;

/* {{{ php_tracer_functions[]
 *
 * Every user visible function must have an entry in php_tracer_functions[].
 */
const zend_function_entry php_tracer_functions[] = {
	PHP_FE(confirm_php_tracer_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in php_tracer_functions[] */
};
/* }}} */

/* {{{ php_tracer_module_entry
 */
zend_module_entry php_tracer_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"php_tracer",
	php_tracer_functions,
	PHP_MINIT(php_tracer),
	PHP_MSHUTDOWN(php_tracer),
	PHP_RINIT(php_tracer),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(php_tracer),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(php_tracer),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PHP_TRACER
ZEND_GET_MODULE(php_tracer)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("php_tracer.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_php_tracer_globals, php_tracer_globals)
    STD_PHP_INI_ENTRY("php_tracer.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_php_tracer_globals, php_tracer_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_php_tracer_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_php_tracer_init_globals(zend_php_tracer_globals *php_tracer_globals)
{
	php_tracer_globals->global_value = 0;
	php_tracer_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */


#if PHP_VERSION_ID >= 50500
static void (*old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
static void tracer_execute_ex(zend_execute_data *execute_data TSRMLS_DC);
#else
static void (*old_execute)(zend_op_array *op_array TSRMLS_DC);
static void tracer_execute(zend_op_array *op_array TSRMLS_DC);
#endif
static void op_array_traverse(zend_op_array *op_array);
static void (*old_execute_internal)(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC);
static void tracer_execute_internal(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC);


clock_t start,end;
PHP_MINIT_FUNCTION(php_tracer)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	slog_init("/home/sibylla/php/logs/php_tracer","/home/sibylla/php/slog.cfg",2,3,1);
		
#if PHP_VERSION_ID>=50500
	old_execute_ex = zend_execute_ex;
	zend_execute_ex = tracer_execute_ex;
#else
	old_execute = zend_execute;
	zend_execute = tracer_execute;
#endif


	old_execute_internal = zend_execute_internal;
	zend_execute_internal = tracer_execute_internal;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(php_tracer)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/

	
#if PHP_VERSION_ID>=50500
	zend_execute_ex = old_execute_ex;
#else
	zend_execute = old_execute;
#endif




	zend_execute_internal = old_execute_internal;

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(php_tracer)
{
/*	
	old_execute = zend_execute;
	zend_execute = tracer_execute;
*/


	start = clock();
	slog(2,SLOG_INFO,"-------------Request Start:  %d, loops: %d per sec\n<br/>",start,CLOCKS_PER_SEC);
	return SUCCESS;

}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(php_tracer)
{

	//zend_execute = old_execute;


	end = clock();
	slog(2,SLOG_INFO,"--------------Request End:  %d,",end);
	int interval = (end - start) ;
	slog(2,SLOG_INFO," pass: %d, interval: %f\n<br/>",interval,interval/CLOCKS_PER_SEC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(php_tracer)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "php_tracer support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


static void tracer_execute(zend_op_array *op_array TSRMLS_DC)
{
	 
	op_array_traverse(op_array);
	// File *fp;
	// fp = fopen("php://output");
	// fprintf(fp,"executed");
	// fclose(fp);
	old_execute(op_array TSRMLS_CC);
	
}

static void tracer_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
{

	time_t time_start;
	// struct tm *tm_start;
	// char *datetime;
	// time(&time_start);
	// tm_start = localtime(&time_start);
	// datetime = asctime(tm_start);

	slog(2,SLOG_INFO,">>>>>>>Execute start: %d  time: %s  \n<br/>",clock(),ctime(&time_start));

	op_array_traverse(execute_data->op_array);


	// File *fp;
	// fp = fopen("php://output");
	// fprintf(fp,"executed");
	// fclose(fp);
	old_execute_ex(execute_data TSRMLS_CC);
	slog(2,SLOG_INFO,">>>>>>>Execute end: %d\n<br/>",clock());
}

static void op_array_traverse(zend_op_array *op_array) {


	int size = op_array->last;

	int i;

	slog(2,SLOG_INFO,"opcode count: %d\n<br/>",size);

	zend_op *opcodes = op_array->opcodes;

	for(i = 0; i <= size; i++) {

		slog(2,SLOG_INFO,"%d: %d\n<br/>",i,opcodes[i].opcode);
			
	}

	

}
static void tracer_execute_internal(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC)
{


	 clock_t start_internal = clock();
	 slog(2,SLOG_INFO,">>>>>>>Internal function start: %d\n<br/>",start_internal);
	 
         zend_op_array *op_array = execute_data_ptr->op_array;
         
         if(op_array->function_name != NULL)
		 slog(2,SLOG_INFO,"function name: %s",op_array->function_name);
	 else {

		 slog(2,SLOG_INFO,"function name: NULL");
	 }

	 if(execute_data_ptr->function_state.function != NULL) {
		 slog(2,SLOG_INFO,"function_state.function: NULL");
	 }
	 else {
		 slog(2,SLOG_INFO,"function_state.function: NULL");

	 }	 

	 slog(2,SLOG_INFO,"execute_internal\n<br/>");
	 //op_array_traverse(execute_data_ptr->op_array);

	 old_execute_internal(execute_data_ptr,return_value_used TSRMLS_CC);

	 clock_t end_internal = clock();
	 slog(2,SLOG_INFO,">>>>>>>Internal function end: %d, ",end_internal); 
	 clock_t interval_internal = end_internal - start_internal;
	 slog(2,SLOG_INFO,"pass: %d, interval: %f\n<br/>",interval_internal,interval_internal/CLOCKS_PER_SEC);

}




/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_php_tracer_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_php_tracer_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "php_tracer", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
