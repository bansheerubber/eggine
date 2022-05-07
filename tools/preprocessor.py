import os
import pathlib
import re
from shutil import copyfile
import subprocess
import sys
import json
import math
import hashlib

total_lines = 0

def flatten(t):
	return [item for sublist in t for item in sublist]

def write_file(filename, contents):
	file = open(filename, "w", encoding='utf-8')
	for line in contents:
		file.write(line)
	file.close()

environment_variables = {}
es_definition_headers = []
es_definition_functions = []
max_depth = 0
files_with_custom_depth = set()

game_object_type_enums = []

keywords = ["game_object_definitions"]

command_pattern = r'^[\s]*?// ##(\d{0,5}) (.+)$'

in_place_extensions = [".h.src"]
extensions = [".cc", ".h", ".h.src", ".frag", ".vert"]
glsl_extensions = [".frag", ".vert"]

extension_regex = re.compile(r"\.[\w0-9_.]+$")

snake_case_regex = re.compile(r"(?<!^)(?=[A-Z])")
def to_snake_case(name):
	global snake_case_regex
	return snake_case_regex.sub("_", name).upper()

network_property_list = {}
class_inheritance_list = {}
network_class_list = []
np_type_to_write = {
	"NP_NUMBER": "writeNumber",
	"NP_VECTOR": "writeVector",
}

np_type_to_read = {
	"NP_NUMBER": "readNumber",
	"NP_VECTOR": "readVector",
}

def get_network_hash():
	hash_string = ""
	for _, network_class in network_class_list:
		hash_string += network_class
		for property in network_property_list[network_class]:
			hash_string += property["np_type"]
			hash_string += "None" if property["np_setter"] == None else property["np_setter"]
			hash_string += property["name"]
	return hashlib.md5(hash_string.encode()).hexdigest()

def remote_object_get_mask_position(classname, property_name):
	for i in range(0, len(network_property_list[classname])):
		property = network_property_list[classname][i]
		if property["name"] == property_name:
			return i
	
	raise Exception(f"could not find property name '{property_name}' for mask")

def remote_object_class_id(classname):
	for i in range(0, len(network_class_list)):
		_, name = network_class_list[i]
		if name == classname:
			return i
	return -1

def remote_object_instantiation(contents):
	for i in range(0, len(network_class_list)):
		classname = network_class_list[i][1]
		contents.append(f"case {i}: {{\n")
		contents.append(f"object = new {classname}();\n")
		contents.append("}\n")

def remote_object_instantiation_headers(contents):
	for filename, _ in network_class_list:
		filename = filename.replace("./src/", "")
		contents.append(f'#include "../{filename}"\n')

def remote_object_definitions(contents, classname):
	global allocate_mask

	if classname in network_property_list:
		contents.append("protected:\n")
		contents.append(allocate_mask.replace("%%size%%", str(math.ceil(len(network_property_list[classname]) / 8))))
		contents.append("public:\n")

	contents.append(get_class_id.replace("%%id%%", str(remote_object_class_id(classname))) + "\n")

	sends_contents = []
	unpack_contents = []
	mask_array = []
	mask_length = 0

	if classname in network_property_list:
		properties = network_property_list[classname]
		mask_length = len(properties)
		for property in properties:
			mask = remote_object_get_mask_position(classname, property["name"])
			mask_array.append((mask, property["name"]))
			sends_contents.append(f"\t\t\tif(packet->stream.queryMask(this, {mask})) {{\n")
			sends_contents.append(f"""\t\t\t\tpacket->stream.{np_type_to_write[property["np_type"]]}(this->{property["name"]});\n""")
			sends_contents.append("\t\t\t}\n\n")

			unpack_contents.append(f"\t\t\tif(mask.read({mask})) {{\n")
			if property["np_setter"] == None:
				unpack_contents.append(f"""\t\t\t\tthis->{property["name"]} = stream.{np_type_to_read[property["np_type"]]}<{property["type"]}>();\n""")
			else:
				unpack_contents.append(f"""\t\t\t\tthis->{property["np_setter"]}(stream.{np_type_to_read[property["np_type"]]}<{property["type"]}>());\n""")
			unpack_contents.append("\t\t\t}\n\n")

	contents.append(pack.replace("%%sends%%", "".join(sends_contents)))
	contents.append(unpack.replace("%%unpacks%%", "".join(unpack_contents)))

	mask_array = sorted(mask_array)
	mask_array = "".join([f'"{i[1]}", ' for i in mask_array])
	contents.append(
		property_to_mask.replace("%%array%%", f"{{ {mask_array}}}").replace("%%length%%", str(mask_length))
	)

