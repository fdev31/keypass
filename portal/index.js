const MAX_PASSWORD_LENGTH = 31; // WARN: Adapt to number of blocks in use!

const UserPreferences = {
  // Default preferences
  defaults: {
    confirm_actions: true,
    password_visibility: false,
    password_length: 12,
  },

  // Save a preference
  save: function (key, value) {
    localStorage.setItem(`userPref_${key}`, JSON.stringify(value));
  },

  // Get a preference (returns default if not found)
  get: function (key) {
    const stored = localStorage.getItem(`userPref_${key}`);
    return stored ? JSON.parse(stored) : this.defaults[key];
  },

  // Reset all preferences to defaults
  reset: function () {
    Object.keys(this.defaults).forEach((key) => {
      this.save(key, this.defaults[key]);
    });
  },
};

const ui_data = {
  mode: "type",
  current_focus: "type-button",
  passwords: [],
  change_focus(elementId) {
    // Remove active class from all buttons
    document.querySelectorAll(".modern-btn").forEach((btn) => {
      btn.classList.remove("active");
    });
    const newFocus = document.getElementById(elementId);

    if (!newFocus) return false;

    // Add active class to selected button
    newFocus.classList.add("active");
    this.current_focus = elementId;
  },
};

function errorHandler(error) {
  alert("Error: " + error);
}

// password stuff
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

function generatePass() {
  const uid = ~~document.getElementById("positionSelect").value;
  const length =
    document.getElementById("passwordInput").value.length ||
    ui_data.passwords[uid]?.len ||
    prompt("How many characters ?", 18);

  if (!length || isNaN(length) || length < 2) {
    return;
  }
  if (length > MAX_PASSWORD_LENGTH) length = MAX_PASSWORD_LENGTH; // Limit to 31 characters for compatibility

  shake("diceIcon");
  const password = generatePassword(length);
  document.getElementById("passwordInput").value = password;
  document.getElementById("typeNewPassBtn").classList.remove("hidden");
  if (ui_data.mode != "add")
    document.getElementById("typeOldPassBtn").classList.remove("hidden");
}

function shake(elementId = "diceIcon") {
  const icon =
    typeof elementId == "string"
      ? document.getElementById(elementId)
      : elementId;
  icon.classList.add("wiggle");

  // Remove wiggle class after animation completes
  setTimeout(() => {
    icon.classList.remove("wiggle");
  }, 500);
}
function toggleButton(buttonElement, options = {}) {
  const setting = buttonElement.getAttribute("data-setting") || options.setting;
  const currentState = UserPreferences.get(setting) !== false;

  const newState = options.noflip ? currentState : !currentState;

  UserPreferences.save(setting, newState);
  const mode = newState ? "enabled" : "disabled";
  buttonElement.textContent = buttonElement.getAttribute(`data-${mode}-text`);
  shake(buttonElement);
  const fn = buttonElement.getAttribute("data-changed") || "()=>{}";
  eval(fn)(newState);
}

async function togglePasswordVisibility(visible) {
  passwordInput.type = visible ? "text" : "password";
  if (ui_data.mode == "edit" && passwordInput.value.length == 0) {
    const pass = await fetch(`/fetchPass?id=${positionSelect.value}`);
    passwordInput.value = await pass.text();
  }
}

function initToggleButtons() {
  const elements = document.querySelectorAll(".togglableButton");
  elements.forEach((button) => {
    toggleButton(button, { noflip: true });
    button.addEventListener("click", () => {
      toggleButton(button);
    });
  });
}

function hideAll() {
  Array.from(document.getElementsByClassName("mainScreen")).forEach(
    hideElement,
  );
}

function hideElement(elementId) {
  const el =
    typeof elementId == "string"
      ? document.getElementById(elementId)
      : elementId;
  if (!el.classList.contains("hidden")) {
    el.classList.add("hidden");
    // el.classList.add("fade-out");
    // setTimeout(() => {
    //   el.classList.add("hidden");
    //   // el.classList.remove("fade-out");
    // }, 300);
  }
}
function showElement(elementId) {
  const el =
    typeof elementId == "string"
      ? document.getElementById(elementId)
      : elementId;

  if (el.classList.contains("hidden")) {
    el.classList.remove("hidden");
    // el.classList.add("fade-in");
    // setTimeout(() => {
    //   el.classList.remove("fade-in");
    // }, 400);
  }
}

