
#pip install --upgrade firebase-admin
import os
import sys
from datetime import datetime
import firebase_admin
from firebase_admin import credentials, db, messaging
from google.cloud import storage

bucket_name = 'ttl-iot.appspot.com'
source_file_name = sys.argv[1]
destination_blob_name = sys.argv[2]

## Function to send mobile cloud notification to all the users
def sendCloudNotification():
    topic = 'Alert'
    message = messaging.Message(
    topic=topic, android=messaging.AndroidConfig(priority='high',
                                        data = {'Topic':'332'},
            notification=messaging.AndroidNotification(
                title='Github Actions Update',
                body='Github actions successfully finished at '+current_time,
                image="https://i.pinimg.com/564x/92/fb/38/92fb38bd608b0647cbc7b33270f86e56.jpg")))
    response = messaging.send(message) # Response is a message ID string.
    print('\nSuccessfully sent cloud notification :', response)
    

workspace = os.environ.get("GITHUB_WORKSPACE")
filename = 'Private-key.json'
keypath = os.path.join(workspace, filename)


cred = firebase_admin.credentials.Certificate(keypath)

storage_client = storage.Client.from_service_account_json(keypath)
bucket = storage_client.bucket(bucket_name)
blob = bucket.blob(destination_blob_name)
blob.upload_from_filename(source_file_name)
print("File {} uploaded to {}.".format(source_file_name, destination_blob_name))


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
now = datetime.now()
current_time = now.strftime("%y-%m-%d %H:%M:%S")
ref.set(current_time)
sendCloudNotification()
print("\nCurrent Time =", current_time)


    


