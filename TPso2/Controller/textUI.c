#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "textUI.h"

int verifyPositions(pDATA data, int x, int y, int positions);

void interpretaComandoControlador(TCHAR* command, pDATA data) {
	_puttchar(L'\n');

	TCHAR* action, *argument = NULL, *extra = NULL;

	//get first word from command line
	action = _tcstok_s(command, L" ", &argument);

	if (!_tcscmp(action, TEXT("exit"))) {
		_tprintf(TEXT("see ya\n"));
	}
	else if (!_tcscmp(action, TEXT("help"))) {
		if (argument == 0 || _tcslen(argument)){
			_tprintf(L"%s: extra operand '%s' \n\n", action, argument);
			return;
		}
		_tprintf(TEXT("Comandos disponiveis:\n"));
		_tprintf(TEXT("- criar [nome] [x] [y] --> Criar novos aeroportos\n"));
		_tprintf(TEXT("- listar               --> Lista aeroportos, aviões e passageiros\n"));
		_tprintf(TEXT("- exit                 --> Encerra o sistema\n"));
		//Dúvida: é suposto chamar o comando criar e entrar num menu de criação oou ex: 'criar AeroportoXPTO 200 432' ou seja criar nomeAeroporto X Y?
		//Dúvida: igual para suspender/ativar, é suposto entrar num menu ou como fizemos em SO que era sAviaoXpTO aAviaoXpTO sendo o s para suspender e a para ativar?
	}
	else if (!_tcscmp(action, TEXT("listar"))) {
		if (argument == 0 || _tcslen(argument)) {
			_tprintf(L"%s: extra operand '%s' \n\n", action, argument);
			return;
		}
		if (data->nrAirports == 0) {
			_tprintf(TEXT("Não existe nenhum aeroporto!\n\n"));
			return;
		}
		_tprintf(TEXT("Aeroportos:\n"));
		for (int i = 0; i < data->nrAirports; i++)
				_tprintf(TEXT("%s - [%d,%d]\n"), (data->airports+i)->name, (data->airports + i)->coordinates[0], (data->airports + i)->coordinates[1]);

		_tprintf(TEXT("Avioes:\n"));
		for (int i = 0; i < data->maxAirplanes; i++)
			if(data->planes[i].velocity != -1)
				_tprintf(TEXT("plane in [%d,%d]\n"), data->planes[i].current.x, data->planes[i].current.y);
	}
	else if (!_tcscmp(action, TEXT("criar"))) {

		if (argument == 0 || !_tcslen(argument)) {
			_tprintf(L"%s: airport name required\n\n", action);
			return;
		}
		else {
			airport new;

			TCHAR* name = _tcstok_s(argument, L" ", &argument);
			_tcscpy_s(new.name, NAMESIZE, name);

			if (argument == 0 || !_tcslen(argument)) {
				_tprintf(L"%s %s: airport x coordinate required\n\n", action, new.name);
				return;
			}
			else {
				TCHAR* readInt = NULL;
				readInt = _tcstok_s(argument, L" ", &argument);

				if (argument == 0 || !_tcslen(readInt)) {
					_tprintf(L"%s %s: airport x coordinate required\n\n", action, new.name);
					return;
				}

				new.coordinates[X] = _tcstol(readInt, &extra, 10);

				if (_tcslen(extra)) {
					_tprintf(L"%s %s %d: '%s' is extra\nTry: '%s %s %d'\n\n", action, new.name, new.coordinates[X], extra, action, new.name, new.coordinates[X]);
					return;
				}

				if (new.coordinates[X] < 0 || new.coordinates[X] > 999) {
					_tprintf(L"X coordinate must be higher or equal to 0 and lower than 1000\n\n");
					return;
				}

				readInt = NULL;
				readInt = _tcstok_s(argument, L" ", &argument);

				if (argument == 0 || !readInt || !_tcslen(readInt)) {
					_tprintf(L"%s %s %d: airport y coordinate required\n\n",action, new.name, new.coordinates[X]);
					return;
				}

				new.coordinates[Y] = _tcstol(readInt, &extra, 10);

				if (_tcslen(extra)) {
					_tprintf(L"%s %s %d %d: '%s' is extra\nTry: '%s %s %d %d'\n\n", action, new.name, new.coordinates[X], new.coordinates[Y], extra, action, new.name, new.coordinates[X], new.coordinates[Y]);
					return;
				}

				if (new.coordinates[Y] < 0 || new.coordinates[Y] > 999) {
					_tprintf(L"Y coordinate must be higher or equal to 0 and lower than 1000\n\n");
					return;
				}
			}

			//tcstol can return 0 from error, not sure if 0 from input or error...
			if (new.coordinates[X] == 0 || new.coordinates[Y] == 0) {
				TCHAR anwser = L'k',temp;
				TCHAR extraAnwser[200];

				_tprintf(L"\nAdd Airport\nName: %s\nCoordinates: [%d,%d]\n\nAdd?! [Y/n]", new.name, new.coordinates[X], new.coordinates[Y]);

				while ((anwser != L'y' && anwser != L'Y' && anwser != L'n' && anwser != L'N') || (_tcslen(extraAnwser) != 0)) {
					extraAnwser[0] = '\0';
					_tscanf_s(L" %c", &anwser, 1);
					_tscanf_s(L"%[^\n]", extraAnwser, 200);
				}
				while ((temp = _gettchar()) != '\n' && temp != EOF);
				if (anwser == L'n' || anwser == L'N') {
					_puttchar(L'\n');
					return;
				}
			}

			if (data->nrAirports >= data->maxAirports) {
				_tprintf(TEXT("Nrº máximo de aeroportos atingido.\n\n"));
				return;
			}

			for (int i = 0; i < data->nrAirports; i++)
				if (!_tcscmp(new.name, data->airports[i].name)) {
					_tprintf(TEXT("Nome de aeroporto deve ser único.\n\n"));
					return;
				}

			if (verifyPositions(data, new.coordinates[X], new.coordinates[Y], 10)) {
				data->map->matrix[new.coordinates[X]][new.coordinates[Y]] = 2;
				data->airports[data->nrAirports] = new;
				data->nrAirports++;
				_tprintf(TEXT("Aeroporto '%s' foi adicionado com sucesso nas coordenadas [%d,%d]\n"), new.name, new.coordinates[X], new.coordinates[Y]);
			}
		}
	}
	else
		_tprintf(TEXT("Comando '%s' inexistente\n"),action);
	_puttchar(L'\n');
}

int verifyPositions(pDATA data, int x, int y,int positions) {
	int lastX,lastY,firstX,firstY;

	if (x < 0 || y < 0 || x >= MAPSIZE || y >= MAPSIZE)
		return 0;

	if (data->map->matrix[x][y] == 2) {
		_tprintf(TEXT("Já existe um aeroporto na posição [%d,%d]."), x, y);
		return 0;
	}

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
			else if (data->map->matrix[coluna][linha] == 2) {
				_tprintf(TEXT("Aeroporto [%d,%d] está num raio de 10 posições...\n"), coluna,linha);
				return 0;
			}
	return 1;
}






/* AIRPORT CREATION BY FORM */
/*
TCHAR name[NAMESIZE];
		_tprintf(TEXT("Nome aeroporto: "));
		_fgetts(name, NAMESIZE, stdin);
		name[_tcslen(name) - 1] = '\0';

		for(int i = 0; i < data->nrAirports; i++)
			if (!_tcscmp(name, data->airports[i].name)) {
				_tprintf(TEXT("Nome de aeroporto deve ser único.\n\n"));
				return;
			}

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
*/