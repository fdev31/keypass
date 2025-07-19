package com.example.keypass;

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.Manifest;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

import no.nordicsemi.android.ble.data.Data;
import no.nordicsemi.android.ble.observer.ConnectionObserver;

interface InputCallback {
    void onInput(String input);
}

public class SettingsActivity extends AppCompatActivity implements KeyPassBleManager.DataCallback, ConnectionObserver {

    private static final String TAG = "KeyPass::SettingsActivity";
    private static final String PREFS_NAME = "KeyPassPrefs";
    private static final String PREF_CONFIRM_ACTIONS = "confirm_actions";
    private static final String PREF_HIDE_PASSWORDS = "hide_passwords";
    private static final String PREF_PASSPHRASE = "app_passphrase";

    private Switch confirmActionsSwitch;
    private Switch hidePasswordsSwitch;
    private Button backupPasswordsButton;
    private EditText backupOutputEditText;
    private Button changeWifiPasswordButton;
    private Button resetPassphraseButton;
    private Button factoryResetButton;
    private Button showPassphraseButton;
    private EditText passphraseOutputEditText;

    private KeyPassBleManager bleManager;
    private boolean isPassphraseVisible = false;

    private StringBuilder receivedDataBuffer = new StringBuilder();
    private int expectedDataSize = -1;

    private void setupBleMessageHandlers() {

        BleMessageProcessor.getInstance().registerJsonHandler("dump", (key, value) -> {
            Log.d(TAG, "Dump handler called with value: " + value);
            String dumpData;
            if (value instanceof JSONObject) {
                dumpData = ((JSONObject) value).toString();
            } else {
                dumpData = value.toString();
            }

            Log.d(TAG, "Processing dump data: " + dumpData);

            if (backupOutputEditText != null) {
                final String finalDumpData = dumpData;
                runOnUiThread(() -> {
                    backupOutputEditText.setText(finalDumpData);
                    Log.d(TAG, "backupOutputEditText set to: " + finalDumpData);
                });
            } else {
                Log.e(TAG, "backupOutputEditText is null, cannot set text!");
            }
        });

        BleMessageProcessor.getInstance().registerJsonHandler("passphrase", (key, value) -> {
            String passphrase = value.toString();
            runOnUiThread(() -> passphraseOutputEditText.setText(passphrase));
        });

        BleMessageProcessor.getInstance().registerJsonHandler("status", (key, value) -> {
            String status = value.toString();
            runOnUiThread(() -> showToast("Command Status: " + status));
        });

        // Default JSON handler
        BleMessageProcessor.getInstance().setDefaultJsonHandler((key, value) -> {
            runOnUiThread(() -> showToast("Received: " + value.toString()));
        });
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "SettingsActivity onCreate called.");
        setContentView(R.layout.activity_settings);

        // Initialize UI elements
        confirmActionsSwitch = findViewById(R.id.confirmActionsSwitch);
        hidePasswordsSwitch = findViewById(R.id.hidePasswordsSwitch);
        backupPasswordsButton = findViewById(R.id.backupPasswordsButton);
        if (backupPasswordsButton == null) Log.e(TAG, "backupPasswordsButton is null!");
        backupOutputEditText = findViewById(R.id.backupOutputEditText);
        if (backupOutputEditText == null) Log.e(TAG, "backupOutputEditText is null!");
        changeWifiPasswordButton = findViewById(R.id.changeWifiPasswordButton);
        resetPassphraseButton = findViewById(R.id.resetPassphraseButton);
        factoryResetButton = findViewById(R.id.factoryResetButton);
        showPassphraseButton = findViewById(R.id.showPassphraseButton);
        passphraseOutputEditText = findViewById(R.id.passphraseOutputEditText);

