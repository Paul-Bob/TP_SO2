#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "comandosControlador.h"

int verifyPositions(pDATA data, int x, int y, int positions);

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
		if (data->nrAirports == 0) {
			_tprintf(TEXT("Não existe nenhum aeroporto!\n\n"));
			return;
		}
		_tprintf(TEXT("Aeroportos:\n"));
		for (int i = 0; i < data->nrAirports; i++)
				_tprintf(TEXT("%s - [%d,%d]\n"), (data->airports+i)->name, (data->airports + i)->coordinates[0], (data->airports + i)->coordinates[1]);
	}
	else if (!_tcscmp(comando, TEXT("criar"))) {

		if (data->nrAirports >= data->maxAirports) {
			_tprintf(TEXT("Nrº máximo de aeroportos atingido.\n"));
			return;
		}
		
		TCHAR name[NAMESIZE];
		_tprintf(TEXT("Nome aeroporto: "));
		_fgetts(name, NAMESIZE, stdin); 
		name[_tcslen(name) - 1] = '\0';

		for(int i = 0; i < data->nrAirports; i++)
			if (!_tcscmp(name, data->airports[i].name)) {
				_tprintf(TEXT("Nome de aeroporto deve ser único.\n\n"));
				return;
			}

		TCHAR coordinate[100];
		int x = -1, y = -1, temp;
		do {
			_tprintf(TEXT("Coordenada X entre 0 e 999 : "));
			while (!_tscanf_s(L"%d", &x)) {
				while ((temp = _gettchar()) != '\n' && temp != EOF);
				_tprintf(TEXT("Insere dígito entre 0 e 999 : "));
			}
		} while (x < 0 || x >= 1000);

		//limpa stdin
		while ((temp = _gettchar()) != '\n' && temp != EOF);

		do {
			_tprintf(TEXT("Coordenada Y entre 0 e 999 : "));
			while (!_tscanf_s(L"%d", &y)) {
				while ((temp = _gettchar()) != '\n' && temp != EOF);
				_tprintf(TEXT("Insere dígito entre 0 e 999 : "));
			}
		} while (y < 0 || y >= 1000);

		while ((temp = _gettchar()) != '\n' && temp != EOF);

		if (verifyPositions(data, x, y, 10)) {
			airport new;
			new.coordinates[0] = x;
			new.coordinates[1] = y;
			_tcscpy_s(new.name,NAMESIZE,name);
			data->map[x][y].airport = &data->airports[data->nrAirports];
			data->airports[data->nrAirports] = new;
			data->nrAirports++;
			_tprintf(TEXT("Aeroporto '%s' foi adicionado com sucesso nas coordenadas [%d,%d]\n"),name,x,y);
		}
	}
	else
		_tprintf(TEXT("Comando inexistente\n"));
	_puttchar(L'\n');
}

int verifyPositions(pDATA data, int x, int y,int positions) {
	int lastX,lastY,firstX,firstY;

	if (x < 0 || y < 0 || x >= MAPSIZE || y >= MAPSIZE || data->map[x][y].airport != NULL)
		return 0;

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

	for (int linha = firstY; linha <= lastY; linha++)
		for (int coluna = firstX; coluna <= lastX; coluna++)
			if (coluna == x && y == linha)
				continue;
			else if (data->map[coluna][linha].airport != NULL) {
				_tprintf(TEXT("Aeroporto %s [%d,%d] está num raio de 10 posições...\n"), data->map[coluna][linha].airport->name,coluna,linha);
				return 0;
			}
	return 1;
}