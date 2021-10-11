#include "configuracoes.h"
#include <vector>

BOOLEAN verifica_distancia_entre_pontos(int x1, int x2, int y1, int y2){
	if ( floor( sqrt((x2-x1)*(x2-x1))+((y2-y1)*(y2-y1)) ) >= 10 ){
		return 1;
	}
	return 0;
}

void imprime_aeroportos(Aeroporto* aeroportos, int n_aeroportos){
	for (int i = 0; i < n_aeroportos; i++){
		_tprintf(TEXT("\n%s x: %i | y: %i"), aeroportos[i].nome, aeroportos[i].posicao.x, aeroportos[i].posicao.y);
	}
}

void imprime_avioes(Aviao* av, int n_avioes){
	for (int i = 0; i < n_avioes; i++){
		_tprintf(TEXT("\nnome: %s\t posições: x:%i y:%i"), av[i].nome, av[i].posicao.x, av[i].posicao.y);
	}
}

void imprime_passageiros(Espaco_Aereo* ea){
	for (int i = 0; i < ea->n_aeroportos; i++){
		if (ea->aeroportos[i].n_pass_a_espera > 0){
			_tprintf(TEXT("\n\nPassageiros à espera no aeroporto: %s\n"), ea->aeroportos[i].nome);
			for (int j = 0; j < ea->aeroportos[i].n_pass_a_espera; j++){
				_tprintf(TEXT("\nPassageiro: %s | origem: %s | destino: %s ."), ea->aeroportos[i].pass_a_espera[j]->nome, ea->aeroportos[i].pass_a_espera[j]->viagem.origem, ea->aeroportos[i].pass_a_espera[j]->viagem.destino );
			}
		}
	}

	for (int i = 0; i < ea->n_avioes; i++){
		if (ea->avioes[i].n_passageiros > 0){
			_tprintf(TEXT("\n\nPassageiros no avião: %s\n"), ea->avioes[i].nome);
			for (int j = 0; j < ea->avioes[i].capacidade_pass; j++){
				if (ea->avioes[i].pass[j] != NULL && j <= (ea->avioes[i].n_passageiros - 1)){
					_tprintf(TEXT("\nPassageiro: %s | origem: %s | destino: %s ."), ea->avioes[i].pass[j]->nome, ea->avioes[i].pass[j]->viagem.origem, ea->avioes[i].pass[j]->viagem.destino);
					_tprintf(TEXT(" x: %i | y: %i"), ea->avioes[i].pass[j]->posicao.x, ea->avioes[i].pass[j]->posicao.y);
				}
			}
		}
	}
}

int verifica_aviao_estacionado(Espaco_Aereo* ea, Aviao* av){
	for (int i = 0; i < ea->n_aeroportos; i++){
		if (_tcscmp(ea->aeroportos[i].nome, av->viagem.origem) == 0){
			for (int j = 0; j < ea->aeroportos[i].n_estacionados; j++){
				if ( ea->aeroportos[i].estacionados[j]->pid == av->pid){
					return j;
				}
			}
		}
	}
	return ERRO;
}

int verifica_existe_aviao(Aviao* av, Espaco_Aereo* ea){	//-1 - nao existe
	for (int i = 0; i < ea->n_avioes; i++){
		if (av->pid == ea->avioes[i].pid){
			return i;
		}
	}
	return ERRO;
}

int verifica_existe_aeroporto(Espaco_Aereo* ea ,TCHAR* nome){
	if (ea->n_aeroportos > 0){
		for (int i = 0; i < ea->n_aeroportos; i++){
			if ( _tcscmp(nome, ea->aeroportos[i].nome) == 0 ){
				return i;
			}
		}
	}
	return ERRO;
}

