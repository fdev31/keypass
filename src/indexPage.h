#ifndef _INDEXPAGE_H
#define _INDEXPAGE_H
const static char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>KeyPass</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
* {
margin: 0;
padding: 0;
box-sizing: border-box;
}

body {
background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
min-height: 100vh;
font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
display: flex;
flex-direction: column;
align-items: center;
padding: 20px;
position: relative;
overflow-x: hidden;
}

body::before {
content: '';
position: fixed;
top: 0;
left: 0;
width: 100%;
height: 100%;
background:
radial-gradient(circle at 20% 50%, rgba(120, 119, 198, 0.3) 0%, transparent 50%),
radial-gradient(circle at 80% 20%, rgba(255, 118, 117, 0.3) 0%, transparent 50%),
radial-gradient(circle at 40% 80%, rgba(255, 177, 153, 0.3) 0%, transparent 50%);
animation: float 20s ease-in-out infinite;
z-index: -1;
}

@keyframes float {
0%, 100% { transform: translateY(0px) rotate(0deg); }
33% { transform: translateY(-20px) rotate(1deg); }
66% { transform: translateY(10px) rotate(-1deg); }
}

.container {
width: 100%;
max-width: 600px;
animation: fadeInUp 0.8s ease-out;
}

@keyframes fadeInUp {
from {
opacity: 0;
transform: translateY(30px);
}
to {
opacity: 1;
transform: translateY(0);
}
}

.header {
text-align: center;
margin-bottom: 30px;
animation: slideDown 0.6s ease-out;
}

@keyframes slideDown {
from {
opacity: 0;
transform: translateY(-20px);
}
to {
opacity: 1;
transform: translateY(0);
}
}

h1 {
color: white;
font-size: 2.5rem;
font-weight: 700;
margin-bottom: 10px;
text-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
background: linear-gradient(135deg, #fff, #f0f8ff);
-webkit-background-clip: text;
-webkit-text-fill-color: transparent;
background-clip: text;
}

.subtitle {
color: rgba(255, 255, 255, 0.8);
font-size: 1.1rem;
font-weight: 300;
}

.button-row {
display: flex;
gap: 15px;
margin-bottom: 30px;
animation: slideUp 0.8s ease-out 0.2s both;
}

@keyframes slideUp {
from {
opacity: 0;
transform: translateY(20px);
}
to {
opacity: 1;
transform: translateY(0);
}
}
.menu-btn {
background-color: rgba(0, 0, 0, 0.1) !important;
}

.modern-btn {
flex: 1;
padding: 15px 25px;
background: rgba(255, 255, 255, 0.1);
backdrop-filter: blur(20px);
border: 1px solid rgba(255, 255, 255, 0.2);
border-radius: 15px;
color: white;
font-weight: 600;
font-size: 16px;
cursor: pointer;
position: relative;
overflow: hidden;
transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
}

.modern-btn::before {
content: '';
position: absolute;
top: 0;
left: -100%;
width: 100%;
height: 100%;
background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.2), transparent);
transition: left 0.5s;
}

.modern-btn:hover::before {
left: 100%;
}

.modern-btn:hover {
transform: translateY(-2px);
box-shadow: 0 15px 45px rgba(0, 0, 0, 0.2);
background: rgba(255, 255, 255, 0.15);
border-color: rgba(255, 255, 255, 0.3);
}

.modern-btn:active {
transform: translateY(0);
transition: transform 0.1s;
}

.modern-btn.active {
background: linear-gradient(135deg, #ff6b6b, #ff8e53);
border-color: rgba(255, 255, 255, 0.4);
box-shadow: 0 0 30px rgba(255, 107, 107, 0.4);
}

.modern-btn.active::before {
background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.3), transparent);
}

.glass-panel {
background: rgba(255, 255, 255, 0.1);
backdrop-filter: blur(20px);
border: 1px solid rgba(255, 255, 255, 0.2);
border-radius: 20px;
padding: 30px;
box-shadow: 0 15px 50px rgba(0, 0, 0, 0.1);
animation: scaleIn 0.5s ease-out;
position: relative;
overflow: hidden;
}

.glass-panel::before {
content: '';
position: absolute;
top: 0;
left: 0;
right: 0;
height: 1px;
background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.5), transparent);
}

@keyframes scaleIn {
from {
opacity: 0;
transform: scale(0.9);
}
to {
opacity: 1;
transform: scale(1);
}
}

.form-group {
margin-bottom: 20px;
animation: fadeInLeft 0.6s ease-out;
}

