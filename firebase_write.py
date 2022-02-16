
#pip install --upgrade firebase-admin
import os
from datetime import datetime
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

cwd = os.getcwd()
print(cwd)
dir_path = os.path.dirname(os.path.realpath(__file__))
print(dir_path)

#workspace = "D:" # for windows
workspace = os.environ.get("GITHUB_WORKSPACE") #sys.argv[1]
filename = 'Private-key.json'


keypath = os.path.join(workspace, filename)
cred = firebase_admin.credentials.Certificate(keypath)

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

