import hashlib
import os
import json
import firebase_admin
from firebase_admin import credentials, db, messaging, firestore
from google.cloud import storage
from dateutil import parser

workspace = os.environ.get("GITHUB_WORKSPACE")
event_context_string = os.environ.get("EVENT_CONTEXT")

event_context_json = json.loads(event_context_string)
filename = 'Python_Scripts/Private-key.json'
keypath = os.path.join(workspace, filename)
cred = firebase_admin.credentials.Certificate(keypath)
raw_bin_file_path = 'Excavator/build/esp32.esp32.esp32/Excavator.ino.bin'
raw_bin_file_path = os.path.join(workspace, raw_bin_file_path)
commit_timestamp = parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%Y%m%d_%H%M%S")

commit_time = parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%H%M%S")
commit_date = parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%Y%m%d")
def file_as_bytes(file):
    with file:
        return file.read()
def checksum(bin_file_path):
    return hashlib.md5(file_as_bytes(open(bin_file_path, 'rb'))).hexdigest()
checksum = 'w3r3wc3rc443r34rc344r344c' #checksum(bin_file_path)
def firebase_login():
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
firebase_login()

## get last firmware Version
f_db = firestore.client()
doc_ref_firmware = f_db.collection('Firmware')
query = doc_ref_firmware.order_by("_firmware_version",  direction=firestore.Query.DESCENDING).limit(1)
results = query.stream()
for doc in results:
  document = doc.to_dict()
  last_firmware_version = document['_firmware_version']
  last_firmware_checksum = document['_firmware_checksum']
print("last_firmware_version :", last_firmware_version)
print("last_firmware_checksum :", last_firmware_checksum)

current_firmware_version = int(last_firmware_version) + 1;
current_firmware_name = 'Firmware_' + commit_timestamp + '_' + str(version)


event_context_json['_firmware_version'] = current_firmware_version
event_context_json['_firmware_checksum'] = checksum
event_context_json['_firmware_date'] = int(commit_date)
event_context_json['_firmware_time'] = int(commit_time)
doc_ref = doc_ref_firmware.document(current_firmware_name).set(event_context_json)

# filepath = "C:/Users/kumbh/Documents/Arduino/GIT Repo/ESP-32-IoT-Excavator/Excavator/firmware_20220219_134302_6.bin"

# print("   ")
# print("md5 checksum of new firmware   ")
# print(checksum)
#
# ref = db.reference('Excavator/Firmware')
# print("\nWriting to Firebase")
# ref.child("Name").set(destination_blob_name)
# ref.child("md5_checksum").set(checksum)
# print(destination_blob_name)