BOOLEAN verifica_existe_pass( Espaco_Aereo* ea, Passageiros* pass, int opcao, int* iter_i, int* iter_j){	// 0 - aeroporto 1 - aviao
	if (opcao == 0){		//verificar no aeroporto
		int iter_ap = verifica_existe_aeroporto(ea, pass->viagem.origem);
		if (iter_ap != ERRO){
			for (int i = 0; i < ea->aeroportos[iter_ap].n_pass_a_espera; i++){
				if (ea->aeroportos[iter_ap].pass_a_espera[i]->pid == pass->pid){
					*iter_i = iter_ap;
					*iter_j = i;
					return TRUE;
				}
			}
		}
	}else if (opcao == 1){	//verificar no aviao
		for (int i = 0; i < ea->n_avioes; i++){
			for (int j = 0; j < ea->avioes[i].capacidade_pass; j++){
				if (j <= (ea->avioes[i].n_passageiros - 1)){
					if (ea->avioes[i].pass[j]->pid == pass->pid){
						*iter_i = i;
						*iter_j = j;
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

void liberta_posicoes_mapa(Espaco_Aereo* ea){
	for (int i = 0; i < ea->max_avioes; i++){
		for (int j = 0; j < ea->n_aeroportos; j++){
			if ( ea->MAPA[i].x == ea->aeroportos[j].posicao.x && ea->MAPA[i].y == ea->aeroportos[j].posicao.y ){
				ea->MAPA[i].x = ERRO;
				ea->MAPA[i].y = ERRO;
			}
		}
	}	
}

BOOLEAN remover_aviao(Espaco_Aereo* ea, Aviao* av){
	std::vector<Aviao*> vector_av;
	std::vector<Aviao*> vector_envia_cada_av;
	
	for (int i = 0; i < ea->n_avioes; i++){
		vector_av.push_back( &ea->avioes[i] );
		vector_envia_cada_av.push_back( ea->controla_av->envia_cada_aviao[i] );
	}

	for (int i = 0; i < (int)vector_av.size(); i++){
		if ( vector_av[i]->pid == av->pid){
			vector_av.erase( vector_av.begin() + i );
			vector_envia_cada_av.erase( vector_envia_cada_av.begin() + i );
			ZeroMemory(&ea->avioes[i], sizeof(Aviao));
			ea->n_avioes--;
		}
	}

	for (int i = 0; i < ea->n_avioes; i++){
		CopyMemory( &ea->avioes[i], vector_av[i], sizeof(Aviao));
		CopyMemory( ea->controla_av->envia_cada_aviao[i], vector_envia_cada_av[i], sizeof(Aviao*));
	}
	
	return TRUE;
}

BOOLEAN remover_passageiro(Espaco_Aereo* ea, Passageiros* pass){
	std::vector<Passageiros*> vector_pass;

	for (int i = 0; i < ea->n_pass; i++){
		vector_pass.push_back( &ea->todos_passageiros[i] );
	}

	for (int i = 0; i < (int)vector_pass.size(); i++){
		if ( vector_pass[i]->pid == pass->pid ){
			vector_pass.erase( vector_pass.begin() + i );
			ea->n_pass--;
		}
	}

	if ( (int)vector_pass.size() == 0 ){
		ZeroMemory(&ea->todos_passageiros[0], sizeof(Passageiros));
		ea->todos_passageiros = (Passageiros*)realloc(ea->todos_passageiros, sizeof(Passageiros));
	}else{
		ea->todos_passageiros = (Passageiros*)realloc(ea->todos_passageiros, sizeof(Passageiros) * (int)vector_pass.size());
		for (int i = 0; i < ea->n_pass; i++) {
			CopyMemory(&ea->todos_passageiros[i], vector_pass[i], sizeof(Passageiros));
		}
	}

	return TRUE;
}

void fechar_controlador(Espaco_Aereo* ea){
	ea->terminar = TRUE;
	
	_tprintf(TEXT("Todos os programas vão fechar"));
	for (int i = 0; i < ea->n_avioes; i++){
		envia_evento(ea->avioes[i].pid, EV_FECHAR_TUDO);
		UnmapViewOfFile(ea->controla_av->envia_cada_aviao[i]);
		CloseHandle(ea->controla_av->TH_SINAL_VIDA[i]);
		CloseHandle(ea->controla_av->H_FICH_CADA_AVIAO[i]);
	}

	for (int i = 0; i < ea->n_pass; i++){
		envia_evento(ea->todos_passageiros[i].pid, EV_FECHAR_TUDO);
	}

	CloseHandle(ea->controla_pass->TH_COMUNICA_PASS);
	CloseHandle(ea->controla_pass->H_FIFO_PASS);
	CloseHandle(ea->controla_pass->H_FIFO_CONTROLADOR);
	CloseHandle(ea->controla_pass->H_PACIENCIA_VIAJAR);

	UnmapViewOfFile(ea->controla_av->buffer);

	CloseHandle(ea->controla_av->TH_CONSOME_BUFFER);
	CloseHandle(ea->controla_av->TH_RESPONDE_AVIAO);
	CloseHandle(ea->controla_av->SEM_CONSOME);
	CloseHandle(ea->controla_av->SEM_PRODUZ);
	CloseHandle(ea->controla_av->H_FICH_MEM_PARTILHADA);
	CloseHandle(ea->controla_av->H_FICH_BUFFER_CIRCULAR);

	UnmapViewOfFile(ea->MAPA);
	CloseHandle(ea->H_MTX_GERAL);
	CloseHandle(ea->H_FICH_MAPA);
	CloseHandle(ea->TH_ATUALIZA_MAPA);
	CloseHandle(ea->EV_ATUALIZA_MAPA);

	//exit(0);
}

int configura_tudo(Espaco_Aereo* ea, Controlo_Avioes* controla_avioes, Controlo_Passageiros* controlo_pass){
/*
	ea->controla_av = controla_avioes;
	ea->controla_pass = controlo_pass;
*/
	ea->todos_passageiros = NULL;
	ea->controla_pass->suspendido = FALSE;
	ea->controla_av->suspendido = FALSE;

	ea->n_pass = 0;
	ea->n_avioes = 0;
	ea->n_aeroportos = 0;
	ea->terminar = FALSE;

	controla_avioes->TH_SINAL_VIDA = (HANDLE*)malloc(sizeof(HANDLE) * ea->max_avioes);
	controla_avioes->H_FICH_CADA_AVIAO = (HANDLE*)malloc(sizeof(HANDLE) * ea->max_avioes);
	controla_avioes->envia_cada_aviao = (Aviao**)malloc(sizeof(Aviao*) * ea->max_avioes);

	ea->aeroportos = (Aeroporto*)malloc(sizeof(Aeroporto) * ea->max_aeroportos);
	ea->avioes = (Aviao*)malloc(sizeof(Aviao) * ea->max_avioes);

	if (ea->aeroportos == NULL || ea->avioes == NULL){
		_tprintf(TEXT("Erro ao alocar memoria... Programa vai desligar!"));
		return 0;
	}

	//configura buffer circular
	if ((controla_avioes->H_FICH_BUFFER_CIRCULAR = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Buffer_Circular), FICH_BUFFER_CIRCULAR)) == NULL){
		_tprintf(TEXT("Erro ao mapear o ficheiro"));
		return 0;
	}else if (GetLastError() == ERROR_ALREADY_EXISTS){
		_tprintf(TEXT("Já existe um controlador a funcionar"));
		return 0;
	}

	if ( (controla_avioes->buffer = (Buffer_Circular*)MapViewOfFile(controla_avioes->H_FICH_BUFFER_CIRCULAR, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Buffer_Circular))) == NULL ){
		_tprintf(TEXT("Erro ao criar a view do ficheiro"));
		CloseHandle(controla_avioes->H_FICH_BUFFER_CIRCULAR);
		return 0;
	}
	controla_avioes->buffer->proxima_pos_escrita = 0;
	controla_avioes->buffer->proxima_pos_leitura = 0;

	//configura mem partilhada para o mapa / partilha da localização de posicoes livres
	if ((ea->H_FICH_MAPA = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Localizacao) * ea->max_avioes, FICH_MEM_PARTILHADA_MAP)) == NULL){
		_tprintf(TEXT("Erro ao mapear o ficheiro"));
		return 0;
	}
	if ( (ea->MAPA = (Localizacao*)MapViewOfFile(ea->H_FICH_MAPA, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Localizacao) * ea->max_avioes)) == NULL ){
		_tprintf(TEXT("Erro ao criar a view do ficheiro"));
		CloseHandle(ea->H_FICH_MAPA);
		return 0;
	}

	if ( (ea->EV_ATUALIZA_MAPA = CreateEvent(NULL, FALSE, FALSE, H_EV_ATUALIZA_MAPA)) == NULL ){
		_tprintf(TEXT("Erro ao criar o evento para atualizar o mapa do espaço aereo"));
		return 0;
	}

	if ( (ea->H_MTX_GERAL = CreateMutex(NULL, FALSE, MUTEX_GERAL)) == NULL){
		_tprintf(TEXT("Erro ao criar o mutex"));
		return 0;
	}

	if ( (controla_avioes->SEM_CONSOME = CreateSemaphore(NULL, 0, TAM_BUFFER_CIRCULAR, SEMAFORO_CONSOME)) == NULL){
		_tprintf(TEXT("Erro ao criar o semáforo consumidor"));
		return 0;
	}

	if ( (controla_avioes->SEM_PRODUZ = CreateSemaphore(NULL, TAM_BUFFER_CIRCULAR, TAM_BUFFER_CIRCULAR, SEMAFORO_PRODUZ)) == NULL){
		_tprintf(TEXT("Erro ao criar o semáforo produtor"));
		return 0;
	}

	controla_avioes->TH_CONSOME_BUFFER = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Th_consome_avioes, ea, 0, NULL);
	ea->TH_ATUALIZA_MAPA = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Th_atualiza_mapa, ea, 0, NULL);
	if (controla_avioes->TH_CONSOME_BUFFER == NULL || ea->TH_ATUALIZA_MAPA == NULL) {
		_tprintf(TEXT("Erro ao criar a thread!"));
		return 0;
	}

	//pipe do controlador para receber mensages de todos os passageiros
	controlo_pass->H_FIFO_CONTROLADOR = CreateNamedPipe(PIPE_CONTROLADOR, PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, sizeof(Passageiros), sizeof(Passageiros), 0, NULL );
	if (controlo_pass->H_FIFO_CONTROLADOR == INVALID_HANDLE_VALUE){
		return 0;
	}

	controlo_pass->TH_COMUNICA_PASS = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Th_comunica_pass, ea, 0, NULL);
	if (controlo_pass->TH_COMUNICA_PASS == NULL){
		_tprintf(TEXT("Erro ao criar a thread!"));
		return 0;
	}

	return 1;
}

