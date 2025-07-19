package com.example.keypass;

import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;

/**
 * Utility class for processing BLE messages across different activities
 */
public class BleMessageProcessor {
    private static final String TAG = "KeyPass::BleMessageProcessor";

    // Singleton instance
    private static BleMessageProcessor instance;

    // Buffer for incoming data
    private StringBuilder receivedDataBuffer = new StringBuilder();
    private int expectedDataSize = -1;

    // Handlers for different message types
    private Map<String, MessageHandler> jsonHandlers = new HashMap<>();
    private Map<String, TextHandler> textHandlers = new HashMap<>();

    // Default handler for unrecognized messages
    private MessageHandler defaultJsonHandler;
    private TextHandler defaultTextHandler;

    // Interfaces for message handlers
    public interface MessageHandler {
        void handleMessage(String key, Object value);
    }

    public interface TextHandler {
        void handleText(String text);
    }

    private BleMessageProcessor() {
        // Private constructor for singleton
    }

    public static synchronized BleMessageProcessor getInstance() {
        if (instance == null) {
            instance = new BleMessageProcessor();
        }
        return instance;
    }

    /**
     * Register a handler for JSON messages with a specific key
     */
    public void registerJsonHandler(String key, MessageHandler handler) {
        jsonHandlers.put(key, handler);
    }

    /**
     * Register a handler for text messages with a specific prefix
     */
    public void registerTextHandler(String prefix, TextHandler handler) {
        textHandlers.put(prefix, handler);
    }

    /**
     * Set a default handler for JSON messages
     */
    public void setDefaultJsonHandler(MessageHandler handler) {
        this.defaultJsonHandler = handler;
    }

    /**
     * Set a default handler for text messages
     */
    public void setDefaultTextHandler(TextHandler handler) {
        this.defaultTextHandler = handler;
    }

    /**
     * Process a chunk of data received from BLE
     */
    public void processDataChunk(String chunk) {
        if (chunk == null) {
            Log.d(TAG, "Received null data chunk");
            return;
        }

        Log.d(TAG, "Received data chunk: " + chunk + " (length: " + chunk.length() + ")");
        receivedDataBuffer.append(chunk);

        processReceivedData();
    }

    /**
     * Process the buffer to extract complete messages
     */
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

                    processFullMessage(fullMessage);

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

    /**
     * Process a complete message
     */
    private void processFullMessage(String message) {
        Log.d(TAG, "Processing full message");
        // Check if the message looks like JSON
        if (message.trim().startsWith("{") || message.trim().startsWith("[")) {
            // Try to process as JSON
            processJson(message);
        } else {
            // Assume it's plain text
            Log.d(TAG, "Message does not look like JSON, processing as plain text.");
            processText(message);
        }
    }

    /**
     * Process a JSON message
     */
    private void processJson(String json) {
        Log.d(TAG, "Processing JSON response");
        try {
            JSONObject jsonObject = new JSONObject(json);

            boolean handled = false;

            // Check for registered handlers
            for (Map.Entry<String, MessageHandler> entry : jsonHandlers.entrySet()) {
                String key = entry.getKey();
                if (jsonObject.has(key)) {
                    Object value = jsonObject.get(key);
                    entry.getValue().handleMessage(key, value);
                    handled = true;
                    break;
                }
            }

            // If no specific handler was found, use default
            if (!handled && defaultJsonHandler != null) {
                defaultJsonHandler.handleMessage(null, jsonObject);
            }

        } catch (JSONException e) {
            Log.e(TAG, "Error parsing JSON response: " + json, e);
        }
    }

    /**
     * Process a plain text message
     */
    private void processText(String text) {
        boolean handled = false;

        // Check for registered handlers
        for (Map.Entry<String, TextHandler> entry : textHandlers.entrySet()) {
            String prefix = entry.getKey();
            if (text.startsWith(prefix)) {
                entry.getValue().handleText(text);
                handled = true;
                break;
            }
        }

        // If no specific handler was found, use default
        if (!handled && defaultTextHandler != null) {
            defaultTextHandler.handleText(text);
        }
    }

    /**
     * Clear all registered handlers
     */
    public void clearHandlers() {
        jsonHandlers.clear();
        textHandlers.clear();
        defaultJsonHandler = null;
        defaultTextHandler = null;
    }

    /**
     * Reset the buffer and expected size
     */
    public void reset() {
        receivedDataBuffer.setLength(0);
        expectedDataSize = -1;
    }
}
