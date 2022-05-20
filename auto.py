#!/bin/env python3

import sys
import pyinotify
import subprocess
import os
from time import time, sleep

class EventHandler(pyinotify.ProcessEvent):
	def process_IN_CREATE(self, event):
		make()
	
	def process_IN_MODIFY(self, event):
		make()

last_compile = 0
target = sys.argv[1] if len(sys.argv) == 2 else ""

def make():
	global last_compile
	global target
	if time() - last_compile > 1:
		if target:
			if target == "docker":
				os.system("docker cp src/. eggine:/eggine/src")
			else:
				os.system(f"make {target} -j8")
		else:
			os.system(f"make -j8")

		print("---------------------------------------------------------------------------------")

		last_compile = time()

if target:
	if target == "docker":
		os.system("docker kill eggine")
		os.system("docker run -d --rm --name eggine -it ghcr.io/bansheerubber/eggine-linux:latest")
		os.system("docker cp src/. eggine:/eggine/src")
	else:
		os.system("make clean")
		os.system(f"make {target} -j8")
else:
	os.system("make clean")
	os.system(f"make -j8")

print("---------------------------------------------------------------------------------")

handler = EventHandler()
watch_manager = pyinotify.WatchManager()
notifier = pyinotify.Notifier(watch_manager, handler)
wdd = watch_manager.add_watch("./src/", pyinotify.IN_CREATE | pyinotify.IN_MODIFY, rec=True)
wdd = watch_manager.add_watch("./resources/", pyinotify.IN_CREATE | pyinotify.IN_MODIFY, rec=True)
notifier.loop()
