import hashlib
import os
import json
import firebase_admin
from firebase_admin import credentials, db, firestore, messaging
from google.cloud import storage
from dateutil import parser
class bc:
    HEADER = '\033[95m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

workspace = os.environ.get("GITHUB_WORKSPACE")
event_context_string = os.environ.get("EVENT_CONTEXT")
machine = os.environ.get("MACHINE")

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
def checksum(file_path):
    return hashlib.md5(file_as_bytes(open(file_path, 'rb'))).hexdigest()
current_firmware_checksum = checksum(raw_bin_file_path)

def firebase_login():
    try:
      firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://ttl-iot-default-rtdb.europe-west1.firebasedatabase.app'
    })
    except ValueError:
      print( bc.OKGREEN +'\nGoogle Firebase - Already initialized\n' + bc.ENDC)
    except:
      print(bc.FAIL + '\nGoogle Firebase - Error occured\n'+ bc.ENDC)
    else:
      print(bc.WARNING + '\nGoogle Firebase - Initialized Successfully\n'+ bc.ENDC)
firebase_login()

## Get last firmware Version & CHecksum
f_db = firestore.client()
doc_ref_firmware = f_db.collection('Firmware')
doc_ref_firmware_machine = f_db.collection(machine)

query = doc_ref_firmware.where("_firmware_machine", "==", machine).order_by("_firmware_version",  direction=firestore.Query.DESCENDING).limit(1)
results = query.stream()

for doc in results:
  document = doc.to_dict()
  last_firmware_name = doc.id
  last_firmware_version = document['_firmware_version']
  last_firmware_checksum = document['_firmware_checksum']

current_firmware_version = int(last_firmware_version) + 1
current_firmware_name = 'Firmware_' + commit_timestamp + '_' + str(current_firmware_version)
current_firmware_file_name = current_firmware_name + '.bin'


event_context_json['_firmware_machine'] = machine
event_context_json['_firmware_version'] = current_firmware_version
event_context_json['_firmware_checksum'] = current_firmware_checksum
event_context_json['_firmware_date'] = int(commit_date)
event_context_json['_firmware_time'] = int(commit_time)

doc_ref = doc_ref_firmware.document(current_firmware_name).set(event_context_json)
doc_ref_machine = doc_ref_firmware_machine.document(current_firmware_name).set(event_context_json)
print( bc.WARNING + '\nmd5 checksum of previous firmware ' + last_firmware_name + ' : ' + last_firmware_checksum + bc.ENDC)
print( bc.OKGREEN +   'md5 checksum of current  firmware ' + current_firmware_name + ' : ' + current_firmware_checksum + bc.ENDC)

if (last_firmware_checksum == current_firmware_checksum):
    deploy = False
    print( bc.FAIL + bc.BOLD +"\nBoth S/W have same checksum " + current_firmware_checksum + bc.ENDC)
else:
    deploy = True
    print( bc.OKGREEN  + bc.BOLD + "\nBoth S/W have different checksum " + current_firmware_checksum + bc.ENDC)

def upload_to_cloud():
    storage_client = storage.Client.from_service_account_json(keypath)
    bucket = storage_client.bucket('ttl-iot.appspot.com')
    blob = bucket.blob(current_firmware_file_name)
    blob.upload_from_filename(raw_bin_file_path)
    print(bc.OKGREEN + "New firmware uploaded to {}.".format(current_firmware_file_name)  + bc.ENDC)
upload_to_cloud()

def sendCloudNotification():
    topic = 'Alert'
    message = messaging.Message(
    topic=topic, android=messaging.AndroidConfig(priority='high',
                                        data = {'Topic':'332'},
            notification=messaging.AndroidNotification(
                title='Firmware Uploaded',
                body='Github actions successfully uploaded new firmware on '+ parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%d/%m/%Y at %H:%M:%S"),
                image="https://i.pinimg.com/564x/92/fb/38/92fb38bd608b0647cbc7b33270f86e56.jpg")))
    response = messaging.send(message) # Response is a message ID string.
    print('\nSuccessfully sent cloud notification :', response)
sendCloudNotification()

## Save to environment
env_file = os.getenv('GITHUB_ENV')
env_dict = {'last_firmware_name': last_firmware_name, 'current_firmware_checksum': current_firmware_checksum, 'current_firmware_name': current_firmware_name}
print(env_dict)
with open(env_file, "a") as myfile:
    for key in env_dict:
        myfile.write( key +  '=' + env_dict[key] + '\n')

# ref = db.reference('Excavator/Firmware')
# print("\nWriting to Firebase")
# ref.child("Name").set(destination_blob_name)
# ref.child("md5_checksum").set(checksum)
# print(destination_blob_name)
