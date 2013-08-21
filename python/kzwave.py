import os
from collections import defaultdict

rulemap = defaultdict(list)

def init():
	rules = os.listdir("rules")

	gvars = {'rulemap': rulemap }

	for rule in rules:
		f = open("rules/" + rule, "r")
		exec f in gvars

def runrules(node, nodeclass, location, name, data):
	for func in rulemap.get(node):
		if nodeclass == "COMMAND_CLASS_SWITCH_BINARY":
			func(location, name, data.get("value"))

if __name__ == "__main__":
	init()
