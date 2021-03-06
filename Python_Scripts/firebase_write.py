
#pip install --upgrade firebase-admin
import os
from datetime import datetime
import firebase_admin
from firebase_admin import credentials, db, messaging
from dateutil import parser

## Function to send mobile cloud notification to all the users
def sendCloudNotification():
    topic = 'Alert'
    message = messaging.Message(
    topic=topic, android=messaging.AndroidConfig(priority='high',
                                        data = {'Topic':'332'},
            notification=messaging.AndroidNotification(
                title='Github Actions Update',
                body='Github actions successfully finished on '+current_time,
                image="https://i.pinimg.com/564x/92/fb/38/92fb38bd608b0647cbc7b33270f86e56.jpg")))
    response = messaging.send(message) # Response is a message ID string.
    print('\nSuccessfully sent cloud notification :', response)
    
#Workspace = "D:" # for windows
workspace = os.environ.get("GITHUB_WORKSPACE") #sys.argv[1]
filename = 'Private-key.json'

keypath = os.path.join(workspace, filename)
cred = firebase_admin.credentials.Certificate(keypath)

try:
  firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://ttl-iot-default-rtdb.europe-west1.firebasedatabase.app'
})  # Initialize the app with a service account, granting admin privileges
except ValueError:
  print('\nFirebase - Already initialized')
except:
  print('\nFirebase - Error occured')
else:
  print('\nFirebase - Initialized Successfully')

ref = db.reference('CheckLive')

print("\nWriting to Firebase")
# now = datetime.now()
# current_time = now.strftime("%y-%m-%d %H:%M:%S")

commit_timestamp = parser.parse(os.environ.get(COMMIT_TIMESTAMP)).strftime("%Y%m%d_%H%M%S")

ref.set(commit_timestamp)
sendCloudNotification()
print("\nCurrent Time =", commit_timestamp)


    


