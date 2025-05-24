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

function wiggleDots() {
  // Add wiggle class
  const dieIcon = document.getElementById("diceIcon");
  dieIcon.classList.add("wiggle-dots");

  // Remove wiggle class after animation completes
  setTimeout(() => {
    dieIcon.classList.remove("wiggle-dots");
  }, 500);
}

function generatePass() {
  let length = 12;
  length = prompt("Password length (default 12):", length);
  if (!length || isNaN(length) || length < 8) {
    return;
  }
  wiggleDots();
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
    default:
      console.error("Invalid action");
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