void cria_aeroporto(Espaco_Aereo* ea, Aeroporto* ae_p){
	Aeroporto ae;
	if (ae_p != NULL){
		CopyMemory(&ae, ae_p, sizeof(Aviao));
	}
	
	_tprintf(TEXT("\nNome Aeroporto: "));
	_tscanf(TEXT("%s"), ae.nome);

	_tprintf(TEXT("\nLocalização x e y: "));
	_tscanf(TEXT("%d %d"), &ae.posicao.x, &ae.posicao.y);

	if (ae.posicao.x > 1000 || ae.posicao.x < 0 || ae.posicao.y > 1000 || ae.posicao.y < 0) {
		return;
	}

	ae.n_pass_a_espera = 0;
	ae.n_estacionados = 0;
	ae.estacionados = NULL;
	ae.pass_a_espera = NULL;

	if (ea->n_aeroportos > 0){
		for (int i = 0; i < ea->n_aeroportos; i++){
			if ( verifica_distancia_entre_pontos(ea->aeroportos[i].posicao.x, ae.posicao.x, ea->aeroportos[i].posicao.y, ae.posicao.y) ){
				if ( verifica_existe_aeroporto(ea, ae.nome) == -1 ){
					CopyMemory(&ea->aeroportos[ea->n_aeroportos], &ae, sizeof(Aeroporto));
					ea->n_aeroportos++;
					break;
				}
			}else{
				_tprintf(TEXT("ERRO AO ADICIONAR AEROPORTO"));
				return;
			}
		}
	}else{
		CopyMemory(&ea->aeroportos[ea->n_aeroportos], &ae, sizeof(Aeroporto));
		ea->n_aeroportos++;
	}
	InvalidateRect(ea->teste, NULL, TRUE);
}