.form-group:nth-child(2) { animation-delay: 0.1s; }
.form-group:nth-child(3) { animation-delay: 0.2s; }
.form-group:nth-child(4) { animation-delay: 0.3s; }

@keyframes fadeInLeft {
from {
opacity: 0;
transform: translateX(-20px);
}
to {
opacity: 1;
transform: translateX(0);
}
}

label {
display: block;
color: rgba(255, 255, 255, 0.9);
font-weight: 500;
margin-bottom: 8px;
font-size: 14px;
text-transform: uppercase;
letter-spacing: 0.5px;
}

input, select {
width: 100%;
padding: 15px;
background: rgba(255, 255, 255, 0.1);
border: 1px solid rgba(255, 255, 255, 0.2);
border-radius: 10px;
color: white;
font-size: 16px;
transition: all 0.3s ease;
backdrop-filter: blur(10px);
}

input::placeholder {
color: rgba(255, 255, 255, 0.5);
}

input:focus, select:focus {
outline: none;
border-color: #ff6b6b;
box-shadow: 0 0 20px rgba(255, 107, 107, 0.3);
background: rgba(255, 255, 255, 0.15);
}

select option {
background: #764ba2;
color: white;
}

.submit-btn {
width: 100%;
padding: 15px;
background: linear-gradient(135deg, #ff6b6b, #ff8e53);
border: none;
border-radius: 10px;
color: white;
font-weight: 600;
font-size: 16px;
cursor: pointer;
transition: all 0.3s ease;
box-shadow: 0 10px 30px rgba(255, 107, 107, 0.3);
position: relative;
overflow: hidden;
}

.submit-btn::before {
content: '';
position: absolute;
top: 0;
left: -100%;
width: 100%;
height: 100%;
background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.2), transparent);
transition: left 0.5s;
}

.submit-btn:hover::before {
left: 100%;
}

.submit-btn:hover {
transform: translateY(-2px);
box-shadow: 0 15px 40px rgba(255, 107, 107, 0.4);
}

.password-grid {
display: grid;
grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
gap: 15px;
animation: slideUp 0.6s ease-out;
}

.password-card {
background: rgba(255, 255, 255, 0.1);
backdrop-filter: blur(20px);
border: 1px solid rgba(255, 255, 255, 0.2);
border-radius: 15px;
padding: 20px;
cursor: pointer;
transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
position: relative;
overflow: hidden;
animation: fadeInScale 0.5s ease-out;
}

.password-card:nth-child(odd) { animation-delay: 0.1s; }
.password-card:nth-child(even) { animation-delay: 0.2s; }

@keyframes fadeInScale {
from {
opacity: 0;
transform: scale(0.8);
}
to {
opacity: 1;
transform: scale(1);
}
}

.password-card::before {
content: '';
position: absolute;
top: 0;
left: -100%;
width: 100%;
height: 100%;
background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.1), transparent);
transition: left 0.5s;
}

.password-card:hover::before {
left: 100%;
}

.password-card:hover {
transform: translateY(-5px) scale(1.02);
box-shadow: 0 20px 50px rgba(0, 0, 0, 0.2);
background: rgba(255, 255, 255, 0.15);
border-color: rgba(255, 255, 255, 0.3);
}

.password-name {
color: white;
font-weight: 600;
font-size: 18px;
margin-bottom: 5px;
text-align: center;
}

.hidden {
display: none !important;
}

.fade-out {
animation: fadeOut 0.3s ease-out forwards;
}

.fade-in {
animation: fadeIn 0.3s ease-out forwards;
}

@keyframes fadeOut {
to {
opacity: 0;
transform: translateY(-10px);
}
}

@keyframes fadeIn {
from {
opacity: 0;
transform: translateY(10px);
}
to {
opacity: 1;
transform: translateY(0);
}
}

/* Mobile responsiveness */
@media (max-width: 768px) {
.container {
padding: 0 10px;
}

h1 {
font-size: 2rem;
}

.button-row {
flex-direction: column;
gap: 10px;
}

.password-grid {
grid-template-columns: 1fr;
}
}

/* Loading animation */
.loading {
display: inline-block;
width: 20px;
height: 20px;
border: 2px solid rgba(255, 255, 255, 0.3);
border-radius: 50%;
border-top-color: white;
animation: spin 1s ease-in-out infinite;
}

@keyframes spin {
to { transform: rotate(360deg); }
}

@keyframes wiggle-dots {
0% { transform: rotate(0deg); }
20% { transform: rotate(-10deg); }
40% { transform: rotate(10deg); }
60% { transform: rotate(-7deg); }
80% { transform: rotate(7deg); }
100% { transform: rotate(0deg); }
}

