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
summary {
cursor: pointer;
color: white;
font-weight: 600;
font-size: 16px;
padding: 10px;
transition: background 0.3s ease;
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
.column {
flex-direction: column !important;
}
#wifiPassForm {
max-height: 0;
overflow: hidden;
transition: max-height 0.3s ease-out, opacity 0.3s ease-out, margin 0.3s ease-out;
opacity: 0;
margin: 0;
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

.button-column {
display: flex;
flex-direction: column;
gap: 15px;
margin-bottom: 30px;
animation: slideUp 0.8s ease-out 0.2s both;
}
.button-row {
display: flex;
flex-direction: row;
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
@keyframes fadeInRight {
from {
opacity: 0;
transform: translateX(20px);
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

}

.password-card:nth-child(odd) { animation: fadeInLeft 0.5s ease-out; }
.password-card:nth-child(even) { animation: fadeInRight 0.5s ease-out; }

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
#passwordInput, #newWifiPass, #confirmWifiPass {
/* use a monospace font */
font-family: 'Courier New', Courier, monospace;
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
flex-direction: row;
gap: 10px;
}

.button-column {
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

@keyframes wiggle {
0% { transform: rotate(0deg); }
20% { transform: rotate(-10deg); }
40% { transform: rotate(10deg); }
60% { transform: rotate(-7deg); }
80% { transform: rotate(7deg); }
100% { transform: rotate(0deg); }
}

.wiggle {
animation: wiggle 0.5s ease-in-out;
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

<button id="add-button" onclick="shake('add-button'); toggleCreatePassForm()" class="modern-btn menu-btn" role="button" style="position: absolute; top: 20px; left: 20px; padding: 0; font-size: 200%; width: 50px; height: 50px; z-index: 10;">+</button>
<button id="settings-button" onclick="shake('settings-button'); toggleSettings()" class="modern-btn menu-btn" role="button" style="position: absolute; top: 20px; right: 20px; padding: 0; font-size: 200%; width: 50px; height: 50px; z-index: 10;">
&#x1F4AB;
</button>
</div>
<div class="mainScreen hidden" id="settingsForm">
<div class="glass-panel button-column">
<button class="modern-btn column togglableButton" role="button" data-setting="confirm_actions" data-enabled-text="Confirm actions" data-disabled-text="Just run actions">Ask for confirmations</button>
<button class="modern-btn column togglableButton" role="button" data-setting="password_visibility" data-enabled-text="Show passwords" data-disabled-text="Hide passwords">Ask for confirmations</button>
<button class="modern-btn column" role="button" onclick="window.location.pathname = '/dump'" style="background: rgba(107, 255, 107, 0.2);">Backup passwords blob</button>
<button class="modern-btn column" role="button" onclick="toggleWifiPassForm()">Change Wi-Fi password</button>
<div id="wifiPassForm" class="hidden" style="overflow: hidden; transition: all 0.3s ease-out;">
<div class="glass-panel" style="margin-top: 10px;">
<div class="form-group">
<input class="passwordLength" type="password" id="newWifiPass" placeholder="New Wi-Fi password" minlength="8" maxlength="31" required>
<input class="passwordLength" type="password" id="confirmWifiPass" placeholder="Confirm password" minlength="8" maxlength="31" required>
<button type="button" onclick="updateWifiPass()" class="submit-btn">
<span class="btn-text">Update Password</span>
<span class="loading hidden"></span>
</button>
</div>
</div>
</div>
<button class="modern-btn column" role="button" onclick="checkPassphrase({force:true})" style="background: rgba(255, 107, 107, 0.2);">Reset passphrase</button>
<button class="modern-btn column" role="button" onclick="confirmFactoryReset()" style="background: rgba(255, 107, 107, 0.2);">Factory Reset</button>
<input type="file" id="blobFileInput" style="display: none;" />
<button class="modern-btn column" role="button" onclick="uploadBlobFile()">Recover passwords from blob</button>
<button class="modern-btn column" role="button" onclick="alert(localStorage.getItem('keypass_passphrase'))">Show passphrase</button>
</div>
</div>

<div class="mainScreen hidden" id="editForm">
<div class="glass-panel">
<form id="editFormContent" method="get" action="/editPass">
<input type="hidden" name="id" id="positionSelect" disabled>

<div class="form-group">
<label for="passLabel">Name</label>
<input class="passwordLength" id="passLabel" name="name" type="text" required placeholder="Enter password name" maxlength="15">

<label for="passwordInput">Password</label>
<div style="display: flex; gap: 10px;">
<input class="passwordLength" id="passwordInput" type="password" name="password" placeholder="Enter password..." maxlength="31">

<button id="toggleVisibilityIcon" type="button" class="modern-btn togglableButton"
data-setting="password_visibility"
data-enabled-text="&#x1F640;" data-disabled-text="&#x1f648;"
data-changed="togglePasswordVisibility"

style="width: 50px; flex: 0 0 auto; padding: 0; font-size: 200%;">&#x1f648;</button>
<button type="button" onclick="generatePass()" class="modern-btn" style="width: 50px; flex: 0 0 auto; padding: 0;"><div class="icon"><div id="diceIcon" class="icon-die"></div></div></button>

</div>
<div style="display: flex; gap: 10px; margin-top: 10px;">
<button id="typeOldPassBtn" type="button" onclick="typeOldPass()" class="modern-btn hidden" style="width: auto; flex: auto;">&#x2328;&nbsp;&nbsp;Type old (saved) pass</button>
<button id="typeNewPassBtn" type="button" onclick="typeNewPass()" class="modern-btn hidden" style="width: auto; flex: auto;">&#x2328;&nbsp;&nbsp;Type new pass</button>
</div>
<label for="layoutSelect">Keyboard layout</label>
<select name="lang" id="layoutSelect" required>
<option value="bitlocker">F1-F12 (bitlocker)</option>
<option value="fr">French</option>
<option value="us">US</option>
</select>
<input type="hidden" name="layout" id="layoutIndex">
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
<details id="advancedTypingOptions" style="padding: 1ex">
<summary>Advanced options</label></summary>
<div class="form-group">
<p style="padding: 1ex">
<div>
<label for="typePasswordPressEnter">After typing a password:</label>
<button id="typePasswordPressEnter" class="modern-btn column togglableButton" role="button" data-setting="press_enter" data-enabled-text="Press &crarr;" data-disabled-text="Do not press &crarr;"></button></div>
<p style="padding: 1ex">
<label for="layoutOverrideSelect">Keyboard layout override:</label>
<select name="lang" id="layoutOverrideSelect" required>
<option value="">Password specific</option>
<option value="fr">French</option>
<option value="us">US</option>
</select>
</p>
</div>
<p>&nbsp;</p>
</details>
</div>
</div>
<script>
<!-- WARN: The following line must not be changed and is replaced with index.js content automatically -->
const MAX_PASSWORD_LENGTH=31;const UserPreferences={defaults:{confirm_actions:true,password_visibility:false,password_length:12,},save:function(key,value){localStorage.setItem(`userPref_${key}`,JSON.stringify(value));},get:function(key){const stored=localStorage.getItem(`userPref_${key}`);return stored?JSON.parse(stored):this.defaults[key];},reset:function(){Object.keys(this.defaults).forEach((key)=>{this.save(key,this.defaults[key]);});},};const ui_data={mode:"type",current_focus:"type-button",passwords:[],change_focus(elementId){document.querySelectorAll(".modern-btn").forEach((btn)=>{btn.classList.remove("active");});const newFocus=document.getElementById(elementId);if(!newFocus)return false;newFocus.classList.add("active");this.current_focus=elementId;},};function errorHandler(error){alert("Error: "+error);}
function getRandomChar(){const charset="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0"+"123456789!@#$%^&*()_+-=[]{}|;:,.<>?";const charsetSize=charset.length;return charset[Math.floor(Math.random()*charsetSize)];}
function generatePassword(length){let password="";for(let i=0;i<length;++i){password+=getRandomChar();}
return password;}
function generatePass(){const uid=~~document.getElementById("positionSelect").value;const length=document.getElementById("passwordInput").value.length||ui_data.passwords[uid]?.len||prompt("How many characters ?",18);if(!length||isNaN(length)||length<2){return;}
if(length>MAX_PASSWORD_LENGTH)length=MAX_PASSWORD_LENGTH;shake("diceIcon");const password=generatePassword(length);document.getElementById("passwordInput").value=password;document.getElementById("typeNewPassBtn").classList.remove("hidden");if(ui_data.mode!="add")
document.getElementById("typeOldPassBtn").classList.remove("hidden");}
function shake(elementId="diceIcon"){const icon=typeof elementId=="string"?document.getElementById(elementId):elementId;icon.classList.add("wiggle");setTimeout(()=>{icon.classList.remove("wiggle");},500);}
function toggleButton(buttonElement,options={}){const setting=buttonElement.getAttribute("data-setting")||options.setting;const currentState=UserPreferences.get(setting)!==false;const newState=options.noflip?currentState:!currentState;UserPreferences.save(setting,newState);const mode=newState?"enabled":"disabled";buttonElement.textContent=buttonElement.getAttribute(`data-${mode}-text`);shake(buttonElement);const fn=buttonElement.getAttribute("data-changed")||"()=>{}";eval(fn)(newState);}
async function togglePasswordVisibility(visible){passwordInput.type=visible?"text":"password";if(ui_data.mode=="edit"&&passwordInput.value.length==0){const pass=await fetch(`/fetchPass?id=${positionSelect.value}`);passwordInput.value=await pass.text();}}
function initToggleButtons(){const elements=document.querySelectorAll(".togglableButton");elements.forEach((button)=>{toggleButton(button,{noflip:true});button.addEventListener("click",()=>{toggleButton(button);});});}
function hideAll(){Array.from(document.getElementsByClassName("mainScreen")).forEach(hideElement,);}
function hideElement(elementId){const el=typeof elementId=="string"?document.getElementById(elementId):elementId;if(!el.classList.contains("hidden")){el.classList.add("hidden");}}
function showElement(elementId){const el=typeof elementId=="string"?document.getElementById(elementId):elementId;if(el.classList.contains("hidden")){el.classList.remove("hidden");}}
function showPasswords(){if(document.getElementById("passList").classList.contains("hidden")){hideAll();setTimeout(()=>showElement("passList"),100);}}
function leaveEditForm(uid){layoutSelect.selectedIndex=1;passwordInput.value="";}
function showEditForm(uid){if(document.getElementById("editForm").classList.contains("hidden")){positionSelect.value=uid;positionSelect.disabled=true;hideAll();setTimeout(()=>showElement("editForm"),300);}}
function setMode(action){if(ui_data.mode==action){return;}
if(ui_data.mode=="edit"||ui_data.mode=="add"){leaveEditForm();}
if(ui_data.mode=="type"){hideElement(advancedTypingOptions);}
ui_data.mode=action;ui_data.change_focus(`${action}-button`);switch(action){case"edit":showPasswords();break;case"type":showElement("advancedTypingOptions");showPasswords();break;case"add":const uid=ui_data.passwords.length;fillForm({name:""});showEditForm(uid);break;case"settings":showSettings();break;default:console.error("Invalid action");}}
function showSettings(){if(document.getElementById("settingsForm").classList.contains("hidden")){hideAll();setTimeout(()=>showElement("settingsForm"),300);}}
function fillForm(data){if(data.layout!=undefined)layoutSelect.selectedIndex=data.layout+1;if(data.uid!=undefined){positionSelect.value=data.uid;}else{positionSelect.value=-1;}
if(data.name!=undefined)passLabel.value=data.name;}
async function checkPassphrase(opts){const{force}=opts||{};const storedPassphrase=localStorage.getItem("keypass_passphrase");const storedMagic=localStorage.getItem("keypass_magicnumber");if(!storedPassphrase||force){const newPassphrase=prompt("Please set up a passphrase to secure your passwords:",);if(newPassphrase){let newPin=prompt("Please set up a PIN number to secure your passwords:",);newPin=newPin&&/^\d+$/.test(newPin)?parseInt(newPin,10):null;if(newPin){await setPassPhrase(newPassphrase,newPin);localStorage.setItem("keypass_passphrase",newPassphrase);localStorage.setItem("keypass_magicnumber",newPin);return true;}else{alert("A PIN CODE (number) is required to use KeyPass.");return await checkPassphrase();}}else{alert("A passphrase is required to use KeyPass.");return await checkPassphrase();}}else{await setPassPhrase(storedPassphrase,storedMagic);return true;}}
async function setPassPhrase(phrase,pin){return fetch(`/passphrase?p=${phrase}&k=${pin}`).catch(errorHandler);}
function typeOldPass(){const uid=positionSelect.value;fetch(`/typePass?id=${uid}`).catch(errorHandler);}
function typeNewPass(){const password=passwordInput.value;const escaped=encodeURIComponent(password);const layout=layoutSelect.value;fetch(`/typeRaw?text=${escaped}&layout=${layout}`).catch(errorHandler);}
function passwordClick(event,uid){if(uid===undefined&&event){const card=event.target.closest(".password-card");if(card){uid=parseInt(card.dataset.passwordId);}}
switch(ui_data.mode){case"edit":fillForm({uid:uid,name:ui_data.passwords[uid].name,layout:ui_data.passwords[uid].layout,});showEditForm(uid);break;default:const card=event?event.target.closest(".password-card"):document.querySelector(`[data-password-id="${uid}"]`);if(card){card.style.transform="scale(0.95)";setTimeout(()=>{card.style.transform="";},150);}
const layout=layoutOverrideSelect.selectedIndex;const press_enter=UserPreferences.get("press_enter");const prefix=`/typePass?id=${uid}&ret=${press_enter}`;const url=layout>0?`${prefix}&layout=${layout - 1}`:prefix;fetch(url).catch(errorHandler);}}
function updateWifiPass(){const newPass=newWifiPass.value;const confirmPass=confirmWifiPass.value;if(!newPass)return;if(newPass!==confirmPass){alert("Passwords do not match. Please try again.");return;}
if(newPass.length<8){alert("Password must be at least 8 characters long.");return;}
fetch(`/updateWifiPass?newPass=${newPass}`).then((response)=>response.text()).catch(errorHandler);const loadingEl=document.querySelector("#wifiPassForm .loading");const btnText=document.querySelector("#wifiPassForm .btn-text");if(loadingEl&&btnText){btnText.classList.add("hidden");loadingEl.classList.remove("hidden");}
setTimeout(()=>{toggleWifiPassForm();newWifiPass.value="";confirmWifiPass.value="";if(loadingEl&&btnText){btnText.classList.remove("hidden");loadingEl.classList.add("hidden");}},1000);}
function toggleCreatePassForm(){if(ui_data.mode==="add"){setMode("type");}else{setMode("add");}}
function toggleSettings(){if(ui_data.mode==="settings"){setMode("type");}else{setMode("settings");}}
async function getPasswords(){try{const req=await fetch("/list");const passwords=await req.json();ui_data.passwords=passwords.passwords;const count=passwords.passwords.length;const total=passwords.free+count;subtitle.innerText=`Stored ${count} over ${total}`;if(passwords.passwords.length===0){return setMode("add");}
const passList=document.querySelector("#passList .password-grid");const domData=[];for(const pass of passwords.passwords){domData.push(`
    <div class="password-card" role="button" data-password-id="${pass.uid}">
        <div class="password-name">${pass.name}</div>
    </div>
    `);}
passList.innerHTML=domData.join("");}catch(error){alert("Error: Unable to fetch passwords. Please try again later."+error);}}
function confirmFactoryReset(){if(confirm("Are you sure you want to factory reset KeyPass? This will delete ALL your saved passwords!",)){if(confirm("FINAL WARNING: This action cannot be undone. Proceed with factory reset?",)){fetch("/reset").then((response)=>{if(response.ok){alert("Factory reset successful. The device will reload the page.");setTimeout(()=>window.location.reload(),1000);}else{alert("Factory reset failed.");}}).catch(errorHandler);}}}
function uploadBlobFile(){if(blobFileInput.style.display==="none"){blobFileInput.style.display="block";alert("Once the file is selected, click the same button again to confirm.");blobFileInput.click();return;}
if(blobFileInput.files.length===0){alert("Please select a file first.");return;}
const file=blobFileInput.files[0];fetch("/restore",{method:"POST",headers:{"Content-Type":"application/octet-stream"},body:file,}).then((response)=>{if(!response.ok){throw new Error(`Server responded with ${response.status}: ${response.statusText}`,);}
return response.text();}).then((result)=>{blobFileInput.style.display="none";alert(result);}).catch((error)=>{alert(`Error: ${error.message}`);});}
function toggleWifiPassForm(){if(wifiPassForm.classList.contains("hidden")){wifiPassForm.classList.remove("hidden");wifiPassForm.style.display="block";wifiPassForm.style.maxHeight="0";wifiPassForm.style.opacity="0";wifiPassForm.style.margin="0";void wifiPassForm.offsetWidth;wifiPassForm.style.maxHeight="500px";wifiPassForm.style.opacity="1";wifiPassForm.style.margin="10px 0";}else{wifiPassForm.style.maxHeight="0";wifiPassForm.style.opacity="0";wifiPassForm.style.margin="0";setTimeout(()=>{wifiPassForm.classList.add("hidden");},300);}}
function editFormHandler(e){e.preventDefault();const submitBtn=this.querySelector(".submit-btn");const btnText=submitBtn.querySelector(".btn-text");const loading=submitBtn.querySelector(".loading");btnText.style.opacity="0";loading.classList.remove("hidden");submitBtn.disabled=true;positionSelect.disabled=false;const formData=new FormData(this);if((formData.get("password")||"").match(/^\s*$/)){formData.delete("password");}
formData.delete("lang");const params=new URLSearchParams(formData).toString();fetch("/editPass?"+params).then((response)=>response.text()).catch(errorHandler).finally(async(data)=>{await getPasswords();btnText.style.opacity="1";loading.classList.add("hidden");submitBtn.disabled=false;setMode("type");});}
document.addEventListener("DOMContentLoaded",function(){const handler=()=>{layoutIndex.value=layoutSelect.selectedIndex-1;};layoutSelect.addEventListener("change",handler);handler();editFormContent.addEventListener("submit",editFormHandler);initToggleButtons();});function setupPasswordCardEvents(){const passwordGrid=document.querySelector(".password-grid");let pressTimer;let longPressTriggered=false;let currentCard=null;const longPressDuration=800;passwordGrid.removeEventListener("mousedown",handleMouseDown);passwordGrid.removeEventListener("mouseup",handleMouseUp);passwordGrid.removeEventListener("mouseleave",handleMouseLeave);passwordGrid.removeEventListener("touchstart",handleTouchStart);passwordGrid.removeEventListener("touchend",handleTouchEnd);passwordGrid.removeEventListener("touchcancel",handleTouchCancel);passwordGrid.addEventListener("mousedown",handleMouseDown);passwordGrid.addEventListener("mouseup",handleMouseUp);passwordGrid.addEventListener("mouseleave",handleMouseLeave);passwordGrid.addEventListener("touchstart",handleTouchStart);passwordGrid.addEventListener("touchend",handleTouchEnd);passwordGrid.addEventListener("touchcancel",handleTouchCancel);function handleMouseDown(e){const card=e.target.closest(".password-card");if(!card)return;currentCard=card;longPressTriggered=false;pressTimer=setTimeout(function(){const passwordId=parseInt(card.dataset.passwordId);handleLongPress(passwordId);longPressTriggered=true;},longPressDuration);}
function handleMouseUp(e){if(!currentCard)return;clearTimeout(pressTimer);if(!longPressTriggered){passwordClick(e);}
currentCard=null;}
function handleMouseLeave(e){if(currentCard){clearTimeout(pressTimer);currentCard=null;}}
function handleTouchStart(e){const card=e.target.closest(".password-card");if(!card)return;currentCard=card;longPressTriggered=false;pressTimer=setTimeout(function(){const passwordId=parseInt(card.dataset.passwordId);handleLongPress(passwordId);longPressTriggered=true;},longPressDuration);}
function handleTouchEnd(e){if(!currentCard)return;clearTimeout(pressTimer);if(!longPressTriggered){e.preventDefault();passwordClick(e);}
currentCard=null;}
function handleTouchCancel(e){if(currentCard){clearTimeout(pressTimer);currentCard=null;}}}
function handleLongPress(passwordId){if(ui_data.mode!=="edit"){setMode("edit");}
fillForm({uid:passwordId,name:ui_data.passwords[passwordId].name,layout:ui_data.passwords[passwordId].layout,});showEditForm(passwordId);}
window.onload=async()=>{await checkPassphrase();await getPasswords();setupPasswordCardEvents();document.querySelectorAll(".passwordLength").forEach((el)=>{el.setAttribute("maxlength",MAX_PASSWORD_LENGTH);});};</script>
</body>
</html>
<!-- vim:ts=2:sw=2:et:
-->
)=====";
#endif
