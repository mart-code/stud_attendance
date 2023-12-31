// Inside your Arduino code where attendance data is ready
void sendData() {
  String url = "http://your-nodejs-server-ip:3000/uploadData";
  url += "?matric=" + matric + "&classHeld=" + nClass + "&attend=" + nAttend;
  url += "&present=" + nPresent + "&absent=" + nAbsent;
  
  // Make HTTP POST request to the server
  // Use the appropriate library for HTTP requests on Arduino
}
