#!/usr/bin/env python3
from email.mime.text import MIMEText

import smtplib
import os
import json

config='--config_path=../etc/config.default.json'
record='--record_path=../'
result_old='result_old.json'
result_new='result_new.json'
result_intermediate='result_intermediate.json'
ferror='2>../error.log'
monitor_start='./stun_monitor'
invalid_servers=''

os.system(monitor_start + ' ' + config + ' '+record + result_intermediate + ' ' + ferror)
if os.path.getsize('../error.log') == 0:
	if os.path.exists('../'+result_old):
		os.remove('../'+result_old)
	if os.path.exists('../'+result_new):
		os.rename('../'+result_new,'../'+result_old)
	os.rename('../'+result_intermediate,'../'+result_new)

	if os.path.exists('../'+result_old):
		with open('../'+result_old) as data_file:
			data_old=json.load(data_file)
		with open('../'+result_old) as data_file:
			data_new=json.load(data_file)

		for itnew in data_new["Servers"]:
			if itnew["IsActive"] == 'No':
				for itold in data_old["Servers"]:
					if itnew["IP"] == itold["IP"] and itold["IsActive"] == 'No':
						invalid_servers+=itnew["IP"]+'\n'

if invalid_servers:
	msg=MIMEText(invalid_servers,'plain','utf-8')
	to_addr='vitamin1master@gmail.com'
	password='seriyboyko1'
	from_addr='vitamin1master@live.com'

	msg['Subject'] = 'Invalid servers'
	msg['From'] = from_addr
	msg['To'] = to_addr

	s = smtplib.SMTP('smtp-mail.outlook.com')
	s.ehlo()
	s.starttls()
	s.login(from_addr, password)
	s.sendmail(from_addr, [to_addr], msg.as_string())
	s.quit()