.wiggle-dots {
animation: wiggle-dots 0.5s ease-in-out;
}
.icon {
display: flex;
justify-content: center;
align-items: center;
width: 100%;
height: 100%;
margin: 0;
}
.icon-die {
background-color: #fff;
border-radius: 3px;
width: 24px;
height: 24px;
position: relative;
margin: auto;
}
.icon-die:before{
background-color: #000;
width: 7px;
height: 7px;
border-radius: 50%;
position: absolute;
top: 4px;
left: 4px;
content: "";
}
.icon-die:after {
background-color: #000;
width: 7px;
height: 7px;
border-radius: 50%;
position: absolute;
top: 13px;
left: 13px;
content: "";
}
</style>
</head>
<body>
<div class="container">
<div class="header">
<h1>KeyPass</h1>
<p id="subtitle" class="subtitle">Password Management System</p>
</div>

<div class="button-row">
<button id="type-button" onclick="setMode('type')" class="modern-btn menu-btn active" role="button">
Type
</button>
<button id="edit-button" onclick="setMode('edit')" class="modern-btn menu-btn" role="button">
Edit
</button>
<button id="add-button" onclick="setMode('add')" class="modern-btn menu-btn" role="button">
Add
</button>
<button id="type-button" onclick="setMode('settings')" class="modern-btn menu-btn" role="button" style="flex: 0 0 auto; padding: 0; font-size: 200%; width: 50px; height: 50px;">
&#x1F4AB;
</button>
</div>

<div class="mainScreen hidden" id="settingsForm">
<div class="glass-panel">
<form id="settingsFormContent" method="get" action="/settings">
<div class="form-group">
<label for="wifiPassInput">Wi-Fi Password</label>
<input id="wifiPassInput" type="password" name="wifiPass" placeholder="Enter Wi-Fi password..." required>
</div>
<button type="submit" class="submit-btn">
<span class="btn-text">Save Settings</span>
<span class="loading hidden"></span>
</button>
</form>
<button class="menu-btn modern-btn" onclick="confirmFactoryReset()" style="background: rgba(255, 107, 107, 0.2);">
Factory Reset
</button>
</div>
</div>

<div class="mainScreen hidden" id="editForm">
<div class="glass-panel">
<form id="editFormContent" method="get" action="/editPass">
<input type="hidden" name="id" id="positionSelect" disabled>

<div class="form-group">
<label for="layoutSelect">Layout</label>
<select name="lang" id="layoutSelect" required>
<option value="fr">French</option>
<option value="us">US</option>
</select>
<input type="hidden" name="layout" id="layoutIndex">
</div>

<div class="form-group">
<label for="passLabel">Name</label>
<input id="passLabel" name="name" type="text" required placeholder="Enter password name (15 characters max)" maxlength="15">
</div>

<div class="form-group">

<label for="passwordInput">Password</label>
<div style="display: flex; gap: 10px;">
<input id="passwordInput" type="password" name="password" placeholder="Enter password...">
<button id="toggleVisibilityIcon" type="button" onclick="toggleVisibility()" class="modern-btn"  style="width: 50px; flex: 0 0 auto; padding: 0; font-size: 200%;">&#x1f648;</button>
<button type="button" onclick="generatePass()" class="modern-btn" style="width: 50px; flex: 0 0 auto; padding: 0;"><div class="icon"><div id="diceIcon" class="icon-die"></div></div></button>

</div>
<div style="display: flex; gap: 10px; margin-top: 10px;">
<button id="typeOldPassBtn" type="button" onclick="typeOldPass()" class="modern-btn hidden" style="width: auto; flex: auto;">&#x2328; Type old pass</button>
<button id="typeNewPassBtn" type="button" onclick="typeNewPass()" class="modern-btn hidden" style="width: auto; flex: auto;">&#x2328; Type new pass</button>
</div>
</div>

<button type="submit" class="submit-btn">
<span class="btn-text">Save Password</span>
<span class="loading hidden"></span>
</button>
</form>
</div>
</div>

<div id="passList" class="mainScreen">
<div class="password-grid">
<!-- Passwords will be loaded here -->
</div>
</div>
<footer>
<div style="display: flex; gap: 15px; margin-top: 20px;">
<button class="menu-btn modern-btn" onclick="updateWifiPass()">
Change KeyPass' password
</button>
</div>
</footer>
</div>
<script>
<!-- WARN: The following line must not be changed and is replaced with index.js content automatically -->
const ui_data = {
  mode: "type",
  current_focus: "type-button",
  passwords: [],
  change_focus(elementId) {
    // Remove active class from all buttons
    document.querySelectorAll(".modern-btn").forEach((btn) => {
      btn.classList.remove("active");
    });

    // Add active class to selected button
    const newFocus = document.getElementById(elementId);
    newFocus.classList.add("active");
    this.current_focus = elementId;
  },
};

