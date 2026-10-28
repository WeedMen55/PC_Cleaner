#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <sstream>

#pragma comment(lib, "advapi32.lib")

// ── Active un privilège dans le token courant ─────────────────────────────────
static void EnablePrivilege(LPCWSTR privName) {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    LookupPrivilegeValueW(NULL, privName, &tp.Privileges[0].Luid);
    AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
    CloseHandle(hToken);
}

// ── TakeOwnership / GrantFullAccess ──────────────────────────────────────────
static void TakeOwnership(const std::wstring& path) {
    EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
    EnablePrivilege(SE_RESTORE_NAME);
    EnablePrivilege(L"SeDebugPrivilege");
    PSID pSID = nullptr;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    AllocateAndInitializeSid(&SIDAuth, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &pSID);
    SetNamedSecurityInfoW((LPWSTR)path.c_str(), SE_FILE_OBJECT,
        OWNER_SECURITY_INFORMATION, pSID, NULL, NULL, NULL);
    if (pSID) FreeSid(pSID);
}

static void GrantFullAccess(const std::wstring& path) {
    PSID pSID = nullptr;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    AllocateAndInitializeSid(&SIDAuth, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &pSID);
    EXPLICIT_ACCESSW ea = {};
    ea.grfAccessPermissions = GENERIC_ALL;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea.Trustee.ptstrName = (LPWSTR)pSID;
    PACL pACL = nullptr;
    SetEntriesInAclW(1, &ea, NULL, &pACL);
    SetNamedSecurityInfoW((LPWSTR)path.c_str(), SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL);
    if (pACL) LocalFree(pACL);
    if (pSID) FreeSid(pSID);
}

// ── Vide tous les journaux de l'Observateur d'événements ──────────────────────
static void CleanEventLogs() {
    std::cout << "[*] Vidage de l'Observateur d'événements Windows...\n";
    // Utilise wevtutil pour lister et vider chaque journal silencieusement
    FILE* pipe = _wpopen(L"powershell -Command \"wevtutil el | ForEach-Object { wevtutil cl \\\"$_\\\" 2>&1 }\"", L"r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            // Lecture silencieuse ou traçage optionnel
        }
        _pclose(pipe);
        std::cout << "[OK] Journaux d'événements vidés.\n";
    }
    else {
        std::cout << "[FAIL] Impossible d'exécuter la commande de nettoyage des logs.\n";
    }
}

// ── Supprime tout dans C:\Windows\Prefetch ────────────────────────────────────
static void CleanPrefetch() {
    std::filesystem::path prefetch = "C:\\Windows\\Prefetch";
    std::cout << "[*] Nettoyage Prefetch...\n";
    try {
        for (const auto& entry : std::filesystem::directory_iterator(prefetch)) {
            try {
                std::filesystem::remove_all(entry);
                std::cout << "[OK] " << entry.path().filename().string() << "\n";
            }
            catch (...) {
                std::cout << "[FAIL] " << entry.path().filename().string() << "\n";
            }
        }
    }
    catch (std::filesystem::filesystem_error& e) {
        std::cout << "[ERREUR Prefetch] " << e.what() << "\n";
    }
}

