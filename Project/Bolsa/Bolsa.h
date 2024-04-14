#pragma once

#include <Windows.h>
#include <tchar.h>

#include "Commands.h"
#define NCLIENTES _T("NCLIENTES")
#define STARTING_NUM_OF_NCLIENTES 5

void ExecutaComando(const CMD comando);

void ADDC();

void LISTC();

void STOCK();

void USERS();

void PAUSE();

void CLOSE();

DWORD getNCLIENTES();
