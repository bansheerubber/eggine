import re
import sys

enum_type = re.sub(r'(?<!^)(?=[A-Z])', '_', sys.argv[1]).upper()
print(f"GameObjectType getType() {{ return {enum_type}; }}")
