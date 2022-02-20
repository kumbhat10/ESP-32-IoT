
#pip install --upgrade firebase-admin
import os
import sys
from datetime import datetime
import firebase_admin
from firebase_admin import credentials, db, messaging
from google.cloud import storage
from dateutil import parser

workspace = os.environ.get("GITHUB_WORKSPACE")


commit_timestamp = parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%Y%m%d_%H%M%S")
destination_blob_name = 'Firmware_' + commit_timestamp + '_10'

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

ref.set(destination_blob_name)

print("\New Firmware =", destination_blob_name)


    


