#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "comandosControlador.h"

void interpretaComandoControlador(TCHAR* comando) {
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
		_tprintf(TEXT("Coming soon\n"));
	}
	else
		_tprintf(TEXT("Comando inexistente\n"));
	_puttchar(L'\n');
}