function showPasswords() {
  // check if visible
  if (document.getElementById("passList").classList.contains("hidden")) {
    hideAll();
    setTimeout(() => showElement("passList"), 100);
  }
}

function leaveEditForm(uid) {
  layoutSelect.selectedIndex = 1;
  passwordInput.value = "";
}

function showEditForm(uid) {
  if (document.getElementById("editForm").classList.contains("hidden")) {
    positionSelect.value = uid;
    positionSelect.disabled = true;
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
  if (ui_data.mode == "type") {
    hideElement(advancedTypingOptions);
  }
  ui_data.mode = action;
  ui_data.change_focus(`${action}-button`);

  switch (action) {
    case "edit":
      showPasswords();
      break;
    case "type":
      showElement("advancedTypingOptions");
      showPasswords();
      break;
    case "add":
      const uid = ui_data.passwords.length;
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
  if (data.layout != undefined) layoutSelect.selectedIndex = data.layout + 1;
  if (data.uid != undefined) {
    positionSelect.value = data.uid;
  } else {
    positionSelect.value = -1;
  }

  if (data.name != undefined) passLabel.value = data.name;
}

// MCU Actions
async function checkPassphrase(opts) {
  const { force } = opts || {};
  const storedPassphrase = localStorage.getItem("keypass_passphrase");

  if (!storedPassphrase || force) {
    // First time setup - prompt for new passphrase
    const newPassphrase = prompt(
      "Please set up a passphrase to secure your passwords:",
    );
    if (newPassphrase) {
      await setPassPhrase(newPassphrase);
      localStorage.setItem("keypass_passphrase", newPassphrase);
      return true;
    } else {
      // User cancelled - might need to retry or restrict access
      alert("A passphrase is required to use KeyPass.");
      return await checkPassphrase(); // Prompt again
    }
  } else {
    // Use the stored passphrase
    await setPassPhrase(storedPassphrase);
    return true;
  }
}

async function setPassPhrase(phrase) {
  return fetch(`/passphrase?p=${phrase}`).catch(errorHandler);
}

function typeOldPass() {
  const uid = positionSelect.value;
  fetch(`/typePass?id=${uid}`).catch(errorHandler);
}
function typeNewPass() {
  const password = passwordInput.value;
  const escaped = encodeURIComponent(password);
  const layout = layoutSelect.value;
  fetch(`/typeRaw?text=${escaped}&layout=${layout}`).catch(errorHandler);
}
function passwordClick(event, uid) {
  // If uid is not provided, try to get it from the event target
  if (uid === undefined && event) {
    const card = event.target.closest(".password-card");
    if (card) {
      uid = parseInt(card.dataset.passwordId);
    }
  }

  switch (ui_data.mode) {
    case "edit":
      fillForm({
        uid: uid,
        name: ui_data.passwords[uid].name,
        layout: ui_data.passwords[uid].layout,
      });
      showEditForm(uid);
      break;
    default: // "type"
      // Add visual feedback for typing action
      const card = event
        ? event.target.closest(".password-card")
        : document.querySelector(`[data-password-id="${uid}"]`);
      if (card) {
        card.style.transform = "scale(0.95)";
        setTimeout(() => {
          card.style.transform = "";
        }, 150);
      }
      const layout = layoutOverrideSelect.selectedIndex;
      const press_enter = UserPreferences.get("press_enter");
      const prefix = `/typePass?id=${uid}&ret=${press_enter}`;
      const url = layout > 0 ? `${prefix}&layout=${layout - 1}` : prefix;
      fetch(url).catch(errorHandler);
  }
}

function updateWifiPass() {
  const newPass = newWifiPass.value;
  const confirmPass = confirmWifiPass.value;
  if (!newPass) return;
  // ask again and check if similar
  if (newPass !== confirmPass) {
    alert("Passwords do not match. Please try again.");
    return;
  }
  if (newPass.length < 8) {
    alert("Password must be at least 8 characters long.");
    return;
  }
  fetch(`/updateWifiPass?newPass=${newPass}`)
    .then((response) => response.text())
    .catch(errorHandler);

  const loadingEl = document.querySelector("#wifiPassForm .loading");
  const btnText = document.querySelector("#wifiPassForm .btn-text");
  if (loadingEl && btnText) {
    btnText.classList.add("hidden");
    loadingEl.classList.remove("hidden");
  }

  // Simulate API call (remove this in production)
  setTimeout(() => {
    // Hide the form with animation
    toggleWifiPassForm();

    // Reset form and loading state
    newWifiPass.value = "";
    confirmWifiPass.value = "";

    if (loadingEl && btnText) {
      btnText.classList.remove("hidden");
      loadingEl.classList.add("hidden");
    }
  }, 1000);
}

function toggleCreatePassForm() {
  if (ui_data.mode === "add") {
    setMode("type");
  } else {
    setMode("add");
  }
}
function toggleSettings() {
  if (ui_data.mode === "settings") {
    setMode("type");
  } else {
    setMode("settings");
  }
}

async function getPasswords() {
  try {
    const req = await fetch("/list");
    const passwords = await req.json();
    ui_data.passwords = passwords.passwords;
    const count = passwords.passwords.length;
    const total = passwords.free + count;
    subtitle.innerText = `Stored ${count} over ${total}`;

    if (passwords.passwords.length === 0) {
      return setMode("add");
    }

    const passList = document.querySelector("#passList .password-grid");
    const domData = [];

    for (const pass of passwords.passwords) {
      domData.push(`
    <div class="password-card" role="button" data-password-id="${pass.uid}">
        <div class="password-name">${pass.name}</div>
    </div>
    `);
    }

    passList.innerHTML = domData.join("");
  } catch (error) {
    alert("Error: Unable to fetch passwords. Please try again later." + error);
  }
}

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
        .catch(errorHandler);
    }
  }
}
function uploadBlobFile() {
  // Get both file and text inputs
  passwdRecoveryInputs.style.display = "block";
  const fileInput = document.getElementById("blobFileInput");
  const textInput = document.getElementById("blobTextInput");

  // Determine which one to use based on content
  let contentToUpload;
  let contentSource;

  // Check if we have a file selected
  if (fileInput.files && fileInput.files.length > 0) {
    contentToUpload = fileInput.files[0];
    contentSource = "file";
  }
  // Otherwise check if we have text pasted
  else if (textInput.value.trim()) {
    const pastedText = textInput.value.trim();
    contentToUpload = new Blob([pastedText], { type: "text/plain" });
    contentSource = "text";
  }
  // If neither, ask the user to provide content
  else {
    alert("Please select a file or paste blob content first.");
    return;
  }

  // Show loading indicator
  const button = document.querySelector('button[onclick="uploadBlobFile()"]');
  const originalText = button.textContent;
  button.textContent = "Uploading...";
  button.disabled = true;

  // Use the fetch API to send the content to the server
  fetch("/restore", {
    method: "POST",
    headers: { "Content-Type": "application/octet-stream" },
    body: contentToUpload,
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error(
          `Server responded with ${response.status}: ${response.statusText}`,
        );
      }
      return response.text();
    })
    .then((result) => {
      // Reset inputs
      fileInput.value = "";
      textInput.value = "";

      // Reset button
      button.textContent = originalText;
      button.disabled = false;

      // Show success message
      alert(result);

      // Refresh the page to show restored data
      setTimeout(() => {
        window.location.reload();
      }, 1000);
    })
    .catch((error) => {
      // Reset button
      button.textContent = originalText;
      button.disabled = false;

      // Show error message
      alert(`Error: ${error.message}`);
    });
}

