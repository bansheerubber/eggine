import os

process = os.popen("cd ../../; ./version.sh")
output = process.read().strip()
print(f"this->version = \"{output}\";")