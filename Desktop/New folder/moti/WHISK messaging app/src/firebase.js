import firebase from "firebase/app";
import "firebase/auth";

// For Firebase JS SDK v7.20.0 and later, measurementId is optional
export const auth = firebase.initializeApp ( {
    apiKey: "AIzaSyBkBPT4O3wrLZPDvAo3qCmJoE5RDVeD7ZQ",
    authDomain: "unerase-9ac27.firebaseapp.com",
    projectId: "unerase-9ac27",
    storageBucket: "unerase-9ac27.appspot.com",
    messagingSenderId: "193346465920",
    appId: "1:193346465920:web:7f59fc8fcc80d296e83665",
   /* measurementId: "G-4RJZZE80HL" */
  }).auth();