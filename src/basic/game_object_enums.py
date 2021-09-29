import os
import json

print("INVALID = 0,")
for enum in json.loads(os.getenv("game_object_type_enums")):
	print(f"{enum},")