
#pip install --upgrade firebase-admin
import os

import sys
import json
from datetime import datetime
import time
# import firebase_admin
# from firebase_admin import credentials
# from firebase_admin import db


a = os.environ.get("OS")
print(a)
print("\n  Below is input arguments")
input = os.environ.get("FIREBASE_SA_JSON") #sys.argv[1]
print(input)

print("\n  Below is type of data")
print(type(input))

data = json.loads(input) 
print("\n  Below is parsed JSON data")
print(data)
print("\n  Below is type of data")
print(type(data))


# evalstr = "a = " + input
# print("\n  Below is evalstr")
# print(evalstr)
# eval(evalstr)
# print("\n  Below is a")
# print(a)
# print("\n  Below is type of a")
# print(type(a))

#cred = firebase_admin.credentials.Certificate(data )

# try:
  # firebase_admin.initialize_app(cred, {
    # 'databaseURL': 'https://ttl-iot-default-rtdb.europe-west1.firebasedatabase.app'
# })  # Initialize the app with a service account, granting admin privileges
# except ValueError:
  # print('Firebase - Already initialized')
# except:
  # print('Firebase - Error occured')
# else:
  # print('Firebase - Initialized Successfully')

# ref = db.reference('CheckLive')

# print("Writing to Firebase")
# now = datetime.now()
# current_time = now.strftime("%y-%m-%d %H:%M:%S")
# print("Current Time =", current_time)
# ref.set(current_time)

