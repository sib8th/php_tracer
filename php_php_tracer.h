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

#ifndef PHP_PHP_TRACER_H
#define PHP_PHP_TRACER_H

extern zend_module_entry php_tracer_module_entry;
#define phpext_php_tracer_ptr &php_tracer_module_entry

#ifdef PHP_WIN32
#	define PHP_PHP_TRACER_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PHP_TRACER_API __attribute__ ((visibility("default")))
#else
#	define PHP_PHP_TRACER_API
#endif

//extern "C" {
#ifdef ZTS
#include "TSRM.h"
#endif

#include <sys/time.h>
#include <time.h>
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>  
#include "php_globals.h"

//}
//#include <iostream>
//#include <string>
//#include <vector>
//using namespace std;
PHP_MINIT_FUNCTION(php_tracer);
PHP_MSHUTDOWN_FUNCTION(php_tracer);
PHP_RINIT_FUNCTION(php_tracer);
PHP_RSHUTDOWN_FUNCTION(php_tracer);
PHP_MINFO_FUNCTION(php_tracer);


const char* get_node_type(int type);
const char* get_event_type(int i);
const char* get_error_name(int type);

typedef enum { false, true }bool;

/*Error or Caught Exception*/
typedef struct tracer_event{

  uint lineno;
  char *msg;
  int type;
  int event_type;

}tracer_event;

typedef struct tracer_fcall{

  long start,end;
  long interval;
  char *scope_name;

  uint lineno;
  int type;
  char **arguments;
  char **parameters;
  ulong arg_count;
  ulong param_count;
  
} tracer_fcall;

typedef struct tracer_fcall_entry{

  tracer_fcall data;

  struct tracer_fcall_entry * pre_fcall;

  GSList * fcall_list;
  GSList * event_list;

}tracer_fcall_entry;

typedef struct tracer_request_info{
   zval **host;
   zval **ip;
   zval **uri;
   zval **script_name;
   zval **ts;
   zval **method;
   zend_bool is_set;
}tracer_request_info;


ZEND_BEGIN_MODULE_GLOBALS(php_tracer)

	long module_start;
  long module_end;
  tracer_fcall_entry *fcalls;
  tracer_fcall_entry *current_fcall;
  tracer_request_info request_info;

ZEND_END_MODULE_GLOBALS(php_tracer)



/* In every utility function you add that needs to use variables 
   in php_php_tracer_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as PHP_TRACER_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#ifdef ZTS
#define TRACER_G(v) TSRMG(php_tracer_globals_id, zend_php_tracer_globals *, v)
#else
#define TRACER_G(v) (php_tracer_globals.v)
#endif


#define NODE_ENTRY 0
#define NODE_DB 1
#define NODE_EXTERNAL 2
#define NODE_USERDEF 3
#define NODE_TYPE(index)  get_node_type(index)
#define TRACER_CREATE_FCALL(name)  \
do {\
name = (tracer_fcall_entry *) emalloc(sizeof(tracer_fcall_entry)); \
name->data.scope_name = (char *)emalloc(100*sizeof(char)); \
name->data.type = 0; \
name->data.arguments = NULL; \
name->data.parameters = NULL; \
name->data.param_count = 0; \
name->data.arg_count = 0; \
name->pre_fcall = NULL; \
name->fcall_list = NULL; \
name->event_list = NULL; \
} while(0)

 

# define zend_is_auto_global_str(name) (zend_is_auto_global(ZEND_STRL((name)) TSRMLS_CC))
#define TRACER_ERROR 0
#define TRACER_EXCEPTION 1
#define EVENT_TYPE(index)  get_event_type(index)
#define ERROR_NAME(index) get_error_name(index)
#define TRACER_CREATE_EVENT(name)  \
do {\
name = (tracer_event *) emalloc(sizeof(tracer_event)); \
name->msg = (char *)emalloc(500*sizeof(char)); \
name->type = TRACER_ERROR; \
name->lineno = 0; \
} while(0)



#define TRACER_RI(element) (TRACER_G(request_info).element)
#define TRACER_RI_STRVAL(var)  Z_STRVAL_PP(TRACER_RI(var))
#define TRACER_RI_LVAL(var)  Z_LVAL_PP(TRACER_RI(var))
#define TRACER_INIT_REQUEST(name) \
  //name = (tracer_request_info *) malloc(sizeof(tracer_request_info)); \
  name.host = (char *)emalloc(200*sizeof(char)); \
  name.ip = (char *)emalloc(100*sizeof(char)); \
  name.url = (char *)emalloc(500*sizeof(char)); \
  name.method = (char *)emalloc(50*sizeof(char)); \
  name.script_name = (char *)emalloc(500*sizeof(char)); \

#define zend_is_auto_global_str(name) (zend_is_auto_global(ZEND_STRL((name)) TSRMLS_CC))
#define TRACER_FD(fcall) ((fcall)->data)
#define SET_REQUEST_INFO(name, dest, type) \
  zend_hash_find(Z_ARRVAL_P(tmp), name, sizeof(name), (void**)&TRACER_RI(dest))
#define FETCH_HTTP_GLOBALS(name) (tmp = PG(http_globals)[TRACK_VARS_##name])


#define TRACER_COPY_STRING(dst,src)  \
strcpy(dst,src)
#define TRACER_ADD_TO_LIST(list,object) \
do { \
list = g_slist_append(list,object); \
} while(0)


static void obtain_request_info();
static bool load_parameters(tracer_fcall_entry* entry,zend_execute_data *execute_data);
static bool load_arguments(tracer_fcall_entry* entry, zend_execute_data *execute_data);
static const char  *convert_arguments(ulong arg_count,char** arguments,bool is_param);
static void parse_data();
static void parse_fcall(smart_str *str,tracer_fcall_entry* entry);
static void parse_fcall_data(smart_str *str, tracer_fcall_entry *entry);
static void parse_event(smart_str *str,tracer_event* event);
static void parse_request(smart_str *str);
static void parse_trace(smart_str *str);
static void print_and_free_trace(tracer_fcall_entry *entry, int level);
static void print_request_data();

#endif	/* PHP_PHP_TRACER_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
