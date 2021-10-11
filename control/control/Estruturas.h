 #pragma once
#include <tchar.h>

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


//CENAS PARA OS AVIOES
#define FICH_BUFFER_CIRCULAR TEXT("FICH_BUFFER_CIRCULAR")
#define FICH_MEM_PARTILHADA TEXT("FICH_MEM_PARTILHADA")
#define FICH_MEM_PARTILHADA_MAP TEXT("FICH_MEM_PARTILHADA_MAP")
#define MTX_PRODUTOR TEXT("MTX_PRODUTOR")
#define SEMAFORO_CONSOME TEXT("SEM_CONSOME")
#define SEMAFORO_PRODUZ TEXT("SEM_PRODUZ")
#define H_EV_CONTROL_TO_AV TEXT("H_EV_CONTROL_TO_AV")
#define H_EV_ATUALIZA_MAPA TEXT("H_EV_ATUALIZA_MAPA")
#define EV_FECHAR_TUDO TEXT("EV_FECHAR_TUDO")


//CENAS PARA OS PASSAGEIROS
#define PIPE_CONTROLADOR TEXT("\\\\.\\pipe\\PIPE_CONTROLADOR")
#define PIPE_PASS TEXT("\\\\.\\pipe\\PIPE_PASS")


#define MUTEX_GERAL TEXT("MUTEX_GERAL")



#define ERRO -1
#define ESPERA_ILIMITADA -1
#define LOGIN 1
#define SAIR 2
#define VERIFICA_EXISTE_AEROPORTO 2
#define PRONTO_A_VIAJAR 3
#define UPDATE_CORDENADAS 4
#define EMBARCAR_PASSAGEIROS 5
#define CHEGOU_AO_DESTINO 6
#define ENTROU_EM_AVIAO 7
#define PILOTO_REFORMOU_SE 8

#define PEDE_POSICAO 3
#define PERDI_A_PACIENCIA 5
#define MORREU 8

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
    int velocidade, pid;        //velocidade: casas por segundo
    int capacidade_pass, n_passageiros;
    BOOLEAN em_movimento;
    Viagem viagem;          //pos destino, origem e nomes dos mesmos
    Passageiros** pass;
    Localizacao posicao;    //pos atual do aviao
} Aviao;

typedef struct aeroporto {
    TCHAR nome[MAX_LEN_NOME_AEROPORTO];
    int n_estacionados;
    int n_pass_a_espera;
    Aviao** estacionados;    //array estacionados
    Passageiros** pass_a_espera;
    Localizacao posicao;
} Aeroporto;

typedef struct controla_passageiros{
    HANDLE TH_COMUNICA_PASS;
    HANDLE H_FIFO_CONTROLADOR, H_FIFO_PASS;
    HANDLE H_PACIENCIA_VIAJAR;

    BOOLEAN suspendido;

    Passageiros* pass;      //E o recebido e o enviado
} Controlo_Passageiros;

typedef struct buffer_circular {
    int proxima_pos_escrita;
    int proxima_pos_leitura;
    Aviao buffer[TAM_BUFFER_CIRCULAR];
}Buffer_Circular;

typedef struct controla_av{
    HANDLE TH_CONSOME_BUFFER, TH_RESPONDE_AVIAO;
    HANDLE* TH_SINAL_VIDA;
    HANDLE SEM_PRODUZ, SEM_CONSOME;
    HANDLE H_FICH_BUFFER_CIRCULAR, H_FICH_MEM_PARTILHADA;

    BOOLEAN suspendido;

    Buffer_Circular* buffer;
    HANDLE* H_FICH_CADA_AVIAO;      //array dicheiros
    Aviao** envia_cada_aviao;       //array de pontiros para avioes
} Controlo_Avioes;

typedef struct espaco_aereo{
    HANDLE H_MTX_GERAL;

    HANDLE H_FICH_MAPA;
    HANDLE TH_ATUALIZA_MAPA;
    HANDLE EV_ATUALIZA_MAPA;

    Controlo_Passageiros* controla_pass;
    Controlo_Avioes* controla_av;

    BOOLEAN terminar;

    HWND teste;

    int max_avioes, max_aeroportos;
    int n_avioes, n_aeroportos, n_pass;
    Aviao* avioes;
    Aeroporto* aeroportos;
    Passageiros* todos_passageiros;
    Localizacao* MAPA;  //tem todas as posicoes ocupadas por avioes
} Espaco_Aereo;

typedef struct th_consome{
    Espaco_Aereo* ea;
    Buffer_Circular* Buffer;
} Th_Consome;