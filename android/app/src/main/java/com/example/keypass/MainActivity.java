package com.example.keypass;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.view.LayoutInflater;
import android.webkit.ConsoleMessage;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.List;
import no.nordicsemi.android.ble.data.Data;
import no.nordicsemi.android.ble.observer.ConnectionObserver;
import no.nordicsemi.android.support.v18.scanner.BluetoothLeScannerCompat;
import no.nordicsemi.android.support.v18.scanner.ScanCallback;
import no.nordicsemi.android.support.v18.scanner.ScanFilter;
import no.nordicsemi.android.support.v18.scanner.ScanResult;
import no.nordicsemi.android.support.v18.scanner.ScanSettings;

public class MainActivity extends AppCompatActivity implements KeyPassBleManager.DataCallback, ConnectionObserver {

    private static final String TAG = "KeyPass";
    private static final int PERMISSIONS_REQUEST_CODE = 1;
    private static final String TARGET_URL = "http://4.3.2.1/";
    private static final String DEVICE_NAME = "KeyPass"; // The name of your BLE device
    private static final String PREFS_NAME = "KeyPassPrefs";
    private static final String PREF_SSID = "ssid";
    private static final String PREF_MODE = "mode"; // true for BLE, false for WiFi
    private static final String PREF_HIDE_PASSWORDS = "hide_passwords";

    private static final String[] REQUIRED_PERMISSIONS = new String[]{
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_WIFI_STATE,
            Manifest.permission.CHANGE_WIFI_STATE,
            Manifest.permission.ACCESS_NETWORK_STATE,
            Manifest.permission.BLUETOOTH_SCAN,
            Manifest.permission.BLUETOOTH_CONNECT
    };

    // WiFi UI
    private LinearLayout wifiLayout;
    private WebView webView;
    private EditText ssidEditText;
    private Button connectButton;
    private LinearLayout connectionForm;
    private ConnectivityManager connectivityManager;
    private WifiManager wifiManager;
    private ConnectivityManager.NetworkCallback networkCallback;

    // BLE UI
    private LinearLayout bleLayout;
    private RecyclerView passwordRecyclerView;
    private Button addButton, settingsButton;
    private PasswordAdapter passwordAdapter;
    private List<Password> passwordList = new ArrayList<>();
    private TextView bleStatusTextView;

    // Common UI
    private Switch modeSwitch;

    // BLE
    private KeyPassBleManager bleManager;
    private BluetoothLeScannerCompat scanner;
    private boolean isScanning = false;
    private StringBuilder receivedDataBuffer = new StringBuilder();
    private int expectedDataSize = -1;
    private EditText currentPasswordEditTextForFetch;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Common UI
        modeSwitch = findViewById(R.id.modeSwitch);

        // WiFi UI
        wifiLayout = findViewById(R.id.wifiLayout);
        webView = findViewById(R.id.webView);
        ssidEditText = findViewById(R.id.ssidEditText);
        connectButton = findViewById(R.id.connectButton);
        connectionForm = findViewById(R.id.connectionForm);

        // BLE UI
        bleLayout = findViewById(R.id.bleLayout);
        passwordRecyclerView = findViewById(R.id.passwordRecyclerView);
        addButton = findViewById(R.id.addButton);
        settingsButton = findViewById(R.id.settingsButton);
        bleStatusTextView = findViewById(R.id.bleStatusTextView);

        // Setup WiFi
        setupWifi();

        // Setup BLE
        setupBle();

        // Load saved mode
        SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
        boolean savedModeIsBle = prefs.getBoolean(PREF_MODE, false); // Default to WiFi

        modeSwitch.setChecked(savedModeIsBle);
        if (savedModeIsBle) {
            wifiLayout.setVisibility(View.GONE);
            bleLayout.setVisibility(View.VISIBLE);
            if (!bleManager.isDeviceConnected()) {
                startScan();
            }
        } else {
            wifiLayout.setVisibility(View.VISIBLE);
            bleLayout.setVisibility(View.GONE);
        }

