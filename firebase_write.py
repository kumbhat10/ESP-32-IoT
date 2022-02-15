
#pip install --upgrade firebase-admin
import os
import sys
import json
from datetime import datetime
import time
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db


print("\n  Below is input arguments")
#key = os.environ.get("FIREBASE_SA_JSON") #sys.argv[1]  #
key = os.environ.get("FIREBASE_PRIVATE_KEY")

p = key
print("\n  Below is input arguments")

print(p)
print("\n")
data = {
  "type": "service_account",
  "project_id": "ttl-iot",
  "private_key_id": "60b933117f50f3e8a5d091b27ee11eaf07864dcc",
  "private_key": key,
  "client_email": "firebase-adminsdk-yrbg2@ttl-iot.iam.gserviceaccount.com",
  "client_id": "102474256127526520693",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-yrbg2%40ttl-iot.iam.gserviceaccount.com"
}

print(data)
print("\n  Below is input arguments")
print(type(data))
cred = firebase_admin.credentials.Certificate(data )

try:
  firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://ttl-iot-default-rtdb.europe-west1.firebasedatabase.app'
})  # Initialize the app with a service account, granting admin privileges
except ValueError:
  print('Firebase - Already initialized')
except:
  print('Firebase - Error occured')
else:
  print('Firebase - Initialized Successfully')

ref = db.reference('CheckLive')

print("Writing to Firebase")
now = datetime.now()
current_time = now.strftime("%y-%m-%d %H:%M:%S")
print("Current Time =", current_time)
ref.set(current_time)

