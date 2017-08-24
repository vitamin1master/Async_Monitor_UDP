#!/usr/bin/env python3

from src_script import data_analysis, send_message, parse_command_line, parse_config

config_py=parse_command_line.start_parse_command_line()
data = parse_config.start_parse(config_py)
if data:
	script_name = data[0]
	result_old = data[1]
	result_new = data[2]
	error_log = data[3]
	config = data[4]
	
	addressee = data[5]
	destination = data[6]
	smtp_source = data[7]
	password = data[8]

	message = data_analysis.start_analysis(script_name, config, result_new, result_old, error_log)

	if message:
	    send_message.send_message(message, addressee, destination, smtp_source, password)
else:
	print('script.py: Could not parse ' + config_py)