#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "textUI.h"

int verifyPositionsGUI(pDATA data, int x, int y, int positions, HWND hwndList);

void interpretaComandoControladorGUI(TCHAR* command, pDATA data, HWND hwndList) {
	TCHAR message[500] = _T("");
	SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT(""));

	TCHAR* action = NULL, * argument = NULL, * extra = NULL;

	//get first word from command line
	action = _tcstok_s(command, L" ", &argument);

	if (action == NULL)
		return;

	if (!_tcscmp(action, _TEXT("exit"))) {
		for (int i = 0; i < data->maxAirplanes; i++)
			data->planes[i].velocity = -500;
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("see ya\n"));
	}
	else if (!_tcscmp(action, TEXT("help"))) {
		if (argument == 0 || _tcslen(argument)) {
			_stprintf_s(message, 500, _TEXT("%s: extra operand '%s'"), action, argument);
			SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
			return;
		}
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Comandos disponiveis:\n"));
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("- criar [nome] [x] [y] --> Criar novos aeroportos\n"));
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("- listar                        --> Lista aeroportos, aviões e passageiros\n"));
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("- suspender               --> Suspende o registo de novos aviões\n"));
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("- ativar                      --> Ativa o registo de novos aviões\n"));
		
		//Dúvida: igual para suspender/ativar, é suposto entrar num menu ou como fizemos em SO que era sAviaoXpTO aAviaoXpTO sendo o s para suspender e a para ativar?
	}
	else if (!_tcscmp(action, TEXT("listar"))) {
		if (argument == 0 || _tcslen(argument)) {
			_stprintf_s(message, 500,  _TEXT(" % s: extra operand '%s'"), action, argument);
			SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
			return;
		}
		if (data->nrAirports == 0) {
			SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Não existe nenhum aeroporto!"));
			return;
		}
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Aeroportos:\n"));
		for (int i = 0; i < data->nrAirports; i++){
			_stprintf_s(message, 500, TEXT("[%3d,%3d] - %s\n"),
				(data->airports + i)->coordinates[0], (data->airports + i)->coordinates[1], (data->airports + i)->name);
			SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
		}

		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT(""));

		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Avioes:\n"));
		for (int i = 0; i < data->maxAirplanes; i++)
			if (data->planes[i].velocity != -1) {
				_stprintf_s(message, 500, TEXT("[%3d,%3d] - Avião ID %d"), data->planes[i].current.x, data->planes[i].current.y, data->planes[i].planeID);
				SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
				if (!_tcscmp(data->planes[i].actualAirport, _T("Fly"))) {
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Atualmente em voo!"));
					_stprintf_s(message, 500, TEXT("Aeroporto partida: %s"), data->planes[i].departureAirport);
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
					_stprintf_s(message, 500, TEXT("Aeroporto destino: %s"), data->planes[i].destinAirport);
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
				}
				else
				{
					_stprintf_s(message, 500, TEXT("Atualmente em repouso no aeroporto %s"), data->planes[i].actualAirport);
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT(""));
				}
			}
	}
	else if (!_tcscmp(action, TEXT("criar"))) {

		if (argument == 0 || !_tcslen(argument)) {
			_stprintf_s(message, 500, _TEXT("%s: airport name required"), action);
			SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
			return;
		}
		else {
			airport new;

			TCHAR* name = _tcstok_s(argument, L" ", &argument);
			_tcscpy_s(new.name, NAMESIZE, name);

			if (argument == 0 || !_tcslen(argument)) {
				_stprintf_s(message, 500, _TEXT("%s %s: airport x coordinate required"), action, new.name);
				SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
				return;
			}
			else {
				TCHAR* readInt = NULL;
				readInt = _tcstok_s(argument, L" ", &argument);

 				if (argument == 0 || !_tcslen(readInt)) {
					_stprintf_s(message, 500, _TEXT("%s %s: airport x coordinate required"), action, new.name);
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
					return;
				}

				new.coordinates[X] = _tcstol(readInt, &extra, 10);

				if (_tcslen(extra)) {
					_stprintf_s(message, 500, _TEXT("%s %s %d: '%s' is extra\nTry: '%s %s %d'"), action, new.name, new.coordinates[X], extra, action, new.name, new.coordinates[X]);
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
					return;
				}

				if (new.coordinates[X] < 0 || new.coordinates[X] > 999) {
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)L"X coordinate must be higher or equal to 0 and lower than 1000");
					return;
				}

				readInt = NULL;
				readInt = _tcstok_s(argument, L" ", &argument);

				if (argument == 0 || !readInt || !_tcslen(readInt)) {
					_stprintf_s(message, 500, _TEXT("%s %s %d: airport y coordinate required"), action, new.name, new.coordinates[X]);
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
					return;
				}

				new.coordinates[Y] = _tcstol(readInt, &extra, 10);

				if (_tcslen(extra)) {
					_stprintf_s(message, 500, _TEXT("%s %s %d %d: '%s' is extra\nTry: '%s %s %d %d'\n\n"), action, new.name, new.coordinates[X], new.coordinates[Y], extra, action, new.name, new.coordinates[X], new.coordinates[Y]);
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
					return;
				}

				if (new.coordinates[Y] < 0 || new.coordinates[Y] > 999) {
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)L"Y coordinate must be higher or equal to 0 and lower than 1000");
					return;
				}
			}

			//tcstol can return 0 from error, not sure if 0 from input or error...
			/*if (new.coordinates[X] == 0 || new.coordinates[Y] == 0) {
				TCHAR anwser = L'k', temp;
				TCHAR extraAnwser[200];

				SendMessage(hwndList, LB_ADDSTRING, 0, L"\nAdd Airport\nName: %s\nCoordinates: [%d,%d]\n\nAdd?! [Y/n]", new.name, new.coordinates[X], new.coordinates[Y]);

				while ((anwser != L'y' && anwser != L'Y' && anwser != L'n' && anwser != L'N') || (_tcslen(extraAnwser) != 0)) {
					extraAnwser[0] = '\0';
					_tscanf_s(L" %c", &anwser, 1);
					_tscanf_s(L"%[^\n]", extraAnwser, 200);
				}
				while ((temp = _gettchar()) != '\n' && temp != EOF);
				if (anwser == L'n' || anwser == L'N') {
						SendMessage(hwndList, LB_ADDSTRING, 0, TEXT(""));
					return;
				}
			}*/

			if (data->nrAirports >= data->maxAirports) {
				SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Nrº máximo de aeroportos atingido."));
				return;
			}

			for (int i = 0; i < data->nrAirports; i++)
				if (!_tcscmp(new.name, data->airports[i].name)) {
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)TEXT("Nome de aeroporto deve ser único."));
					return;
				}

			if (verifyPositionsGUI(data, new.coordinates[X], new.coordinates[Y], 80, hwndList)) {
				data->map->matrix[new.coordinates[X]][new.coordinates[Y]] = 2;
				data->airports[data->nrAirports] = new;
				data->nrAirports++;
				_stprintf_s(message, 500, _TEXT("Aeroporto '%s' foi adicionado com sucesso nas coordenadas [%d,%d]"), new.name, new.coordinates[X], new.coordinates[Y]);
				SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
			}
		}
	}
	else if (!_tcscmp(action, TEXT("suspender"))) {
	planeRegisterSuspended(data, TRUE);
	_stprintf_s(message, 500, TEXT("O registo de aviões foi suspenso\n"));
	SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
	
		}
	else if (!_tcscmp(action, TEXT("ativar"))) {
	planeRegisterSuspended(data, FALSE);
	_stprintf_s(message, 500, TEXT("O registo de aviões foi reativado\n"));
	SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
	}

	else {
		_stprintf_s(message, 500, TEXT("Comando '%s' inexistente\n"), action);
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
	}


}

int verifyPositionsGUI(pDATA data, int x, int y, int positions, HWND hwndList) {
	TCHAR message[500] = _T("");
	int lastX, lastY, firstX, firstY;

	if (x < 0 || y < 0 || x >= MAPSIZE || y >= MAPSIZE)
		return 0;

	if (data->map->matrix[x][y] == 2) {
		_stprintf_s(message, 500, TEXT("Já existe um aeroporto na posição [%d,%d]."), x, y);
		SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
		return 0;
	}

	if (x < positions) {
		lastX = positions + x;
		firstX = 0;
	}
	else if (MAPSIZE - 1 - x < positions) {
		lastX = MAPSIZE - 1;
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
	else if (MAPSIZE - 1 - y < positions) {
		lastY = MAPSIZE - 1;
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
				_stprintf_s(message, 500, TEXT("Aeroporto [%d,%d] está num raio de %d posições..."), coluna, linha, positions);
				SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)message);
				return 0;
			}
	return 1;
}