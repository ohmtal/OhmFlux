package com.ohmtal.irontuner;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.content.Context;
import android.os.Build;
import android.os.IBinder;
import androidx.core.app.NotificationCompat;


public class IronTunerService extends Service {
    private static final String CHANNEL_ID = "SdlServiceChannel";
    private static final int NOTIF_ID = 1;

    //--------------------------------------------------------------------------
    private Notification buildNotification(String title, String text) {

        Intent notificationIntent = new Intent(this, IronTunerActivity.class);
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);

        PendingIntent pendingIntent = PendingIntent.getActivity(
                this,
                0,
                notificationIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );

        int iconId = getResources().getIdentifier("ic_launcher", "mipmap", getPackageName());

               // .setSmallIcon(android.R.drawable.ic_menu_info_details)
        return new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle(title)
                .setContentText(text)
                .setSmallIcon(iconId != 0 ? iconId : android.R.drawable.ic_menu_info_details)
                .setContentIntent(pendingIntent)
                .setPriority(NotificationCompat.PRIORITY_LOW)
                .setOngoing(true)
                .build();

    }
    //--------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------
    private void updateNotification(String newText) {
        Notification notification = buildNotification("Iron Tuner", newText);
        NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        manager.notify(NOTIF_ID, notification);
    }

    //--------------------------------------------------------------------------
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
    //--------------------------------------------------------------------------
    @Override public IBinder onBind(Intent intent) { return null; }
}



