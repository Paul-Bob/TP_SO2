#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "comandosControlador.h"

void interpretaComandoControlador(TCHAR* comando, pDATA data) {
	_puttchar(L'\n');
	if (!_tcscmp(comando, TEXT("exit"))) {
		_tprintf(TEXT("see ya\n"));
	}
	else if (!_tcscmp(comando, TEXT("help"))) {
		_tprintf(TEXT("Comandos disponiveis:\n"));
		_tprintf(TEXT("- exit   --> Encerra o sistema\n"));
		_tprintf(TEXT("- criar  --> Criar novos aeroportos\n"));
		_tprintf(TEXT("- listar --> Lista aeroportos, aviões e passageiros\n"));
		//Dúvida: é suposto chamar o comando criar e entrar num menu de criação oou ex: 'criar AeroportoXPTO 200 432' ou seja criar nomeAeroporto X Y?
		//Dúvida: igual para suspender/ativar, é suposto entrar num menu ou como fizemos em SO que era sAviaoXpTO aAviaoXpTO sendo o s para suspender e a para ativar?
	}
	else if (!_tcscmp(comando, TEXT("listar"))) {
		_tprintf(TEXT("Coming soon\n"));
	}
	else if (!_tcscmp(comando, TEXT("criar"))) {
		if (data->nrAirports >= data->maxAirports) {
			_tprintf(TEXT("Nrº máximo de aeroportos atingido.\n"));
			return;
		}
		//data->airports[data->nrAirports].name
		TCHAR name[NAMESIZE];
		_tprintf(TEXT("Nome aeroporto: "));
		_fgetts(name, NAMESIZE, stdin); 
		name[_tcslen(name) - 1] = '\0';

		for(int i = 0; i < data->nrAirports; i++)
			if (!_tcscmp(name, data->airports[i].name)) {
				_tprintf(TEXT("Nome de aeroporto deve ser único.\n"));
				return;
			}

		TCHAR coordinate[100];
		int x, y;
		do {
			_tprintf(TEXT("Coordenada X entre 0 e 999 : "));
			_fgetts(coordinate, 100, stdin);
			x = _tcstol(coordinate, NULL, 0);
		} while (x < 0 || x >= 1000);

		do {
			_tprintf(TEXT("Coordenada Y entre 0 e 999 : "));
			_fgetts(coordinate, 100, stdin);
			y = _tcstol(coordinate, NULL, 0);
		} while (y < 0 || y >= 1000);

		//falta função para verificar o mapa para não ter nenhum aeroporto numa area de 10*10 posições
		verifyPositions(data, x, y, 10);
	}
	else
		_tprintf(TEXT("Comando inexistente\n"));
	_puttchar(L'\n');
}

int verifyPositions(pDATA data, int x, int y,int positions) {
	int lastX,lastY,firstX,firstY;

	if (x < positions) {
		lastX = positions + x;
		firstX = 0;
	}
	else if (MAPSIZE-1 - x < positions) {
		lastX = MAPSIZE-1;
		firstX = x - positions;
	}
	else {
		lastX = x + positions;
		firstX = x - positions;
	}

	if (y < positions) {
		lastY = positions + y;
		firstY = 0;
	}
	else if (MAPSIZE-1 - y < positions) {
		lastY = MAPSIZE-1;
		firstY = y - positions;
	}
	else {
		lastY = y + positions;
		firstY = y - positions;
	}

	_tprintf(TEXT("verificar x no intervalo [%d,%d]\nverificar y no intervalo [%d,%d]\n"),firstX,lastX,firstY,lastY);

	for (int linha = firstY; linha <= lastY; linha++) {
		for (int coluna = firstX; coluna <= lastX; coluna++)
			if (coluna == x && y == linha)
				_tprintf(TEXT("(XXXXXXX) "));
			else
				_tprintf(TEXT("(%3d,%3d) "),coluna,linha);
		_puttchar(L'\n');
	}
	_puttchar(L'K');
}