// Make the passwdRecoveryInputs div more user-friendly
function initializeRecoveryInputs() {
  const container = document.getElementById("passwdRecoveryInputs");
  const fileInput = document.getElementById("blobFileInput");
  const textInput = document.getElementById("blobTextInput");
  container.style.display = "none";

  // Style the container
  container.style.marginTop = "10px";
  container.style.marginBottom = "10px";

  // Hide file input initially (it's ugly by default)
  fileInput.style.display = "none";

  // Add toggle buttons
  const buttonRow = document.createElement("div");
  buttonRow.className = "button-row";
  buttonRow.style.marginBottom = "10px";

  const fileButton = document.createElement("button");
  fileButton.className = "modern-btn";
  fileButton.textContent = "Select File";
  fileButton.onclick = function (e) {
    e.preventDefault();
    fileInput.click();
  };

  const textButton = document.createElement("button");
  textButton.className = "modern-btn";
  textButton.textContent = "Use Text Input";
  textButton.onclick = function (e) {
    e.preventDefault();
    textInput.focus();
  };

  buttonRow.appendChild(fileButton);
  buttonRow.appendChild(textButton);

  // Add a file name display element
  const fileNameDisplay = document.createElement("div");
  fileNameDisplay.style.fontSize = "14px";
  fileNameDisplay.style.color = "rgba(255, 255, 255, 0.8)";
  fileNameDisplay.style.marginBottom = "10px";
  fileNameDisplay.style.display = "none";

  // Listen for file selection
  fileInput.addEventListener("change", function () {
    if (fileInput.files.length > 0) {
      fileNameDisplay.textContent = `Selected file: ${fileInput.files[0].name}`;
      fileNameDisplay.style.display = "block";
      textInput.value = ""; // Clear text input when file is selected
    } else {
      fileNameDisplay.style.display = "none";
    }
  });

  // Insert our new elements at the beginning of the container
  container.insertBefore(fileNameDisplay, container.firstChild);
  container.insertBefore(buttonRow, container.firstChild);
}
function toggleWifiPassForm() {
  if (wifiPassForm.classList.contains("hidden")) {
    // Show the form
    wifiPassForm.classList.remove("hidden");
    wifiPassForm.style.display = "block";
    wifiPassForm.style.maxHeight = "0";
    wifiPassForm.style.opacity = "0";
    wifiPassForm.style.margin = "0";

    void wifiPassForm.offsetWidth;

    // Animate to visible state
    wifiPassForm.style.maxHeight = "500px"; // Adjust based on your form's actual height
    wifiPassForm.style.opacity = "1";
    wifiPassForm.style.margin = "10px 0";
  } else {
    // Animate to hidden state
    wifiPassForm.style.maxHeight = "0";
    wifiPassForm.style.opacity = "0";
    wifiPassForm.style.margin = "0";

    // After animation completes, hide the element
    setTimeout(() => {
      wifiPassForm.classList.add("hidden");
    }, 300); // Match your transition duration
  }
}

