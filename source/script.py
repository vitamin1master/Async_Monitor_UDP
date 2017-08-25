#!/usr/bin/env python3
import smtplib
from email.mime.text import MIMEText
import os
import json
import argparse

def parse_command_line():
	parser = argparse.ArgumentParser()

	parser.add_argument('exe_path', action='store', type=str, help='Path to executable file')
	parser.add_argument('config_path', action='store', type=str, help='Path to the executable configuration file')
	parser.add_argument('result_old_path', action='store', type=str, help='Path to old file with results')
	parser.add_argument('result_new_path', action='store', type=str, help='Path to new file with results')
	parser.add_argument('error_log_path', action='store', type=str, help='Path to error log')

	parser.add_argument('from_address', action='store', type=str, help='Mail form which the message will be sent')
	parser.add_argument('to_address', action='store', type=str, help='Mail to which the message will be sent')

	args=parser.parse_args()

	data= args.exe_path, args.config_path, args.result_old_path, args.result_new_path, args.error_log_path, args.from_address, args.to_address

	return data

def send_message(message, from_address, to_address):
	msg=initialization_message(message, from_address, to_address)

	s = smtplib.SMTP('localhost')
	s.sendmail(from_address, [to_address], msg.as_string())
	s.quit()

def initialization_message(message, from_address, to_address):
	msg=MIMEText(message,'plain','utf-8')
	msg['Subject'] = 'stun_monitor'
	msg['From'] = from_address
	msg['To'] = to_address

	return msg

def get_str_file(file):
	text = ''
	if os.path.exists(file):
		with open(file) as data:
			text = data.read()
	else:
		return False
	return text

def get_json(file):
	data=''
	if os.path.exists(file):
		try:
			with open(file) as json_data:
				data = json.load(json_data)
		except ValueError:
			return False
	else:
		return False
	return data

class data_analyzer:
	def __init__(self, _executable_file, _config_path, _result_new, _result_old, _error_log):
		self.__executable_file = _executable_file
		self.__config_path = _config_path
		self.__result_new = _result_new
		self.__result_old = _result_old
		self.__error_log = _error_log

		if os.path.exists(_result_new):
			os.remove(_result_new)
		self.__command='./' + _executable_file + ' ' + '--config_path=' + _config_path + \
			' ' + '--record_path=' + _result_new + ' ' + '2>' + _error_log

	def start_data_analysis(self):
		message = ''

		if os.system(self.__command) == 0:
			if os.path.exists(self.__result_old):
				message=self.__data_analysis__()
			self.__end_of_analysis__()
		else:
			message='Monitor errors log: ' + get_str_file(self.__error_log)
		return message

	def __end_of_analysis__(self):
		if os.path.exists(self.__result_old):
			os.remove(self.__result_old)
		os.rename(result_new,result_old)

	def __data_analysis__(self):
		data_old = get_json(self.__result_old)
		data_new = get_json(self.__result_new)
		message = ''

		if data_old == False or data_old == False:
			message = 'data_analysis.py: Could not open ' + self.__result_old
			message += ' or ' + self.__result_new
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


data = parse_command_line()
if data:
	exe_path = data[0]
	config_path = data[1]
	result_old = data[2]
	result_new = data[3]
	error_log = data[4]
	
	from_address = data[5]
	to_address = data[6]

	analyzer = data_analyzer(exe_path, config_path, result_new, result_old, error_log)
	message = analyzer.start_data_analysis()

	if message:
		send_message(message, from_address, to_address)
else:
	print('script.py: Could not parse command line')