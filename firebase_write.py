
#pip install --upgrade firebase-admin
import os
import sys
import json
from datetime import datetime
import time
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

cwd = os.getcwd()
print(cwd)
dir_path = os.path.dirname(os.path.realpath(__file__))
print("\n dir_path")
print("\n  Below is input arguments")
#key = os.environ.get("FIREBASE_SA_JSON") #sys.argv[1]  #
key = os.environ.get("FIREBASE_PRIVATE_KEY")
data = os.environ.get("FIREBASE_SA_JSON")

print(data)
print("\n  Below is input key")
print(type(key))
print("\n  Below is input data")
print(type(data))

data1 = json.loads(data)
print(data1)
print("\n  Below is input data1")
print(type(data1))

data2 = json.dumps(data)
print(data2)
print("\n  Below is input data2")
print(type(data2))

keypath = "/firebase-key.json"

cred = firebase_admin.credentials.Certificate(keypath )

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

