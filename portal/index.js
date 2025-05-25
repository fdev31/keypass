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

    // Add active class to selected button
    const newFocus = document.getElementById(elementId);
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
  const length = ui_data.passwords[uid]?.len || prompt("Length:", 12);

  if (!length || isNaN(length) || length < 4) {
    return;
  }
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
  ui_data.change_focus(`${action}-button`);

  switch (action) {
    case "edit":
      showPasswords();
      break;
    case "type":
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
  if (data.layout != undefined)
    document.getElementById("layoutSelect").selectedIndex = data.layout;
  if (data.uid != undefined) {
    document.getElementById("positionSelect").value = data.uid;
  } else {
    document.getElementById("positionSelect").value = -1;
  }

  if (data.name != undefined)
    document.getElementById("passLabel").value = data.name;
}

// MCU Actions
function typeOldPass() {
  const uid = document.getElementById("positionSelect").value;
  fetch(`/typePass?id=${uid}`).catch(errorHandler);
}

function typeNewPass() {
  const password = document.getElementById("passwordInput").value;
  const escaped = encodeURIComponent(password);
  const layout = document.getElementById("layoutSelect").value;
  fetch(`/typeRaw?text=${escaped}&layout=${layout}`).catch(errorHandler);
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
      fetch(`/typePass?id=${uid}`).catch(errorHandler);
  }
}

function updateWifiPass() {
  const newPass = document.getElementById("newWifiPass").value;
  const confirmPass = document.getElementById("confirmWifiPass").value;
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
    document.getElementById("newWifiPass").value = "";
    document.getElementById("confirmWifiPass").value = "";

    if (loadingEl && btnText) {
      btnText.classList.remove("hidden");
      loadingEl.classList.add("hidden");
    }
  }, 1000);
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

function toggleWifiPassForm() {
  const form = document.getElementById("wifiPassForm");

  if (form.classList.contains("hidden")) {
    // Show the form
    form.classList.remove("hidden");
    form.style.display = "block";
    form.style.maxHeight = "0";
    form.style.opacity = "0";
    form.style.margin = "0";

    // Force reflow
    void form.offsetWidth;

    // Animate to visible state
    form.style.maxHeight = "500px"; // Adjust based on your form's actual height
    form.style.opacity = "1";
    form.style.margin = "10px 0";
  } else {
    // Animate to hidden state
    form.style.maxHeight = "0";
    form.style.opacity = "0";
    form.style.margin = "0";

    // After animation completes, hide the element
    setTimeout(() => {
      form.classList.add("hidden");
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

  document.getElementById("positionSelect").disabled = false;

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
  const layoutSelect = document.getElementById("layoutSelect");
  const layoutIndex = document.getElementById("layoutIndex");

  layoutSelect.addEventListener("change", function () {
    layoutIndex.value = layoutSelect.selectedIndex;
  });
  layoutIndex.value = layoutSelect.selectedIndex;

  document
    .getElementById("editFormContent")
    .addEventListener("submit", editFormHandler);
  initToggleButtons();
});

// Load passwords on page load
window.onload = async () => {
  await getPasswords();
};
