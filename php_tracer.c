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
//extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_errors.h"
#include "zend_exceptions.h"
#include "ext/standard/php_smart_str.h"	
#include "ext/standard/php_string.h"	
//}
#include "slog.h"
#include "php_php_tracer.h"
#include "php_tracer_public.h"
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
	
	//TRACER_INIT_REQUEST((TRACER_G(request_info)));
	/*Init Log*/
	//string home_prefix = string(getenv("HOME"));
	//slog_init((home_prefix + "/php/logs/php_tracer").c_str(),(home_prefix+"/php/slog.cfg").c_str(),2,3,1);
	
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
	

	slog_init("/home/liangzx/php/logs/php_tracer","/home/liangzx/php/slog.cfg",2,3,1);
	slog(2,SLOG_INFO,"-------------Request Start-------------");
	//php_printf("Request Start</br>");

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



/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(php_tracer)
{
	/*print trace iteratively*/
	
	slog(2,SLOG_INFO,"----------------------TRACE-----------------------");
	
	 tracer_fcall_entry *entry =  TRACER_G(fcalls);
	 parse_data();	
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

	//php_printf("Request End");
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

	struct timeval current_time;
	gettimeofday(&current_time,NULL);

	//clock_t execute_start = clock();

	tracer_fcall_entry *entry = TRACER_G(current_fcall);  //Get the Outmost entry or the caller

	if(execute_data->op_array != NULL) {

		/* Outmost Entry*/
		if(execute_data->op_array->function_name == NULL) {

			entry->data.start = current_time.tv_usec;
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
			entry->data.start = current_time.tv_usec;
			if(execute_data->prev_execute_data != NULL)
				entry->data.lineno = execute_data->prev_execute_data->opline->lineno;
			
			 	load_parameters(entry,execute_data);
			 	//load_arguments(entry,execute_data);
			 	 // for(int i = 0; i < TRACER_FD(entry).param_count; i++) {
			 	 // 	slog(1,SLOG_INFO,"param%d %s",i,TRACER_FD(entry).parameters[i]);
			 	 // }
			 	 // for(int i = 0; i < TRACER_FD(entry).arg_count; i++) {
			 	 // 	slog(1,SLOG_INFO,"arg%d %s",i,TRACER_FD(entry).arguments[i]);
			 	 // }

			
			old_execute_ex(execute_data TSRMLS_CC);
		}

			

	}
	
	//Resume caller status
	if(entry->pre_fcall != NULL)
		TRACER_G(current_fcall) = entry->pre_fcall;

	gettimeofday(&current_time,NULL);
	//clock_t execute_end = clock();
	entry->data.end = current_time.tv_usec;
	//php_printf("data.end: %d ",entry->data.end);
	//entry->data.interval = (double) ((entry->data.end - entry->data.start) / CLOCKS_PER_SEC);
	entry->data.interval = entry->data.end - entry->data.start;

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

	//clock_t internal_start = clock();

	struct timeval current_time;
	gettimeofday(&current_time,NULL);

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
					new_fcall->data.start = current_time.tv_usec;

					/*Mysql function*/
					if(strstr(internal_name,"mysql_") == internal_name) {
						slog(1,SLOG_INFO,"%s",internal_name);

						new_fcall->data.type = NODE_DB;
						// if(strcmp(internal_name,"mysql_query") == 0) {										
						// 	slog(1,SLOG_INFO,"mysql_query: %s",execute_data_ptr->function_state.function->common.arg_info->name);
						// }
					}
					/*Other external function*/
					else {
						new_fcall->data.type = NODE_EXTERNAL;
					}

					TRACER_COPY_STRING(new_fcall->data.scope_name,internal_name);
					if(execute_data_ptr->prev_execute_data != NULL)
						new_fcall->data.lineno = execute_data_ptr->opline->lineno;
					load_parameters(new_fcall,execute_data_ptr);
					load_arguments(new_fcall,execute_data_ptr);
					 // for(int i = 0; i < TRACER_FD(new_fcall).param_count; i++) {
			 	 // 	slog(1,SLOG_INFO,"param%d %s",i,TRACER_FD(new_fcall).parameters[i]);
			 	 // 	}
				  	//for(int i = 0; i < TRACER_FD(new_fcall).arg_count; i++) {
				  	//	slog(1,SLOG_INFO,"arg%d %s",i,TRACER_FD(new_fcall).arguments[i]);
				  	//}

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


		//clock_t internal_end = clock();
		gettimeofday(&current_time,NULL);
		new_fcall->data.end = current_time.tv_usec;
		//new_fcall->data.interval = (double) ((new_fcall->data.end - new_fcall->data.start) / CLOCKS_PER_SEC);
		new_fcall->data.interval = new_fcall->data.end - new_fcall->data.start;

	}
	
	slog(2,SLOG_INFO,"************Execute Internal End**********");

}