int interpreta_comandos(Espaco_Aereo* ea){
	TCHAR comando[50];
	while (1){
		_tprintf(TEXT("\nIntroduza um comando: "));
		_tscanf(TEXT("%s"), comando);
		if ( _tcscmp(comando, TEXT("criaAp")) == 0){			//criaa - cria aeroporto

			if (ea->max_aeroportos == ea->n_aeroportos){
				_tprintf(TEXT("Não é possivel adicionar mais aeroportos"));
			}else{
				cria_aeroporto(ea, NULL);
			}
		}else if (_tcscmp(comando, TEXT("verAp")) == 0){		//Imprimir aeroportos
			if (ea->n_aeroportos == 0){
				_tprintf(TEXT("Não existem aeroportos no sistema!"));
			}else{
				imprime_aeroportos(ea->aeroportos, ea->n_aeroportos);
			}
		}else if (_tcscmp(comando, TEXT("verAv")) == 0) {		//imprime avioes registados
			if (ea->n_avioes == 0) {
				_tprintf(TEXT("Não existem aviões no sistema!"));
			}else {
				imprime_avioes(ea->avioes, ea->n_avioes);
			}
		}else if (_tcscmp(comando, TEXT("verPass")) == 0) {
			if (ea->n_pass > 0){
				imprime_passageiros(ea);
			}else{
				_tprintf(TEXT("Não existem passageiros no sistema!"));
			}
		}else if( _tcscmp(comando, TEXT("suspende_A")) == 0 ){
			if ( ea->controla_av->suspendido == TRUE ){
				ea->controla_av->suspendido = FALSE;
			}else{
				ea->controla_av->suspendido = TRUE;
			}
		}else if( _tcscmp(comando, TEXT("suspende_P")) == 0 ){
			if (ea->controla_pass->suspendido == TRUE){
				ea->controla_pass->suspendido = FALSE;
			}else{
				ea->controla_pass->suspendido = TRUE;
			}
		}else if ( _tcscmp(comando, TEXT("sair")) == 0 ){
			fechar_controlador(ea);
		}
	}
}

void organiza_aeroporto(Espaco_Aereo* ea, Aeroporto* ap, BOOLEAN pass_a_espera){
	if ( pass_a_espera ){
		std::vector<Passageiros**> vetor;
		for (int i = 0; i < ap->n_pass_a_espera; i++){
			if ( !ap->pass_a_espera[i]->em_viagem ){
				vetor.push_back( &ap->pass_a_espera[i] );
			}
		}

		ap->n_pass_a_espera = (int)vetor.size();
		if (ap->n_pass_a_espera == 0){
			ap->pass_a_espera = (Passageiros**)realloc(ap->pass_a_espera, sizeof(Passageiros*));
		}else{
			ap->pass_a_espera = (Passageiros**)realloc(ap->pass_a_espera, sizeof(Passageiros*) * ap->n_pass_a_espera);
		}

		for (int i = 0; i < ap->n_pass_a_espera; i++){
			CopyMemory(ap->pass_a_espera[i], vetor[i], sizeof(Passageiros*));
		}


	}else{
		std::vector<Aviao**> vetor;
		for (int i = 0; i < ap->n_estacionados; i++){
			if ( !ap->estacionados[i]->em_movimento ){
				vetor.push_back( &ap->estacionados[i] );
			}
		}

		ap->n_estacionados = (int)vetor.size();
		if ( ap->n_estacionados == 0 ){
			ap->estacionados = (Aviao**)realloc(ap->estacionados, sizeof(Aviao*));
		}else{
			ap->estacionados = (Aviao**)realloc(ap->estacionados, sizeof(Aviao*) * ap->n_estacionados);
		}

		for (int i = 0; i < ap->n_estacionados; i++){
			CopyMemory(ap->estacionados[i], vetor[i], sizeof(Aviao*));
		}

	}
}

BOOLEAN regista_aviao(Espaco_Aereo* ea, Aviao* novo_av){
	int iter_ap = verifica_existe_aeroporto(ea, novo_av->ae_base);
	if (iter_ap != -1) {
		_tprintf(TEXT("Avião em condições para ser adicionado"));

		wsprintf(novo_av->nome, TEXT("AV_%i"), novo_av->pid);
		novo_av->em_movimento = FALSE;
		novo_av->posicao.x = ea->aeroportos[iter_ap].posicao.x;
		novo_av->posicao.y = ea->aeroportos[iter_ap].posicao.y;
		wcscpy(novo_av->viagem.origem, novo_av->ae_base);
		novo_av->viagem.pos_origem.x = novo_av->posicao.x;
		novo_av->viagem.pos_origem.y = novo_av->posicao.y;
		novo_av->n_passageiros = 0;

		CopyMemory(&ea->avioes[ea->n_avioes], novo_av, sizeof(Aviao));
		ea->avioes[ea->n_avioes].pass = (Passageiros**)malloc(sizeof(Passageiros*) * novo_av->capacidade_pass);

		ea->aeroportos[iter_ap].estacionados = (Aviao**)realloc(ea->aeroportos[iter_ap].estacionados, sizeof(Aviao*) * (ea->aeroportos[iter_ap].n_estacionados + 1));
		ea->aeroportos[iter_ap].estacionados[ea->aeroportos[iter_ap].n_estacionados] = &ea->avioes[ea->n_avioes];

		ea->n_avioes++;
		ea->aeroportos[iter_ap].n_estacionados++;

		return TRUE;
	}
	return FALSE;
}

int envia_evento(int pid, const wchar_t* nome_evento){
	HANDLE este_handle;

	TCHAR nome[100];
	wsprintf(nome, TEXT("%s_%i"), nome_evento , pid);
	
	if ((este_handle = OpenEvent(EVENT_ALL_ACCESS, FALSE, nome)) == NULL){
		_tprintf(TEXT("Deu erro"));
		return 0;
	}

	SetEvent(este_handle);
	return 0;
}

