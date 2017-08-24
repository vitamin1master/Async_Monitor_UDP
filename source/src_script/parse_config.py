#!/usr/bin/env python3

from src_script import data_analysis
import os
import json

def start_parse(config_path):
	config_py=data_analysis.get_json(config_path)

	if config_py:
		try:
			script_name = config_py["script_name"]
			result_old = config_py["result_old"]
			result_new = config_py["result_new"]
			error_log = config_py["error_log"]
			config = config_py["config"]

			addressee = config_py["addressee"]
			destination = config_py["destination"]
			smtp_source = config_py["smtp_source"]
			password = config_py["password"]

		except KeyError:
			return False

		data = script_name, result_old, result_new, error_log, config, addressee, destination, smtp_source, password
		return data