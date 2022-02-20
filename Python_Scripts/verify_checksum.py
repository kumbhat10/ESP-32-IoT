import hashlib
import os
import firebase_admin
from firebase_admin import credentials, db, messaging
from google.cloud import storage
from dateutil import parser
from dateutil import parser

def file_as_bytes(file):
    with file:
        return file.read()

workspace = os.environ.get("GITHUB_WORKSPACE")
source_file_path = 'Excavator/build/esp32.esp32.esp32/Excavator.ino.bin'  
source_file_name = os.path.join(workspace, source_file_path)
commit_timestamp = parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%Y%m%d_%H%M%S")
destination_blob_name = 'Firmware_' + commit_timestamp + '_10.bin'
# filepath = "C:/Users/kumbh/Documents/Arduino/GIT Repo/ESP-32-IoT-Excavator/Excavator/firmware_20220219_134302_6.bin"
checksum = hashlib.md5(file_as_bytes(open(source_file_name, 'rb'))).hexdigest()
print("   ")
print(checksum)

filename = 'Python_Scripts/Private-key.json'
keypath = os.path.join(workspace, filename)
cred = firebase_admin.credentials.Certificate(keypath)
try:
  firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://ttl-iot-default-rtdb.europe-west1.firebasedatabase.app'
})
except ValueError:
  print('\nFirebase - Already initialized')
except:
  print('\nFirebase - Error occured')
else:
  print('\nFirebase - Initialized Successfully')

ref = db.reference('Excavator/Firmware')
print("\nWriting to Firebase")
ref.child("Name").set(destination_blob_name)
ref.child("md5_checksum").set(checksum)
print(destination_blob_name)