function editFormHandler(e) {
  e.preventDefault();

  // Show loading state
  const submitBtn = this.querySelector(".submit-btn");
  const btnText = submitBtn.querySelector(".btn-text");
  const loading = submitBtn.querySelector(".loading");

  btnText.style.opacity = "0";
  loading.classList.remove("hidden");
  submitBtn.disabled = true;

  positionSelect.disabled = false;

  const formData = new FormData(this);
  if ((formData.get("password") || "").match(/^\s*$/)) {
    formData.delete("password");
  }
  formData.delete("lang");
  const params = new URLSearchParams(formData).toString();

  fetch("/editPass?" + params)
    .then((response) => response.text())
    .catch(errorHandler)
    .finally(async (data) => {
      await getPasswords();
      // Reset button state
      btnText.style.opacity = "1";
      loading.classList.add("hidden");
      submitBtn.disabled = false;
      setMode("type");
    });
}
// Enhanced form submission with loading states
document.addEventListener("DOMContentLoaded", function () {
  const handler = () => {
    layoutIndex.value = layoutSelect.selectedIndex - 1;
  };
  layoutSelect.addEventListener("change", handler);
  handler();

  editFormContent.addEventListener("submit", editFormHandler);
  initToggleButtons();
  initializeRecoveryInputs();
});
// Set up event delegation for password cards to handle both clicks and long presses

