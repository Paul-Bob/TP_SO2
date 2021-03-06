#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "textUI.h"

int verifyPositions(pDATA data, int x, int y, int positions);

void interpretaComandoControlador(TCHAR* command, pDATA data) {
	_puttchar(L'\n');

	TCHAR* action = NULL, *argument = NULL, *extra = NULL;

	//get first word from command line
	action = _tcstok_s(command, L" ", &argument);

	if (action == NULL)
		return;

	if (!_tcscmp(action, TEXT("exit"))) {
		for (int i = 0; i < data->maxAirplanes; i++)
			data->planes[i].velocity = -500;
		_tprintf(TEXT("see ya\n"));
	}
	else if (!_tcscmp(action, TEXT("help"))) {
		if (argument == 0 || _tcslen(argument)){
			_tprintf(L"%s: extra operand '%s' \n\n", action, argument);
			return;
		}
		_tprintf(TEXT("Comandos disponiveis:\n"));
		_tprintf(TEXT("- criar [nome] [x] [y] --> Criar novos aeroportos\n"));
		_tprintf(TEXT("- listar               --> Lista aeroportos, avi?es e passageiros\n"));
		_tprintf(TEXT("- suspender            --> Suspende registo de novos avi?es\n"));
		_tprintf(TEXT("- ativar               --> Ativa registo de novos avi?es\n"));
		_tprintf(TEXT("- exit                 --> Encerra o sistema\n"));
		//D?vida: igual para suspender/ativar, ? suposto entrar num menu ou como fizemos em SO que era sAviaoXpTO aAviaoXpTO sendo o s para suspender e a para ativar?
	}
	else if (!_tcscmp(action, TEXT("listar"))) {
		if (argument == 0 || _tcslen(argument)) {
			_tprintf(L"%s: extra operand '%s' \n\n", action, argument);
			return;
		}
		if (data->nrAirports == 0) {
			_tprintf(TEXT("N?o existe nenhum aeroporto!\n\n"));
			return;
		}
		_tprintf(TEXT("Aeroportos:\n"));
		for (int i = 0; i < data->nrAirports; i++) {
			_tprintf(TEXT("[%3d,%3d] - %s\n"),
				(data->airports + i)->coordinates[0], (data->airports + i)->coordinates[1], (data->airports + i)->name);
			_tprintf(TEXT("---> Passageiros sem avi?o:\n"));
			for (int j = 0; j < data->nrPassengers;j++){
				if(data->passengers[j].planeID == -1 && !_tcscmp(data->passengers[j].origin, data->airports[i].name)) {
				_tprintf(TEXT("    [ - ] - %s %d\n"), data->passengers[j].name, data->passengers[j].planeID);
				}
			}
			_puttchar(L'\n');

			for (int k = 0; k < data->maxAirplanes; k++) {
				if (!_tcscmp(data->planes[k].actualAirport, data->airports[i].name)) {
					_tprintf(TEXT("[%3d,%3d] - Avi?o ID %d\n"), data->planes[k].current.x, data->planes[k].current.y, data->planes[k].planeID);
					_tprintf(TEXT("---> Passageiros embarcados:\n"));
					for (int j = 0; j < data->nrPassengers; j++) {
						if (data->passengers[j].planeID == data->planes[k].planeID) {
							_tprintf(TEXT("    [ - ] - %s\n"), data->passengers[j].name);
						}
					}
				}
			}
		}
		_puttchar(L'\n');

		_tprintf(TEXT("Avioes em voo:\n"));
		for (int i = 0; i < data->maxAirplanes; i++)
			if (data->planes[i].velocity != -1 && !_tcscmp(data->planes[i].actualAirport, _T("Fly"))) {
				_tprintf(TEXT("[%3d,%3d] - Avi?o ID %d\n"), data->planes[i].current.x, data->planes[i].current.y, data->planes[i].planeID);
					_tprintf(TEXT("Aeroporto partida: %s\n"), data->planes[i].departureAirport);
					_tprintf(TEXT("Aeroporto destino: %s\n"), data->planes[i].destinAirport);
					_tprintf(TEXT("---> Passageiros embarcados:\n"));
					for (int j = 0; j < data->nrPassengers; j++) {
						if (data->passengers[j].planeID == data->planes[i].planeID) {
							_tprintf(TEXT("    [ - ] - %s\n"), data->passengers[j].name);
						}
					}
				_puttchar(L'\n');
			}
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
				_tprintf(TEXT("Nr? m?ximo de aeroportos atingido.\n\n"));
				return;
			}

			for (int i = 0; i < data->nrAirports; i++)
				if (!_tcscmp(new.name, data->airports[i].name)) {
					_tprintf(TEXT("Nome de aeroporto deve ser ?nico.\n\n"));
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
	else if (!_tcscmp(action, TEXT("suspender"))) {
		planeRegisterSuspended(data, TRUE);
		_tprintf(TEXT("O registo de avi?es foi suspenso.\n"));
	}
	else if (!_tcscmp(action, TEXT("ativar"))) {
	planeRegisterSuspended(data, FALSE);
	_tprintf(TEXT("O registo de avi?es foi retomado.\n"));
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
		_tprintf(TEXT("J? existe um aeroporto na posi??o [%d,%d]."), x, y);
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
				_tprintf(TEXT("Aeroporto [%d,%d] est? num raio de 10 posi??es...\n"), coluna,linha);
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
				_tprintf(TEXT("Nome de aeroporto deve ser ?nico.\n\n"));
				return;
			}

		int x = -1, y = -1, temp;
		do {
			_tprintf(TEXT("Coordenada X entre 0 e 999 : "));
			while (!_tscanf_s(L"%d", &x)) {
				while ((temp = _gettchar()) != '\n' && temp != EOF);
				_tprintf(TEXT("Insere d?gito entre 0 e 999 : "));
			}
		} while (x < 0 || x >= 1000);

		//limpa stdin
		while ((temp = _gettchar()) != '\n' && temp != EOF);

		do {
			_tprintf(TEXT("Coordenada Y entre 0 e 999 : "));
			while (!_tscanf_s(L"%d", &y)) {
				while ((temp = _gettchar()) != '\n' && temp != EOF);
				_tprintf(TEXT("Insere d?gito entre 0 e 999 : "));
			}
		} while (y < 0 || y >= 1000);

		while ((temp = _gettchar()) != '\n' && temp != EOF);
*/