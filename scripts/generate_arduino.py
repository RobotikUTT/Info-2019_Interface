#!/usr/bin/python3

import os
from xml.etree import ElementTree

interface_dir = os.path.dirname(os.path.realpath(__file__)) + "/../"

output_file = interface_dir + "arduino/protocol.gen.hpp"
input_file = interface_dir + "can/frames.xml"

root = ElementTree.parse(input_file).getroot()

content = [
	"// PROTOCOL (frame_id, param_length, [params sizes]) --> generated by python"
]
names = []

for frame in root:
	name = frame.attrib["name"]
	id = int(frame.attrib["id"])
	params = []

	names.append(name.upper())

	for param in frame:
		if param.tag == "description":
			continue

		param_name = param.attrib["name"]
		param_size = 1 if param.tag == "byte" else 2

		params.append((param_name, param_size))

	content.append("const uint8_t {}[] = {{{}, {}}};"
		.format(name.upper(), id, ", ".join([str(len(params))] + [str(size) for name, size in params])))

content.append("const uint8_t[] FRAMES[] = {{{}}};".format(", ".join(names)))
content.append("const int FRAMES_LENGTH = {};".format(len(names)))
content.append("// END OF PROTOCOL")

file = open(output_file, "w")
file.write("\n".join(content))
file.write("\n")
file.close()
