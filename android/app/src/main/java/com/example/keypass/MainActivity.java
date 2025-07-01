package com.example.keypass;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.wifi.WifiNetworkSpecifier;
import android.os.Bundle;
import android.util.Log;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "KeyPassWiFi";
    private static final int PERMISSIONS_REQUEST_CODE = 1;
    private static final String TARGET_SSID = "KeyPass";
    private static final String TARGET_URL = "http://4.3.2.1/";

    private static final String[] REQUIRED_PERMISSIONS = new String[]{
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_WIFI_STATE,
            Manifest.permission.CHANGE_WIFI_STATE
    };

    private WebView webView;
    private ConnectivityManager connectivityManager;
    private ConnectivityManager.NetworkCallback networkCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        webView = findViewById(R.id.webView);
        webView.setWebViewClient(new WebViewClient());

        connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);

        if (hasAllRequiredPermissions()) {
            startNetworkRequest();
        } else {
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, PERMISSIONS_REQUEST_CODE);
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
            if (hasAllRequiredPermissions()) {
                startNetworkRequest();
            } else {
                Toast.makeText(this, "All permissions are required to connect to KeyPass.", Toast.LENGTH_LONG).show();
                finish(); // Close the app if permissions are denied
            }
        }
    }
    private void startNetworkRequest() {
        try {
        Log.d(TAG, "Attempting to connect to " + TARGET_SSID);
        Toast.makeText(this, "Looking for " + TARGET_SSID + "...", Toast.LENGTH_SHORT).show();

        final WifiNetworkSpecifier specifier = new WifiNetworkSpecifier.Builder()
                .setSsid(TARGET_SSID)
                .build();

        final NetworkRequest request = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                .setNetworkSpecifier(specifier)
                .build();

        networkCallback = new ConnectivityManager.NetworkCallback() {
            @Override
            public void onAvailable(@NonNull Network network) {
                super.onAvailable(network);
                Log.d(TAG, "Network available: " + network);
                // This is the key part: bind the process to the new network
                connectivityManager.bindProcessToNetwork(network);
                runOnUiThread(() -> {
                    webView.loadUrl(TARGET_URL);
                    Toast.makeText(MainActivity.this, "Connected to " + TARGET_SSID, Toast.LENGTH_SHORT).show();
                });
                }

                @Override
                public void onLost(@NonNull Network network) {
                    super.onLost(network);
                    Log.d(TAG, "KeyPass network connection lost");

                    connectivityManager.bindProcessToNetwork(null);
                    runOnUiThread(() -> {
                        webView.loadUrl("about:blank");
                        Toast.makeText(MainActivity.this, "KeyPass connection lost", Toast.LENGTH_SHORT).show();
                    });
                }

                @Override
                public void onUnavailable() {
                    super.onUnavailable();
                    Log.w(TAG, "KeyPass network is unavailable");
                    runOnUiThread(() -> {
                        Toast.makeText(MainActivity.this, "KeyPass network not found or connection failed", Toast.LENGTH_LONG).show();
                    });
                }

                @Override
                public void onCapabilitiesChanged(@NonNull Network network, @NonNull NetworkCapabilities networkCapabilities) {
                    super.onCapabilitiesChanged(network, networkCapabilities);
                    Log.d(TAG, "Network capabilities changed: " + networkCapabilities.toString());
                }

                @Override
                public void onLinkPropertiesChanged(@NonNull Network network, @NonNull android.net.LinkProperties linkProperties) {
                    super.onLinkPropertiesChanged(network, linkProperties);
                    Log.d(TAG, "Link properties changed");
                }
            };
            Log.d(TAG, "NetworkCallback created");

            // Request the network - this should trigger system WiFi connection dialog
            if (!hasAllRequiredPermissions()) {
                Log.e(TAG, "Not all permissions granted!");
                Toast.makeText(this, "Missing required permissions", Toast.LENGTH_LONG).show();
                return;
            }

            Log.d(TAG, "Requesting KeyPass network...");
            connectivityManager.requestNetwork(request, networkCallback);

            Toast.makeText(this, "Looking for KeyPass network...", Toast.LENGTH_SHORT).show();
            Log.d(TAG, "Network request submitted successfully");

        } catch (Exception e) {
            Log.e(TAG, "Exception in startNetworkRequest: " + e.getMessage(), e);
            Toast.makeText(this, "Network request failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (connectivityManager != null && networkCallback != null) {
            connectivityManager.unregisterNetworkCallback(networkCallback);
            connectivityManager.bindProcessToNetwork(null);
        }
    }
}
