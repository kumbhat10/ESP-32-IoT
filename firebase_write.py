
#pip install --upgrade firebase-admin
import sys
import json
from datetime import datetime

import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

print(sys.argv[1])
data = json.loads(sys.argv[1]) 
print("\n")
print(data)
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
# ref.get()
ref.set(current_time)