function getRandomChar() {
  const charset =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0" +
    "123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
  const charsetSize = charset.length;
  return charset[Math.floor(Math.random() * charsetSize)];
}

// Function to generate a random password of given length
function generatePassword(length) {
  let password = "";
  for (let i = 0; i < length; ++i) {
    password += getRandomChar();
  }
  return password;
}

function wiggleDots(elementId = "diceIcon") {
  // Add wiggle class
  const icon = document.getElementById(elementId);
  icon.classList.add("wiggle-dots");

  // Remove wiggle class after animation completes
  setTimeout(() => {
    icon.classList.remove("wiggle-dots");
  }, 500);
}

function generatePass() {
  let length = 12;
  length = prompt("Password length (default 12):", length);
  if (!length || isNaN(length) || length < 8) {
    return;
  }
  wiggleDots("diceIcon");
  const password = generatePassword(length);
  document.getElementById("passwordInput").value = password;
  document.getElementById("typeNewPassBtn").classList.remove("hidden");
  if (ui_data.mode != "add")
    document.getElementById("typeOldPassBtn").classList.remove("hidden");
}

function updateWifiPass() {
  const newPass = prompt("Enter new password for KeyPass:");
  if (!newPass) return;
  // ask again and check if similar
  const newPassCheck = prompt("Confirm new password:");
  if (newPass !== newPassCheck) {
    alert("Passwords do not match. Please try again.");
    return;
  }
  if (newPass.length < 8) {
    alert("Password must be at least 8 characters long.");
    return;
  }
  fetch(`/updateWifiPass?newPass=${newPass}`)
    .then((response) => response.text())
    .catch((error) => {
      alert("Error:", error);
    });
}

function hideAll() {
  Array.from(document.getElementsByClassName("mainScreen")).forEach((el) => {
    el.classList.add("fade-out");
    setTimeout(() => {
      el.classList.add("hidden");
      el.classList.remove("fade-out");
    }, 300);
  });
}

function showElement(elementId) {
  const el = document.getElementById(elementId);
  el.classList.remove("hidden");
  el.classList.add("fade-in");
  setTimeout(() => {
    el.classList.remove("fade-in");
  }, 300);
}

function showPasswords() {
  // check if visible
  if (document.getElementById("passList").classList.contains("hidden")) {
    hideAll();
    setTimeout(() => showElement("passList"), 300);
  }
}

function leaveEditForm(uid) {
  document.getElementById("passwordInput").value = "";
  document.getElementById("typeNewPassBtn").classList.add("hidden");
  document.getElementById("typeOldPassBtn").classList.add("hidden");
}

function showEditForm(uid) {
  if (document.getElementById("editForm").classList.contains("hidden")) {
    const el = document.getElementById("positionSelect");
    el.value = uid;
    el.disabled = true;
    hideAll();
    setTimeout(() => showElement("editForm"), 300);
  }
}

function setMode(action) {
  if (ui_data.mode == action) {
    return;
  }
  if (ui_data.mode == "edit" || ui_data.mode == "add") {
    leaveEditForm();
  }
  ui_data.mode = action;

  switch (action) {
    case "edit":
      ui_data.change_focus("edit-button");
      showPasswords();
      break;
    case "type":
      ui_data.change_focus("type-button");
      showPasswords();
      break;
    case "add":
      const uid = ui_data.passwords.length;
      ui_data.change_focus("add-button");
      fillForm({ name: "" });
      showEditForm(uid);
      break;
    case "settings":
      showSettings();
      break;
    default:
      console.error("Invalid action");
  }
}

function showSettings() {
  if (document.getElementById("settingsForm").classList.contains("hidden")) {
    hideAll();
    setTimeout(() => showElement("settingsForm"), 300);
  }
}

function fillForm(data) {
  if (data.layout != undefined)
    document.getElementById("layoutSelect").selectedIndex = data.layout;
  if (data.uid != undefined)
    document.getElementById("positionSelect").value = data.uid;
  if (data.name != undefined)
    document.getElementById("passLabel").value = data.name;
}

