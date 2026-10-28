PC CLEANER 


1. Journaux de l'Observateur d'événements Windows (CleanEventLogs)
Tous les journaux d'événements Windows (wevtutil) : Il vide l'intégralité des logs système, d'application, de sécurité et d'administration de Windows de manière silencieuse.

2. Dossier Prefetch (CleanPrefetch)
C:\Windows\Prefetch : Il supprime tout le contenu de ce dossier (fichiers .pf), qui sert normalement à accélérer le lancement des applications.

3. Fichiers et dossiers système / utilisateur (CleanAppData)
Le programme récupère le profil utilisateur (USERPROFILE) et cible les éléments suivants :

Fichiers système divers :

C:\Windows\WindowsUpdate.log (Journal des mises à jour Windows).

Corbeille et Dossiers Temp :

C:\$Recycle.Bin (La Corbeille Windows).

C:\Windows\Temp (Dossier temporaire de Windows).

C:\Windows\tracing (Fichiers de traçage réseau/système).

%USERPROFILE%\AppData\Local\Temp (Dossier temporaire de l'utilisateur).

Historique et Récents (Windows Explorer) :

%USERPROFILE%\AppData\Roaming\Microsoft\Windows\Recent (Fichiers et dossiers récents).

%USERPROFILE%\AppData\Roaming\Microsoft\Windows\Recent\AutomaticDestinations (Destinations automatiques de la barre des tâches).

%USERPROFILE%\AppData\Roaming\Microsoft\Windows\Recent\CustomDestinations (Destinations personnalisées).

Graçage, Crashs et Internet :

%USERPROFILE%\AppData\Local\Microsoft\Windows\Burn (Fichiers temporaires de gravure CD/DVD).

%USERPROFILE%\AppData\Local\CrashDumps (Rapports de plantage d'applications).

C:\Windows\Minidump (Fichiers de vidage mémoire en cas d'écran bleu / BSOD).

%USERPROFILE%\AppData\Local\Temporary Internet Files (Fichiers Internet temporaires).

%USERPROFILE%\AppData\Local\Microsoft\Windows\History (Historique de navigation Internet Explorer / système).

Données Google Chrome :

%USERPROFILE%\AppData\Local\Google\Chrome\User Data\Default\Cache\Cache_Data (Cache web de Chrome).

%USERPROFILE%\AppData\Local\Google\Chrome\User Data\Default\History (Historique de navigation Chrome).

%USERPROFILE%\AppData\Local\Google\Chrome\User Data\Default\History-journal (Journal de l'historique Chrome).

%USERPROFILE%\AppData\Local\Google\Chrome\User Data\Default\Visited Links (Liens visités sur Chrome).