DWORD WINAPI Th_atualiza_mapa(LPVOID param){
	Espaco_Aereo* ea = (Espaco_Aereo*)param;

	for (int i = 0; i < ea->max_avioes; i++){
		ea->MAPA[i].x = ERRO;
		ea->MAPA[i].y = ERRO;
	}

	while ( !ea->terminar ){							//enquanto nao for para parar o programa
		WaitForSingleObject(ea->EV_ATUALIZA_MAPA, INFINITE);
		if ( ea->n_avioes > 0 ){
			WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);
			//int i;				//enquanto houver havioes no sistema
			for (int i = 0; i < ea->n_avioes; i++){								//atualiza posicao ocupada no mapa pelo aviao
				if ( ea->controla_av->envia_cada_aviao[i]->em_movimento != ea->avioes[i].em_movimento){		//se na mem partilhada ta em movimento e no local nao esta copia para o local o estado
					ea->avioes[i].em_movimento = ea->controla_av->envia_cada_aviao[i]->em_movimento;	//o aviao local passa a ter os dados do que recebeu ou seja passa a true
					int iter_ap = verifica_existe_aeroporto(ea, ea->avioes[i].viagem.origem);
					if (iter_ap != ERRO){
						//ea->aeroportos[iter_ap].estacionados[verifica_aviao_estacionado(ea, &ea->avioes[i])] = NULL;
						organiza_aeroporto(ea, &ea->aeroportos[iter_ap], FALSE);		//tira do estacionamento
					}
				}

				ea->avioes[i].posicao.x = ea->controla_av->envia_cada_aviao[i]->posicao.x;
				ea->avioes[i].posicao.y = ea->controla_av->envia_cada_aviao[i]->posicao.y;
				ea->MAPA[i].x = ea->avioes[i].posicao.x;
				ea->MAPA[i].y = ea->avioes[i].posicao.y;

				for (int j = 0; j < ea->avioes[i].capacidade_pass; j++){		//atualiza posicao atutomaticamente no passageiro
					if (ea->avioes[i].pass[j] != NULL && (j <= ea->avioes[i].n_passageiros - 1)){
						ea->avioes[i].pass[j]->posicao.x = ea->avioes[i].posicao.x;
						ea->avioes[i].pass[j]->posicao.y = ea->avioes[i].posicao.y;
					}
				}
				
			}

			for (int i = 0; i < ea->max_avioes; i++){	//COLOCA POSICOES DO MAPA QUE JA TENHAM POSICOES DE AEROPORTO A VAZIO COM -1 "ERRO"
				if (ea->avioes[i].posicao.x == ea->avioes[i].viagem.pos_destino.x && ea->avioes[i].posicao.y == ea->avioes[i].viagem.pos_destino.y){
					ea->MAPA[i].x = ERRO;
					ea->MAPA[i].y = ERRO;
				}
			}

			InvalidateRect(ea->teste, NULL, TRUE);

			liberta_posicoes_mapa(ea);

			ReleaseMutex(ea->H_MTX_GERAL);
		}
	}
	return 0;
}

DWORD WINAPI Th_sinal_de_vida(LPVOID param){
	Espaco_Aereo* ea = (Espaco_Aereo*)param;
	int n_aviao = ea->n_avioes - 1;
	Aviao* este_av = &ea->avioes[n_aviao];
	HANDLE handles[2];	//0 - evento / 1 - timer

	TCHAR nome[100];
	wsprintf(nome, TEXT("%s_BEEP_%i") , TEXT("EV"), ea->avioes[ea->n_avioes - 1].pid);
	handles[0] = OpenEvent(EVENT_ALL_ACCESS, FALSE, nome);
	wsprintf(nome, TEXT("%s_BEEP_%i") , TEXT("TMP"), ea->avioes[ea->n_avioes - 1].pid);
	handles[1] = CreateWaitableTimer(NULL, TRUE, nome);

	if (handles[0] == NULL || handles[0] == NULL ){
		return 0;
	}

	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -30000000LL;

	while ( !ea->terminar ){
		SetWaitableTimer(handles[1], &liDueTime, 0, NULL, NULL, 0);
		DWORD esperou_por = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
		if ( esperou_por == 0){
			n_aviao = verifica_existe_aviao(este_av, ea);
		}
		else if (esperou_por == 1) {
			_tprintf(TEXT("vai o aviao po crlh"));

			WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);

			//informa passageiros que morreram ou piloto reformou se
			if (ea->avioes[n_aviao].n_passageiros > 0) {
				if (ea->avioes[n_aviao].em_movimento) {
					for (int i = 0; i < ea->avioes[n_aviao].capacidade_pass; i++) {
						if (ea->avioes[n_aviao].pass[i] != NULL && (i <= ea->avioes[n_aviao].n_passageiros - 1)){
							ea->avioes[n_aviao].pass[0]->action = MORREU;
							envia_evento(ea->avioes[n_aviao].pass[0]->pid, EV_FECHAR_TUDO);
							remover_passageiro(ea, ea->avioes[n_aviao].pass[0]);
						}
					}
				}
				else {
					int iter_ap_origem = verifica_existe_aeroporto(ea, ea->avioes[n_aviao].viagem.origem);
					for (int i = 0; i < ea->avioes[n_aviao].capacidade_pass; i++) {
						if (ea->avioes[n_aviao].pass[i] != NULL && (i <= ea->avioes[n_aviao].n_passageiros - 1)){
							ea->aeroportos[iter_ap_origem].pass_a_espera = (Passageiros**)realloc(ea->aeroportos[iter_ap_origem].pass_a_espera, sizeof(Passageiros*) * (ea->aeroportos[iter_ap_origem].n_pass_a_espera + 1));
							ea->aeroportos[iter_ap_origem].pass_a_espera[ea->aeroportos[iter_ap_origem].n_pass_a_espera] = ea->avioes[n_aviao].pass[i];
							ea->aeroportos[iter_ap_origem].n_pass_a_espera++;
						}
					}
					ea->avioes[n_aviao].em_movimento = TRUE;		//movimento true pq a funcao baseia se nisso
					organiza_aeroporto(ea, &ea->aeroportos[iter_ap_origem], FALSE);
					//ea->aeroportos[iter_ap_origem].n_estacionados--;
				}
			}

			remover_aviao(ea, &ea->avioes[n_aviao]);

			if ( ea->n_avioes == 0 ){
				UnmapViewOfFile(ea->controla_av->envia_cada_aviao[n_aviao]);
			}

			InvalidateRect(ea->teste, NULL, TRUE);
			
			ReleaseMutex(ea->H_MTX_GERAL);

			CloseHandle(handles[0]);
			CloseHandle(handles[1]);
			if (n_aviao != ERRO) {
				CloseHandle(ea->controla_av->H_FICH_CADA_AVIAO[n_aviao]);
			}
		}
	}
	return 0;
}

