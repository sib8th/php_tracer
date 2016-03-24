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
extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include <stdio.h>
#include <string.h>
//#include <time.h>
#include <stdlib.h>  
}
#include "slog.h"
#include "php_php_tracer.h"
/* If you declare any globals in php_php_tracer.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(php_tracer)


/* True global resources - no need for thread safety here */
static int le_php_tracer;

/* {{{ php_tracer_functions[]
 *
 * Every user visible function must have an entry in php_tracer_functions[].
 */

// const zend_function_entry php_tracer_functions[] = {
// 	PHP_FE(confirm_php_tracer_compiled,	NULL)		 //For testing, remove later. 
// 	PHP_FE_END	/* Must be the last line in php_tracer_functions[] */
// };

/* }}} */

/* {{{ php_tracer_module_entry
 */
zend_module_entry php_tracer_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"php_tracer",
	//php_tracer_functions,
	NULL,
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

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("php_tracer.module_start",      "0", PHP_INI_ALL, OnUpdateLong, module_start, zend_php_tracer_globals, php_tracer_globals)
    STD_PHP_INI_ENTRY("php_tracer.module_end",     "0", PHP_INI_ALL, OnUpdateLong, module_end, zend_php_tracer_globals, php_tracer_globals)
PHP_INI_END()
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
#define TRACER_CREATE_FCALL() (tracer_fcall_entry *) malloc(sizeof(tracer_fcall_entry))

