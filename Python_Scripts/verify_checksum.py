import hashlib
import os

from dateutil import parser

def file_as_bytes(file):
    with file:
        return file.read()


workspace = os.environ.get("GITHUB_WORKSPACE")
source_file_path = 'Excavator/build/esp32.esp32.esp32/Excavator.ino.bin'  
source_file_name = os.path.join(workspace, source_file_path)

# filepath = "C:/Users/kumbh/Documents/Arduino/GIT Repo/ESP-32-IoT-Excavator/Excavator/firmware_20220219_134302_6.bin"
print(hashlib.md5(file_as_bytes(open(source_file_name, 'rb'))).hexdigest())


