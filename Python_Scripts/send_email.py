import yagmail
import os
import zipfile

sender_email = "tatatechnologiesdk@gmail.com"
receiver_email = "kumbhat10@gmail.com"
password = "Kanakrajj10"

yagmail.register(sender_email, password)
yag = yagmail.SMTP(sender_email)
to = [receiver_email, "dushyant.kumbhat@tatatechnologies.com"]

subject = 'Github CI-CD Actions - Successfull '
body = 'Successfully build new firmware and uploaded to google cloud. Please check below link'
html = '<a href="https://pypi.python.org/pypi/sky/">Click me!</a>'
img = 'https://i.pinimg.com/564x/88/19/4a/88194a590eb82d70365f9887aa1091ee.jpg'

workspace = os.environ.get("GITHUB_WORKSPACE")
source_file_path = 'Excavator/build/esp32.esp32.esp32/Excavator.ino.bin'  
source_file_name = os.path.join(workspace, source_file_path)

commit_timestamp = parser.parse(os.environ.get("COMMIT_TIMESTAMP")).strftime("%Y%m%d_%H%M%S")
destination_blob_name = 'Firmware_' + commit_timestamp + '_10.bin'

zipf = zipfile.ZipFile(destination_blob_name +'.zip', 'w')
zipf.write(source_file_name)

yag.send(to = to, subject = subject, contents = [body, html, img], attachments=[destination_blob_name +'.zip', source_file_name])

# import smtplib, ssl

# port = 587  # For starttls
# smtp_server = "smtp.gmail.com"

# message = "This message is sent from python"

# context = ssl.create_default_context()

# with smtplib.SMTP(smtp_server, port) as server:
    # server.ehlo()  # Can be omitted
    # server.starttls(context=context)
    # server.ehlo()  # Can be omitted
    # server.login(sender_email, password)
    # server.sendmail(sender_email, receiver_email, message)