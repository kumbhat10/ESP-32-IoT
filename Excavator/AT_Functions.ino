 void SendMessage()
{
  Serial.println ("Sending Message");
  Serial7600.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(100);
  Serial.println ("Set SMS Number");
  Serial7600.println("AT+CMGS=\"447496393966\"\r"); //Mobile phone number to send message
  delay(100);
  Serial.println ("Set SMS Content");
  Serial7600.println("Excavator was restarted");// Messsage content
  delay(100);
  Serial.println ("Sending SMS");
  Serial7600.println((char)26);// ASCII code of CTRL+Z
  Serial.println ("Message has been sent ->SMS Dushyant Kumbhat");
} 