        // Initialize SharedPreferences
        SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);

        // Set initial state of switches and listeners
        confirmActionsSwitch.setChecked(prefs.getBoolean(PREF_CONFIRM_ACTIONS, true)); // Default to true
        confirmActionsSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            prefs.edit().putBoolean(PREF_CONFIRM_ACTIONS, isChecked).apply();
            showToast("Confirm actions: " + (isChecked ? "Enabled" : "Disabled"));
        });

        hidePasswordsSwitch.setChecked(prefs.getBoolean(PREF_HIDE_PASSWORDS, false)); // Default to false
        hidePasswordsSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            prefs.edit().putBoolean(PREF_HIDE_PASSWORDS, isChecked).apply();
            showToast("Hide passwords: " + (isChecked ? "Enabled" : "Disabled"));
            // TODO: Notify MainActivity to update password visibility
        });

        // Initialize BLE Manager (assuming it's a singleton or passed from MainActivity)
        // For now, we'll create a new instance, but ideally, it should be shared.
        setupBleMessageHandlers();
        bleManager = KeyPassBleManager.getInstance(this);
        bleManager.setConnectionObserver(this);
        bleManager.setDataCallback(this);

        // Set button listeners
        backupPasswordsButton.setOnClickListener(v -> {
            Log.d(TAG, "Backup Passwords button clicked.");
            if (bleManager.isDeviceConnected()) {
                Log.d(TAG, "BLE Manager is connected. Sending dump command.");
                bleManager.send("{cmd:\"dump\"}");
            } else {
                Log.d(TAG, "BLE Manager is NOT connected. Showing toast.");
                showToast("Not connected to KeyPass device.");
            }
        });

        changeWifiPasswordButton.setOnClickListener(v -> {
            if (bleManager.isDeviceConnected()) {
                showInputDialog("Change WiFi Password", "Enter new WiFi password:", "New WiFi Password", newPassword -> {
                    sendCommand(String.format("{\"cmd\":\"updateWifiPass\",\"newPass\":\"%s\"}", newPassword));
                });
            } else {
                showToast("Not connected to KeyPass device.");
            }
        });

        resetPassphraseButton.setOnClickListener(v -> {
            showResetPassphraseDialog();
        });

        factoryResetButton.setOnClickListener(v -> {
            if (bleManager.isDeviceConnected()) {
                showConfirmationDialog("Factory Reset", "Are you sure you want to perform a factory reset? All data will be erased.", () -> {
                    sendCommand("reset");
                });
            } else {
                showToast("Not connected to KeyPass device.");
            }
        });

        showPassphraseButton.setOnClickListener(v -> {
            isPassphraseVisible = !isPassphraseVisible;
            String storedPassphrase = prefs.getString(PREF_PASSPHRASE, "");

            if (isPassphraseVisible) {
                if (storedPassphrase.isEmpty()) {
                    passphraseOutputEditText.setText("No passphrase set.");
                    showToast("No passphrase set.");
                } else {
                    passphraseOutputEditText.setText(storedPassphrase);
                }
                showPassphraseButton.setText("Hide Passphrase");
            } else {
                passphraseOutputEditText.setText("********"); // Mask the passphrase
                showPassphraseButton.setText("Show Passphrase");
            }
        });

        // Initially mask the passphrase
        passphraseOutputEditText.setText("********");

        passphraseOutputEditText.setOnClickListener(v -> {
            showToast("Use 'Show Passphrase' button to view.");
        });

        // Initial UI state based on BLE connection
        updateUIForBleConnection(bleManager.isDeviceConnected());
    }

    private void showResetPassphraseDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Set Passphrase");
        builder.setMessage("Please set a passphrase for your application. This will be used to encrypt/decrypt your data.");
        builder.setCancelable(true); // Allow dismissing

        final EditText input = new EditText(this);
        input.setHint("Enter Passphrase");
        input.setInputType(android.text.InputType.TYPE_CLASS_TEXT | android.text.InputType.TYPE_TEXT_VARIATION_PASSWORD);
        builder.setView(input);

        builder.setPositiveButton("Set Passphrase", (dialog, which) -> {
            String newPassphrase = input.getText().toString();
            if (newPassphrase.isEmpty()) {
                showToast("Passphrase cannot be empty!");
                // Do not re-show dialog, user can click reset again
            } else {
                SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
                prefs.edit().putString(PREF_PASSPHRASE, newPassphrase).apply();
                showToast("Passphrase set successfully!");
                passphraseOutputEditText.setText(newPassphrase); // Update displayed passphrase

                // Send passphrase to microcontroller
                if (bleManager != null && bleManager.isDeviceConnected()) {
                    String cmd = String.format("{\"cmd\":\"passphrase\",\"p\":\"%s\"}", newPassphrase);
                    bleManager.send(cmd);
                    Log.d(TAG, "Sent passphrase to device: " + cmd);
                } else {
                    Log.w(TAG, "BLE Manager not connected, cannot send passphrase to device.");
                }
                dialog.dismiss();
            }
        });
        builder.setNegativeButton("Cancel", (dialog, which) -> dialog.cancel());

        builder.show();
    }

    private void sendCommand(String command) {
        if (confirmActionsSwitch.isChecked()) {
            showConfirmationDialog("Confirm Action", "Send command: " + command + "?", () -> {
                bleManager.send(command);
            });
        } else {
            bleManager.send(command);
        }
    }

    private void showToast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    private void showConfirmationDialog(String title, String message, Runnable onConfirm) {
        new AlertDialog.Builder(this)
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton("Yes", (dialog, which) -> onConfirm.run())
                .setNegativeButton("No", null)
                .show();
    }

    private void showInputDialog(String title, String message, String hint, InputCallback onInput) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(message);

        final EditText input = new EditText(this);
        input.setHint(hint);
        builder.setView(input);

        builder.setPositiveButton("OK", (dialog, which) -> {
            String inputText = input.getText().toString();
            onInput.onInput(inputText);
        });
        builder.setNegativeButton("Cancel", (dialog, which) -> dialog.cancel());

        builder.show();
    }

    @Override
    public void onDataReceived(@NonNull BluetoothDevice device, @NonNull Data data) {
        Log.d(TAG, "onDataReceived: CALLED");
        String text = data.getStringValue(0);
        if (text == null) {
            Log.d(TAG, "onDataReceived: Received null data");
            return;
        }

        Log.d(TAG, "onDataReceived raw chunk: " + text + " (length: " + text.length() + ")");
        // Use the shared message processor
        BleMessageProcessor.getInstance().processDataChunk(text);
    }
    private void processReceivedData() {
        while (true) {
            if (expectedDataSize == -1) {
                int newlineIndex = receivedDataBuffer.indexOf("\n");
                if (newlineIndex != -1) {
                    String header = receivedDataBuffer.substring(0, newlineIndex);
                    receivedDataBuffer.delete(0, newlineIndex + 1);

                    String[] headerParts = header.split(",");
                    if (headerParts.length == 3) {
                        try {
                            expectedDataSize = Integer.parseInt(headerParts[0].trim());
                            Log.d(TAG, "Parsed header. Total size: " + expectedDataSize + " bytes. Buffer now has " + receivedDataBuffer.length() + " bytes.");
                        } catch (NumberFormatException e) {
                            Log.e(TAG, "Could not parse header numbers: " + header + ". Clearing buffer.", e);
                            expectedDataSize = -1;
                            receivedDataBuffer.setLength(0);
                            break; // Malformed header, stop processing this buffer
                        }
                    } else {
                        Log.e(TAG, "Invalid header format: " + header + ". Expected 'total_size,num_chunks,chunk_size'. Clearing buffer.");
                        expectedDataSize = -1;
                        receivedDataBuffer.setLength(0);
                        break; // Invalid header, stop processing this buffer
                    }
                } else {
                    // No complete header yet, wait for more data
                    break;
                }
            }

            // If we have an expected size, try to complete the message
            if (expectedDataSize != -1) {
                if (receivedDataBuffer.length() >= expectedDataSize) {
                    Log.d(TAG, "Complete chunked message received. Buffer size: " + receivedDataBuffer.length() + ", Expected size: " + expectedDataSize);
                    String fullMessage = receivedDataBuffer.substring(0, expectedDataSize);
                    Log.d(TAG, "Processing full message: " + fullMessage);
                    receivedDataBuffer.delete(0, expectedDataSize);

                    processJson(fullMessage);

                    Log.d(TAG, "Message passed to processJson: " + fullMessage);

                    expectedDataSize = -1; // Reset for next message
                    // Continue loop to check if there's another message in the buffer
                } else {
                    // Not enough data for the full message yet, wait for more data
                    break;
                }
            } else {
                // No expected size and no header found, nothing more to process for now
                break;
            }
        }
    }

    private void processJson(String json) {
        Log.d(TAG, "processJson: Entry with JSON: " + json);
        try {
            JSONObject jsonResponse = new JSONObject(json);
            if (jsonResponse.has("passphrase")) {
                passphraseOutputEditText.setText(jsonResponse.getString("passphrase"));
            } else if (jsonResponse.has("status")) {
                showToast("Command Status: " + jsonResponse.getString("status"));
            } else {
                showToast("Received: " + json);
            }
        } catch (JSONException e) {
            Log.e(TAG, "Error parsing JSON response: " + json, e);
            showToast("Received non-JSON data or error: " + json);
        }
    }

    @Override
    public void onDataSent(@NonNull BluetoothDevice device, @NonNull Data data) {
        Log.d(TAG, "onDataSent: " + data.getStringValue(0));
    }

    // ConnectionObserver methods
    @Override
    public void onDeviceConnecting(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceConnecting: " + deviceName);
        updateUIForBleConnection(false);
    }

    @Override
    public void onDeviceConnected(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceConnected: " + deviceName);
        updateUIForBleConnection(true);
    }

    @Override
    public void onDeviceFailedToConnect(@NonNull BluetoothDevice device, int reason) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.e(TAG, "onDeviceFailedToConnect: " + deviceName + ", reason: " + reason);
        updateUIForBleConnection(false);
        showToast("Failed to connect to KeyPass device.");
    }

    @Override
    public void onDeviceReady(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceReady: " + deviceName);
        updateUIForBleConnection(true);
    }

    @Override
    public void onDeviceDisconnecting(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceDisconnecting: " + deviceName);
        updateUIForBleConnection(false);
    }

    @Override
    public void onDeviceDisconnected(@NonNull BluetoothDevice device, int reason) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceDisconnected: " + deviceName + ", reason: " + reason);
        updateUIForBleConnection(false);
        showToast("Disconnected from KeyPass device.");
    }

    public void onBondingRequired(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onBondingRequired: " + deviceName);
    }

    public void onBondingSucceeded(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onBondingSucceeded: " + deviceName);
    }

    public void onBondingFailed(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onBondingFailed: " + deviceName);
    }

    public void onBondNotSupported(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onBondNotSupported: " + deviceName);
    }

    private void updateUIForBleConnection(boolean isConnected) {
        backupPasswordsButton.setEnabled(isConnected);
        changeWifiPasswordButton.setEnabled(isConnected);
        resetPassphraseButton.setEnabled(isConnected);
        factoryResetButton.setEnabled(isConnected);
        showPassphraseButton.setEnabled(isConnected);
    }

    @Override
    protected void onResume() {
        super.onResume();
        // Re-register BLE callbacks and message handlers
        if (bleManager != null) {
            bleManager.setConnectionObserver(this);
            bleManager.setDataCallback(this);
            setupBleMessageHandlers();
            updateUIForBleConnection(bleManager.isDeviceConnected());
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        // Unregister BLE callbacks and clear message handlers
        if (bleManager != null) {
            bleManager.setConnectionObserver(null);
            bleManager.setDataCallback(null);
        }
        BleMessageProcessor.getInstance().clearHandlers();
    }
}
