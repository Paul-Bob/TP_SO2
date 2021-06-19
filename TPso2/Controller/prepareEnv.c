#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "prepareEnv.h"

#define SIZE 200
#define MAX_AIRPORTS 20
#define MAX_AIRPLANES 2


int openCreateKey(HKEY* key, DWORD* result, TCHAR* path) {
	if (RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, key, result) != ERROR_SUCCESS) {
		_tprintf(TEXT("Chave não foi criada nem aberta, erro!\n"));
		RegCloseKey(*key);
		return 0;
	}
	return 1;
}


void createPair(HKEY key, DWORD valor, TCHAR* attribute) {
	if (RegSetValueEx(key, attribute, 0, REG_DWORD, (LPBYTE)&valor, sizeof(valor)) != ERROR_SUCCESS)
		_tprintf(TEXT("Erro ao criar par attribute valor '%s'."),attribute);
}

void verifyPair(HKEY key) {
	TCHAR valor[SIZE];
	DWORD type, size = sizeof(valor);

	if (RegQueryValueEx(key, TEXT("maxAirplanes"), NULL, &type, (LPBYTE)&valor, &size) != ERROR_SUCCESS)
		createPair(key, MAX_AIRPLANES, L"maxAirplanes");

	if (RegQueryValueEx(key, TEXT("maxAirports") , NULL, &type, (LPBYTE)&valor, &size) != ERROR_SUCCESS)
		createPair(key, MAX_AIRPORTS , L"maxAirports" );
}

int prepareEnvironment(TCHAR* path){
	HKEY key;
	DWORD result;

	if (!openCreateKey(&key, &result,path))
		return 0;

	if (result == REG_CREATED_NEW_KEY) {
		createPair(key, MAX_AIRPLANES, L"maxAirplanes");
		createPair(key, MAX_AIRPORTS , L"maxAirports");
	}
	else if (result == REG_OPENED_EXISTING_KEY) {
		verifyPair(key);
	}

	RegCloseKey(key);

	return 1;
}

int getMax(TCHAR* attribute, TCHAR* path) {
	HKEY key;
	DWORD result;
	DWORD valor = 0;
	DWORD type, size = sizeof(valor);

	if (!openCreateKey(&key, &result,path))
		return 0;

	if (result == REG_CREATED_NEW_KEY) {
		RegCloseKey(key);
		return 0;
	}
	else if (result == REG_OPENED_EXISTING_KEY) {
		if (RegQueryValueEx(key, attribute, NULL, &type, (LPBYTE)&valor, &size) != ERROR_SUCCESS) {
			_tprintf(L"Can't acces registry '%s'...\n", attribute);
			RegCloseKey(key);
			return 0;
		}
	}
	RegCloseKey(key);
	return valor;
}