        // Mode switch listener
        modeSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            SharedPreferences.Editor editor = getSharedPreferences(PREFS_NAME, MODE_PRIVATE).edit();
            editor.putBoolean(PREF_MODE, isChecked);
            editor.apply();

            if (isChecked) { // BLE mode
                Log.d(TAG, "Switching to BLE mode");
                wifiLayout.setVisibility(View.GONE);
                bleLayout.setVisibility(View.VISIBLE);
                if (bleManager.isDeviceConnected()) {
                    Log.d(TAG, "Device already connected");
                } else {
                    startScan();
                }
            } else { // WiFi mode
                Log.d(TAG, "Switching to WiFi mode");
                wifiLayout.setVisibility(View.VISIBLE);
                bleLayout.setVisibility(View.GONE);
                stopScan();
            }
        });

        if (!hasAllRequiredPermissions()) {
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, PERMISSIONS_REQUEST_CODE);
        }
    }

    private void setupWifi() {
        webView.getSettings().setJavaScriptEnabled(true);
        webView.getSettings().setDomStorageEnabled(true);
        webView.setWebViewClient(new WebViewClient());
        webView.setWebChromeClient(new WebChromeClient() {
            @Override
            public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
                Log.d("WebViewConsole", consoleMessage.message() + " -- From line "
                        + consoleMessage.lineNumber() + " of "
                        + consoleMessage.sourceId());
                return true;
            }
        });

        connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);

        SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
        String lastSsid = prefs.getString(PREF_SSID, "KeyPass");
        ssidEditText.setText(lastSsid);

        connectButton.setOnClickListener(v -> {
            if (!hasAllRequiredPermissions()) {
                ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, PERMISSIONS_REQUEST_CODE);
            } else {
                if (connectButton.getText().toString().equals("Load")) {
                    webView.loadUrl(TARGET_URL);
                    if (getSupportActionBar() != null) {
                        getSupportActionBar().hide();
                    }
                } else {
                    connectToWifi();
                }
            }
        });

        TextWatcher textWatcher = new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                if (hasAllRequiredPermissions()) {
                    updateButtonState();
                }
            }
            @Override
            public void afterTextChanged(Editable s) {}
        };
        ssidEditText.addTextChangedListener(textWatcher);
    }

    private void setupBle() {
        bleManager = new KeyPassBleManager(this);
        bleManager.setConnectionObserver(this);
        bleManager.setDataCallback(this);
        scanner = BluetoothLeScannerCompat.getScanner();

        passwordRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        passwordAdapter = new PasswordAdapter(passwordList,
                password -> { // on item click
                    String cmd = String.format("{\"cmd\":\"typePass\",\"id\":%d}", password.getId());
                    bleManager.send(cmd);
                },
                password -> { // on item long click
                    showEditPasswordDialog(password);
                });
        passwordRecyclerView.setAdapter(passwordAdapter);

        addButton.setOnClickListener(v -> {
            showAddPasswordDialog();
        });

        settingsButton.setOnClickListener(v -> {
            startActivity(new Intent(this, SettingsActivity.class));
        });
    }

    private void showAddPasswordDialog() {
        showEditPasswordDialog(null); // Call with null for add mode
    }

    private void showEditPasswordDialog(Password passwordToEdit) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        LayoutInflater inflater = this.getLayoutInflater();
        View dialogView = inflater.inflate(R.layout.dialog_add_password, null);
        builder.setView(dialogView);

        EditText nameEditText = dialogView.findViewById(R.id.nameEditText);
        EditText passwordEditText = dialogView.findViewById(R.id.passwordEditText);
        Button generatePasswordButton = dialogView.findViewById(R.id.generatePasswordButton);
        ImageButton togglePasswordVisibilityButton = dialogView.findViewById(R.id.togglePasswordVisibilityButton);
        RadioGroup layoutRadioGroup = dialogView.findViewById(R.id.layoutRadioGroup);
        RadioButton layoutBitlocker = dialogView.findViewById(R.id.layoutBitlocker);
        RadioButton layoutFR = dialogView.findViewById(R.id.layoutFR);
        RadioButton layoutUS = dialogView.findViewById(R.id.layoutUS);
        Button savePasswordButton = dialogView.findViewById(R.id.savePasswordButton);

        AlertDialog dialog = builder.create();

        // Get hide passwords preference
        SharedPreferences prefs = getSharedPreferences(PREFS_NAME, MODE_PRIVATE);
        boolean hidePasswords = prefs.getBoolean(PREF_HIDE_PASSWORDS, false); // Default to false

        // Set initial password visibility
        if (hidePasswords) {
            passwordEditText.setInputType(android.text.InputType.TYPE_CLASS_TEXT | android.text.InputType.TYPE_TEXT_VARIATION_PASSWORD);
            togglePasswordVisibilityButton.setImageResource(android.R.drawable.ic_menu_view); // Eye icon
        } else {
            passwordEditText.setInputType(android.text.InputType.TYPE_CLASS_TEXT | android.text.InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
            togglePasswordVisibilityButton.setImageResource(android.R.drawable.ic_menu_close_clear_cancel); // Slash eye icon
        }

        // Pre-fill if in edit mode
        if (passwordToEdit != null) {
            nameEditText.setText(passwordToEdit.getName());
            Log.d(TAG, "Editing password: " + passwordToEdit.getName() + ", layout: " + passwordToEdit.getLayout());
            // Password field is not pre-filled for security reasons
            // Set appropriate radio button for layout
            switch (passwordToEdit.getLayout()) {
                case -1:
                    layoutBitlocker.setChecked(true);
                    break;
                case 0:
                    layoutFR.setChecked(true);
                    break;
                case 1:
                    layoutUS.setChecked(true);
                    break;
            }
            dialog.setTitle("Edit Password");
        } else {
            dialog.setTitle("Add New Password");
        }

        togglePasswordVisibilityButton.setOnClickListener(v -> {
            if (passwordEditText.getInputType() == (android.text.InputType.TYPE_CLASS_TEXT | android.text.InputType.TYPE_TEXT_VARIATION_PASSWORD)) {
                passwordEditText.setInputType(android.text.InputType.TYPE_CLASS_TEXT | android.text.InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD);
                togglePasswordVisibilityButton.setImageResource(android.R.drawable.ic_menu_close_clear_cancel); // Slash eye icon

                // If password field is empty and in edit mode, fetch password
                if (passwordToEdit != null && passwordEditText.getText().toString().isEmpty()) {
                    currentPasswordEditTextForFetch = passwordEditText;
                    String cmd = String.format("{\"cmd\":\"fetchPass\",\"id\":%d}", passwordToEdit.getId());
                    bleManager.send(cmd);
                }
            } else {
                passwordEditText.setInputType(android.text.InputType.TYPE_CLASS_TEXT | android.text.InputType.TYPE_TEXT_VARIATION_PASSWORD);
                togglePasswordVisibilityButton.setImageResource(android.R.drawable.ic_menu_view); // Eye icon
            }
            // Move cursor to the end of the text
            passwordEditText.setSelection(passwordEditText.getText().length());
        });

        generatePasswordButton.setOnClickListener(v -> {
            // Prompt for length if password field is empty
            if (passwordEditText.getText().toString().isEmpty()) {
                AlertDialog.Builder lengthBuilder = new AlertDialog.Builder(this);
                lengthBuilder.setTitle("Generate Password");
                lengthBuilder.setMessage("Enter password length:");
                final EditText lengthInput = new EditText(this);
                lengthInput.setInputType(android.text.InputType.TYPE_CLASS_NUMBER);
                lengthBuilder.setView(lengthInput);
                lengthBuilder.setPositiveButton("Generate", (dialogInterface, i) -> {
                    try {
                        int length = Integer.parseInt(lengthInput.getText().toString());
                        if (length > 0) {
                            passwordEditText.setText(generateRandomPassword(length));
                        } else {
                            Toast.makeText(this, "Length must be greater than 0", Toast.LENGTH_SHORT).show();
                        }
                    } catch (NumberFormatException e) {
                        Toast.makeText(this, "Invalid length", Toast.LENGTH_SHORT).show();
                    }
                });
                lengthBuilder.setNegativeButton("Cancel", (dialogInterface, i) -> dialogInterface.cancel());
                lengthBuilder.show();
            } else {
                // If password field has text, just generate a new one with default length (e.g., 16)
                passwordEditText.setText(generateRandomPassword(16));
            }
        });

        savePasswordButton.setOnClickListener(v -> {
            String name = nameEditText.getText().toString();
            String password = passwordEditText.getText().toString();
            int layout = -1; // Default to Bitlocker
            int checkedRadioButtonId = layoutRadioGroup.getCheckedRadioButtonId();
            if (checkedRadioButtonId == R.id.layoutFR) {
                layout = 0;
            } else if (checkedRadioButtonId == R.id.layoutUS) {
                layout = 1;
            }

            if (name.isEmpty() || password.isEmpty()) {
                Toast.makeText(this, "Name and Password cannot be empty", Toast.LENGTH_SHORT).show();
                return;
            }

            int id = (passwordToEdit != null) ? passwordToEdit.getId() : passwordList.size(); // Use existing ID or list size for new

            String cmd = String.format("{\"cmd\":\"editPass\",\"id\":%d,\"name\":\"%s\",\"password\":\"%s\",\"layout\":%d}", id, name, password, layout);
            bleManager.send(cmd);
            dialog.dismiss();
        });

        dialog.show();
    }

    private String generateRandomPassword(int length) {
        String chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+";
        SecureRandom random = new SecureRandom();
        StringBuilder sb = new StringBuilder(length);
        for (int i = 0; i < length; i++) {
            sb.append(chars.charAt(random.nextInt(chars.length())));
        }
        return sb.toString();
    }

    private void startScan() {
        if (isScanning) {
            return;
        }
        Log.d(TAG, "Starting BLE scan");
        isScanning = true;
        bleStatusTextView.setText("BLE Status: Scanning...");
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        List<ScanFilter> filters = new ArrayList<>();
        filters.add(new ScanFilter.Builder().setServiceUuid(new ParcelUuid(KeyPassBleManager.NORDIC_UART_SERVICE_UUID)).build());
        scanner.startScan(filters, new ScanSettings.Builder().build(), scanCallback);
    }

    private void stopScan() {
        if (isScanning) {
            Log.d(TAG, "Stopping BLE scan");
            isScanning = false;
            bleStatusTextView.setText("BLE Status: Scan Stopped");
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            scanner.stopScan(scanCallback);
        }
    }

    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, @NonNull ScanResult result) {
            if (ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            Log.d(TAG, "Found target device with UUID: " + result.getDevice().getName());
            stopScan();
            bleManager.connect(result.getDevice()).enqueue();
        }

        @Override
        public void onBatchScanResults(@NonNull List<ScanResult> results) {
            // Do nothing
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.e(TAG, "Scan failed with error code: " + errorCode);
            bleStatusTextView.setText("BLE Status: Scan Failed (" + errorCode + ")");
        }
    };

    @Override
    public void onDataReceived(@NonNull BluetoothDevice device, @NonNull Data data) {
        String text = data.getStringValue(0);
        if (text == null) return;

        Log.d(TAG, "onDataReceived chunk: " + text);
        receivedDataBuffer.append(text);

        processReceivedData();
    }

    private void processReceivedData() {
        if (expectedDataSize == -1) {
            int newlineIndex = receivedDataBuffer.indexOf("\n");
            if (newlineIndex != -1) {
                String header = receivedDataBuffer.substring(0, newlineIndex);
                receivedDataBuffer.delete(0, newlineIndex + 1);

                String[] headerParts = header.split(",");
                if (headerParts.length == 3) {
                    try {
                        expectedDataSize = Integer.parseInt(headerParts[0].trim());
                        // int numChunks = Integer.parseInt(headerParts[1].trim()); // Not currently used
                        // int chunkSize = Integer.parseInt(headerParts[2].trim()); // Not currently used
                        Log.d(TAG, "Parsed header. Total size: " + expectedDataSize + " bytes. Buffer now has " + receivedDataBuffer.length() + " bytes.");
                    } catch (NumberFormatException e) {
                        Log.e(TAG, "Could not parse header numbers: " + header + ". Clearing buffer.", e);
                        expectedDataSize = -1;
                        receivedDataBuffer.setLength(0); // Clear buffer to avoid processing bad data
                    }
                } else {
                    Log.e(TAG, "Invalid header format: " + header + ". Expected 'total_size,num_chunks,chunk_size'. Clearing buffer.");
                    expectedDataSize = -1;
                    receivedDataBuffer.setLength(0); // Clear buffer if header is not in expected format
                }
            }
        } else {
            checkBufferForCompleteMessage();
        }
    }

    private void checkBufferForCompleteMessage() {
        if (expectedDataSize > 0 && receivedDataBuffer.length() >= expectedDataSize) {
            Log.d(TAG, "Complete chunked message received. Buffer size: " + receivedDataBuffer.length() + ", Expected size: " + expectedDataSize);
            String fullMessage = receivedDataBuffer.substring(0, expectedDataSize);
            
            receivedDataBuffer.delete(0, expectedDataSize);

            processJson(fullMessage);

            expectedDataSize = -1;
            
            if (receivedDataBuffer.length() > 0) {
                Log.d(TAG, "More data in buffer, processing again.");
                processReceivedData();
            }
        }
    }

    @Override
    public void onDataSent(@NonNull BluetoothDevice device, @NonNull Data data) {
        Log.d(TAG, "onDataSent: " + data.getStringValue(0));
    }

    private void processJson(String json) {
        Log.d(TAG, "Processing JSON: " + json);
        try {
            JSONObject jsonObject = new JSONObject(json);
            if (jsonObject.has("passwords")) {
                JSONArray passwords = jsonObject.getJSONArray("passwords");
                passwordList.clear();
                for (int i = 0; i < passwords.length(); i++) {
                    JSONObject password = passwords.getJSONObject(i);
                    int uid = password.getInt("uid");
                    String name = password.getString("name");
                    int layout = password.optInt("layout", -1); // Default to -1 if not present
                    passwordList.add(new Password(uid, name, layout));
                }
                runOnUiThread(() -> passwordAdapter.notifyDataSetChanged());
            } else if (jsonObject.has("s") && jsonObject.has("m")) {
                int status = jsonObject.getInt("s");
                String message = jsonObject.getString("m");
                if (currentPasswordEditTextForFetch != null && status == 200) {
                    runOnUiThread(() -> {
                        currentPasswordEditTextForFetch.setText(message);
                        currentPasswordEditTextForFetch = null; // Clear the reference
                    });
                } else {
                    Log.d(TAG, "Received status/message: " + status + ": " + message);
                }
            }
        } catch (JSONException e) {
            Log.e(TAG, "Error parsing JSON", e);
        }
    }

    private boolean hasAllRequiredPermissions() {
        for (String permission : REQUIRED_PERMISSIONS) {
            if (ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    private void updateButtonState() {
        if (hasAllRequiredPermissions()) {
            WifiInfo wifiInfo = wifiManager.getConnectionInfo();
            String currentSsid = wifiInfo != null ? wifiInfo.getSSID().replace("\"", "") : null;
            String targetSsid = ssidEditText.getText().toString();

            if (targetSsid.equals(currentSsid)) {
                connectButton.setText("Load");
                connectionForm.setVisibility(View.GONE);
            } else {
                connectButton.setText("Connect");
                connectionForm.setVisibility(View.VISIBLE);
            }
        }
    }

    private void connectToWifi() {
        String ssid = ssidEditText.getText().toString();
        SharedPreferences.Editor editor = getSharedPreferences(PREFS_NAME, MODE_PRIVATE).edit();
        editor.putString(PREF_SSID, ssid);
        editor.apply();
        startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS));
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateButtonState();
        registerNetworkCallback();
    }

    @Override
    protected void onPause() {
        super.onPause();
        unregisterNetworkCallback();
    }

    private void registerNetworkCallback() {
        final NetworkRequest request = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                .build();

        networkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(@NonNull Network network) {
                super.onAvailable(network);
                Log.d(TAG, "Found a Wi-Fi network without internet, binding to it.");
                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
                    connectivityManager.bindProcessToNetwork(network);
                }
                runOnUiThread(() -> {
                    Toast.makeText(getApplicationContext(), "KeyPass network connected", Toast.LENGTH_SHORT).show();
                    updateButtonState();
                    webView.loadUrl(TARGET_URL);
                });
                if (getSupportActionBar() != null) {
                    getSupportActionBar().hide();
                }
            }

            @Override
            public void onLost(@NonNull Network network) {
                super.onLost(network);
                Log.d(TAG, "Lost connection to the network.");
                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
                    connectivityManager.bindProcessToNetwork(null);
                }
                runOnUiThread(() -> {
                    Toast.makeText(getApplicationContext(), "KeyPass network disconnected", Toast.LENGTH_SHORT).show();
                    updateButtonState();
                });
            }
        };

        connectivityManager.registerNetworkCallback(request, networkCallback);
    }

    private void unregisterNetworkCallback() {
        if (networkCallback != null) {
            try {
                connectivityManager.unregisterNetworkCallback(networkCallback);
            } catch (IllegalArgumentException e) {
                Log.w(TAG, "Network callback not registered.");
            }
            networkCallback = null;
        }
    }

    @Override
    public void onDeviceConnecting(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceConnecting: " + deviceName);
        bleStatusTextView.setText("BLE Status: Connecting to " + deviceName + "...");
    }

    @Override
    public void onDeviceConnected(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceConnected: " + deviceName);
        bleStatusTextView.setText("BLE Status: Connected to " + deviceName);
    }

    @Override
    public void onDeviceFailedToConnect(@NonNull BluetoothDevice device, int reason) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.e(TAG, "onDeviceFailedToConnect: " + deviceName + ", reason: " + reason);
        bleStatusTextView.setText("BLE Status: Failed to connect (" + reason + ")");
    }

    @Override
    public void onDeviceReady(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceReady: " + deviceName);
        bleStatusTextView.setText("BLE Status: Ready");
        bleManager.send("{\"cmd\":\"list\"}");
    }

    @Override
    public void onDeviceDisconnecting(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceDisconnecting: " + deviceName);
        bleStatusTextView.setText("BLE Status: Disconnecting...");
    }

    @Override
    public void onDeviceDisconnected(@NonNull BluetoothDevice device, int reason) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onDeviceDisconnected: " + deviceName + ", reason: " + reason);
        bleStatusTextView.setText("BLE Status: Disconnected");
    }

    public void onBondingRequired(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onBondingRequired: " + deviceName);
        bleStatusTextView.setText("BLE Status: Bonding Required");
    }

    public void onBondingSucceeded(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onBondingSucceeded: " + deviceName);
        bleStatusTextView.setText("BLE Status: Bonding Succeeded");
    }

    public void onBondingFailed(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onBondingFailed: " + deviceName);
        bleStatusTextView.setText("BLE Status: Bonding Failed");
    }

    public void onBondNotSupported(@NonNull BluetoothDevice device) {
        String deviceName = (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) ? "Unknown Device" : device.getName();
        Log.d(TAG, "onBondNotSupported: " + deviceName);
        bleStatusTextView.setText("BLE Status: Bond Not Supported");
    }
}

class Password {
    private int id;
    private String name;
    private int layout;

    public Password(int id, String name) {
        this.id = id;
        this.name = name;
        this.layout = -1; // Default to Bitlocker
    }

    public Password(int id, String name, int layout) {
        this.id = id;
        this.name = name;
        this.layout = layout;
    }

    public int getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public int getLayout() {
        return layout;
    }
}