package com.ohmtal.irontuner;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import androidx.core.app.NotificationCompat;

public class IronTunerService extends Service {
    private static final String CHANNEL_ID = "SdlServiceChannel";
    private static final int NOTIF_ID = 1;

    private Notification buildNotification(String title, String text) {
        return new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle(title)
                .setContentText(text)
                .setSmallIcon(android.R.drawable.ic_menu_info_details)
                .setPriority(NotificationCompat.PRIORITY_LOW)
                .setOngoing(true) // deny swipe away
                .build();
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        createNotificationChannel();

        if (intent != null && intent.hasExtra("update_text")) {
            String newText = intent.getStringExtra("update_text");
            updateNotification(newText);
        } else {
            startForeground(NOTIF_ID, buildNotification("Iron Tuner", "playing music ;)"));
        }

        return START_NOT_STICKY;
    }



    public void updateNotificationFromCpp(String text) {
        Intent intent = new Intent(this, IronTunerService.class);
        intent.putExtra("update_text", text);
        this.startService(intent);
    }

    public void updateNotification(String text) {
        NotificationManager manager = getSystemService(NotificationManager.class);
        if (manager != null) {
            manager.notify(NOTIF_ID, buildNotification("Iron Tuner", text));
        }
    }


    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel serviceChannel = new NotificationChannel(
                    CHANNEL_ID, "SDL Service Channel", NotificationManager.IMPORTANCE_LOW);
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(serviceChannel);
            }
        }
    }



    @Override public IBinder onBind(Intent intent) { return null; }
}



    // @Override
    // public int onStartCommand(Intent intent, int flags, int startId) {
    //     createNotificationChannel();
    //     startForeground(NOTIF_ID, buildNotification("Iron Tuner", "Running..."));
    //     if (intent != null && intent.hasExtra("update_text")) {
    //         updateNotification(intent.getStringExtra("update_text"));
    //     }
    //
    //     return START_NOT_STICKY;
    // }


    // public void updateNotification(final String text) {
    //     runOnUiThread(new Runnable() {
    //         @Override
    //         public void run() {
    //             // Logik zum Update der Notification
    //         }
    //     });
    // @Override
    // public int onStartCommand(Intent intent, int flags, int startId) {
    //     createNotificationChannel();
    //     Notification notification = new NotificationCompat.Builder(this, CHANNEL_ID)
    //             .setContentTitle("Iron Tuner")
    //             .setContentText("Running in background")
    //             .setSmallIcon(android.R.drawable.ic_menu_info_details)
    //             .setPriority(NotificationCompat.PRIORITY_LOW)
    //             .build();
    //
    //     startForeground(1, notification);
    //     return START_NOT_STICKY;
    // }

// import android.app.Notification;
// import android.app.NotificationChannel;
// import android.app.NotificationManager;
// import android.app.Service;
// import android.content.Intent;
// import android.os.Build;
// import android.os.IBinder;
// import androidx.core.app.NotificationCompat;
//
// public class IronTunerService extends Service {
//     private static final String CHANNEL_ID = "SdlServiceChannel";
//
//     @Override
//     public int onStartCommand(Intent intent, int flags, int startId) {
//         createNotificationChannel();
//         Notification notification = new NotificationCompat.Builder(this, CHANNEL_ID)
//                 .setContentTitle("Iron Tuner")
//                 .setContentText("running in background")
//                 .setSmallIcon(android.R.drawable.ic_menu_info_details)
//                 .build();
//
//         startForeground(1, notification);
//         return START_NOT_STICKY;
//     }
//
//     private void createNotificationChannel() {
//         if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
//             NotificationChannel serviceChannel = new NotificationChannel(
//                     CHANNEL_ID, "SDL Service Channel", NotificationManager.IMPORTANCE_DEFAULT);
//             getSystemService(NotificationManager.class).createNotificationChannel(serviceChannel);
//         }
//     }
//
//     @Override public IBinder onBind(Intent intent) { return null; }
// }
//
