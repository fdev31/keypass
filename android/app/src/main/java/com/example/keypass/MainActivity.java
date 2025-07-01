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
    
    private WebView webView;
    private ConnectivityManager connectivityManager;
    private ConnectivityManager.NetworkCallback networkCallback;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        try {
            Log.d(TAG, "onCreate started");
            setContentView(R.layout.activity_main);
            Log.d(TAG, "setContentView completed");

            webView = findViewById(R.id.webView);
            if (webView == null) {
                Log.e(TAG, "WebView not found in layout!");
                return;
            }
            Log.d(TAG, "WebView found");
            
            webView.setWebViewClient(new WebViewClient());
            Log.d(TAG, "WebViewClient set");
            
            connectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
            if (connectivityManager == null) {
                Log.e(TAG, "ConnectivityManager is null!");
                return;
            }
            Log.d(TAG, "ConnectivityManager obtained");

            // Check and request permissions
            if (checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "Requesting location permission");
                requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, PERMISSIONS_REQUEST_CODE);
            } else {
                Log.d(TAG, "Permission already granted, starting network request");
                startNetworkRequest();
            }
            
            Log.d(TAG, "onCreate completed successfully");
        } catch (Exception e) {
            Log.e(TAG, "Exception in onCreate: " + e.getMessage(), e);
            Toast.makeText(this, "Error: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    private boolean hasAllRequiredPermissions() {
        return checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED &&
               checkSelfPermission(Manifest.permission.ACCESS_WIFI_STATE) == PackageManager.PERMISSION_GRANTED &&
               checkSelfPermission(Manifest.permission.CHANGE_WIFI_STATE) == PackageManager.PERMISSION_GRANTED;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == PERMISSIONS_REQUEST_CODE) {
            boolean allGranted = grantResults.length > 0;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }
            if (allGranted) {
                Log.d(TAG, "All permissions granted after request");
                startNetworkRequest();
            } else {
                Log.e(TAG, "Not all permissions granted");
                Toast.makeText(this, "All permissions required for WiFi scanning", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void startNetworkRequest() {
        try {
            Log.d(TAG, "Starting network request for KeyPass WiFi");
            
            // Create the WiFi network specifier
            // This will show system WiFi dialog if password is needed
            final WifiNetworkSpecifier specifier = new WifiNetworkSpecifier.Builder()
                    .setSsid(TARGET_SSID)
                    .build();
            Log.d(TAG, "WifiNetworkSpecifier created");

            // Create network request
            final NetworkRequest request = new NetworkRequest.Builder()
                    .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                    .removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET) // Important: removes internet requirement
                    .setNetworkSpecifier(specifier)
                    .build();
            Log.d(TAG, "NetworkRequest created");

            // Set up network callback
            networkCallback = new ConnectivityManager.NetworkCallback() {
                @Override
                public void onAvailable(@NonNull Network network) {
                    super.onAvailable(network);
                    Log.d(TAG, "KeyPass network is now available");
                    
                    // Bind this process to use the KeyPass network
                    connectivityManager.bindProcessToNetwork(network);
                    
                    runOnUiThread(() -> {
                        webView.loadUrl(TARGET_URL);
                        Toast.makeText(MainActivity.this, "Connected to KeyPass!", Toast.LENGTH_SHORT).show();
                        Log.d(TAG, "Loading URL: " + TARGET_URL);
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
