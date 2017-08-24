#!/usr/bin/env python3

import argparse

def start_parse_command_line():
    parser = argparse.ArgumentParser()
    parser.add_argument('config_path', action='store', type=str, help='Enter config path')

    args=parser.parse_args()
    return args.config_path
