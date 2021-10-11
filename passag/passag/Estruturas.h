#pragma once
#include <tchar.h>
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

#define PIPE_CONTROLADOR TEXT("\\\\.\\pipe\\PIPE_CONTROLADOR")
#define PIPE_PASS TEXT("\\\\.\\pipe\\PIPE_PASS")

#define MUTEX_GERAL TEXT("MUTEX_GERAL")
#define EV_FECHAR_TUDO TEXT("EV_FECHAR_TUDO")

#define MAX_LEN_NOME_AEROPORTO 10
#define MAX_LEN_NOME_PASS 20

#define TRUE 1
#define FALSE 0

#define VIAJANDO 50

#define ERRO -1
#define ESPERA_ILIMITADA -1
#define LOGIN 1
#define SAIR 2
#define PEDE_POSICAO 3
#define UPDATE_CORDENADAS 4
#define PERDI_A_PACIENCIA 5
#define CHEGOU_AO_DESTINO 6
#define ENTROU_EM_AVIAO 7
#define MORREU 8

typedef struct posicao{
    int x;
    int y;
} Localizacao;

typedef struct viagem {
    Localizacao pos_origem;
    Localizacao pos_destino;
    TCHAR origem[MAX_LEN_NOME_AEROPORTO];
    TCHAR destino[MAX_LEN_NOME_AEROPORTO];
} Viagem;


typedef struct passageiros{
    int action;
    TCHAR nome[MAX_LEN_NOME_PASS];
    BOOLEAN em_viagem;
    Viagem viagem;
    Localizacao posicao;
    int tempo_espera, pid;
}Passageiros;

typedef struct controla_pass{
    HANDLE TH_COMM_CONTROLADOR;
    HANDLE H_FIFO_CONTROL, H_FIFO_PASS;
    HANDLE EV_FECHAR;

    BOOLEAN terminar, viajando, em_aviao;

    HANDLE TH_PACIENCIA, TH_FECHAR;
    Passageiros* pass;
} Controla_Pass;