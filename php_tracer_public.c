#include "php_tracer_public.h"

void smart_str_wrap_quotes_s(smart_str *str,const char* content) {
	if(content == NULL) {
		smart_str_appends(str,"null");	
		return;
	}
	smart_str_appends(str,"\"");
	smart_str_appends(str,content);
	smart_str_appends(str,"\"");
}
// void smart_str_wrap_quotes_l(smart_str *str,long content) {
// 	//smart_str_appends(str,"\"");
// 	smart_str_append_long(str,content);
// 	//smart_str_appends(str,"\"");
// }
void smart_str_wrap_quotes_sc(smart_str *str,const char* content) {
	
	if(content == NULL) {
		smart_str_appends(str,"null,");	
		return;
	}
	smart_str_appends(str,"\"");
	smart_str_appends(str,content);
	smart_str_appends(str,"\",");
}
void smart_str_wrap_quotes_lc(smart_str *str,long content) {
	//smart_str_appends(str,"\"");
	smart_str_append_long(str,content);
	//smart_str_appends(str,"\",");
	smart_str_appendc(str,',');
}
void generate_uuid(char* str){
	uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid,str);
    str;
}


