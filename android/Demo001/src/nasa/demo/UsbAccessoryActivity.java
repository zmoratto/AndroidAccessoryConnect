package nasa.demo;

import java.io.ByteArrayInputStream;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.Thread;
import java.lang.String;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import com.android.future.usb.UsbAccessory;
import com.android.future.usb.UsbManager;

import nasa.demo.proto.SourceProto.OffboardData;

public class UsbAccessoryActivity extends Activity implements Runnable
{
    static final String TAG = "Demo001";
    private static final String ACTION_USB_PERMISSION = "nasa.demo.Demo001.action.USB_PERMISSION";

    private UsbManager mUsbManager;
    private PendingIntent mPermissionIntent;
    private boolean mPermissionRequestPending;

    UsbAccessory mAccessory;
    ParcelFileDescriptor mFileDescriptor;
    FileInputStream mInputStream;
    FileOutputStream mOutputStream;

    // Special Intent receiver to catch the event an Accessory
    // disconnects or when we receive an intent that says the user has
    // granted permission for us to use the accessory.
    private final BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {
            @Override
                public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (ACTION_USB_PERMISSION.equals(action)) {
                    synchronized (this) {
                        UsbAccessory accessory = UsbManager.getAccessory(intent);
                        if (intent.getBooleanExtra(
                                                   UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                            openAccessory(accessory);
                        } else {
                            Log.d(TAG, "permission denied for accessory " + accessory);
                        }
                        mPermissionRequestPending = false;
                    }
                } else if (UsbManager.ACTION_USB_ACCESSORY_DETACHED.equals(action)) {
                    UsbAccessory accessory = UsbManager.getAccessory(intent);
                    if ( accessory != null && accessory.equals(mAccessory) ) {
                        closeAccessory();
                    }
                }
            }
        };

    // Setup USB Manager and pull out the only USB Accessory
    // available. (We're not a host so we can only have one USB
    // device.
    @Override
        public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mUsbManager = UsbManager.getInstance(this);
        mPermissionIntent =
            PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION),0);
        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_ACCESSORY_DETACHED);
        registerReceiver(mUsbReceiver, filter);

        if (getLastNonConfigurationInstance() != null) {
            mAccessory = (UsbAccessory) getLastNonConfigurationInstance();
            openAccessory(mAccessory);
        }
    }

    private void openAccessory(UsbAccessory accessory) {
        mFileDescriptor = mUsbManager.openAccessory(accessory);
        if (mFileDescriptor != null) {
            mAccessory = accessory;
            FileDescriptor fd = mFileDescriptor.getFileDescriptor();
            mInputStream = new FileInputStream(fd);
            mOutputStream = new FileOutputStream(fd);

            // The following spawns a thread that runs only the 'run'
            // method in this class. That method is the blocking
            // receiver that waits for input from the accessory.
            Thread thread = new Thread(null, this, "UsbAccessoryActivity");

            thread.start();
            Log.d(TAG, "Accessory opened");
        } else {
            Log.d(TAG, "Accessory failed to open");
        }
    }

    private void closeAccessory() {
        try {
            if (mFileDescriptor != null) {
                mFileDescriptor.close();
            }
        } catch (IOException e) {
        } finally {
            mFileDescriptor = null;
            mAccessory = null;
        }
    }

    // On resume, reconnect to our accessory
    @Override
        public void onResume() {
        super.onResume();

        Log.d(TAG,"Resuming application");

        Intent intent = getIntent();
        if (mInputStream != null && mOutputStream != null) {
            return;
        }

        UsbAccessory[] accessories = mUsbManager.getAccessoryList();
        // We only pull the first accessory, because in client mode,
        // there should only be one.
        UsbAccessory accessory = (accessories == null ? null : accessories[0]);
        if (accessory != null) {
            if (mUsbManager.hasPermission(accessory)) {
                openAccessory(accessory);
            } else {
                // We should request for permission since we can't open that accessory
                synchronized (mUsbReceiver) {
                    // We don't want to DOS the user so we keep track
                    // that we're only sending one request to the user
                    // at a time to open the accessory.
                    if (!mPermissionRequestPending) {
                        mUsbManager.requestPermission(accessory,
                                                      mPermissionIntent);
                        mPermissionRequestPending = true;
                    }
                }
            }
        } else {
            Log.d(TAG, "mAccessory is null");
        }
    }

    @Override
        public void onPause() {
        super.onPause();
        closeAccessory(); // Let's not keep running in the background
        Log.d(TAG,"Application put on Pause");
    }

    @Override
        public void onDestroy() {
        Log.d(TAG,"Destroying and unregistering");
        unregisterReceiver(mUsbReceiver); // No longer watching intents
        super.onDestroy();
    }

    // The infinite loop that is the receiver. I'm guess the android
    // task scheduler isn't very aggressive or is just very busy. Thus
    // the buffer is huge and the code is specifically designed for
    // looking for multiple messages inside the buffer.
    //
    // It might also be the other way around. To avoid eating too much
    // juice, we just wait until there is 16k worth of data to
    // process.
    public void run() {
        byte[] array = new byte[1024];
        Log.d(TAG, "Started USB listening thread.");
        while (true) {
            try {
                int r = mInputStream.read(array);
                Log.d(TAG, "Got " + r + " bytes.");
                OffboardData data = OffboardData.parseFrom( new ByteArrayInputStream(array, 0, r) );
                if ( data.isInitialized() ) {
                    Log.d(TAG, "Have time: " + data.getHour() );
                }
            } catch (com.google.protobuf.InvalidProtocolBufferException e) {
                Log.d(TAG, "Invalid protobuf: " + e);
                continue;
            } catch (IOException e) {
                Log.d(TAG, "USB read error: " + e);
                break;
            } 
        }
        Log.d(TAG, "Exiting USB listening thread.");
    }
}