static void php_php_tracer_init_globals(zend_php_tracer_globals *php_tracer_globals)
{
	php_tracer_globals->module_start = 0;
	php_tracer_globals->module_end = 0;
	php_tracer_globals->fcalls = TRACER_CREATE_FCALL();
	php_tracer_globals->current_fcall = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */

/*
#if PHP_VERSION_ID >= 50500
*/




static void (*old_execute_ex)(zend_execute_data *execute_data TSRMLS_DC);
static void tracer_execute_ex(zend_execute_data *execute_data TSRMLS_DC);
static void (*old_execute_internal)(zend_execute_data *execute_data_ptr, zend_fcall_info *fci,int return_value_used TSRMLS_DC);
static void tracer_execute_internal(zend_execute_data *execute_data_ptr, zend_fcall_info *fci,int return_value_used TSRMLS_DC);
static zend_op_array* (*old_compile_string)(zval *source_string, char *filename TSRMLS_DC);
static zend_op_array* (*tracer_compile_string)(zval *source_string, char *filename TSRMLS_DC);
static void iterative_print_trace(tracer_fcall_entry *entry);





/*
#else
static void (*old_execute)(zend_op_array *op_array TSRMLS_DC);
static void tracer_execute(zend_op_array *op_array TSRMLS_DC);
#endif

//static void op_array_traverse(zend_op_array *op_array);
//static void test_function(zend_execute_data *execute_data_ptr TSRMLS_DC);
static void (*old_execute_internal)(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC);
static void tracer_execute_internal(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC);

*/


static clock_t request_start,request_end;
static int function_level;









PHP_MINIT_FUNCTION(php_tracer)
{
	/* If you have INI entries, uncomment these lines */
	REGISTER_INI_ENTRIES();
	
	


	// PHP_TRACER_G(module_start) = clock();
	// slog(2,SLOG_INFO,"-------------MODULE START-------------");

	string home_prefix = string(getenv("HOME"));
	slog_init((home_prefix + "/php/logs/php_tracer").c_str(),(home_prefix+"/php/slog.cfg").c_str(),2,3,1);
	//slog_init("/home/liangzx/php/logs/php_tracer","/home/liangzx/php/slog.cfg",2,3,1);


	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(php_tracer)
{
	/* uncomment this line if you have INI entries*/
	UNREGISTER_INI_ENTRIES();
	

	// PHP_TRACER_G(module_end) = clock();

	// int interval = (PHP_TRACER_G(module_end) - PHP_TRACER_G(module_start) ;
	// slog(2,SLOG_INFO,"-------------MODULE END------------- pass: %d, interval: %f\n<br/>",interval,interval/CLOCKS_PER_SEC);
	



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

	request_start = clock();
	slog(2,SLOG_INFO,"-------------Request Start-------------");



/*
#if PHP_VERSION_ID>=50500
*/
	old_execute_ex = zend_execute_ex;
	zend_execute_ex = tracer_execute_ex;
/*
#else
	old_execute = zend_execute;
	zend_execute = tracer_execute;
#endif
*/

	old_execute_internal = zend_execute_internal;
	zend_execute_internal = tracer_execute_internal;

	TRACER_G(current_fcall) = TRACER_G(fcalls);
	TRACER_G(fcalls)->pre_fcall = NULL;

	function_level = 0;




	return SUCCESS;



}
/* }}} */

static void iterative_print_trace(tracer_fcall_entry *entry)
{

	slog(2,SLOG_INFO,"start: %d, end: %d, interval: %f, scope_name: %s, type: %d",
		entry->data.start,entry->data.end,entry->data.interval,(entry->data.scope_name).c_str(),entry->data.type);

	if((entry->fcall_list).empty()) return;

	vector<tracer_fcall_entry *>::iterator itr;
	for(itr = entry->fcall_list.begin();itr!=entry->fcall_list.end();itr++)
	{
		iterative_print_trace(*itr);
	}
}


/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(php_tracer)
{

	//zend_execute = old_execute;
	tracer_fcall_entry *entry =  TRACER_G(fcalls);
	slog(2,SLOG_INFO,"----------------------TRACE-----------------------");
   
	

  

	request_end = clock();
	int request_interval = (request_end - request_start) ;

	slog(2,SLOG_INFO,"-------------Request END------------- pass loops: %d, interval: %f\n<br/>",request_interval,request_interval/CLOCKS_PER_SEC);
	



/*#if PHP_VERSION_ID>=50500
*/
	zend_execute_ex = old_execute_ex;
/*
#else
	zend_execute = old_execute;
#endif
*/
	zend_execute_internal = old_execute_internal;

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


/*
#if PHP_VERSION_ID>=50500
*/
static void tracer_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
{

	//time_t execute_start;
	// struct tm *tm_start;
	// char *datetime;
	// time(&execute_start);
	// tm_start = localtime(&execute_start);
	// datetime = asctime(tm_start);
	//slog(2,SLOG_INFO,">>>>>>>Execute start: %d  time: %s  \n<br/>",clock(),ctime(&execute_start));

	tracer_fcall_entry *entry = TRACER_G(current_fcall);
	

	
	clock_t execute_start = clock();
	slog(2,SLOG_INFO,"************Execute start**********<br/>");


	//op_array_traverse(execute_data->op_array);
	//test_function(execute_data TSRMLS_CC);


	if(execute_data->op_array!=NULL) {

		if(execute_data->op_array->function_name == NULL) {

			entry->data.start = execute_start;
			entry->data.type = NODE_ENTRY;
			entry->data.scope_name = string(execute_data->op_array->filename);

			slog(2,SLOG_INFO,"######<%d> Global Entry", function_level);

			old_execute_ex(execute_data TSRMLS_CC);

			slog(2,SLOG_INFO,"######<%d> Global Exit", function_level);
		}

		else{

			tracer_fcall_entry* new_fcall = TRACER_CREATE_FCALL();
			entry->fcall_list.push_back(new_fcall);
			new_fcall->pre_fcall = entry;
			TRACER_G(current_fcall) = new_fcall;
			entry = new_fcall;
			entry->data.type = NODE_USERDEF;
			entry->data.scope_name = string(execute_data->op_array->function_name);
			entry->data.start = execute_start;

			

			slog(2,SLOG_INFO,"######<%d> Enter Function: %s",++function_level,execute_data->op_array->function_name);

			old_execute_ex(execute_data TSRMLS_CC);

			slog(2,SLOG_INFO,"######<%d> Exit Function: %s",function_level--,execute_data->op_array->function_name);
		}

			

	}

	//old_execute_ex(execute_data TSRMLS_CC);

	entry->data.end = clock();
	entry->data.interval = (double) ((entry->data.end - entry->data.start) / CLOCKS_PER_SEC);
	if(entry->pre_fcall != NULL)
		TRACER_G(current_fcall) = entry->pre_fcall;

	clock_t execute_end = clock();
	clock_t execute_interval = execute_end - execute_start;
	slog(2,SLOG_INFO,"************Execute end**********: pass loops: %d, interval: %f\n<br/>",execute_interval,execute_interval/CLOCKS_PER_SEC);
	
}
/*
#else
static void tracer_execute(zend_op_array *op_array TSRMLS_DC)
{
	 
	op_array_traverse(op_array);
	// File *fp;
	// fp = fopen("php://output");
	// fprintf(fp,"executed");
	// fclose(fp);
	old_execute(op_array TSRMLS_CC);
	
}
#endif
*/
static void tracer_execute_internal(zend_execute_data *execute_data_ptr, zend_fcall_info *fci,int return_value_used TSRMLS_DC)
{


	tracer_fcall_entry *entry = TRACER_G(current_fcall);
	tracer_fcall_entry* new_fcall = TRACER_CREATE_FCALL();
	entry->fcall_list.push_back(new_fcall);
	new_fcall->pre_fcall = entry;
	TRACER_G(current_fcall) = new_fcall;
	entry = new_fcall;

	clock_t internal_start = clock();
	slog(2,SLOG_INFO,">>>>>>>Internal function start: %d\n<br/>",internal_start);
	//php_printf(">>>>>>>Internal function start: %d\n<br/>",internal_start);
	 
    zend_op_array *op_array = execute_data_ptr->op_array;  


    //test_function(execute_data_ptr TSRMLS_CC);
	//op_array_traverse(execute_data_ptr->op_array);

    const char *outer_scope = "Global";
    const char *internal_name = "Empty";


    entry->data.start = internal_start;

    if(execute_data_ptr->op_array!=NULL){

    	
    	if(execute_data_ptr->op_array->function_name !=NULL){

    		outer_scope = execute_data_ptr->op_array->function_name;

    	}


    	if(execute_data_ptr->function_state.function != NULL) {

			if(execute_data_ptr->function_state.function->common.function_name !=NULL) {


				internal_name = execute_data_ptr->function_state.function->common.function_name;


				if(strspn(execute_data->op_array->function_name,"mysql_") == 0) {

					entry->data.type = NODE_DB;
					
				}
				else {
					entry->data.type = NODE_EXTERNAL;
				}
				entry->data.scope_name = string(internal_name);

			   slog(2,SLOG_INFO,"######<%d> Enter Internal Function: %s, Outer Scope: %s",++function_level,internal_name,outer_scope);


			}
			else {
				slog(2,SLOG_INFO,"Internal Function Name: %s",internal_name);	 	
			}
		}
		else {

		 	slog(2,SLOG_INFO,"Internal Function: %s",internal_name);

		}			  	
	}



	old_execute_internal(execute_data_ptr,fci,return_value_used TSRMLS_CC);

	entry->data.end = clock();
	entry->data.interval = (double) ((entry->data.end - entry->data.start) / CLOCKS_PER_SEC);
	TRACER_G(current_fcall) = entry->pre_fcall;

	slog(2,SLOG_INFO,"######<%d> Exit Internal Function: %s, Outer Scope: %s",function_level--,internal_name,outer_scope);


	clock_t internal_end = clock();
	clock_t internal_interval = internal_end - internal_start;

	slog(2,SLOG_INFO,">>>>>>>Internal function end<<<<<<<<<<: %d, pass: %d, interval: %f\n<br/>",internal_interval,internal_interval/CLOCKS_PER_SEC);
	
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
