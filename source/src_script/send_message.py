#!/usr/bin/env python3

import smtplib
import getpass
from email.mime.text import MIMEText

def send_message(message, from_address, to_address, smtp_source, password):
    msg=initialization_message(message, from_address, to_address)

    s = smtplib.SMTP(smtp_source)
    s.ehlo()
    s.starttls()
    s.login(from_address, password)
    s.sendmail(from_address, [to_address], msg.as_string())
    s.quit()

def initialization_message(message, from_address, to_address):
    msg=MIMEText(message,'plain','utf-8')
    msg['Subject'] = 'Invalid servers'
    msg['From'] = from_address
    msg['To'] = to_address

    return msg