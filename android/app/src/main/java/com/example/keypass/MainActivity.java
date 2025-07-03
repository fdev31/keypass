package com.example.keypass;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiNetworkSuggestion;
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
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "KeyPassWiFi";
    private static final int PERMISSIONS_REQUEST_CODE = 1;
    private static final String TARGET_URL = "http://4.3.2.1/";

    private static final String[] REQUIRED_PERMISSIONS = new String[]{
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_WIFI_STATE,
            Manifest.permission.CHANGE_WIFI_STATE,
            Manifest.permission.ACCESS_NETWORK_STATE
    };

    private WebView webView;
    private EditText ssidEditText;
    private EditText passwordEditText;
    private Button connectButton;
    private LinearLayout connectionForm;
    private ConnectivityManager connectivityManager;
    private WifiManager wifiManager;
    private ConnectivityManager.NetworkCallback networkCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        webView = findViewById(R.id.webView);
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

        ssidEditText = findViewById(R.id.ssidEditText);
        passwordEditText = findViewById(R.id.passwordEditText);
        connectButton = findViewById(R.id.connectButton);
        connectionForm = findViewById(R.id.connectionForm);

        connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        wifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);

        connectButton.setOnClickListener(v -> {
            if (!hasAllRequiredPermissions()) {
                ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, PERMISSIONS_REQUEST_CODE);
            } else {
                if (connectButton.getText().toString().equals("Load")) {
                    webView.loadUrl(TARGET_URL);
                }
                else {
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

    private void updateButtonState() {
        if (!hasAllRequiredPermissions()) {
            connectButton.setText("Grant Permissions");
            connectionForm.setVisibility(View.VISIBLE);
            return;
        }

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_WIFI_STATE) != PackageManager.PERMISSION_GRANTED) {
            connectButton.setText("Grant Permissions");
            connectionForm.setVisibility(View.VISIBLE);
            return;
        }
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        String currentSsid = (wifiInfo != null) ? wifiInfo.getSSID() : null;
        if (currentSsid != null) {
            currentSsid = currentSsid.replace("\"", "");
        }

        String targetSsid = ssidEditText.getText().toString();

        if (targetSsid.equals(currentSsid)) {
            connectButton.setText("Load");
            connectionForm.setVisibility(View.GONE);
        } else {
            connectButton.setText("Connect");
            connectionForm.setVisibility(View.VISIBLE);
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

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSIONS_REQUEST_CODE) {
            updateButtonState();
            if (!hasAllRequiredPermissions()) {
                Toast.makeText(getApplicationContext(), "All permissions are required to use this app.", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void connectToWifi() {
        String ssid = ssidEditText.getText().toString();
        String password = passwordEditText.getText().toString();

        if (ssid.isEmpty()) {
            Toast.makeText(getApplicationContext(), "Please enter an SSID.", Toast.LENGTH_SHORT).show();
            return;
        }
        if (password.length() < 8) {
            Toast.makeText(getApplicationContext(), "Password must be at least 8 characters.", Toast.LENGTH_SHORT).show();
            return;
        }

        // final WifiNetworkSuggestion suggestion = new WifiNetworkSuggestion.Builder()
        //         .setSsid(ssid)
        //         .setWpa2Passphrase(password)
        //         .build();
        //
        // final List<WifiNetworkSuggestion> suggestions = new ArrayList<>();
        // suggestions.add(suggestion);
        //
        // final int status = wifiManager.addNetworkSuggestions(suggestions);
        // if (status != WifiManager.STATUS_NETWORK_SUGGESTIONS_SUCCESS) {
        //     Log.w(TAG, "Could not add network suggestion. Status: " + status);
        //     Toast.makeText(getApplicationContext(), "Could not add network suggestion. Please try connecting manually.", Toast.LENGTH_LONG).show();
        // } else {
        //     Toast.makeText(getApplicationContext(), "Network suggested. Please connect to it now.", Toast.LENGTH_LONG).show();
        // }

        startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS));
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
}
