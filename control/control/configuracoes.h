#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <tchar.h>
#include <fcntl.h>
#include <windows.h>
#include <io.h> // biblioteca a adicionar para uinicde
#include "Estruturas.h"
#include <math.h>

void imprime_aeroportos(Aeroporto* , int );
void imprime_avioes(Aviao* , int );
int verifica_existe_aviao(Aviao* , Espaco_Aereo* );
int verifica_existe_aeroporto(Espaco_Aereo*  ,TCHAR* );
int configura_tudo(Espaco_Aereo*, Controlo_Avioes*, Controlo_Passageiros*);
BOOLEAN verifica_distancia_entre_pontos(int, int, int, int);
void cria_aeroporto(Espaco_Aereo*, Aeroporto*);
int interpreta_comandos(Espaco_Aereo*);
void organiza_passageiros_aviao(Aviao*);
void organiza_espera_aeroporto(Espaco_Aereo*);
BOOLEAN envia_ao_pass (Controlo_Passageiros*);
int envia_evento(int, const wchar_t*);

void fechar_controlador(Espaco_Aereo*);

DWORD WINAPI Th_consome_avioes(LPVOID);
DWORD WINAPI Th_atualiza_mapa(LPVOID);
DWORD WINAPI Th_comunica_pass(LPVOID);