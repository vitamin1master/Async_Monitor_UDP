#!/usr/bin/env python3

import os
import json

def start_analysis(script_name, config, result_new, result_old, ferror):
	command = initialization_files(script_name, config,result_new, ferror)
	message=''

	if os.system(command) == 0:
		if os.path.exists(result_old):
			message=data_analysis(result_old,result_new)
		end_of_analysis(result_old,result_new)
	else:
		message='Monitor errors log: ' + get_str_file(ferror)
	return message

def initialization_files(script_name, config, result_new, ferror):
	if os.path.exists(result_new):
		os.remove(result_new)
	command='./' + script_name + ' ' + '--config_path=' + config + \
			' ' + '--record_path=' + result_new + ' ' + '2>' + ferror
	return command

def data_analysis(result_old, result_new):
	data_old = get_json(result_old)
	data_new = get_json(result_new)
	message = ''

	if data_old == False or data_old == False:
		message = 'data_analysis.py: Could not open ' + result_old
		message += ' or ' + result_new
		return message
		
	invalid_servers=''
	try:
		for itnew in data_new["Servers"]:
			if itnew["IsActive"] == 'No':
				for itold in data_old["Servers"]:
					if itnew["IP"] == itold["IP"] and itold["IsActive"] == 'No':
						invalid_servers+=itnew["IP"]+'\n'

		if invalid_servers:
			message='Non-working servers:\n'+invalid_servers

	except KeyError:
		message='data_analysis.py: Invalid monitoring result'

	return message

def get_json(file):
	if os.path.exists(file):
		try:
			with open(file) as json_data:
				data = json.load(json_data)
		except ValueError:
			data=False
	else:
		data = False
	return data

def end_of_analysis(result_old, result_new):
	if os.path.exists(result_old):
		os.remove(result_old)
	os.rename(result_new,result_old)

def get_str_file(file):
	if os.path.exists(file):
		with open(file) as data:
			text = data.read()
	else:
		message='data_analysis.py: Could not open file'
		return message
	return text
