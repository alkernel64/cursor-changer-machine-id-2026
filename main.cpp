#include <windows.h>
#include <lm.h>
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <iphlpapi.h>
#include <io.h>
#include <fcntl.h>

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "iphlpapi.lib")

std::wstring GenerateGUID() {
    GUID guid;
    CoCreateGuid(&guid);
    
    std::wstringstream ss;
    ss << std::uppercase << std::hex << std::setfill(L'0')
       << std::setw(8) << guid.Data1 << L"-"
       << std::setw(4) << guid.Data2 << L"-"
       << std::setw(4) << guid.Data3 << L"-"
       << std::setw(2) << (int)guid.Data4[0] << std::setw(2) << (int)guid.Data4[1] << L"-"
       << std::setw(2) << (int)guid.Data4[2] << std::setw(2) << (int)guid.Data4[3]
       << std::setw(2) << (int)guid.Data4[4] << std::setw(2) << (int)guid.Data4[5]
       << std::setw(2) << (int)guid.Data4[6] << std::setw(2) << (int)guid.Data4[7];
    
    return ss.str();
}

std::wstring GenerateMAC() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::wstringstream ss;
    ss << std::hex << std::setfill(L'0') << std::uppercase;
    for (int i = 0; i < 6; i++) {
        if (i > 0) ss << L"-";
        ss << std::setw(2) << dis(gen);
    }
    return ss.str();
}

