import yagmail
import os
import zipfile

sender_email = "tatatechnologiesdk@gmail.com"
receiver_email = "kumbhat10@gmail.com"
password = "Kanakrajj10"

yagmail.register(sender_email, password)
yag = yagmail.SMTP(sender_email)
to = [receiver_email, "dushyant.kumbhat@tatatechnologies.com"]

subject = 'Email from Python Testing'
body = 'This is obviously the body'
html = '<a href="https://pypi.python.org/pypi/sky/">Click me!</a>'
img = 'https://i.pinimg.com/564x/88/19/4a/88194a590eb82d70365f9887aa1091ee.jpg'

original_file_path = os.path.join('C:/Users/kumbh/Documents/Arduino/GIT Repo/ESP-32-IoT-Excavator/Excavator', 'Excavator.ino.esp32.bin')

zip_file_name = os.environ.get("GITHUB_WORKSPACE")
zipf = zipfile.ZipFile('Python.zip', 'w')
zipf.write(original_file_path)

yag.send(to = to, subject = subject, contents = [body, html, img], attachments=["Python.zip",original_file_path])

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