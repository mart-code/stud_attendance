const express = require("express");
const firebase = require("firebase");
const path = require("path");
const port = 3000;
const app = express();

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyCRoCdQRvcbDNhb9oqxpESL0aPacape1zo",
  authDomain: "attendance-system-4003e.firebaseapp.com",
  databaseURL: "https://attendance-system-4003e-default-rtdb.firebaseio.com",
  projectId: "attendance-system-4003e",
  storageBucket: "attendance-system-4003e.appspot.com",
  messagingSenderId: "39359822173",
  appId: "1:39359822173:web:01b4b5bde13bd5433e10b4",
};

firebase.initializeApp(firebaseConfig);

// Set the path to the public folder where your HTML file is located
const publicPath = path.join(__dirname, "public");

app.use(express.json()); // Middleware to parse JSON requests

// Handle POST requests from Arduino
app.post("/uploadData", (req, res) => {
  // Extract data from the request (matric, class, attendance, etc.)
  const { matric, classHeld, attend, present, absent } = req.body;

  // Create a reference to the Firebase Realtime Database
  const db = firebase.database();

  // Push the data to the database
  const attendanceRef = db.ref("attendance");
  const newAttendanceEntry = attendanceRef.push();
  newAttendanceEntry.set({
    matric,
    classHeld,
    attend,
    present,
    absent,
    timestamp: firebase.database.ServerValue.TIMESTAMP,
  });

  res.sendStatus(200);
});

// Start the server
app.listen(port, () => {
  console.log(`Server is running on port ${port}`);
});

app.get("/", (req, res) => {
  // Use path.join to get the absolute path to index.html
  const indexPath = path.join(publicPath, "index.html");

  // Send the file
  res.sendFile(indexPath);
});
