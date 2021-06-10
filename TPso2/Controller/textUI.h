#ifndef COMANDOSCONTROLADOR_H
#define COMANDOSCONTROLADOR_H

#include "../HF/structs.h"

void interpretaComandoControlador(TCHAR* command, pDATA data);
void interpretaComandoControladorGUI(TCHAR* command, pDATA data, HWND hwndList);

#endif