static void tracer_error_cb(int type, const char *error_filename, const uint error_lineno, const char *format, va_list args)
{
	slog(1,SLOG_INFO,"************Test error cb start**********");	
	php_printf("Test error cb");

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
	slog(1,SLOG_INFO,"************Test exception hook start**********");	
	php_printf("Test exception hook");
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






static void obtain_request_info() {
  zval *tmp;

  if(TRACER_RI(is_set)) {
  	return;
  }
  TRACER_RI(is_set) = 1;

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

static bool load_parameters(tracer_fcall_entry* entry,zend_execute_data *execute_data) {

	slog(1,SLOG_INFO,"LOAD PARAMETERS");
	
	zend_arg_info* arg_info = NULL;
	
	if(execute_data->function_state.function == NULL) {
		
		return false;
	}

	arg_info = execute_data->function_state.function->common.arg_info;

	if(arg_info == NULL) {
		TRACER_FD(entry).param_count = 0;
		return false;
	}

	TRACER_FD(entry).param_count = execute_data->function_state.function->common.required_num_args;
	int len = TRACER_FD(entry).param_count;
	slog(1,SLOG_INFO,"param_count: %d",len);
	TRACER_FD(entry).parameters = (char**)emalloc(sizeof(char *)*len);
	char **dst = TRACER_FD(entry).parameters;
	if(dst == NULL) {
		slog(1,SLOG_INFO,"no space for parameters");
		return false;;
	}
	int i;
	for(i = 0; i < len; i++) {
		const char *str = (*arg_info).name;
		dst[i] = (char*)emalloc(sizeof(char) * 100);
		TRACER_COPY_STRING(dst[i],str);
		//slog(1,SLOG_INFO,"parameter%d %s",i,dst[i]);
		arg_info++;	
	}

	slog(1,SLOG_INFO,"LOAD PARAMETERS");

	return true;
	
}


static bool load_arguments(tracer_fcall_entry* entry, zend_execute_data *execute_data) {

	slog(1,SLOG_INFO,"LOAD ARGUMENTS");
	zval**arguments = (zval **)execute_data->function_state.arguments;
	TRACER_FD(entry).arg_count = execute_data->opline->extended_value;
	int len = TRACER_FD(entry).arg_count;
	if(len == 0 || arguments == NULL) return false;

	TRACER_FD(entry).arguments = (char**)emalloc(sizeof(char *)*len);
	char **dst = TRACER_FD(entry).arguments;
	if(dst == NULL) {
		slog(1,SLOG_INFO,"no space for arguments");
		return false;
	}
	int i;
	for(i = 0; i < len; i++) {
		zval tmpcopy = **(arguments-len+i);
		zval_copy_ctor(&tmpcopy);
		INIT_PZVAL(&tmpcopy);
		convert_to_string(&tmpcopy);
		//convert_to_string(*(arguments-len + i));
		char *str = (char *)Z_STRVAL(tmpcopy);
		dst[i] = (char*)emalloc(sizeof(char) * 100);
		TRACER_COPY_STRING(dst[i],str);
		//slog(1,SLOG_INFO,"argument%d %s",i,dst[i]);
	}
	slog(1,SLOG_INFO,"LOAD ARGUMENTS");

	return true;
}


static const char  *convert_arguments(ulong arg_count,char** arguments,bool is_param) {	

	if(arguments == NULL || arg_count == 0) return " ";
	
	smart_str s = {0};
 	//string s ="";
	
	if(is_param)
		smart_str_appendc(&s,'$');
		//s = "$";
		
	smart_str_appends(&s,arguments[0]);
	//s+=string(arguments[0]);
	int i;
	for(i = 1; i < arg_count; i++) {
		if(is_param) {
			smart_str_appends(&s,", $");
			//s+=", $";
		}
		else {
			smart_str_appends(&s,", ");
			//s+=", ";
		}
		smart_str_appends(&s,arguments[i]);
		

	}
	 
	smart_str_0(&s);

	return s.c;
	
}

static void print_and_free_trace(tracer_fcall_entry *entry,int level)
{
	if(entry == NULL) return;

	smart_str blank_char = {0};
	//string blank_char = "";
	int i;
	for(i = 0; i < level; i++)
	{
		//blank_char += "--";
		smart_str_appends(&blank_char,"--");
	}
	smart_str_appendc(&blank_char,'>');
	smart_str_0(&blank_char);
	//blank_char += ">";
	//const char * arg = convert_arguments(TRACER_FD(entry).arg_count,TRACER_FD(entry).arguments);
	 slog(3,SLOG_INFO,"%s%s(%s)(%ld--%ld, interval: %ld, %s, line %d),arguments[%d]: %s",
	 	blank_char.c,
	 	entry->data.scope_name,
		convert_arguments(TRACER_FD(entry).param_count,TRACER_FD(entry).parameters,true),
	 	entry->data.start,
	 	entry->data.end,
	 	entry->data.interval,
	 	NODE_TYPE(entry->data.type),
	 	entry->data.lineno,
	 	TRACER_FD(entry).arg_count,
	 	convert_arguments(TRACER_FD(entry).arg_count,TRACER_FD(entry).arguments,false)
	);

	//  php_printf("%s%s(%s)(%d--%d, loops: %d,interval: %f, %s, line %d),arguments[%d]: %s</br>",
	//  	blank_char.c_str(),
	//  	entry->data.scope_name,
	// 	convert_arguments(TRACER_FD(entry).param_count,TRACER_FD(entry).parameters,true),
	//  	entry->data.start,
	//  	entry->data.end,
	//  	entry->data.end - entry->data.start,
	//  	entry->data.interval,
	//  	NODE_TYPE(entry->data.type),
	//  	entry->data.lineno,
	//  	TRACER_FD(entry).arg_count,
	//  	convert_arguments(TRACER_FD(entry).arg_count,TRACER_FD(entry).arguments,false)
	// );
	

	
	
/*	slog(1,SLOG_INFO,"%s%s(%d--%d, loops: %d,interval: %f, %s, line %d)",
		blank_char.c_str(),
		entry->data.scope_name,
		entry->data.start,
		entry->data.end,
		entry->data.end - entry->data.start,
		entry->data.interval,
		NODE_TYPE(entry->data.type),
		entry->data.lineno);
*/	
	
	
	GSList *elist = entry->event_list;
	if(elist != NULL) {
		for(i = 0; i < g_slist_length(entry->event_list); i++)
		{
		  	tracer_event *event = (tracer_event *)(elist->data);
		  	slog(1,SLOG_INFO,"%s[%s]%s:%s(line %d)",blank_char.c,
		EVENT_TYPE(event->event_type),ERROR_NAME(event->type),event->msg,event->lineno);

		  	elist = g_slist_next(elist);
		}
		g_slist_free(entry->event_list);
	}



    if(entry->fcall_list == NULL||g_slist_length(entry->fcall_list) == 0) 
	{
		//delete(entry);
		return;
	}
    GSList *plist = entry->fcall_list;
    for(i = 0; i < g_slist_length(entry->fcall_list);i++)
    {	
          print_and_free_trace((tracer_fcall_entry *)(plist->data),level+1);
          plist = g_slist_next(plist);

    }

    
    g_slist_free(entry->fcall_list);
 

}
static char* convert_str_pp(zval ** val) {
	if((val) == NULL) return "null";
	return (char*)Z_STRVAL_PP(val);
}
static ulong convert_l_pp(zval ** val) {
	if((val) == NULL) return 0;
	return (ulong)Z_LVAL_PP(val);
} 
static void print_request_data() {
	slog(1,SLOG_INFO,"----------------REQUEST DATA------------------");
	//slog(1,SLOG_INFO,"host: %s",TRACER_RI(host)==NULL?"NULL":TRACER_RI_STRVAL(host));
	// FILE *fp = NULL;
	// if(fp = fopen("/home/liangzx/MySites/a.txt","w")) {
	// fprintf(fp,"host: %s, ip: %s, uri: %s, script: %s,method: %s",
	// 	TRACER_RI(host)==NULL?"NULL":TRACER_RI_STRVAL(host),TRACER_RI(ip)==NULL?"NULL":TRACER_RI_STRVAL(ip),
	// 	TRACER_RI(uri)==NULL?"NULL":TRACER_RI_STRVAL(uri),TRACER_RI(script_name)==NULL?"NULL":TRACER_RI_STRVAL(script_name),
	// 	 TRACER_RI(method)==NULL?"NULL":TRACER_RI_STRVAL(method)
	// 	);
	// fclose(fp);
	// }
	// php_printf("host: %s, ip: %s, uri: %s, script: %s,method: %s",
	// 	TRACER_RI(host)==NULL?"NULL":TRACER_RI_STRVAL(host),TRACER_RI(ip)==NULL?"NULL":TRACER_RI_STRVAL(ip),
	// 	TRACER_RI(uri)==NULL?"NULL":TRACER_RI_STRVAL(uri),TRACER_RI(script_name)==NULL?"NULL":TRACER_RI_STRVAL(script_name),
	// 	 TRACER_RI(method)==NULL?"NULL":TRACER_RI_STRVAL(method)
	// 	);
	//time_t time_stamp = TRACER_RI(ts) == NULL?0:TRACER_RI_LVAL(ts);

	/*slog(1,SLOG_INFO,"host: %s, ip: %s, uri: %s, script: %s,method: %s, ts: %s",
		TRACER_RI(host)==NULL?"NULL":TRACER_RI_STRVAL(host),TRACER_RI(ip)==NULL?"NULL":TRACER_RI_STRVAL(ip),
		TRACER_RI(uri)==NULL?"NULL":TRACER_RI_STRVAL(uri),TRACER_RI(script_name)==NULL?"NULL":TRACER_RI_STRVAL(script_name),
		 TRACER_RI(method)==NULL?"NULL":TRACER_RI_STRVAL(method),ctime(&time_stamp));*/
	slog(1,SLOG_INFO,"host: %s, ip: %s, uri: %s, script: %s,method: %s, ts: %s",
		convert_str_pp(TRACER_RI(host)),convert_str_pp(TRACER_RI(ip)),
		convert_str_pp(TRACER_RI(uri)),convert_str_pp(TRACER_RI(script_name)),
		 convert_str_pp(TRACER_RI(method)),ctime(convert_l_pp(&TRACER_RI(ts))));	
	// convert_to_string(*(TRACER_RI(host)));
	// slog(1,SLOG_INFO,"host: %s",*(TRACER_RI(host)));
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
static void parse_data() {

	smart_str str = {0};
	smart_str_appendc(&str,'{');
	parse_request(&str);
	smart_str_appendc(&str,',');
	parse_trace(&str);
	smart_str_appendc(&str,'}');
	smart_str_0(&str);
	php_printf("parse result: %s\n",str.c);
    smart_str_free(&str);


}

static void parse_request(smart_str *str) {

	smart_str_appends(str,"\"request\":");

	smart_str_appendc(str,'{');

	smart_str_appends(str,"\"host\":");
	smart_str_wrap_quotes_sc(str,convert_str_pp(TRACER_RI(host)));
	smart_str_appends(str,"\"ip\":");
	smart_str_wrap_quotes_sc(str,convert_str_pp(TRACER_RI(ip)));
	smart_str_appends(str,"\"uri\":");
	smart_str_wrap_quotes_sc(str,convert_str_pp(TRACER_RI(uri)));
	smart_str_appends(str,"\"script_name\":");
	smart_str_wrap_quotes_sc(str,convert_str_pp(TRACER_RI(script_name)));
	smart_str_appends(str,"\"ts\":");
	smart_str_wrap_quotes_lc(str,convert_l_pp(TRACER_RI(ts)));
	smart_str_appends(str,"\"method\":");
	smart_str_wrap_quotes_s(str,convert_str_pp(TRACER_RI(host)));

	smart_str_appendc(str,'}');

}
static void parse_trace(smart_str *str) {

	tracer_fcall_entry *entry = TRACER_G(fcalls);

	smart_str_appends(str,"\"trace\":");
	
	smart_str_appendc(str,'{');

	parse_fcall(str,entry);
	//smart_str_appendc(str,',');
	//parse_fcall_data(str,entry);

	smart_str_appendc(str,'}');

}

static void parse_fcall_data(smart_str *str, tracer_fcall_entry *entry) {

		if(entry == NULL) return;

		smart_str_appends(str,"\"data\":");
		
		smart_str_appendc(str,'{');

		smart_str_appends(str,"\"start\":");
		smart_str_wrap_quotes_lc(str,entry->data.start);
		smart_str_appends(str,"\"end\":");
		smart_str_wrap_quotes_lc(str,entry->data.end);
		//php_printf(" dataend: %d ",entry->data.end);
		smart_str_appends(str,"\"interval\":");
		smart_str_wrap_quotes_lc(str,entry->data.interval);
		smart_str_appends(str,"\"lineno\":");
		smart_str_wrap_quotes_lc(str,entry->data.lineno);
		smart_str_appends(str,"\"type\":");
		smart_str_wrap_quotes_lc(str,entry->data.type);
		smart_str_appends(str,"\"name\":");
		smart_str_wrap_quotes_sc(str,entry->data.scope_name);

		smart_str_appends(str,"\"arg_count\":");
		smart_str_wrap_quotes_lc(str,entry->data.arg_count);
		int i = 0;
		smart_str_appends(str,"\"arguments\":");
		smart_str_appendc(str,'[');	
		if(entry->data.arg_count > 0) {
			while(i < (entry->data.arg_count - 1)) {
				smart_str_appendc(str,'{');	
				smart_str_appends(str,"\"val\":");
				smart_str_wrap_quotes_s(str,entry->data.arguments[i]);
				smart_str_appends(str,"},");
				++i;
			}
			if(i < entry->data.arg_count) {
				smart_str_appendc(str,'{');	
				smart_str_appends(str,"\"val\":");
				smart_str_wrap_quotes_s(str,entry->data.arguments[i]);
				smart_str_appendc(str,'}');
			}
		}
		
		smart_str_appends(str,"],");

		smart_str_appends(str,"\"param_count\":");
		smart_str_wrap_quotes_lc(str,entry->data.param_count);
		i = 0;
		smart_str_appends(str,"\"parameters\":");
		smart_str_appendc(str,'[');	
		if(entry->data.param_count > 0) {
			while(i < entry->data.param_count - 1) {
				smart_str_appendc(str,'{');	
				smart_str_appends(str,"\"val\":");
				smart_str_wrap_quotes_s(str,entry->data.parameters[i]);
				smart_str_appends(str,"},");
				++i;
			}
			if(i < entry->data.param_count) {
				smart_str_appendc(str,'{');	
				smart_str_appends(str,"\"val\":");
				smart_str_wrap_quotes_s(str,entry->data.parameters[i]);
				smart_str_appendc(str,'}');
			}
		}
		
		smart_str_appendc(str,']');

		smart_str_appendc(str,'}');
}

static void parse_event(smart_str *str,tracer_event* event) {

	//smart_str_appendc(str,'{');

	smart_str_appends(str,"\"lineno\":");
	smart_str_wrap_quotes_lc(str,event->lineno);
	smart_str_appends(str,"\"type\":");
	smart_str_wrap_quotes_lc(str,event->type);
	smart_str_appends(str,"\"event_type\":");
	smart_str_wrap_quotes_lc(str,event->event_type);
	smart_str_appends(str,"\"msg\":");
	smart_str_wrap_quotes_s(str,event->msg);
	//smart_str_appendc(str,'}');

}

static void parse_fcall(smart_str *str,tracer_fcall_entry* entry) {
	if(entry == NULL) return;

	parse_fcall_data(str,entry);
	smart_str_appendc(str,',');

	GSList *glist = NULL;

	glist = entry->fcall_list;
	smart_str_appends(str,"\"fcalls\":");
	smart_str_appendc(str,'[');	
	int i;
	if(glist) {
		
		for(i = 0; i < g_slist_length(entry->fcall_list); i++)
		{
		  	tracer_fcall_entry* fcall = (tracer_fcall_entry *)(glist->data);
			smart_str_appendc(str,'{');	
			 
			 parse_fcall(str,fcall);
			
			smart_str_appendc(str,'}');	
			glist = g_slist_next(glist);
			if(glist) smart_str_appendc(str,',');
		}

		
	}
	smart_str_appendc(str,']');
	
	smart_str_appendc(str,',');

	glist = entry->event_list;
	smart_str_appends(str,"\"events\":");
	smart_str_appendc(str,'[');
	if(glist) {

		for(i = 0; i < g_slist_length(entry->event_list); i++)
		{
			tracer_event* event = (tracer_event *)(glist->data);
			smart_str_appendc(str,'{');	
			 
			parse_event(str,event);

			smart_str_appendc(str,'}');
			glist = g_slist_next(glist);
			if(glist) smart_str_appendc(str,',');
		}
	}
		smart_str_appendc(str,']');
	

}

// static void parse_data() {
// 	TSRMLS_FETCH();
// 	tracer_fcall_entry *entry = TRACER_G(fcalls);
// 	tracer_request_info *request_info = &TRACER_G(request_info);

// } 
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
