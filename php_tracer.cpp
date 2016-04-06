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
#include "zend_errors.h"
#include "zend_exceptions.h"
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
/* Remove comments and fill if you need to have entries in php.ini*/
PHP_INI_BEGIN()
	//STD_PHP_INI_ENTRY("php_tracer.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_php_tracer_globals, php_tracer_globals)
	//STD_PHP_INI_ENTRY("php_tracer.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_php_tracer_globals, php_tracer_globals)
    STD_PHP_INI_ENTRY("php_tracer.module_start",      "0", PHP_INI_ALL, OnUpdateLong, module_start, zend_php_tracer_globals, php_tracer_globals)
    STD_PHP_INI_ENTRY("php_tracer.module_end",     "0", PHP_INI_ALL, OnUpdateLong, module_end, zend_php_tracer_globals, php_tracer_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_php_tracer_init_globals
*/
/* Uncomment this function if you have INI entries*/
static void php_php_tracer_init_globals(zend_php_tracer_globals *php_tracer_globals)
{
	php_tracer_globals->module_start = 0;
	php_tracer_globals->module_end = 0;
	php_tracer_globals->fcalls = NULL;
	php_tracer_globals->current_fcall = NULL;
	//php_tracer_globals->request_info = NULL;
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
static void (*old_error_cb)(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
static void tracer_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args);
static void (*old_throw_exception_hook)(zval *ex TSRMLS_DC);
static void tracer_throw_exception_hook(zval *ex TSRMLS_DC);
static void print_and_free_trace(tracer_fcall_entry *entry, int level);
static void tracer_event_handler(int event_type,int type,uint lineno,char *msg);




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



PHP_MINIT_FUNCTION(php_tracer)
{
	/* If you have INI entries, uncomment these lines */
	REGISTER_INI_ENTRIES();
	
	TRACER_INIT_REQUEST((TRACER_G(request_info)));
	/*Init Log*/
	string home_prefix = string(getenv("HOME"));
	//slog_init((home_prefix + "/php/logs/php_tracer").c_str(),(home_prefix+"/php/slog.cfg").c_str(),2,3,1);
	slog_init("/home/liangzx/php/logs/php_tracer","/home/liangzx/php/slog.cfg",2,3,1);


	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(php_tracer)
{
	/* uncomment this line if you have INI entries*/
	UNREGISTER_INI_ENTRIES();
	
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
	slog(2,SLOG_INFO,"-------------Request Start-------------");

	obtain_request_info();

	


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

	old_error_cb = zend_error_cb;
	zend_error_cb = tracer_error_cb;

	
	old_throw_exception_hook = zend_throw_exception_hook;
	zend_throw_exception_hook = tracer_throw_exception_hook;

	/*allocate global entry*/
	TRACER_CREATE_FCALL(TRACER_G(fcalls));
	if(TRACER_G(fcalls)) {
		TRACER_G(current_fcall) = TRACER_G(fcalls);
		TRACER_G(fcalls)->pre_fcall = NULL;
	}
	else {
		slog(2,SLOG_ERROR,"no enough space for TRACER_G(fcalls)!");
	}


	return SUCCESS;

}
/* }}} */

static void print_and_free_trace(tracer_fcall_entry *entry,int level)
{
	if(entry == NULL) return;

	string blank_char = "";
	for(int i = 0; i < level; i++)
	{
		blank_char += "--";
	}
	blank_char += ">";

	slog(1,SLOG_INFO,"%s%s(%d--%d, loops: %d,interval: %f, %s, line %d)",blank_char.c_str(),
		entry->data.scope_name,entry->data.start,entry->data.end,entry->data.end - entry->data.start,entry->data.interval,NODE_TYPE(entry->data.type),entry->data.lineno);

	
	GSList *elist = entry->event_list;
	if(elist != NULL) {
		for(int j = 0; j < g_slist_length(entry->event_list); j++)
		{
		  	tracer_event *event = (tracer_event *)(elist->data);
		  	slog(1,SLOG_INFO,"%s[%s]%s:%s(line %d)",blank_char.c_str(),
		EVENT_TYPE(event->event_type),ERROR_NAME(event->type),event->msg,event->lineno);

		  	elist = g_slist_next(elist);
		}
		g_slist_free(entry->event_list);
	}



    if(entry->fcall_list == NULL||g_slist_length(entry->fcall_list) == 0) 
	{
		delete(entry);
		return;
	}
    GSList *plist = entry->fcall_list;
    for(int i = 0; i < g_slist_length(entry->fcall_list);i++)
    {	
          print_and_free_trace((tracer_fcall_entry *)(plist->data),level+1);
          plist = g_slist_next(plist);

    }

    
    g_slist_free(entry->fcall_list);
 

}
static void print_request_data() {
	slog(1,SLOG_INFO,"----------------REQUEST DATA------------------");
	slog(1,SLOG_INFO,"host: %s, ip: %s, uri: %s, script: %s,ts: %d, method: %s",
		TRACER_RI(host),TRACER_RI(ip),TRACER_RI(uri),TRACER_RI(script_name),TRACER_RI(ts),TRACER_RI(method));
}
 


/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(php_tracer)
{
	/*print trace iteratively*/
	slog(2,SLOG_INFO,"----------------------TRACE-----------------------");
	tracer_fcall_entry *entry =  TRACER_G(fcalls);
	if(entry != NULL) {

		print_and_free_trace(entry,1);
	}
	print_request_data();


/*#if PHP_VERSION_ID>=50500
*/
	zend_execute_ex = old_execute_ex;
/*
#else
	zend_execute = old_execute;
#endif
*/
	zend_execute_internal = old_execute_internal;

	zend_error_cb = old_error_cb;

	zend_throw_exception_hook = old_throw_exception_hook;

	slog(2,SLOG_INFO,"-------------Request End-------------");

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
	slog(2,SLOG_INFO,"************Execute start**********");

	clock_t execute_start = clock();

	tracer_fcall_entry *entry = TRACER_G(current_fcall);  //Get the Outmost entry or the caller

	if(execute_data->op_array != NULL) {

		/* Outmost Entry*/
		if(execute_data->op_array->function_name == NULL) {

			entry->data.start = execute_start;
			entry->data.type = NODE_ENTRY;
			if(execute_data->op_array->filename != NULL) {
				/*Get The Name of the script*/
				TRACER_COPY_STRING(entry->data.scope_name,execute_data->op_array->filename);
			}
			else {
				slog(2,SLOG_ERROR,"Fail to get filename");				
			}
			
			old_execute_ex(execute_data TSRMLS_CC);

		}
		/*Enter User Define Function*/
		else{

			tracer_fcall_entry* new_fcall = NULL;
			TRACER_CREATE_FCALL(new_fcall);
			if(new_fcall == NULL) {
				slog(2,SLOG_ERROR,"no enough space!");
				return;
			}
			//add new trace to the function call list of the caller
			TRACER_ADD_TO_LIST(entry->fcall_list,new_fcall);
			//Remember outer scope
			new_fcall->pre_fcall = entry;
			TRACER_G(current_fcall) = new_fcall;

			entry = new_fcall;
			entry->data.type = NODE_USERDEF;
			TRACER_COPY_STRING(entry->data.scope_name,execute_data->op_array->function_name);
			entry->data.start = execute_start;
			if(execute_data->prev_execute_data != NULL)
				entry->data.lineno = execute_data->prev_execute_data->opline->lineno;


			old_execute_ex(execute_data TSRMLS_CC);
		}

			

	}
	
	//Resume caller status
	if(entry->pre_fcall != NULL)
		TRACER_G(current_fcall) = entry->pre_fcall;

	clock_t execute_end = clock();
	entry->data.end = execute_end;
	entry->data.interval = (double) ((entry->data.end - entry->data.start) / CLOCKS_PER_SEC);

	slog(2,SLOG_INFO,"************Execute End**********");

}
/*
#else
static void tracer_execute(zend_op_array *op_array TSRMLS_DC)
{
	
}
#endif
*/
static void tracer_execute_internal(zend_execute_data *execute_data_ptr, zend_fcall_info *fci,int return_value_used TSRMLS_DC)
{
	slog(2,SLOG_INFO,"************Execute Internal start**********");

	clock_t internal_start = clock();

	tracer_fcall_entry *entry = TRACER_G(current_fcall); //Get the caller of internal function
	if(entry == NULL) {
		slog(2,SLOG_ERROR,"entry is NULL after enter execute internal");
		return;
 	}

	tracer_fcall_entry *new_fcall = NULL;
	

    zend_op_array *op_array = execute_data_ptr->op_array;  
    const char *internal_name = "Empty";

    if(execute_data_ptr->op_array!=NULL){

    
    	if(execute_data_ptr->function_state.function != NULL) {

			if(execute_data_ptr->function_state.function->common.function_name !=NULL) {


				internal_name = execute_data_ptr->function_state.function->common.function_name;

				if(strstr(internal_name,"__") != internal_name) {

					TRACER_CREATE_FCALL(new_fcall);
					if(new_fcall == NULL) {
						slog(2,SLOG_ERROR,"no enough space for new_fcall");
					}
				 	TRACER_ADD_TO_LIST(entry->fcall_list,new_fcall);
					new_fcall->pre_fcall = entry;
					TRACER_G(current_fcall) = new_fcall;
					new_fcall->data.start = internal_start;

					/*Mysql function*/
					if(strstr(internal_name,"mysql_") == internal_name) {
						slog(1,SLOG_INFO,"%s",internal_name);

						new_fcall->data.type = NODE_DB;
						
					}
					/*Other external function*/
					else {
						new_fcall->data.type = NODE_EXTERNAL;
					}

					TRACER_COPY_STRING(new_fcall->data.scope_name,internal_name);
					if(execute_data_ptr->prev_execute_data != NULL)
						new_fcall->data.lineno = execute_data_ptr->opline->lineno;


				}		
			 
			}
			else {
				slog(2,SLOG_ERROR,"Internal Function Name is Empty");	 	
			}
		}
		else {
			slog(2,SLOG_ERROR,"Internal Function is NULL");	 	
		}		  	
	}


	old_execute_internal(execute_data_ptr,fci,return_value_used TSRMLS_CC);

	if(new_fcall != NULL){
			//Resume caller status
		if(new_fcall->pre_fcall != NULL)
			TRACER_G(current_fcall) = new_fcall->pre_fcall;


		clock_t internal_end = clock();
		new_fcall->data.end = internal_end;
		new_fcall->data.interval = (double) ((new_fcall->data.end - new_fcall->data.start) / CLOCKS_PER_SEC);

	}
	
	slog(2,SLOG_INFO,"************Execute Internal End**********");

}

static void tracer_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	slog(1,SLOG_INFO,"************Test error cb start**********");	

	//slog(1,SLOG_INFO,"type: %d, error filename: %s, error lineno: %ld,format: %s",type,error_filename,error_lineno,format);		
	char *msg;
	va_list args_copy;
	TSRMLS_FETCH();

	/* A copy of args is needed to be used for the old_error_cb */
	va_copy(args_copy, args);
	vspprintf(&msg, 0, format, args_copy);
	va_end(args_copy);

	tracer_event_handler(TRACER_ERROR,type,error_lineno,msg);

		
	old_error_cb(type,error_filename,error_lineno,format,args);

	slog(1,SLOG_INFO,"************Test error cb end**********");


}

static void tracer_throw_exception_hook(zval *exception TSRMLS_DC)
{
	zval *message, *file, *line;
	zval rv;
	zend_class_entry *default_ce;

		if (!exception) {
			return;
		}

		default_ce = zend_exception_get_default(TSRMLS_C);


		message =  zend_read_property(default_ce, exception, "message", sizeof("message")-1, 0 TSRMLS_CC);
		file = zend_read_property(default_ce, exception, "file", sizeof("file")-1, 0 TSRMLS_CC);
		line = zend_read_property(default_ce, exception, "line", sizeof("line")-1, 0 TSRMLS_CC);

		slog(1,SLOG_INFO,"message: %s, file: %s, line: %d",Z_STRVAL_P(message),Z_STRVAL_P(file),Z_LVAL_P(line));

		 tracer_event_handler(TRACER_EXCEPTION,-1,Z_LVAL_P(line),Z_STRVAL_P(message));


		//process_event(APM_EVENT_EXCEPTION, E_EXCEPTION, Z_STRVAL_P(file), Z_LVAL_P(line), Z_STRVAL_P(message) TSRMLS_CC);
}

static void tracer_event_handler(int event_type,int type,uint lineno,char *msg)
{

	tracer_event *event = NULL;
	TRACER_CREATE_EVENT(event);

	TRACER_COPY_STRING(event->msg,msg);
	event->lineno = lineno;
	event->event_type = event_type;
	event->type = type;

	tracer_fcall_entry *current_fcall = TRACER_G(current_fcall);

	TRACER_ADD_TO_LIST(current_fcall->event_list,event);
}






void obtain_request_info() {
  zval *tmp;

  zend_is_auto_global_str("_SERVER");
  if (FETCH_HTTP_GLOBALS(SERVER)) {
    SET_REQUEST_INFO("REQUEST_URI", uri, IS_STRING);
    SET_REQUEST_INFO("HTTP_HOST", host, IS_STRING);
    SET_REQUEST_INFO("REQUEST_TIME", ts, IS_LONG);
    SET_REQUEST_INFO("SCRIPT_FILENAME", script_name, IS_STRING);
    SET_REQUEST_INFO("REQUEST_METHOD", method, IS_STRING);
    SET_REQUEST_INFO("REMOTE_ADDR", ip, IS_STRING);
  }
}


const char* get_node_type(int type)
{
    switch (type) {
      case NODE_ENTRY: return "entry";
      case NODE_DB: return "database";
      case NODE_EXTERNAL: return "external";
      case NODE_USERDEF: return "user define";
      default: return "unknown";
    }
   
}
const char* get_event_type(int i)
{

      if(i == 0) return "Error"; 
      if (i == 1) return "Exception"; 
      return "unknown"; 
   
}

const char* get_error_name(int type)
{
    switch (type) {
      case E_CORE_ERROR: return "Core Error";
      case E_CORE_WARNING: return "Core Warning";
      case E_PARSE: return "Parse Error";
      case E_COMPILE_ERROR: return "Compile Error";
      case E_COMPILE_WARNING: return "Compile Warning";
      case E_ERROR: return "Error";
      case E_NOTICE: return "Notice";
      case E_STRICT: return "Runtime Notice";
      case E_DEPRECATED: return "Deprecated";
      case E_WARNING: return "Warning";
      case E_USER_ERROR: return "User Error";
      case E_USER_WARNING: return "User Warning";
      case E_USER_NOTICE: return "User Notice";
      case E_USER_DEPRECATED: return "User Deprecated";
      case E_RECOVERABLE_ERROR: return "Catchable Fatal Error";
      default: return "Exception";
  }
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
