
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


current_firmware_name = os.environ.get("current_firmware_name")
current_firmware_checksum = os.environ.get("current_firmware_checksum")
workspace = os.environ.get("GITHUB_WORKSPACE")

filename = 'Python_Scripts/Private-key.json'
keypath = os.path.join(workspace, filename)
cred = firebase_admin.credentials.Certificate(keypath)

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
print( bc.OKGREEN + "\nWriting to Firebase"+ bc.ENDC)
ref = db.reference('Excavator/Control/data/Firmware')
ref.set(current_firmware_name)
print( bc.OKGREEN + "\nWriting to Firebase Firmware"+ bc.ENDC)
ref = db.reference('Excavator/Firmware')
ref.set({'Name': current_firmware_name, 'md5 Checksum': current_firmware_checksum})