DWORD WINAPI Th_consome_avioes(LPVOID param) {
	Espaco_Aereo* ea = (Espaco_Aereo*)param;
	Th_Consome consome;
	Aviao novo_av;
	consome.ea = ea;

	Controlo_Avioes* controla_avioes = ea->controla_av;

	while ( !ea->terminar ) {

		WaitForSingleObject( controla_avioes->SEM_CONSOME, INFINITE);
		WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);

		CopyMemory(&novo_av, &controla_avioes->buffer->buffer[controla_avioes->buffer->proxima_pos_leitura], sizeof(Aviao));
		controla_avioes->buffer->proxima_pos_leitura++;
		if (controla_avioes->buffer->proxima_pos_leitura == TAM_BUFFER_CIRCULAR){
			controla_avioes->buffer->proxima_pos_leitura = 0;
		}

		ReleaseMutex(ea->H_MTX_GERAL);
		ReleaseSemaphore(controla_avioes->SEM_PRODUZ, 1, NULL);

		if (novo_av.action == LOGIN){
			if ( !ea->controla_av->suspendido ){
				if (verifica_existe_aviao(&novo_av, ea) == ERRO || novo_av.action == LOGIN) {
					WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);
					if (regista_aviao(ea, &novo_av) == TRUE){
						_tprintf(TEXT("Avião Registado"));

						TCHAR nome[50];
						wsprintf(nome, TEXT("%s_%i"), FICH_MEM_PARTILHADA , novo_av.pid);
						if ( (controla_avioes->H_FICH_CADA_AVIAO[ea->n_avioes - 1] = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, nome)) == NULL){
							_tprintf(TEXT("DEU ERRO"));
							return 0;
						}
						if ( (controla_avioes->envia_cada_aviao[ea->n_avioes - 1] = (Aviao*)MapViewOfFile(controla_avioes->H_FICH_CADA_AVIAO[ea->n_avioes - 1], FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Aviao))) == NULL){
							_tprintf(TEXT("DEU ERRO"));
							return 0;
						}

						//verificar se nao e melhor colocar diretamente no array aviao
						CopyMemory(controla_avioes->envia_cada_aviao[ea->n_avioes - 1], &ea->avioes[ea->n_avioes - 1], sizeof(Aviao));
						ReleaseMutex(ea->H_MTX_GERAL);
						controla_avioes->TH_SINAL_VIDA[ea->n_avioes - 1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Th_sinal_de_vida, ea, 0, NULL);

						envia_evento(novo_av.pid, H_EV_CONTROL_TO_AV);
					}else{
						envia_evento(novo_av.pid, EV_FECHAR_TUDO);
					}
				}
			}else{
				envia_evento(novo_av.pid, EV_FECHAR_TUDO);
			}
		}else if (novo_av.action == VERIFICA_EXISTE_AEROPORTO){
			int iter_ap = verifica_existe_aeroporto(ea, novo_av.viagem.destino);
			if ( iter_ap != ERRO){
				int iter_av = verifica_existe_aviao(&novo_av, ea);
				WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);

				if ( ea->avioes[iter_av].n_passageiros > 0 ){
					int iter_ap_origem = verifica_existe_aeroporto(ea, novo_av.viagem.origem);
					for (int i = 0; i < ea->avioes[iter_av].capacidade_pass; i++){
						if ( ea->avioes[iter_av].pass[i] != NULL && (i <= ea->avioes[iter_av].n_passageiros - 1) ){
							ea->aeroportos[iter_ap_origem].pass_a_espera = (Passageiros**)realloc(ea->aeroportos[iter_ap_origem].pass_a_espera, sizeof(Passageiros*) * (ea->aeroportos[iter_ap_origem].n_pass_a_espera + 1));
							ea->aeroportos[iter_ap_origem].pass_a_espera[ ea->aeroportos[iter_ap_origem].n_pass_a_espera ] = ea->avioes[iter_av].pass[i];
							ea->aeroportos[iter_ap_origem].n_pass_a_espera++;
							ea->avioes[iter_av].pass[i] = NULL;
						}
					}
					ea->avioes[iter_av].n_passageiros = 0;
				}

				ea->avioes[iter_av].action = VERIFICA_EXISTE_AEROPORTO;
				wcscpy(ea->avioes[iter_av].viagem.destino, novo_av.viagem.destino);
				ea->avioes[iter_av].viagem.pos_destino.x = ea->aeroportos[iter_ap].posicao.x;
				ea->avioes[iter_av].viagem.pos_destino.y = ea->aeroportos[iter_ap].posicao.y;

				CopyMemory(controla_avioes->envia_cada_aviao[iter_av], &ea->avioes[iter_av], sizeof(Aviao));

				envia_evento(novo_av.pid, H_EV_CONTROL_TO_AV );
				ReleaseMutex(ea->H_MTX_GERAL);
			}
		}else if (novo_av.action == EMBARCAR_PASSAGEIROS){
			int iter_ap = verifica_existe_aeroporto(ea, novo_av.viagem.origem);
			if (iter_ap != ERRO){
				int iter_av = verifica_existe_aviao(&novo_av, ea);
				WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);

				for (int i = 0; i < ea->aeroportos[iter_ap].n_estacionados; i++){
					if ( ea->aeroportos[iter_ap].estacionados[i] == &ea->avioes[iter_av] ){
						if ( ea->avioes[iter_av].n_passageiros < ea->avioes[iter_av].capacidade_pass ){
							for (int j = 0; j < ea->aeroportos[iter_ap].n_pass_a_espera; j++){
								if ( _tcscmp(ea->aeroportos[iter_ap].pass_a_espera[j]->viagem.destino, ea->avioes[iter_av].viagem.destino) == 0 ){
									ea->avioes[iter_av].pass[ea->avioes[iter_av].n_passageiros] = ea->aeroportos[iter_ap].pass_a_espera[j];
									//ea->aeroportos[iter_ap].pass_a_espera[j] = NULL;
									ea->aeroportos[iter_ap].pass_a_espera[j]->em_viagem = TRUE;
									ea->avioes[iter_av].n_passageiros++;
									
									ea->controla_pass->pass->action = ENTROU_EM_AVIAO;
									envia_ao_pass(ea->controla_pass);
								}
							}
						}
					}
				}

				organiza_aeroporto(ea, &ea->aeroportos[iter_ap], TRUE);
				
				CopyMemory(controla_avioes->envia_cada_aviao[iter_av], &ea->avioes[iter_av], sizeof(Aviao));

				ReleaseMutex(ea->H_MTX_GERAL);
				envia_evento(novo_av.pid, H_EV_CONTROL_TO_AV );
			}
		}else if (novo_av.action == CHEGOU_AO_DESTINO){
			int iter_av = verifica_existe_aviao(&novo_av, ea);
			int iter_ap = verifica_existe_aeroporto(ea, novo_av.viagem.destino);
			if (iter_av != ERRO){
				WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);

				ea->avioes[iter_av].em_movimento = FALSE;

				wcscpy(ea->avioes[iter_av].viagem.origem, ea->avioes[iter_av].viagem.destino);
				ea->avioes[iter_av].viagem.pos_origem.x = ea->avioes[iter_av].viagem.pos_destino.x;
				ea->avioes[iter_av].viagem.pos_origem.y = ea->avioes[iter_av].viagem.pos_destino.y;

				wcscpy(ea->avioes[iter_av].viagem.destino, TEXT(""));
				ea->avioes[iter_av].viagem.pos_destino.x = ERRO;
				ea->avioes[iter_av].viagem.pos_destino.y = ERRO;

				ea->aeroportos[iter_ap].estacionados = (Aviao**)realloc(ea->aeroportos[iter_ap].estacionados, sizeof(Aviao*) * (ea->aeroportos[iter_ap].n_estacionados + 1));
				ea->aeroportos[iter_ap].estacionados[ ea->aeroportos[iter_ap].n_estacionados ] = &ea->avioes[iter_av];
				ea->aeroportos[iter_ap].n_estacionados++;

				//trata dos passageiros do voo que aterrou
				for (int i = 0; i < ea->avioes[iter_av].capacidade_pass; i++){
					if (ea->avioes[iter_av].pass[i] != NULL && (i <= ea->avioes[iter_av].n_passageiros - 1)){
						ea->controla_pass->pass = ea->avioes[iter_av].pass[i];
						ea->controla_pass->pass->action = CHEGOU_AO_DESTINO;
						envia_ao_pass(ea->controla_pass);
					}
				}

				for (int i = (ea->avioes[iter_av].capacidade_pass - 1); i >= 0; i--){
					if (ea->avioes[iter_av].pass[i] != NULL && (i <= ea->avioes[iter_av].n_passageiros - 1)){
						remover_passageiro(ea, ea->avioes[iter_av].pass[i]);
					}
				}

				//organiza_aeroporto(ea, &ea->aeroportos[iter_ap], FALSE);

				ReleaseMutex(ea->H_MTX_GERAL);
			}
		}
	}
	return 0;
}

