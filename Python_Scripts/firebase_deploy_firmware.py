
#pip install --upgrade firebase-admin
import os
import sys
from datetime import datetime
import firebase_admin
from firebase_admin import credentials, db, messaging

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
commit_timestamp = parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%Y%m%d_%H%M%S")
current_firmware_name = 'Firmware_' + commit_timestamp + '_10'
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

ref = db.reference('Excavator/Control/data/Firmware')
print("\nWriting to Firebase")
ref.set(current_firmware_name)
print(current_firmware_name)
