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
#include "SO2_TP_DLL_2021.h"
#include "Estruturas.h"
#include <math.h>

int fechar_aplicacao(Controlo_Avioes* controlo){
	UnmapViewOfFile(controlo->recebe);
	UnmapViewOfFile(controlo->buffer);
	UnmapViewOfFile(controlo->MAPA);

	TerminateThread(controlo->TH_BEEP, 0);
	TerminateThread(controlo->TH_PRODUTOR, 0);
	TerminateThread(controlo->TH_RECEBE_DO_CONTROLADOR, 0);

	CloseHandle(controlo->TH_PRODUTOR);
	CloseHandle(controlo->TH_BEEP);
	CloseHandle(controlo->TH_RECEBE_DO_CONTROLADOR);
	CloseHandle(controlo->H_MTX_PRODUTOR);
	CloseHandle(controlo->EV_FECHAR);
	CloseHandle(controlo->TH_RECEBE_EV_FECHAR);
	controlo->termina = TRUE;
	exit(0);
}

DWORD WINAPI cria_beep(LPVOID param){
	Controlo_Avioes* controlo = (Controlo_Avioes*)param;

	while ( !controlo->termina ){
		if ( _tcscmp(controlo->o_aviao->nome, TEXT("AV_")) != 0 ){
			Sleep(1000);
			SetEvent(controlo->H_EV_BEEP);
		}
	}
	return 0;
}

DWORD WINAPI recebe_ev_fechar(LPVOID param){
	Controlo_Avioes* controlo = (Controlo_Avioes*)param;
	WaitForSingleObject(controlo->EV_FECHAR, INFINITE);
	_tprintf(TEXT("\nO controlador foi fechado\n"));
	fechar_aplicacao(controlo);
	return 0;
}

DWORD WINAPI recebe_do_controlador(LPVOID param){
	Controlo_Avioes* controlo = (Controlo_Avioes*)param;

	while ( !controlo->termina ){
		WaitForSingleObject(controlo->H_EV_C_TO_AV, INFINITE);

		//_tprintf(TEXT("\nChegou algo\n"));
		
		if ( (_tcscmp(controlo->recebe->nome, TEXT("AV_")) != 0) && (controlo->recebe->pid == controlo->o_aviao->pid) ){
			//_tprintf(TEXT("\nLEU PARA ELE\n"));
			WaitForSingleObject(controlo->H_MTX_PRODUTOR, INFINITE);
			CopyMemory(controlo->o_aviao, controlo->recebe, sizeof(Aviao));
			ReleaseMutex(controlo->H_MTX_PRODUTOR);

			if (controlo->o_aviao->action == LOGIN){
				_tprintf(TEXT("\nENTROU NO ESPAÇO AEREO...\n"));
				controlo->o_aviao->action = ESPERANDO;
			}else if(controlo->o_aviao->action == VERIFICA_EXISTE_AEROPORTO){
				if (controlo->o_aviao->viagem.pos_destino.x >= 0 || controlo->o_aviao->viagem.pos_destino.x <= 1000){
					if (controlo->o_aviao->viagem.pos_destino.y >= 0 || controlo->o_aviao->viagem.pos_destino.y <= 1000){
						controlo->o_aviao->action = PRONTO_A_VIAJAR;
					}
				}
			}
		}
	}
	return 0;
}

DWORD WINAPI th_produtor(LPVOID param){
	Controlo_Avioes* controlo = (Controlo_Avioes*)param;

	while ( !controlo->termina ){
		WaitForSingleObject(controlo->H_EV_AV_TO_C, INFINITE);
		if (controlo->o_aviao->action != ESPERANDO){
			// receber id do produtor isto vem de antes (pode ser opcional) mas convem
			//obter dados para enviar

 			WaitForSingleObject(controlo->SEM_PRODUZ, INFINITE);
			WaitForSingleObject(controlo->H_MTX_PRODUTOR, INFINITE);

			//colocar coisas no buffer
			 
			CopyMemory(&controlo->buffer->buffer[controlo->buffer->proxima_pos_escrita], controlo->o_aviao, sizeof(Aviao));

			controlo->buffer->proxima_pos_escrita++;
			if (controlo->buffer->proxima_pos_escrita == TAM_BUFFER_CIRCULAR) {
				controlo->buffer->proxima_pos_escrita = 0;
			}

			ReleaseMutex(controlo->H_MTX_PRODUTOR);

			ReleaseSemaphore(controlo->SEM_CONSOME, 1, NULL);
			controlo->o_aviao->action = ESPERANDO;
		}
	}

	return 0;
}