class_regex = re.compile(r"class ([a-zA-Z]+).+?{")
inheritance_regex = re.compile(r"public (?:(?:[a-zA-Z]+::([a-zA-Z]+))|([a-zA-Z]+))")
np_property_regex = re.compile(r"NP_PROPERTY\((.+)\)")
np_setter_regex = re.compile(r".+::(.+)")
def handle_network(filename, contents):
	current_class = None
	read_property = False
	for line in contents:
		if match := class_regex.search(line):
			current_class = match.group(1)

			if match := inheritance_regex.findall(line):
				class_inheritance_list[current_class] = list(filter(len, flatten(match)))
		
		if read_property:
			if current_class not in network_property_list:
				network_property_list[current_class] = []
			
			split = line.strip().split("=")[0].strip().split(" ")
			network_property_list[current_class].append({
				"filename": filename,
				"type": " ".join(split[:-1]),
				"np_type": np_type,
				"np_setter": np_setter,
				"name": split[-1].replace(";", ""),
				"inherited": False,
			})

			read_property = False
		elif "NP_PROPERTY(" in line:
			read_property = True
			arguments = [i.strip() for i in np_property_regex.search(line).group(1).split(",")]
			np_type = arguments[0]
			if len(arguments) > 1 and (match := np_setter_regex.match(arguments[1])):
				np_setter = match.group(1)
			else:
				np_setter = None

# headers are always scanned, because they have information that is needed in the preprocessing of other files
def handle_headers(filename, contents):
	global es_definition_headers
	global es_definition_functions
	global game_object_type_enums
	
	read_namespace = False
	for line in contents:
		if "namespace es" in line:
			read_namespace = True
			order = re.compile(r"// order = (\d+)").search(line.strip())
		elif read_namespace:
			if "void define" in line:
				es_definition_headers.append(filename)

				if order == None:
					order = 0
				else:
					order = int(order.group(1))

				es_definition_functions.append((re.compile(r"^void ([\w]+)").match(line.strip()).group(1), order))
				read_namespace = False
		
		if match := re.match(command_pattern, line):
			command = match.group(2).strip()
			if "game_object_definitions" in command:
				rest = " ".join(command.split(" ")[1:])
				game_object_type_enums.append(to_snake_case(rest.split(" ")[0]))
			elif "remote_object_definitions" in command:
				rest = " ".join(command.split(" ")[1:])
				if rest not in network_class_list:
					network_class_list.append((filename, rest))

def preprocess(filename, contents, depth):
	global total_lines
	global es_definition_headers
	global es_definition_functions
	global max_depth
	global files_with_custom_depth

	if "include" not in filename:
		total_lines = total_lines + len(contents)
	
	new_contents = []
	read_namespace = False
	for line in contents:
		if match := re.match(command_pattern, line):
			directory = pathlib.Path(filename).parent

			requested_depth = match.group(1)
			command = match.group(2).strip()

			if requested_depth != None and requested_depth != "":
				requested_depth = int(requested_depth)
				max_depth = max(requested_depth, max_depth)
			else:
				requested_depth = 0
			
			if requested_depth != 0 and filename not in files_with_custom_depth:
				files_with_custom_depth.add(filename)
			
			if requested_depth != depth:
				new_contents.append(line)
				continue

			if command.split(" ")[0].strip() in keywords:
				continue
				
			if "game_object_enums" in command:
				new_contents.append("INVALID = 0,\n")
				for enum in game_object_type_enums:
					new_contents.append(f"{enum},\n")
				continue

			if "remote_object_definitions" in line:
				rest = " ".join(command.split(" ")[1:])
				remote_object_definitions(new_contents, rest)
				continue
		
			if "remote_object_instantiation" in line:
				remote_object_instantiation(new_contents)
				continue
			
			if "remote_object_headers" in line:
				remote_object_instantiation_headers(new_contents)
				continue
		
			if "remote_object_checksum" in line:
				new_contents.append(get_checksum.replace("%%checksum%%", get_network_hash()))
				continue

			if ".py" in command:
				command = f"cd {directory} && python3.10 {command}"
			else:
				command = f"cd {directory} && {command}"

			process = os.popen(command)
			output = process.read()
			if process.close() != None:
				print(f"Encountered preprocessor command error in '{filename}':", file=sys.stderr)
				print(output, file=sys.stderr)
				exit(1)
			else:
				new_contents.append(output)
		else:
			new_contents.append(line)
	return new_contents

def get_remote_object_code(file, number_of_tabs):
	tabs = ""
	for i in range(0, number_of_tabs):
		tabs = tabs + "\t"

	with open(f"src/basic/gen/remoteObject/{file}.cc.tmpl") as start:
		return ''.join(preprocess("", [f"{tabs}{line}" for line in start.readlines()], "."))

def get_env_commands():
	output = ""
	global environment_variables
	for variable, value in environment_variables.items():
		output = f"{output} {variable}='{value}'"
	return output.strip()

