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
int get_time_zone() {
	time_t time_utc;  
    struct tm tm_local;  
  
    // Get the UTC time  
    time(&time_utc);  
  
    // Get the local time  
    // Use localtime_r for threads safe  
    localtime_r(&time_utc, &tm_local);  
  
    time_t time_local;  
    struct tm tm_gmt;  
  
    // Change tm to time_t   
    time_local = mktime(&tm_local);  
  
    // Change it to GMT tm  
    gmtime_r(&time_utc, &tm_gmt);  
  
    int time_zone = tm_local.tm_hour - tm_gmt.tm_hour;  
    if (time_zone < -12) {  
        time_zone += 24;   
    } else if (time_zone > 12) {  
        time_zone -= 24;  
    }
    return time_zone;
}
void  convert_ts_tz(smart_str *str,struct timeval in_time) {
	char timef[100];
	strftime(timef,100, "%Y-%m-%d %H:%M:%S",localtime(&in_time.tv_sec));
	char time_c[100];
	sprintf(time_c,"%s.%03d%+03d00",timef,in_time.tv_usec / 1000,get_time_zone());
	smart_str_appends(str,time_c);
}
char* convert_str_pp(zval ** val) {
	if((val) == NULL) return "null";
	return (char*)Z_STRVAL_PP(val);
}
ulong convert_l_pp(zval ** val) {
	if((val) == NULL) return 0;
	return (ulong)Z_LVAL_PP(val);
} 

