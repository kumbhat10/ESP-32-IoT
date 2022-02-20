
#pip install --upgrade firebase-admin
import os
import sys
from datetime import datetime
import firebase_admin
from firebase_admin import credentials, db, messaging
from google.cloud import storage
from dateutil import parser

bucket_name = 'ttl-iot.appspot.com'
source_file_name = sys.argv[1]
commit_timestamp = parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%Y%m%d_%H%M%S")
destination_blob_name = 'Firmware_' + commit_timestamp + '10'

## Function to send mobile cloud notification to all the users
def sendCloudNotification():
    topic = 'Alert'
    message = messaging.Message(
    topic=topic, android=messaging.AndroidConfig(priority='high',
                                        data = {'Topic':'332'},
            notification=messaging.AndroidNotification(
                title='Firmware Uploaded',
                body='Github actions successfully uploaded new firmware on '+ parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%Y%m%d at %H:%M:%S"),
                image="https://i.pinimg.com/564x/92/fb/38/92fb38bd608b0647cbc7b33270f86e56.jpg")))
    response = messaging.send(message) # Response is a message ID string.
    print('\nSuccessfully sent cloud notification :', response)
    

workspace = os.environ.get("GITHUB_WORKSPACE")
filename = 'Python_Scripts/Private-key.json'
keypath = os.path.join(workspace, filename)

storage_client = storage.Client.from_service_account_json(keypath)
bucket = storage_client.bucket(bucket_name)
blob = bucket.blob(destination_blob_name)
blob.upload_from_filename(source_file_name)
print("File {} uploaded to {}.".format(source_file_name, destination_blob_name))

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

ref = db.reference('Excavator/Firmware Upload')
print("\nWriting to Firebase")
# now = datetime.now()
# current_time = now.strftime("%y-%m-%d %H:%M:%S")
ref.set(commit_timestamp)
sendCloudNotification()
print("\nCurrent Time =", commit_timestamp)


    


