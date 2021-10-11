#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <tchar.h>
#include <fcntl.h>
#include <windows.h>
#include <io.h> // biblioteca a adicionar para uinicde
#include <iostream>
#include <string>

#include "Estruturas.h"

void sair_aplicacao(Controla_Pass* control){
    control->terminar = TRUE;
    DisconnectNamedPipe(control->H_FIFO_PASS);
    CloseHandle(control->H_FIFO_CONTROL);
    CloseHandle(control->H_FIFO_PASS);
    CloseHandle(control->TH_COMM_CONTROLADOR);
    CloseHandle(control->TH_PACIENCIA);
    exit(0);
}

BOOLEAN envia_ao_controlador(Controla_Pass* controla){
    Passageiros Pass;
    CopyMemory(&Pass, controla->pass, sizeof(Passageiros));
    if (WaitNamedPipe(PIPE_CONTROLADOR, NMPWAIT_WAIT_FOREVER) != 0) {
        controla->H_FIFO_CONTROL = CreateFile(PIPE_CONTROLADOR, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (WriteFile(controla->H_FIFO_CONTROL, &Pass, sizeof(Passageiros), NULL, NULL) != 0) {
        DisconnectNamedPipe(controla->H_FIFO_CONTROL);
        CopyMemory(controla->pass, &Pass, sizeof(Passageiros));
        return TRUE;
    }
    CopyMemory(controla->pass, &Pass, sizeof(Passageiros));
    return FALSE;
}

DWORD WINAPI Paciencia (LPVOID param){
    Controla_Pass* controla = (Controla_Pass*)param;
    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);

    LARGE_INTEGER tempo;
    tempo.QuadPart = (LONGLONG)-((controla->pass->tempo_espera) * 10000000);

    SetWaitableTimer(timer, &tempo, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    _tprintf(TEXT("ESPEROU"));

    if ( controla->em_aviao == FALSE){
        controla->pass->action = PERDI_A_PACIENCIA;
        envia_ao_controlador(controla);
        sair_aplicacao(controla);
    }

    return 0;
}

DWORD WINAPI Th_espera_fechar(LPVOID param){
    Controla_Pass* controla = (Controla_Pass*)param;
    WaitForSingleObject(controla->EV_FECHAR, INFINITE);
    _tprintf(TEXT("\nO controlador fo fechado\n"));
    sair_aplicacao(controla);
    return 0;
}

DWORD WINAPI TH_comm_recebe (LPVOID param){
    Controla_Pass* controla = (Controla_Pass*)param;
    Passageiros recebe_pass;
    controla->pass = &recebe_pass;

    while ( !controla->terminar ){
        ConnectNamedPipe(controla->H_FIFO_PASS, NULL);

        if ( ReadFile(controla->H_FIFO_PASS, &recebe_pass, sizeof(Passageiros), NULL, NULL) != 0 ) {
            if (recebe_pass.action == LOGIN){
                if (recebe_pass.tempo_espera != ESPERA_ILIMITADA){
                    controla->TH_PACIENCIA = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Paciencia, controla, 0, NULL);
                }
                _tprintf(TEXT("A partir de agora está à espera que exista um avião com o destino que selecionou"));
            }
            else if (recebe_pass.action == PERDI_A_PACIENCIA) {
                _tprintf(TEXT("Perdi a paciencia"));
                _tprintf(TEXT("\nO programa vai desligar"));
                sair_aplicacao(controla);

            }else if (recebe_pass.action == PEDE_POSICAO){
                _tprintf(TEXT("\nEstou em x: %i | e y: %i\n"), recebe_pass.posicao.x, recebe_pass.posicao.y);

            }else if (recebe_pass.action == CHEGOU_AO_DESTINO){
                _tprintf(TEXT("\nChegou ao destino"));
                _tprintf(TEXT("\nO programa vai desligar"));
                sair_aplicacao(controla);

            }else if (recebe_pass.action == MORREU){
                _tprintf(TEXT("\nO aviao teve um acidente"));
                sair_aplicacao(controla);
            }else if (recebe_pass.action == SAIR){
                _tprintf(TEXT("Ocorreu um erro\nO programa vai desligar"));
                sair_aplicacao(controla);
            }
        }
        FlushFileBuffers(controla->H_FIFO_PASS);
        DisconnectNamedPipe(controla->H_FIFO_PASS);
    }

    CloseHandle(controla->H_FIFO_PASS);
    return 0;
}

int configura(Controla_Pass* controla){
    controla->terminar = FALSE;
    controla->viajando = FALSE;
    controla->em_aviao = FALSE;

/*
    if ( OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_GERAL) == NULL ){
        _tprintf(TEXT("O controlador não está a funcionar"));
        return ERRO;
    }
*/

    TCHAR nome[50];
    wsprintf(nome, TEXT("%s_%i"), PIPE_PASS, controla->pass->pid);
    controla->H_FIFO_PASS = CreateNamedPipe(nome, PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, sizeof(Passageiros), sizeof(Passageiros), 0, NULL );
    
    if (controla->H_FIFO_PASS == INVALID_HANDLE_VALUE){
        return ERRO;
    }

    wsprintf(nome, TEXT("%s_%i"), EV_FECHAR_TUDO , controla->pass->pid);
    if ( (controla->EV_FECHAR = CreateEvent(NULL, FALSE, FALSE, nome)) == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

    controla->TH_FECHAR = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Th_espera_fechar, controla, 0, NULL);
    controla->TH_COMM_CONTROLADOR = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TH_comm_recebe, controla, 0, NULL);
    if (controla->TH_COMM_CONTROLADOR == NULL){
        return ERRO;
    }

    envia_ao_controlador(controla);     //envia para efetuar o login

    return 1;
}