int configura(Controlo_Avioes* controlo){
	controlo->termina = FALSE;

	//abrir buffer circular
	if ( (controlo->H_FICH_BUFFER_CIRCULAR = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FICH_BUFFER_CIRCULAR) ) == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}
	if ((controlo->buffer = (Buffer_Circular*)MapViewOfFile(controlo->H_FICH_BUFFER_CIRCULAR, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Buffer_Circular))) == NULL){
		_tprintf(TEXT("Deu erro"));
		CloseHandle(controlo->H_FICH_BUFFER_CIRCULAR);
		return 0;
	}


	//cria fich mem partilhada deste aviao
	TCHAR nome[50];
	wsprintf(nome, TEXT("%s_%i"), FICH_MEM_PARTILHADA , controlo->o_aviao->pid);
	if ( (controlo->H_FICH_MEM_PARTILHADA = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Aviao), nome)) == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}
	if ((controlo->recebe = (Aviao*)MapViewOfFile(controlo->H_FICH_MEM_PARTILHADA, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Aviao))) == NULL){
		_tprintf(TEXT("Deu erro"));
		CloseHandle(controlo->H_FICH_MEM_PARTILHADA);
		return 0;
	}

	if ( (controlo->H_FICH_MAPA = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FICH_MEM_PARTILHADA_MAP) ) == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}
	if ((controlo->MAPA = (Localizacao*)MapViewOfFile(controlo->H_FICH_MAPA, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Localizacao*))) == NULL){
		_tprintf(TEXT("Deu erro"));
		CloseHandle(controlo->H_FICH_MAPA);
		return 0;
	}

	if ( (controlo->EV_ATUALIZA_MAPA = OpenEvent(EVENT_ALL_ACCESS, FALSE, H_EV_ATUALIZA_MAPA)) == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	wsprintf(nome, TEXT("%s_%i"), EV_FECHAR_TUDO , controlo->o_aviao->pid);
	if ( (controlo->EV_FECHAR = CreateEvent(NULL, FALSE, FALSE, nome)) == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	if ( (controlo->SEM_CONSOME = OpenSemaphore(SEMAPHORE_ALL_ACCESS, NULL, SEMAFORO_CONSOME)) == NULL ){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	if ( (controlo->SEM_PRODUZ = OpenSemaphore(SEMAPHORE_ALL_ACCESS, NULL, SEMAFORO_PRODUZ)) == NULL ){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	if ( (controlo->H_MTX_PRODUTOR = CreateMutex(NULL, FALSE, NULL)) == NULL ){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	wsprintf(nome, TEXT("%s_%i"), H_EV_CONTROL_TO_AV ,controlo->o_aviao->pid);
	if ((controlo->H_EV_C_TO_AV = CreateEvent(NULL, FALSE, FALSE, nome)) == NULL){		//EVENTO PROPRIO PARA RECEBER
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	wsprintf(nome, TEXT("%s_BEEP_%i"), TEXT("EV") ,controlo->o_aviao->pid);
	if ((controlo->H_EV_BEEP = CreateEvent(NULL, FALSE, FALSE, nome)) == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	if ((controlo->H_EV_AV_TO_C = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL){		//EVENTO LOCAL
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	controlo->TH_PRODUTOR = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)th_produtor , controlo, 0, 0);
	controlo->TH_BEEP = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cria_beep , controlo, 0, 0);
	controlo->TH_RECEBE_DO_CONTROLADOR = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recebe_do_controlador , controlo, 0, 0);
	controlo->TH_RECEBE_EV_FECHAR = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recebe_ev_fechar , controlo, 0, 0);
	if (controlo->TH_PRODUTOR == NULL || controlo->TH_BEEP == NULL || controlo->TH_RECEBE_DO_CONTROLADOR == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	return 1;
}

BOOLEAN verifica_pos_ocupada(Controlo_Avioes* controlo, Localizacao* prox){		//true - ocupada / false - livre
	BOOLEAN ocupado = FALSE;
	BOOLEAN termina_ciclo = FALSE;
	int i = 0;

	//while ( (controlo->MAPA[i].x >= ERRO && controlo->MAPA[i].x <= 1000) && (controlo->MAPA[i].y >= ERRO && controlo->MAPA[i].y <= 1000) ){
	while ( !termina_ciclo ){
		if ( controlo->MAPA[i + 1].x == ERRO && controlo->MAPA[i + 1].y == ERRO && controlo->MAPA[i].x != 0 && controlo->MAPA[i].y != 0 ){
			termina_ciclo = TRUE;
		}
		if ( prox->x == controlo->MAPA[i].x && prox->y == controlo->MAPA[i].y){
			ocupado = TRUE;
		}
		i++;
	}
	
	return ocupado;
}

void interpreta_comandos(Controlo_Avioes* controlo){
	TCHAR comando[50];
	while (1){
		_tprintf(TEXT("\nComando: "));
		_tscanf(TEXT("%s"), comando);

		if (_tcscmp(comando, TEXT("destino")) == 0){
			_tprintf(TEXT("\nDigite o aeroporto destino: "));
			_tscanf(TEXT("%s"), controlo->o_aviao->viagem.destino);
			controlo->o_aviao->action = VERIFICA_EXISTE_AEROPORTO;
			SetEvent(controlo->H_EV_AV_TO_C);	//depois disto produz algo

		}else if (_tcscmp(comando, TEXT("embarcar")) == 0){
			if ( _tcscmp(controlo->o_aviao->viagem.destino, TEXT("")) != 0){
				controlo->o_aviao->action = EMBARCAR_PASSAGEIROS;
				SetEvent(controlo->H_EV_AV_TO_C);	//depois disto produz algo
				_tprintf(TEXT("\nEmbarcando Passageiros\n"));
			}
		}else if (_tcscmp(comando, TEXT("viajar")) == 0){
			
			if (controlo->o_aviao->action == PRONTO_A_VIAJAR){
				_tprintf(TEXT("A inicia viagem: "));
				Localizacao tem_loc;
				tem_loc.x = controlo->o_aviao->posicao.x;
				tem_loc.y = controlo->o_aviao->posicao.y;

				while (controlo->o_aviao->posicao.x != controlo->o_aviao->viagem.pos_destino.x || controlo->o_aviao->posicao.y != controlo->o_aviao->viagem.pos_destino.y){
					move(controlo->o_aviao->posicao.x, controlo->o_aviao->posicao.y, controlo->o_aviao->viagem.pos_destino.x, controlo->o_aviao->viagem.pos_destino.y, &tem_loc.x, &tem_loc.y);
					
					if ( !verifica_pos_ocupada(controlo, &tem_loc) ){		//aceita proxima pos
						controlo->o_aviao->posicao.x = tem_loc.x;
						controlo->o_aviao->posicao.y = tem_loc.y;
					}else{
						if (tem_loc.x <= 1000){
							tem_loc.x++;
							if ( !verifica_pos_ocupada(controlo, &tem_loc) ){		//anda uma posicao para o lado direito
								controlo->o_aviao->posicao.x = tem_loc.x;
								controlo->o_aviao->posicao.y = tem_loc.y;
							}else{
								tem_loc.x--;
								if ( !verifica_pos_ocupada(controlo, &tem_loc) ){		//anda uma posicao para o lado esquerdo
									controlo->o_aviao->posicao.x = tem_loc.x;
									controlo->o_aviao->posicao.y = tem_loc.y;
								}
							}
						}else{
							if (tem_loc.y <= 1000){
								tem_loc.y++;
								if ( !verifica_pos_ocupada(controlo, &tem_loc) ){		//anda uma posicao para o cima
									controlo->o_aviao->posicao.x = tem_loc.x;
									controlo->o_aviao->posicao.y = tem_loc.y;
								}
							}else{
								tem_loc.y--;
								if ( !verifica_pos_ocupada(controlo, &tem_loc) ){		//anda uma posicao para o cima
									controlo->o_aviao->posicao.x = tem_loc.x;
									controlo->o_aviao->posicao.y = tem_loc.y;
								}
							}
						}
					}

					controlo->o_aviao->em_movimento = TRUE;
					CopyMemory(controlo->recebe, controlo->o_aviao, sizeof(Aviao));
					SetEvent(controlo->EV_ATUALIZA_MAPA);

					_tprintf(TEXT("\nX:%i\ty:%i"), tem_loc.x, tem_loc.y );
					Sleep( ( 1000 / controlo->o_aviao->velocidade ) );
				}
				_tprintf(TEXT("Chegou ao destino..."));
				controlo->o_aviao->em_movimento = FALSE;
				controlo->o_aviao->action = CHEGOU_AO_DESTINO;
				CopyMemory(controlo->recebe, controlo->o_aviao, sizeof(Aviao));
				SetEvent(controlo->H_EV_AV_TO_C);


			}
		}else if (_tcscmp(comando, TEXT("sair")) == 0){
			break;
		}
	}
}

//capacidade, velocidade, aeroporto inicial
int _tmain(int argc, _TCHAR* argv[]) {
	Aviao AV;
	Controlo_Avioes controlo;

	_tprintf(TEXT("\npid: %i\n"), GetCurrentProcessId());

#ifdef UNICODE
	_setmode(_fileno(stdin),_O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif


	if (argc == 1 || argc > 4){
		_tprintf(TEXT("Defina parãmetros de entrada!! Capacidade; Velociade; Aeroporto Inicial "));
		return 0;
	}


	AV.capacidade_pass = _ttoi(argv[1]);
	AV.velocidade = _ttoi(argv[2]);
	wcscpy(AV.ae_base, argv[3]);
	wcscpy(AV.viagem.origem, argv[3] );
	AV.pid = GetCurrentProcessId();
	wcscpy(AV.nome, TEXT("AV_"));
	controlo.o_aviao = &AV;

	//isto porque 1001 fica fora do espaco
	AV.viagem.pos_destino.x = ERRO;
	AV.viagem.pos_destino.y = ERRO;
	

	if (configura(&controlo) != 1) {
		return 0;
	}

	AV.action = LOGIN;
	SetEvent(controlo.H_EV_AV_TO_C);		//fazer com que produza algo

	interpreta_comandos(&controlo);

	return 0;
}
