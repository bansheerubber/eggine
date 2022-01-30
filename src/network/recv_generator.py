import sys
sys.path.insert(0, "../../tools")
from gen import get_generated_code

# argv1: stream, argv2: message length, argv3: address
CODE = get_generated_code("udp", "recv", 3)
CODE = CODE.replace("%%stream%%", sys.argv[1])
CODE = CODE.replace("%%length%%", sys.argv[2])
CODE = CODE.replace("%%address%%", sys.argv[3])
print(CODE)
