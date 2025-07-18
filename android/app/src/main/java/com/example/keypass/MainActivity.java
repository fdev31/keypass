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
import android.os.Bundle;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.WebChromeClient;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Switch;
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
import java.util.ArrayList;
import java.util.List;
import no.nordicsemi.android.ble.data.Data;
import no.nordicsemi.android.ble.observer.ConnectionObserver;
import no.nordicsemi.android.support.v18.scanner.BluetoothLeScannerCompat;
import no.nordicsemi.android.support.v18.scanner.ScanCallback;
import no.nordicsemi.android.support.v18.scanner.ScanResult;
import no.nordicsemi.android.support.v18.scanner.ScanSettings;

public class MainActivity extends AppCompatActivity implements KeyPassBleManager.DataCallback, ConnectionObserver {

    private static final String TAG = "KeyPass";
    private static final int PERMISSIONS_REQUEST_CODE = 1;
    private static final String TARGET_URL = "http://4.3.2.1/";
    private static final String DEVICE_NAME = "KeyPass"; // The name of your BLE device
    private static final String PREFS_NAME = "KeyPassPrefs";
    private static final String PREF_SSID = "ssid";

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

    // Common UI
    private Switch modeSwitch;

    // BLE
    private KeyPassBleManager bleManager;
    private BluetoothLeScannerCompat scanner;
    private boolean isScanning = false;
    private StringBuilder receivedDataBuffer = new StringBuilder();


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

        // Setup WiFi
        setupWifi();

        // Setup BLE
        setupBle();

        // Mode switch listener
        modeSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
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
        wifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);

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
                    // TODO: implement edit/delete
                });
        passwordRecyclerView.setAdapter(passwordAdapter);

        addButton.setOnClickListener(v -> {
            // TODO: implement add password dialog
        });

        settingsButton.setOnClickListener(v -> {
            startActivity(new Intent(this, SettingsActivity.class));
        });
    }

    private void startScan() {
        if (isScanning) {
            return;
        }
        Log.d(TAG, "Starting BLE scan");
        isScanning = true;
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        scanner.startScan(new ArrayList<>(), new ScanSettings.Builder().build(), scanCallback);
    }

    private void stopScan() {
        if (isScanning) {
            Log.d(TAG, "Stopping BLE scan");
            isScanning = false;
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
            Log.d(TAG, "Scan result: " + result.getDevice().getName());
            if (result.getDevice().getName() != null && result.getDevice().getName().equals(DEVICE_NAME)) {
                Log.d(TAG, "Found target device");
                stopScan();
                bleManager.connect(result.getDevice()).enqueue();
            }
        }

        @Override
        public void onBatchScanResults(@NonNull List<ScanResult> results) {
            // Do nothing
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.e(TAG, "Scan failed with error code: " + errorCode);
        }
    };

    @Override
    public void onDataReceived(@NonNull BluetoothDevice device, @NonNull Data data) {
        Log.d(TAG, "onDataReceived: " + data.getStringValue(0));
        receivedDataBuffer.append(data.getStringValue(0));
        String bufferString = receivedDataBuffer.toString();
        if (bufferString.contains("\n")) {
            String[] parts = bufferString.split("\n");
            for (int i = 0; i < parts.length - 1; i++) {
                processJson(parts[i]);
            }
            receivedDataBuffer = new StringBuilder();
            if (!bufferString.endsWith("\n")) {
                receivedDataBuffer.append(parts[parts.length - 1]);
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
                    passwordList.add(new Password(password.getInt("id"), password.getString("name")));
                }
                runOnUiThread(() -> passwordAdapter.notifyDataSetChanged());
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
                connectivityManager.bindProcessToNetwork(network);
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
                connectivityManager.bindProcessToNetwork(null);
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
        Log.d(TAG, "onDeviceConnecting: " + device.getName());
    }

    @Override
    public void onDeviceConnected(@NonNull BluetoothDevice device) {
        Log.d(TAG, "onDeviceConnected: " + device.getName());
    }

    @Override
    public void onDeviceFailedToConnect(@NonNull BluetoothDevice device, int reason) {
        Log.e(TAG, "onDeviceFailedToConnect: " + device.getName() + ", reason: " + reason);
    }

    @Override
    public void onDeviceReady(@NonNull BluetoothDevice device) {
        Log.d(TAG, "onDeviceReady: " + device.getName());
        bleManager.send("{\"cmd\":\"list\"}");
    }

    @Override
    public void onDeviceDisconnecting(@NonNull BluetoothDevice device) {
        Log.d(TAG, "onDeviceDisconnecting: " + device.getName());
    }

    @Override
    public void onDeviceDisconnected(@NonNull BluetoothDevice device, int reason) {
        Log.d(TAG, "onDeviceDisconnected: " + device.getName() + ", reason: " + reason);
    }

    public void onBondingRequired(@NonNull BluetoothDevice device) {
        Log.d(TAG, "onBondingRequired: " + device.getName());
    }

    public void onBondingSucceeded(@NonNull BluetoothDevice device) {
        Log.d(TAG, "onBondingSucceeded: " + device.getName());
    }

    public void onBondingFailed(@NonNull BluetoothDevice device) {
        Log.d(TAG, "onBondingFailed: " + device.getName());
    }

    public void onBondNotSupported(@NonNull BluetoothDevice device) {
        Log.d(TAG, "onBondNotSupported: " + device.getName());
    }
}

class Password {
    private int id;
    private String name;

    public Password(int id, String name) {
        this.id = id;
        this.name = name;
    }

    public int getId() {
        return id;
    }

    public String getName() {
        return name;
    }
}
