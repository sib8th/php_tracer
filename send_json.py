from elasticsearch import Elasticsearch
from elasticsearch import helpers
import json
import time




def delete_all_records(es):
	return;
	es.delete_by_query(index='php_tracer',doc_type='request', q={'match_all': {}});
	return;
	# es.delete(index='php_tracer',doc_type='request');
	# actions = [];
	# action = {
	# 	'_op_type':'delete',
	# 	'_index':'php_tracer',
	# 	'_type':'request',
	# 	'_id':''
	# }
	# actions.append(action)
	# helpers.bulk(es,actions)
	# del actions[0:len(actions)]

def bulk_insert(es):
	fintput = open('/home/liangzx/parse_result');
	single_jsstr = '';
	actions = [];
	# format = '%Y-%m-%d %X';
	format = '%Y.%m.%d';
	current_time = time.strftime(format,time.localtime(time.time()))
	while 1:
		line = fintput.readline();
		if not line:
			break
		if line == '\n':
			es_object = json.loads(single_jsstr);
			action = {
				"_index":"php_tracer",
				"_type":es_object['doc_type'],
				"_source":es_object['content']
			}
			actions.append(action);
			single_jsstr = '';
		else:
			single_jsstr += line;
	helpers.bulk(es,actions)
	del actions[0:len(actions)]
	fintput.close();
	foutput = open('/home/liangzx/parse_result','w');
	foutput.write('');
	foutput.close();

def main():
	es = Elasticsearch(['10.10.103.177:9200'])	
	choices = {'0':bulk_insert,'1':delete_all_records}
	# choices = {'0':bulk_insert}
	while 1:
		cmd = raw_input('input your choice: \n\t0 (insert)\n\t1 (delete)\n\tq (quit)\n');
		if cmd == 'q':
			break;
		else:
			choices.get(cmd)(es);

main();




#file = open('/home/liangzx/parse_result');
# single_jsstr = '';
# while 1:
# 	line = file.readline();
# 	if not line:
# 		break
# 	if line == '\n':
# 		es.index(index='php_tracer',doc_type='request',body=json.loads(single_jsstr));
# 		single_jsstr = '';
# 	else:
# 		single_jsstr += line;
# file.close();