BOOLEAN envia_ao_pass(Controlo_Passageiros* controla){
	TCHAR nome_pipe[50];
	wsprintf(nome_pipe, TEXT("%s_%i"), PIPE_PASS, controla->pass->pid);

	controla->H_FIFO_PASS = CreateFile(nome_pipe, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (controla->H_FIFO_PASS == INVALID_HANDLE_VALUE){
		return FALSE;
	}

	if ( WriteFile(controla->H_FIFO_PASS, controla->pass, sizeof(Passageiros), NULL, NULL) != 0){
		return TRUE;
	}
	return FALSE;
}

DWORD WINAPI Th_comunica_pass(LPVOID param){
	Espaco_Aereo* ea = (Espaco_Aereo*)param;

	Controlo_Passageiros* controlo_pass = ea->controla_pass;

	Passageiros recebe_pass;
	ea->controla_pass->pass = &recebe_pass;

	while ( !ea->terminar ){
		ConnectNamedPipe(controlo_pass->H_FIFO_CONTROLADOR, NULL);

		if (ReadFile(controlo_pass->H_FIFO_CONTROLADOR, &recebe_pass, sizeof(Passageiros), NULL, NULL) != 0){
			//WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);
			if ( recebe_pass.action == LOGIN){
				if ( !ea->controla_pass->suspendido ){
					int iter_ap_origem = verifica_existe_aeroporto(ea, recebe_pass.viagem.origem);
					int iter_ap_destino = verifica_existe_aeroporto(ea, recebe_pass.viagem.destino);
					if (iter_ap_origem != ERRO && iter_ap_destino != ERRO){
						recebe_pass.posicao.x = ea->aeroportos[iter_ap_origem].posicao.x;
						recebe_pass.em_viagem = FALSE;
						recebe_pass.posicao.y = ea->aeroportos[iter_ap_origem].posicao.y;
						recebe_pass.viagem.pos_origem.x = recebe_pass.posicao.x;
						recebe_pass.viagem.pos_origem.y = recebe_pass.posicao.y;
						recebe_pass.viagem.pos_destino.x = ea->aeroportos[iter_ap_destino].posicao.x;
						recebe_pass.viagem.pos_destino.y = ea->aeroportos[iter_ap_destino].posicao.y;
						ea->todos_passageiros = (Passageiros*)realloc(ea->todos_passageiros, sizeof(Passageiros) * (ea->n_pass + 1));
						if ( ea->todos_passageiros == NULL ){
							_tprintf(TEXT("DEU ERRO"));
							continue;
						}
						CopyMemory(&ea->todos_passageiros[ea->n_pass], &recebe_pass, sizeof(Passageiros));

						int embarcou_aviao = 0;
						//se houver um aviao com o mesmo destino que o passageiro entao embarca automaticamente no aviao
						for (int i = 0; i < ea->aeroportos[iter_ap_origem].n_estacionados; i++){
							if ( _tcscmp(ea->aeroportos[iter_ap_origem].estacionados[i]->viagem.destino , recebe_pass.viagem.destino) == 0){	//encontrou um aviao que esta la ja com o destino definido
								if ( ea->aeroportos[iter_ap_origem].estacionados[i]->n_passageiros < ea->aeroportos[iter_ap_origem].estacionados[i]->capacidade_pass ){
									ea->aeroportos[iter_ap_origem].estacionados[i]->pass[ ea->aeroportos[iter_ap_origem].estacionados[i]->n_passageiros ] = &ea->todos_passageiros[ea->n_pass];
									ea->aeroportos[iter_ap_origem].estacionados[i]->n_passageiros++;
									embarcou_aviao = 1;
									break;
								}
							}
						}
						
						//coloca passageiro a espera no aeroporto
						if (embarcou_aviao == 0){
							int n_pass_espera = ea->aeroportos[iter_ap_origem].n_pass_a_espera;
							ea->aeroportos[iter_ap_origem].pass_a_espera = (Passageiros**)realloc(ea->aeroportos[iter_ap_origem].pass_a_espera, sizeof(Passageiros*) * (n_pass_espera + 1));
							ea->aeroportos[iter_ap_origem].pass_a_espera[n_pass_espera] = &ea->todos_passageiros[ea->n_pass];
							ea->aeroportos[iter_ap_origem].n_pass_a_espera++;
						}

						ea->n_pass++;
						_tprintf(TEXT("\nbem vindo pass: %s"), recebe_pass.nome);
					}else{
						recebe_pass.action = SAIR;
					}
				}else{
					recebe_pass.action = SAIR;
				}
				envia_ao_pass(controlo_pass);
			}else if (recebe_pass.action == PEDE_POSICAO){
				int i = 0, j = 0;
				if ( verifica_existe_pass(ea, &recebe_pass, 1, &i, &j) ){
					ea->avioes[i].pass[j]->action = PEDE_POSICAO;
					CopyMemory(&recebe_pass, ea->avioes[i].pass[j], sizeof(Passageiros));
					envia_ao_pass(controlo_pass);
				}
				_tprintf(TEXT("pediu localizacao"));

			}else if (recebe_pass.action == PERDI_A_PACIENCIA){
				remover_passageiro(ea, &recebe_pass);

			}else if (recebe_pass.action == SAIR){
				int i = 0, j = 0;
				if ( verifica_existe_pass(ea, &recebe_pass, 1, &i, &j) ){
					remover_passageiro(ea, ea->avioes[i].pass[j]);
					ea->avioes[i].pass[j] = NULL;
					ea->avioes[i].n_passageiros--;
					//organizar_pass_aviao(ea, &ea->avioes[i]);
				}else{
					verifica_existe_pass(ea, &recebe_pass, 0, &i, &j);
					remover_passageiro(ea, ea->aeroportos[i].pass_a_espera[j]);
					organiza_aeroporto(ea, &ea->aeroportos[i], TRUE);
				}
			}
			//ReleaseMutex(ea->H_MTX_GERAL);
		}
		FlushFileBuffers(controlo_pass->H_FIFO_CONTROLADOR);
		DisconnectNamedPipe(controlo_pass->H_FIFO_CONTROLADOR);
	}
	
	CloseHandle(controlo_pass->H_FIFO_CONTROLADOR);
	return 0;
}