// ── CleanAppData ──────────────────────────────────────────────────────────────
static bool CleanAppData() {
    // Active tous les privilèges utiles dès le début
    EnablePrivilege(SE_TAKE_OWNERSHIP_NAME);
    EnablePrivilege(SE_RESTORE_NAME);
    EnablePrivilege(SE_BACKUP_NAME);
    EnablePrivilege(L"SeDebugPrivilege");

    char* userProfile = nullptr;
    size_t len = 0;
    _dupenv_s(&userProfile, &len, "USERPROFILE");
    if (!userProfile) {
        std::cout << "[ERREUR] USERPROFILE non défini.\n";
        return false;
    }

    // Supprime WindowsUpdate.log
    if (!DeleteFileW(L"C:\\Windows\\WindowsUpdate.log")) {
        std::cout << "[INFO] WindowsUpdate.log non supprimé (normal si absent).\n";
    }
    else {
        std::cout << "[OK] WindowsUpdate.log supprimé.\n";
    }

    std::vector<std::filesystem::path> paths = {
        L"C:\\$Recycle.Bin",
        L"C:\\Windows\\Temp",
        L"C:\\Windows\\tracing",
        std::string(userProfile) + "\\AppData\\Local\\Temp",
        std::string(userProfile) + "\\AppData\\Roaming\\Microsoft\\Windows\\Recent",
        std::string(userProfile) + "\\AppData\\Roaming\\Microsoft\\Windows\\Recent\\AutomaticDestinations",
        std::string(userProfile) + "\\AppData\\Roaming\\Microsoft\\Windows\\Recent\\CustomDestinations",
        std::string(userProfile) + "\\AppData\\Local\\Microsoft\\Windows\\Burn",
        std::string(userProfile) + "\\AppData\\Local\\CrashDumps",
        std::string(userProfile) + "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Cache\\Cache_Data",
        std::string(userProfile) + "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\History",
        std::string(userProfile) + "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\History-journal",
        std::string(userProfile) + "\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Visited Links",
        std::string(userProfile) + "\\AppData\\Local\\Temporary Internet Files",
        std::string(userProfile) + "\\AppData\\Local\\Microsoft\\Windows\\History",
        "C:\\Windows\\Minidump",
    };
    free(userProfile);

    for (const auto& p : paths) {
        if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p)) {
            std::cout << "[SKIP] " << p.string() << "\n";
            continue;
        }
        std::cout << "[*] Nettoyage : " << p.string() << "\n";
        for (const auto& entry : std::filesystem::directory_iterator(p)) {
            try {
                std::filesystem::remove_all(entry.path());
                std::cout << "  [OK] " << entry.path().filename().string() << "\n";
            }
            catch (std::filesystem::filesystem_error&) {
                TakeOwnership(entry.path().wstring());
                GrantFullAccess(entry.path().wstring());
                try {
                    std::filesystem::remove_all(entry.path());
                    std::cout << "  [OK+] " << entry.path().filename().string() << "\n";
                }
                catch (std::filesystem::filesystem_error& e2) {
                    // Dernier recours : DeleteFileW direct (pour .dmp SYSTEM)
                    if (DeleteFileW(entry.path().wstring().c_str())) {
                        std::cout << "  [OK+] " << entry.path().filename().string() << " (forcé)\n";
                    }
                    else {
                        std::cout << "  [FAIL] " << entry.path().filename().string()
                            << " - " << e2.what() << "\n";
                    }
                }
            }
        }
        std::cout << "  => Terminé : " << p.string() << "\n";
    }
    return true;
}

// ── CleanAppData2 — lancé dans un thread détaché ──────────────────────────────
static void CleanAppData2() {
    auto promise = std::make_shared<std::promise<bool>>();
    std::future<bool> future = promise->get_future();
    std::thread([promise]() {
        try {
            bool result = CleanAppData();
            promise->set_value(result);
        }
        catch (const std::system_error&) {
            promise->set_value(false);
        }
        catch (const std::exception&) {
            promise->set_value(false);
        }
        catch (...) {
            promise->set_value(false);
        }
        }).detach();
    // Attend la fin pour afficher le résultat
    bool ok = future.get();
    std::cout << "\n[CleanAppData] " << (ok ? "Succès" : "Échec") << "\n";
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "=== Nettoyage système ===\n\n";

    // 0. Vidage des journaux d'événements Windows
    CleanEventLogs();
    std::cout << "\n";

    // 1. Prefetch
    CleanPrefetch();
    std::cout << "\n";

    // 2. AppData + Temp + Chrome + Recycle etc. (dans thread)
    CleanAppData2();

    std::cout << "\n=== Terminé ===\n";
    system("pause");
    return 0;
}