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

#ifdef UNICODE
#define _tfgets fgetws
#else
#define _tfgets fgets
#de
#endif

#define FALSE 0
#define TRUE 1

#define MAX_LEN_NOME_AEROPORTO 10
#define MAX_LEN_NOME_PASS 20
#define MAX_LEN_NOME_AVIAO 20

#define FICH_BUFFER_CIRCULAR TEXT("FICH_BUFFER_CIRCULAR")
#define FICH_MEM_PARTILHADA TEXT("FICH_MEM_PARTILHADA")
#define FICH_MEM_PARTILHADA_MAP TEXT("FICH_MEM_PARTILHADA_MAP")

#define MTX_PRODUTOR TEXT("MTX_PRODUTOR")

#define EV_FECHAR_TUDO TEXT("EV_FECHAR_TUDO")

#define SEMAFORO_CONSOME TEXT("SEM_CONSOME")
#define SEMAFORO_PRODUZ TEXT("SEM_PRODUZ")


#define H_EV_CONTROL_TO_AV TEXT("H_EV_CONTROL_TO_AV")
#define H_EV_ATUALIZA_MAPA TEXT("H_EV_ATUALIZA_MAPA")


#define SEMAFOTO_TODOS_AVIOES TEXT("SEM_TODOS_AVIOES")
#define MUTEX_GERAL TEXT("MUTEX_GERAL")

#define SEM_COMUNICA_AVIOES TEXT("SEM_COMMS_aV")



#define ERRO -1
#define LOGIN 1
#define VERIFICA_EXISTE_AEROPORTO 2
#define PRONTO_A_VIAJAR 3
#define UPDATE_CORDENADAS 4
#define EMBARCAR_PASSAGEIROS 5
#define CHEGOU_AO_DESTINO 6
#define PILOTO_REFORMOU_SE 8


#define BEEP_AVIAO 10
#define ESPERANDO 100


#define TAM_BUFFER_CIRCULAR 5

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

typedef struct aviao {
    int action;
    TCHAR nome[MAX_LEN_NOME_AVIAO];
    TCHAR ae_base[MAX_LEN_NOME_AEROPORTO];
    int velocidade, pid;
    int capacidade_pass, n_passageiros;
    BOOLEAN em_movimento;
    Viagem viagem;          //pos destino, origem e nomes dos mesmos
    Passageiros* pass;
    Localizacao posicao;    //pos atual do aviao
} Aviao;

typedef struct {
    int proxima_pos_escrita;
    int proxima_pos_leitura;
    Aviao buffer[TAM_BUFFER_CIRCULAR];
}Buffer_Circular;

typedef struct controla_av{
    HANDLE TH_PRODUTOR, TH_BEEP, TH_RECEBE_DO_CONTROLADOR;
    HANDLE SEM_PRODUZ, SEM_CONSOME;
    HANDLE H_FICH_BUFFER_CIRCULAR, H_FICH_MEM_PARTILHADA, H_FICH_MAPA;
    HANDLE H_EV_AV_TO_C, H_EV_C_TO_AV, H_EV_BEEP, EV_ATUALIZA_MAPA;

    HANDLE H_MTX_GERAL, H_MTX_PRODUTOR;

    HANDLE TH_RECEBE_EV_FECHAR, EV_FECHAR;

    BOOLEAN termina;
    int action;
    int tam_mapa;

    Localizacao* MAPA;  //tem todas as posicoes ocupadas por avioes
    Aviao* o_aviao;     //O PROPRIO AVIAO
    Aviao* recebe;
    Buffer_Circular* buffer;
} Controlo_Avioes;
