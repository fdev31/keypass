package com.example.keypass;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import java.util.UUID;

import no.nordicsemi.android.ble.BleManager;
import no.nordicsemi.android.ble.data.Data;

public class KeyPassBleManager extends BleManager {
    private static final String TAG = "KeyPassBleManager";

    public final static UUID NORDIC_UART_SERVICE_UUID = UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
    private final static UUID UART_RX_CHARACTERISTIC_UUID = UUID.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
    private final static UUID UART_TX_CHARACTERISTIC_UUID = UUID.fromString("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

    private BluetoothGattCharacteristic rxCharacteristic, txCharacteristic;
    private Handler handler;

    public interface DataCallback {
        void onDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data);
        void onDataSent(@NonNull final BluetoothDevice device, @NonNull final Data data);
    }

    private DataCallback dataCallback;

    public void setDataCallback(DataCallback callback) {
        this.dataCallback = callback;
    }

    public KeyPassBleManager(@NonNull final Context context) {
        super(context);
        handler = new Handler(Looper.getMainLooper());
    }

    @NonNull
    @Override
    protected BleManagerGattCallback getGattCallback() {
        return new KeyPassBleManagerGattCallback();
    }

    private class KeyPassBleManagerGattCallback extends BleManagerGattCallback {
        @Override
        protected void initialize() {
            Log.d(TAG, "Initializing BLE manager");
            setNotificationCallback(txCharacteristic).with((device, data) -> {
                if (dataCallback != null) {
                    handler.post(() -> dataCallback.onDataReceived(device, data));
                }
            });
            enableNotifications(txCharacteristic).enqueue();
            Log.d(TAG, "Notifications enabled");
        }

        @Override
        public boolean isRequiredServiceSupported(@NonNull final BluetoothGatt gatt) {
            final BluetoothGattService service = gatt.getService(NORDIC_UART_SERVICE_UUID);
            if (service != null) {
                Log.d(TAG, "Nordic UART service found");
                rxCharacteristic = service.getCharacteristic(UART_RX_CHARACTERISTIC_UUID);
                txCharacteristic = service.getCharacteristic(UART_TX_CHARACTERISTIC_UUID);
            } else {
                Log.e(TAG, "Nordic UART service not found");
            }
            boolean supported = rxCharacteristic != null && txCharacteristic != null;
            Log.d(TAG, "Service supported: " + supported);
            return supported;
        }

        @Override
        protected void onServicesInvalidated() {
            Log.d(TAG, "Services invalidated");
            rxCharacteristic = null;
            txCharacteristic = null;
        }
    }

    public void send(final String text) {
        if (rxCharacteristic != null) {
            Log.d(TAG, "Sending: " + text);
            writeCharacteristic(rxCharacteristic, Data.from(text))
                    .with((device, data) -> {
                        if (dataCallback != null) {
                            handler.post(() -> dataCallback.onDataSent(device, data));
                        }
                    })
                    .enqueue();
        } else {
            Log.e(TAG, "Cannot send data, RX characteristic is null");
        }
    }

    @Override
    protected boolean shouldAutoConnect() {
        return true;
    }

    public boolean isDeviceConnected() {
        return super.isConnected();
    }
}