function setupPasswordCardEvents() {
  const passwordGrid = document.querySelector(".password-grid");
  let pressTimer;
  let longPressTriggered = false;
  let currentCard = null;
  let touchStartY = 0;
  let touchStartX = 0;
  const longPressDuration = 800; // ms
  const scrollThreshold = 10; // pixels of movement to consider it a scroll
  let isScrolling = false;

  // Remove any existing listeners
  passwordGrid.removeEventListener("mousedown", handleMouseDown);
  passwordGrid.removeEventListener("mouseup", handleMouseUp);
  passwordGrid.removeEventListener("mouseleave", handleMouseLeave);
  passwordGrid.removeEventListener("touchstart", handleTouchStart);
  passwordGrid.removeEventListener("touchend", handleTouchEnd);
  passwordGrid.removeEventListener("touchmove", handleTouchMove);
  passwordGrid.removeEventListener("touchcancel", handleTouchCancel);

  // Add listeners with named functions so they can be removed if needed
  passwordGrid.addEventListener("mousedown", handleMouseDown);
  passwordGrid.addEventListener("mouseup", handleMouseUp);
  passwordGrid.addEventListener("mouseleave", handleMouseLeave);

  // Touch support for mobile devices
  passwordGrid.addEventListener("touchstart", handleTouchStart);
  passwordGrid.addEventListener("touchmove", handleTouchMove);
  passwordGrid.addEventListener("touchend", handleTouchEnd);
  passwordGrid.addEventListener("touchcancel", handleTouchCancel);

  function handleMouseDown(e) {
    // Mouse handling code remains the same
    const card = e.target.closest(".password-card");
    if (!card) return;

    currentCard = card;
    longPressTriggered = false;

    pressTimer = setTimeout(function () {
      // Long press action
      const passwordId = parseInt(card.dataset.passwordId);
      handleLongPress(passwordId);
      longPressTriggered = true;
    }, longPressDuration);
  }

  function handleMouseUp(e) {
    if (!currentCard) return;

    clearTimeout(pressTimer);
    if (!longPressTriggered) {
      // Regular click action
      passwordClick(e);
    }
    currentCard = null;
  }

  function handleMouseLeave(e) {
    if (currentCard) {
      clearTimeout(pressTimer);
      currentCard = null;
    }
  }

  // Touch event handlers for mobile devices
  function handleTouchStart(e) {
    const card = e.target.closest(".password-card");
    if (!card) return;

    // Record initial touch position
    const touch = e.touches[0];
    touchStartY = touch.clientY;
    touchStartX = touch.clientX;
    isScrolling = false;

    currentCard = card;
    longPressTriggered = false;

    pressTimer = setTimeout(function () {
      if (!isScrolling) {
        // Long press action
        const passwordId = parseInt(card.dataset.passwordId);
        handleLongPress(passwordId);
        longPressTriggered = true;
      }
    }, longPressDuration);
  }

  function handleTouchMove(e) {
    if (!currentCard) return;

    const touch = e.touches[0];
    const moveY = Math.abs(touch.clientY - touchStartY);
    const moveX = Math.abs(touch.clientX - touchStartX);

    // If user has moved their finger more than the threshold, they're scrolling
    if (moveY > scrollThreshold || moveX > scrollThreshold) {
      isScrolling = true;
      clearTimeout(pressTimer);
    }
  }

  function handleTouchEnd(e) {
    if (!currentCard) return;

    clearTimeout(pressTimer);
    if (!longPressTriggered && !isScrolling) {
      // Only trigger click if not scrolling and not a long press
      e.preventDefault(); // Prevent mousedown/up from firing as well
      passwordClick(e);
    }

    currentCard = null;
    isScrolling = false;
  }

  function handleTouchCancel(e) {
    if (currentCard) {
      clearTimeout(pressTimer);
      currentCard = null;
      isScrolling = false;
    }
  }
}
// Function to handle long press on password card
function handleLongPress(passwordId) {
  // For example, directly switch to edit mode for this password
  if (ui_data.mode !== "edit") {
    setMode("edit");
  }
  fillForm({
    uid: passwordId,
    name: ui_data.passwords[passwordId].name,
    layout: ui_data.passwords[passwordId].layout,
  });
  showEditForm(passwordId);
}

// Load passwords on page load
window.onload = async () => {
  await checkPassphrase();
  await getPasswords();
  setupPasswordCardEvents();
  document.querySelectorAll(".passwordLength").forEach((el) => {
    el.setAttribute("maxlength", MAX_PASSWORD_LENGTH);
  });
};