def handle_file(file, depth = 0):
	global in_place_extensions
	global extensions
	global glsl_extensions
	
	file = file.replace("\\", "/")
	original_file = file
	file_object = pathlib.Path(file)
	tmp_file = file.replace("./src/", "./tmp/")

	if ".src" in file:
		tmp_file = file.replace(".src", ".gen")

	tmp_file_object = pathlib.Path(tmp_file)

	if depth > 0:
		file = tmp_file # modify files in-place after depth 0

	extension = extension_regex.search(original_file)
	if extension == None:
		return
	else:
		extension = extension.group(0)

	parent = file_object.parent
	tmp_parent = tmp_file_object.parent

	if extension in extensions:
		opened_file = open(file, "r", encoding='utf-8')
		file_contents = opened_file.readlines()
		opened_file.close()

		if extension in glsl_extensions:
			file_contents.insert(0, "R\"\"(\n")
			file_contents.append(")\"\"\n")
		
		if os.path.exists(tmp_file):
			src_time = file_object.stat().st_mtime
			tmp_time = tmp_file_object.stat().st_mtime

			if src_time > tmp_time or depth > 0 or ".src" in file: # recopy file if source file is newer than tmp file
				write_file(tmp_file, preprocess(original_file, file_contents, depth))
		else:
			os.makedirs(tmp_parent, exist_ok=True)
			write_file(tmp_file, preprocess(original_file, file_contents, depth))
		
		# handle headers
		if extension == ".h" and depth == 0:
			handle_headers(original_file, file_contents)
			handle_network(original_file, file_contents)
	
	if ".src" in original_file: # copy the generated file over to the /tmp directory
		new_tmp_file = file.replace("./src/", "./tmp/")
		copyfile(tmp_file, new_tmp_file)

allocate_mask = ""
pack = ""
unpack = ""
property_to_mask = ""
get_class_id = ""
get_checksum = ""

if __name__ == "__main__":
	allocate_mask = get_remote_object_code("allocateMask", 2)
	pack = get_remote_object_code("pack", 2)
	unpack = get_remote_object_code("unpack", 2)
	property_to_mask = get_remote_object_code("propertyToMaskPosition", 2)
	get_class_id = get_remote_object_code("getClassId", 2)
	get_checksum = get_remote_object_code("checksum", 3)
	
	total_files = []
	for root, subdirs, files in os.walk("./src"):
		files = [f"{root}/{file}" for file in files]
		for file in files:
			total_files.append(file)
			handle_file(file)

	# handle network class stuff
	def recurse_network_properties(original_class, classname):
		if classname[1] not in class_inheritance_list:
			return

		for inherited_class in class_inheritance_list[classname[1]]:
			if inherited_class in network_property_list:
				properties = network_property_list[inherited_class]
				for property in properties:
					if property["inherited"] == False:
						if original_class[1] not in network_property_list:
							network_property_list[original_class[1]] = []
						
						network_property_list[original_class[1]].append({
							"filename": property["filename"],
							"type": property["type"],
							"np_type": property["np_type"],
							"np_setter": property["np_setter"],
							"name": property["name"],
							"inherited": True,
						})
			recurse_network_properties(original_class, inherited_class)

	for network_class in network_class_list:
		recurse_network_properties(network_class, network_class)

	environment_variables["game_object_type_enums"] = json.dumps(game_object_type_enums)

	for depth in range(1, max_depth + 1):
		for file in files_with_custom_depth:
			handle_file(file, depth)
	
	# handle eggscript.h
	eggscript_header = "./src/engine/eggscript.h"
	eggscript_header_tmp = "./tmp/engine/eggscript.h"
	eggscript_header_contents = []
	file = open(eggscript_header)
	for line in file.readlines():
		eggscript_header_contents.append(line)

		if "#pragma once" in line:
			for header in es_definition_headers:
				tmp_file = header.replace("./src/", "./tmp/")
				relative_path = os.path.relpath(tmp_file, "./tmp/engine/")
				eggscript_header_contents.append(f"#include \"{relative_path}\"\n")
	
	file.close()
	write_file(eggscript_header_tmp, eggscript_header_contents)

	# handle eggscript.cc
	es_definition_functions.sort(key=lambda item: item[1])
	eggscript_code = "./src/engine/eggscript.cc"
	eggscript_code_tmp = "./tmp/engine/eggscript.cc"
	eggscript_code_contents = []
	file = open(eggscript_code)
	for line in file.readlines():
		eggscript_code_contents.append(line)

		if "eggscriptDefinitions" in line:
			for function, _ in es_definition_functions:
				eggscript_code_contents.append(f"es::{function}();\n")
	
	file.close()
	write_file(eggscript_code_tmp, eggscript_code_contents)

	# compile shaders for switch
	for root, subdirs, files in os.walk("./resources/"):
		for file in files:
			regex = re.compile("(vert$)|(frag$)")
			if regex.findall(file):
				stage = "vert" if ".vert" in file else "frag"
				os.system(f"uam {root}/{file} -s {stage} -o {root}/{file}.dksh")
	
	print("Packing carton...")
	os.system(f"carton pack resources --output ./dist/out.carton")
	print("Done")