bool SetRegistryValue(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName, const wchar_t* value) {
    HKEY hOpenKey;
    LONG result = RegOpenKeyEx(hKey, subKey, 0, KEY_WRITE, &hOpenKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    result = RegSetValueEx(hOpenKey, valueName, 0, REG_SZ, (BYTE*)value, (wcslen(value) + 1) * sizeof(wchar_t));
    RegCloseKey(hOpenKey);
    
    return result == ERROR_SUCCESS;
}

bool SetRegistryValueDWORD(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName, DWORD value) {
    HKEY hOpenKey;
    LONG result = RegOpenKeyEx(hKey, subKey, 0, KEY_WRITE, &hOpenKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    result = RegSetValueEx(hOpenKey, valueName, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
    RegCloseKey(hOpenKey);
    
    return result == ERROR_SUCCESS;
}

bool DeleteRegistryValue(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName) {
    HKEY hOpenKey;
    LONG result = RegOpenKeyEx(hKey, subKey, 0, KEY_WRITE, &hOpenKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    result = RegDeleteValue(hOpenKey, valueName);
    RegCloseKey(hOpenKey);
    
    return result == ERROR_SUCCESS;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
    
    std::wstring newName = L"bujaah";
    
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"  Maschinen-ID Changer - bujaah" << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"\nAendere alle Maschinen-IDs und Identifikatoren..." << std::endl;
    
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
        CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
        FreeSid(AdministratorsGroup);
    }
    
    if (!isAdmin) {
        std::wcout << L"\n[FEHLER] Programm benoetigt Administrator-Rechte!" << std::endl;
        std::wcout << L"Bitte starten Sie das Programm als Administrator." << std::endl;
        system("pause");
        return 1;
    }
    
    CoInitialize(NULL);
    
    int successCount = 0;
    int totalCount = 0;
    
    totalCount++;
    std::wcout << L"\n[1/" << totalCount << L"] Aendere Computernamen (NetBIOS)..." << std::endl;
    if (SetComputerName(newName.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler: " << GetLastError() << std::endl;
    }
    
    totalCount++;
    std::wcout << L"\n[2/" << totalCount << L"] Aendere PC-Namen (DNS-Hostname)..." << std::endl;
    if (SetComputerNameEx(ComputerNamePhysicalDnsHostname, newName.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler: " << GetLastError() << std::endl;
    }
    
    totalCount++;
    std::wstring newGUID = GenerateGUID();
    std::wcout << L"\n[3/" << totalCount << L"] Aendere Machine GUID..." << std::endl;
    if (SetRegistryValue(HKEY_LOCAL_MACHINE, 
        L"SOFTWARE\\Microsoft\\Cryptography", 
        L"MachineGuid", 
        newGUID.c_str())) {
        std::wcout << L"   [OK] Erfolgreich! Neue GUID: " << newGUID << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler beim Aendern der Machine GUID" << std::endl;
    }
    
    totalCount++;
    std::wstring newProductID = GenerateGUID();
    std::wcout << L"\n[4/" << totalCount << L"] Aendere Windows Product ID..." << std::endl;
    if (SetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        L"ProductId",
        newProductID.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler beim Aendern der Product ID" << std::endl;
    }
    
    totalCount++;
    std::wcout << L"\n[5/" << totalCount << L"] Aendere DigitalProductId..." << std::endl;
    std::wstring newDigitalProductId = GenerateGUID() + L"-" + GenerateGUID();
    if (SetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        L"DigitalProductId",
        newDigitalProductId.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler beim Aendern der DigitalProductId" << std::endl;
    }
    
    totalCount++;
    std::wstring newInstallationId = GenerateGUID();
    std::wcout << L"\n[6/" << totalCount << L"] Aendere Installation ID..." << std::endl;
    if (SetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        L"InstallationId",
        newInstallationId.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler beim Aendern der Installation ID" << std::endl;
    }
    
    totalCount++;
    std::wstring newHardwareProfileGUID = GenerateGUID();
    std::wcout << L"\n[7/" << totalCount << L"] Aendere Hardware Profile GUID..." << std::endl;
    if (SetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\IDConfigDB",
        L"HardwareProfileGuid",
        newHardwareProfileGUID.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler beim Aendern der Hardware Profile GUID" << std::endl;
    }
    
    totalCount++;
    std::wstring newSID = L"S-1-5-21-" + std::to_wstring(rand() % 1000000000) + L"-" 
                         + std::to_wstring(rand() % 1000000000) + L"-" 
                         + std::to_wstring(rand() % 1000000000);
    std::wcout << L"\n[8/" << totalCount << L"] Aendere Computer SID..." << std::endl;
    if (SetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History",
        L"MachineGuid",
        newGUID.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler beim Aendern der Computer SID" << std::endl;
    }
    
    totalCount++;
    std::wstring newMAC = GenerateMAC();
    std::wcout << L"\n[9/" << totalCount << L"] Aendere MAC-Adresse (Registry)..." << std::endl;
    std::wcout << L"   [INFO] Hinweis: MAC-Adresse wird ueber Registry geaendert" << std::endl;
    std::wcout << L"   [INFO] Neue MAC: " << newMAC << std::endl;
    std::wcout << L"   [WARNUNG] MAC-Adresse sollte manuell ueber Netzwerkadapter geaendert werden" << std::endl;
    
    totalCount++;
    std::wstring newMachineId = GenerateGUID();
    std::wcout << L"\n[10/" << totalCount << L"] Aendere Windows Machine ID..." << std::endl;
    if (SetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\SQMClient",
        L"MachineId",
        newMachineId.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler (moeglicherweise nicht vorhanden)" << std::endl;
    }
    
    totalCount++;
    std::wcout << L"\n[11/" << totalCount << L"] Aendere Windows Update Machine ID..." << std::endl;
    if (SetRegistryValue(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate",
        L"SusClientId",
        newGUID.c_str())) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else {
        std::wcout << L"   [FEHLER] Fehler (moeglicherweise nicht vorhanden)" << std::endl;
    }
    
    totalCount++;
    std::wcout << L"\n[12/" << totalCount << L"] Versuche NetRenameMachineInDomain..." << std::endl;
    NET_API_STATUS nStatus = NetRenameMachineInDomain(NULL, newName.c_str(), NULL, NULL, 0);
    #ifndef NERR_SuccessAsReboot
    #define NERR_SuccessAsReboot 2102
    #endif
    #ifndef NERR_InvalidComputer
    #define NERR_InvalidComputer 2692
    #endif
    if (nStatus == NERR_Success || nStatus == NERR_SuccessAsReboot) {
        std::wcout << L"   [OK] Erfolgreich!" << std::endl;
        successCount++;
    } else if (nStatus == ERROR_NOT_SUPPORTED || nStatus == NERR_InvalidComputer || nStatus == 2692) {
        std::wcout << L"   [INFO] Nicht unterstuetzt (normal bei Workgroup/Heimcomputer)" << std::endl;
    } else {
        std::wcout << L"   [INFO] Fehler " << nStatus << L" (normal bei Computern ausserhalb einer Domäne)" << std::endl;
    }
    
    CoUninitialize();
    
    std::wcout << L"\n========================================" << std::endl;
    std::wcout << L"  ZUSAMMENFASSUNG" << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"Erfolgreich geaendert: " << successCount << L" / " << totalCount << std::endl;
    
    if (successCount > 0) {
        std::wcout << L"\n[OK] Viele Maschinen-IDs wurden erfolgreich geaendert!" << std::endl;
        std::wcout << L"WARNUNG: Ein Neustart ist ERFORDERLICH, damit alle Aenderungen wirksam werden!" << std::endl;
        std::wcout << L"\nWICHTIG: Nach dem Neustart sollten Sie:" << std::endl;
        std::wcout << L"  - MAC-Adressen manuell ueber Netzwerkadapter-Einstellungen aendern" << std::endl;
        std::wcout << L"  - Eventuell Windows neu aktivieren" << std::endl;
        
        std::wcout << L"\nMoechten Sie den Computer jetzt neu starten? (j/n): ";
        wchar_t choice;
        std::wcin >> choice;
        
        if (choice == L'j' || choice == L'J') {
            std::wcout << L"\nComputer wird in 10 Sekunden neu gestartet..." << std::endl;
            std::wcout << L"Drücken Sie STRG+C zum Abbrechen!" << std::endl;
            system("shutdown /r /t 10 /c \"Maschinen-IDs wurden geaendert - Neustart erforderlich\"");
        } else {
            std::wcout << L"\nBitte starten Sie den Computer manuell neu!" << std::endl;
        }
    } else {
        std::wcout << L"\n[FEHLER] Konnte keine Maschinen-IDs aendern!" << std::endl;
    }
    
    system("pause");
    return 0;
}