function typeOldPass() {
  const uid = document.getElementById("positionSelect").value;
  fetch(`/typePass?id=${uid}`).catch((error) => {
    alert("Error:", error);
  });
}

function typeNewPass() {
  const password = document.getElementById("passwordInput").value;
  const escaped = encodeURIComponent(password);
  const layout = document.getElementById("layoutSelect").value;
  fetch(`/typeRaw?text=${escaped}&layout=${layout}`).catch((error) => {
    alert("Error:", error);
  });
}

function passwordClick(uid) {
  switch (ui_data.mode) {
    case "edit":
      fillForm({
        uid: uid,
        name: ui_data.passwords[uid].name,
        layout: ui_data.passwords[uid].layout,
      });
      showEditForm(uid);
      break;
    default:
      // Add visual feedback for typing action
      const card = event.target.closest(".password-card");
      card.style.transform = "scale(0.95)";
      setTimeout(() => {
        card.style.transform = "";
      }, 150);
      fetch(`/typePass?id=${uid}`).catch((error) => {
        alert("Error:", error);
      });
  }
}

async function getPasswords() {
  try {
    const req = await fetch("/list");
    const passwords = await req.json();
    ui_data.passwords = passwords.passwords;
    const count = passwords.passwords.length;
    const total = passwords.free + count;
    document.getElementById("subtitle").innerText =
      `Stored ${count} over ${total}`;

    const passList = document.querySelector("#passList .password-grid");
    const domData = [];

    for (const pass of passwords.passwords) {
      domData.push(`
    <div onclick="passwordClick(${pass.uid})" class="password-card" role="button">
        <div class="password-name">${pass.name}</div>
    </div>
                    `);
    }

    passList.innerHTML = domData.join("");
  } catch (error) {
    alert("Error: Unable to fetch passwords. Please try again later." + error);
  }
}

function toggleVisibility() {
  const passwordInput = document.getElementById("passwordInput");
  const visibility = document.getElementById("toggleVisibilityIcon");

  // Toggle password visibility
  passwordInput.type = passwordInput.type === "password" ? "text" : "password";
  if (passwordInput.type === "text") {
    visibility.innerHTML = "&#x1f64a";
  } else {
    visibility.innerHTML = "&#x1f648;"; // Show eye with slash icon
  }
  wiggleDots("toggleVisibilityIcon");
}

// Enhanced form submission with loading states
document.addEventListener("DOMContentLoaded", function () {
  const layoutSelect = document.getElementById("layoutSelect");
  const layoutIndex = document.getElementById("layoutIndex");

  layoutSelect.addEventListener("change", function () {
    layoutIndex.value = layoutSelect.selectedIndex;
  });
  layoutIndex.value = layoutSelect.selectedIndex;

  document
    .getElementById("editFormContent")
    .addEventListener("submit", function (e) {
      e.preventDefault();

      // Show loading state
      const submitBtn = this.querySelector(".submit-btn");
      const btnText = submitBtn.querySelector(".btn-text");
      const loading = submitBtn.querySelector(".loading");

      btnText.style.opacity = "0";
      loading.classList.remove("hidden");
      submitBtn.disabled = true;

      document.getElementById("positionSelect").disabled = false;

      const formData = new FormData(this);
      if ((formData.get("password") || "").match(/^\s*$/)) {
        formData.delete("password");
      }
      formData.delete("lang");
      const params = new URLSearchParams(formData).toString();

      fetch("/editPass?" + params)
        .then((response) => response.text())
        .catch((error) => {
          alert("Error:", error);
        })
        .finally(async (data) => {
          await getPasswords();
          // Reset button state
          btnText.style.opacity = "1";
          loading.classList.add("hidden");
          submitBtn.disabled = false;
          setMode("type");
        });
    });
});

function confirmFactoryReset() {
  if (
    confirm(
      "Are you sure you want to factory reset KeyPass? This will delete ALL your saved passwords!",
    )
  ) {
    if (
      confirm(
        "FINAL WARNING: This action cannot be undone. Proceed with factory reset?",
      )
    ) {
      fetch("/reset")
        .then((response) => {
          if (response.ok) {
            alert("Factory reset successful. The device will reload the page.");
            setTimeout(() => window.location.reload(), 1000);
          } else {
            alert("Factory reset failed.");
          }
        })
        .catch((error) => {
          console.error("Error:", error);
          alert("Factory reset failed due to an error.");
        });
    }
  }
}

// Load passwords on page load
window.onload = async () => {
  await getPasswords();
};
</script>
</body>
</html>
<!-- vim:ts=2:sw=2:et:
-->
)=====";
#endif
