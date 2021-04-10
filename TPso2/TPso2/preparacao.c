#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "preparacao.h"

#define SIZE 200
#define MAX_AIRPORTS 20
#define MAX_AIRPLANES 30
#define PATH TEXT("SOFTWARE\\TP_SO2\\")

int abreCriaChave(HKEY* key, DWORD* result) {
	if (RegCreateKeyEx(HKEY_CURRENT_USER, PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, key, result) != ERROR_SUCCESS) {
		_tprintf(TEXT("Chave não foi criada nem aberta, erro!\n"));
		RegCloseKey(*key);
		return 0;
	}
	return 1;
}


void criaParInt(HKEY key, DWORD valor, TCHAR* atributo) {
	if (RegSetValueEx(key, atributo, 0, REG_DWORD, (LPBYTE)&valor, sizeof(valor)) != ERROR_SUCCESS)
		_tprintf(TEXT("Erro ao criar par atributo valor '%s'."),atributo);
}

void verificaAtributosValores(HKEY key) {
	TCHAR valor[SIZE];
	DWORD type, size = sizeof(valor);

	if (RegQueryValueEx(key, TEXT("maxAirplanes"), NULL, &type, (LPBYTE)&valor, &size) != ERROR_SUCCESS)
		criaParInt(key, MAX_AIRPLANES, L"maxAirplanes");

	if (RegQueryValueEx(key, TEXT("maxAirports") , NULL, &type, (LPBYTE)&valor, &size) != ERROR_SUCCESS)
		criaParInt(key, MAX_AIRPORTS , L"maxAirports" );
}

int preparaAmbiente(){
	HKEY key;
	DWORD result;

	if (!abreCriaChave(&key, &result))
		return 0;

	if (result == REG_CREATED_NEW_KEY) {
		criaParInt(key, MAX_AIRPLANES, L"maxAirplanes");
		criaParInt(key, MAX_AIRPORTS , L"maxAirports");
	}
	else if (result == REG_OPENED_EXISTING_KEY) {
		verificaAtributosValores(key);
	}

	return 1;
}

int getMaxAirplanes() {
	HKEY key;
	DWORD result;
	DWORD valor = 0;
	DWORD type, size = sizeof(valor);

	if (!abreCriaChave(&key, &result))
		return 0;

	if (result == REG_CREATED_NEW_KEY) {
		return 0;
	}
	else if (result == REG_OPENED_EXISTING_KEY) {
		if (RegQueryValueEx(key, TEXT("maxAirplanes"), NULL, &type, (LPBYTE)&valor, &size) != ERROR_SUCCESS) {
			_tprintf(L"Can't acces registry maxAirplanes...\n");
			return 0;
		}
	}
	return valor;
}

int getMaxAirports() {
	HKEY key;
	DWORD result;
	DWORD valor = 0;
	DWORD type, size = sizeof(valor);

	if (!abreCriaChave(&key, &result))
		return 0;

	if (result == REG_CREATED_NEW_KEY) {
		return 0;
	}
	else if (result == REG_OPENED_EXISTING_KEY) {
		if (RegQueryValueEx(key, TEXT("maxAirports"), NULL, &type, (LPBYTE)&valor, &size) != ERROR_SUCCESS) {
			_tprintf(L"Can't acces registry maxAirports...\n");
			return 0;
		}
	}
	return valor;
}