import os
import pathlib
import re
from shutil import copyfile
import subprocess
import sys

total_lines = 0

def write_file(filename, contents):
	file = open(filename, "w", encoding='utf-8')
	for line in contents:
		file.write(line)
	file.close()

ts_definition_headers = []
ts_definition_functions = []

def preprocess(filename, contents, directory = None):
	global total_lines
	global ts_definition_headers
	global ts_definition_functions

	if "include" not in filename:
		total_lines = total_lines + len(contents)
	
	pattern = r'^[\s]*?##(.+)$'
	new_contents = []
	read_namespace = False
	for line in contents:
		if "namespace ts" in line:
			read_namespace = True
		elif read_namespace:
			if "void define" in line:
				ts_definition_headers.append(filename)
				ts_definition_functions.append(re.compile(r"^void ([\w]+)").match(line.strip()).group(1))
				read_namespace = False
			else:
				read_namespace

		if match := re.match(pattern, line):
			if directory == None:
				directory = pathlib.Path(filename).parent

			command = match.group(1).strip()

			if ".py" in command:
				command = f"cd {directory} && python3 {command}"
			else:
				command = f"cd {directory} && {command}"

			process = os.popen(command)
			output = process.read()
			if process.close() != None:
				print("Encountered preprocessor error:", file=sys.stderr)
				print(output, file=sys.stderr)
				exit(1)
			else:
				new_contents.append(output)
		else:
			new_contents.append(line)
	return new_contents

extensions = [".cc", ".h", ".frag", ".vert"]
glsl_extensions = [".frag", ".vert"]

if __name__ == "__main__":
	for root, subdirs, files in os.walk("./src"):
		files = [f"{root}/{file}" for file in files]
		for file in files:
			file = file.replace("\\", "/")
			file_object = pathlib.Path(file)
			tmp_file = file.replace("./src/", "./tmp/")
			tmp_file_object = pathlib.Path(tmp_file)
			
			extension = file_object.suffix
			parent = file_object.parent
			tmp_parent = tmp_file_object.parent

			if extension in extensions:
				opened_file = open(file_object, "r", encoding='utf-8')
				file_contents = opened_file.readlines()
				opened_file.close()

				if extension in glsl_extensions:
					file_contents.insert(0, "R\"\"(\n")
					file_contents.append(")\"\"\n")
				
				if os.path.exists(tmp_file):
					src_time = file_object.stat().st_mtime
					tmp_time = tmp_file_object.stat().st_mtime

					if src_time > tmp_time: # recopy file if source file is newer than tmp file
						write_file(tmp_file, preprocess(file, file_contents))
					elif extension == ".h":
						preprocess(file, file_contents) # always rescan header files for stuff
				else:
					os.makedirs(tmp_parent, exist_ok=True)
					write_file(tmp_file, preprocess(file, file_contents))
	
	# handle torquescript.h
	torquescript_header = "./src/engine/torquescript.h"
	torquescript_header_tmp = "./tmp/engine/torquescript.h"
	torquescript_header_contents = []
	file = open(torquescript_header)
	for line in file.readlines():
		torquescript_header_contents.append(line)

		if "#pragma once" in line:
			for header in ts_definition_headers:
				tmp_file = header.replace("./src/", "./tmp/")
				relative_path = os.path.relpath(tmp_file, "./tmp/engine/")
				torquescript_header_contents.append(f"#include \"{relative_path}\"")
	
	file.close()
	write_file(torquescript_header_tmp, torquescript_header_contents)

	# handle torquescript.cc
	torquescript_code = "./src/engine/torquescript.cc"
	torquescript_code_tmp = "./tmp/engine/torquescript.cc"
	torquescript_code_contents = []
	file = open(torquescript_code)
	for line in file.readlines():
		torquescript_code_contents.append(line)

		if "torquescriptDefinitions" in line:
			for function in ts_definition_functions:
				torquescript_code_contents.append(f"ts::{function}();")
	
	file.close()
	write_file(torquescript_code_tmp, torquescript_code_contents)