int _tmain(int argc, TCHAR* argv[]) {
    
#ifdef UNICODE
	_setmode( _fileno(stdin), _O_WTEXT);
	_setmode( _fileno(stdout), _O_WTEXT);
	_setmode( _fileno(stderr), _O_WTEXT);
#endif

    Passageiros pass;
    Controla_Pass controla;

    if (argc > 0 && argc <= 5){
        wcscpy(pass.viagem.origem, argv[1]);
        wcscpy(pass.viagem.destino, argv[2]);
        //wsprintf(pass.nome, TEXT("pass_%i"), GetCurrentProcessId());
        wcscpy(pass.nome, argv[3]);
    
        if (argc > 4){
            pass.tempo_espera = _ttoi(argv[4]);
        }else{
            _tprintf(TEXT("%i"), (argc + 1));
            pass.tempo_espera = ESPERA_ILIMITADA;
        }
    
    }else{
        _tprintf(TEXT("Erro ao introduzir argumentos de entrada"));
          return 0;
    }

    pass.pid = GetCurrentProcessId();
    _tprintf(TEXT("\npid: %i\n"), GetCurrentProcessId());
    pass.posicao.x = ERRO;
    pass.posicao.y = ERRO;
    controla.pass = &pass;
    pass.action = LOGIN;

    //pass.tempo_espera = 10;
    //pass.tempo_espera = ESPERA_ILIMITADA;


    if ( _tcscmp(pass.viagem.origem, pass.viagem.destino) == 0 ){
        _tprintf(TEXT("Você já se encontra no seu destino"));
        return 0;
    }


    if ( configura(&controla) == ERRO ){
        _tprintf(TEXT("\nOcorreu um erro na configuração do programa...\nO programa vai fechar!"));
        return 0;
    }


    while ( !controla.terminar ){
        TCHAR comando[50];
        _tprintf(TEXT("\nconmando: "));
        _tscanf(TEXT("%s"), comando);
        if ( _tcscmp(comando, TEXT("sair")) == 0){
            controla.pass->action = SAIR;
            if (WaitNamedPipe(PIPE_CONTROLADOR, NMPWAIT_WAIT_FOREVER) != 0) {
                envia_ao_controlador(&controla);
            }
            controla.terminar = TRUE;
            sair_aplicacao(&controla);
        }else if (_tcscmp(comando, TEXT("pos")) == 0){
            controla.pass->action  = PEDE_POSICAO;
            if (WaitNamedPipe(PIPE_CONTROLADOR, NMPWAIT_WAIT_FOREVER) != 0) {
                envia_ao_controlador(&controla);
            }
        }
    }

	return 0;
}