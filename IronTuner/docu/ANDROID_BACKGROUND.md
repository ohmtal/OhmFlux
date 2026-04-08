## SDL3: Android Background

1. Java: Foreground Service und Benachrichtigung
Zuerst benötigst du einen Standard-Android-Service. Dieser muss in deiner AndroidManifest.xml deklariert werden. 
Service-Klasse (Java):

```
public class MySdlService extends Service {
    private static final String CHANNEL_ID = "SdlServiceChannel";

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        createNotificationChannel();
        Notification notification = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle("SDL3 App läuft")
                .setContentText("Dienst ist im Hintergrund aktiv")
                .setSmallIcon(R.drawable.ic_notification)
                .build();

        // Wichtig: startForeground muss innerhalb von 5 Sek. nach startForegroundService gerufen werden
        startForeground(1, notification);
        return START_NOT_STICKY;
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel serviceChannel = new NotificationChannel(
                    CHANNEL_ID, "SDL Service Channel", NotificationManager.IMPORTANCE_DEFAULT);
            getSystemService(NotificationManager.class).createNotificationChannel(serviceChannel);
        }
    }

    @Override public IBinder onBind(Intent intent) { return null; }
}


```

2. Java: SDLActivity erweitern
In deiner Haupt-Activity, die von SDLActivity erbt, fügst du eine Methode hinzu, um diesen Service zu starten.

```
public class MyGameActivity extends SDLActivity {
    public void startSdlForegroundService() {
        Intent serviceIntent = new Intent(this, MySdlService.class);
        ContextCompat.startForegroundService(this, serviceIntent);
    }
}

```

3. C/C++: SDL_APP_WILLENTERBACKGROUND verarbeiten
In SDL3 empfängt dein C-Code die Lifecycle-Events über die Event-Queue. Sobald die App in den Hintergrund geht, nutzt du JNI, um die Java-Methode deiner Activity aufzurufen. 
Native Code (C++):

```
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

void triggerJavaService() {
    JNIEnv* env = (JNIEnv*)SDL_GetAndroidJNIEnv();
    jobject activity = (jobject)SDL_GetAndroidActivity();
    jclass clazz = env->GetObjectClass(activity);
    
    // Die Java-Methode in MyGameActivity finden und aufrufen
    jmethodID method_id = env->GetMethodID(clazz, "startSdlForegroundService", "()V");
    env->CallVoidMethod(activity, method_id);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(clazz);
}

// In deinem Event-Loop:
SDL_Event event;
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_DID_ENTER_BACKGROUND) {
        // Jetzt den Foreground Service starten, um den Prozess am Leben zu erhalten
        triggerJavaService();
    }
}

```
Wichtige Hinweise

- Berechtigungen: Ab Android 14 musst du den foregroundServiceType in der Manifest-Datei angeben (z. B. specialUse oder mediaPlayback) und die entsprechende Permission FOREGROUND_SERVICE (und ggf. spezifische Typen) anfordern.
    
- Timing: SDL_APP_WILLENTERBACKGROUND (bzw. SDL_EVENT_WILL_ENTER_BACKGROUND in SDL3) signalisiert, dass die App pausiert wird. Das tatsächliche Starten des Services ist oft sicherer beim "Did Enter"-Event, um sicherzustellen, dass das System den Übergang bereits verarbeitet hat.
    
- Renderer: Denke daran, dass nach WILLENTERBACKGROUND der Grafik-Context unter Android oft ungültig wird. Pausiere deine Rendering-Logik sofort. 


---

## Integrieren

Um deinen Java-Quellcode (die SDLActivity-Subklasse und den Service) sauber in den SDL3-Build-Prozess zu integrieren, solltest du die Standard-Projektstruktur von Android nutzen. SDL3 ist so konzipiert, dass du die Bibliothek entweder als AAR-Archiv einbindest oder das android-project-Template von SDL als Basis nimmst. 
Hier ist der empfohlene Weg für die Integration:

1. Verzeichnisstruktur einhalten
Lege deine Java-Dateien in das standardmäßige Source-Verzeichnis deines App-Moduls. Wenn dein Paketname z. B. com.deinefirma.meinspiel lautet, gehört der Code hierhin:

- app/src/main/java/com/deinefirma/meinspiel/MyGameActivity.java

- app/src/main/java/com/deinefirma/meinspiel/MySdlService.java 

2. Anpassung der AndroidManifest.xml 
Damit Android weiß, dass es deine neue Activity und den Service starten soll, musst du das Manifest unter app/src/main/AndroidManifest.xml anpassen: 

```
<manifest ...>
    <!-- Berechtigungen für Foreground Service (Android 14+) -->
    <uses-permission android:name="android.permission.FOREGROUND_SERVICE" />
    <uses-permission android:name="android.permission.FOREGROUND_SERVICE_SPECIAL_USE" />

    <application ...>
        <!-- Deine eigene Activity anstelle der Standard-SDLActivity -->
        <activity android:name=".MyGameActivity" 
                  android:label="@string/app_name"
                  android:exported="true">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>

        <!-- Registrierung des Services -->
        <service android:name=".MySdlService"
                 android:foregroundServiceType="specialUse"
                 android:exported="false" />
    </application>
</manifest>

```
3. Build-Automatisierung mit Gradle
Da SDL3 Gradle verwendet, wird dein Java-Code automatisch mitkompiliert, solange er im src/main/java-Ordner liegt. 

    Stelle sicher, dass in deiner app/build.gradle die Abhängigkeiten für die Notifications korrekt sind (z. B. ***androidx.core:core:1.12.0***).
    Falls du die SDL3-Sourcen direkt mitbaust (nicht via AAR), verlinke das SDL-Verzeichnis in deinem Projekt, wie im README-android beschrieben. 

4. Der "SDL-Weg" für Nachrichten (Alternative zu JNI)
Anstatt direkt komplexes JNI für den Aufruf von C zu Java zu schreiben, bietet SDL3 die Funktion SDL_SendAndroidMessage(). Du kannst in C einen Befehl senden, den du in deiner Java-Klasse in der Methode onUnhandledMessage abfängst: 

    C-Code: SDL_SendAndroidMessage(0x8001, 0);
    Java-Code:
    
```
@Override
protected boolean onUnhandledMessage(int command, Object param) {
    if (command == 0x8001) {
        startSdlForegroundService();
        return true;
    }
    return super.onUnhandledMessage(command, param);
}
```
