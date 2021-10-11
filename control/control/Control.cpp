#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <tchar.h>
#include <fcntl.h>
#include <windows.h>
#include <io.h> // biblioteca a adicionar para uinicde
#include <math.h>

#include "Estruturas.h"
#include "configuracoes.h"
#include "resource.h"

//Espaco_Aereo ea;

int regista_no_registry(HKEY* hKey, /*const TCHAR* obj, */const TCHAR* max_obj, int max){
	//int max;
	//_tprintf(TEXT("\nA quantos %s quer limitar o seu sistema? "), obj);
	//_tscanf(TEXT("%i"), &max);

	if ( RegSetValueEx(*hKey, (TEXT("%s"), max_obj), 0, REG_DWORD, (LPBYTE)(&max), sizeof(max)) == ERROR_SUCCESS){
		_tprintf( TEXT("Chave criada!\n") );
	}else{
		return -1;
	}
	return max;
}

LRESULT CALLBACK Trata_ADD_AP(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam){

	TCHAR recebe[5];
	static Espaco_Aereo* ea = NULL;

	switch (messg) {
	case WM_INITDIALOG:
		ea = (Espaco_Aereo*)lParam;
		break;

	case WM_COMMAND:
		
		if ( LOWORD(wParam) == IDC_BUTTON_CRIA_AP ){
			Aeroporto ae;

			GetDlgItemText(hWnd, IDC_EDIT_NOME_AP, recebe, MAX_LEN_NOME_AEROPORTO );
			wcscpy(ae.nome, recebe);
			GetDlgItemText(hWnd, IDC_EDIT_POS_X, recebe, 4 );
			ae.posicao.x = _ttoi(recebe);
			GetDlgItemText(hWnd, IDC_EDIT_POS_Y, recebe, 4 );
			ae.posicao.y = _ttoi(recebe);

			cria_aeroporto(ea, &ae);
			EndDialog(hWnd, 0);
			return (LRESULT)ea;
		}else if( LOWORD(wParam) == IDOK ){
			EndDialog(hWnd, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}

	return FALSE;
}

LRESULT CALLBACK Trata_Login(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam){

	TCHAR recebe[5];
	static Espaco_Aereo* ea = NULL;

	switch (messg) {
	case WM_INITDIALOG:
		ea = (Espaco_Aereo*)lParam;
		break;

	case WM_COMMAND:
		
		if ( LOWORD(wParam) == IDC_BUTTON_LIGA_CONTROL ){
			if (GetDlgItemText(hWnd, IDC_EDIT_MAX_AV, recebe, 5) > 0) {
				ea->max_avioes = _ttoi(recebe);
			}else {
				EndDialog(hWnd, 0);
			}

			if (GetDlgItemText(hWnd, IDC_EDIT_NOME_AP, recebe, 5) > 0) {
				ea->max_aeroportos = _ttoi(recebe);
			}else {
				EndDialog(hWnd, 0);
			}

			HKEY hKey;
			DWORD result;
			if ( RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\SO2"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &result) != ERROR_SUCCESS) {
				return -1;
			}

			if ( !configura_tudo(ea, ea->controla_av, ea->controla_pass) ){
				PostQuitMessage(0);
				EndDialog(hWnd, 0);
			}

			regista_no_registry(&hKey, TEXT("max_avioes"), ea->max_avioes);
			regista_no_registry(&hKey, TEXT("max_avioes"), ea->max_avioes);

			EndDialog(hWnd, 0);
			return (LRESULT)ea;
		}else if( LOWORD(wParam) == IDOK ){
			EndDialog(hWnd, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	}

	return FALSE;
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC dc;
	HDC bufferDC = NULL;
	HBITMAP bufferHBIT;
	PAINTSTRUCT ps;
	RECT rect;
	TRACKMOUSEEVENT tme;

	BOOLEAN encontrou = FALSE;
	TCHAR texto[499];
	TCHAR recebe[20];

	int x = 0;
	int y = 0;


	Espaco_Aereo* ea;
	ea = (Espaco_Aereo*)GetWindowLongPtr(hWnd, 0);
	if (hWnd != NULL && ea != NULL) {
		ea->teste = hWnd;
	}

	switch (messg) {

	case WM_CREATE:
		EnableMenuItem( GetMenu(hWnd),  ID_CRIAR_AEROPORTO, MF_DISABLED | MF_GRAYED);
		EnableMenuItem( GetMenu(hWnd),  ID_SUSPENDER_AVIOES, MF_DISABLED | MF_GRAYED);
		EnableMenuItem( GetMenu(hWnd),  ID_SUSPENDER_PASSAGEIROS, MF_DISABLED | MF_GRAYED);

		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)){
		
		case ID_LIGAR_CONTROLADOR:
			
			DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_DIALOG_LIGAR_CONTROL), hWnd, Trata_Login, (LPARAM)ea);

			if ( ea->max_aeroportos >= 0 || ea->max_avioes >= 0 ){
				EnableMenuItem( GetMenu(hWnd),  ID_CRIAR_AEROPORTO, MF_ENABLED);

				EnableMenuItem( GetMenu(hWnd),  65535, MF_ENABLED);
				EnableMenuItem( GetMenu(hWnd),  ID_SUSPENDER_AVIOES, MF_ENABLED);
				EnableMenuItem( GetMenu(hWnd),  ID_SUSPENDER_PASSAGEIROS, MF_ENABLED);

				EnableMenuItem( GetMenu(hWnd),  ID_LIGAR_CONTROLADOR, MF_DISABLED | MF_GRAYED);
				
				UpdateWindow(hWnd);
			}

			break;

		case ID_CRIAR_AEROPORTO:
			
			DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_DIALOG_ADD_AP), hWnd, Trata_ADD_AP, (LPARAM)ea);
			UpdateWindow(hWnd);
			break;

		case ID_SUSPENDER_AVIOES:
			if ( ea->controla_av->suspendido ){
				ea->controla_av->suspendido = FALSE;
			}else{
				ea->controla_av->suspendido = TRUE;
			}
			break;

		case ID_SUSPENDER_PASSAGEIROS:
			if ( ea->controla_pass->suspendido ){
				ea->controla_pass->suspendido = FALSE;
			}else{
				ea->controla_pass->suspendido = TRUE;
			}
			break;
		}

	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);

		for (int i = 0; i < ea->n_aeroportos; i++){
			if (x >= ea->aeroportos[i].posicao.x && x <= ea->aeroportos[i].posicao.x + 20){
				if ( y >= ea->aeroportos[i].posicao.y && y <= ea->aeroportos[i].posicao.y + 20 ){
					wsprintf(texto, TEXT("Nome Aeroporto: %s\nN Avioes estacionados: %i\nN Passageiros: %i"), ea->aeroportos[i].nome, ea->aeroportos[i].n_estacionados, ea->aeroportos[i].n_pass_a_espera);
					MessageBox(hWnd, texto, (TEXT("Aeroporto: %s"), ea->aeroportos[i].nome), MB_OK);
				}
			}
		}
		
		break;

	case WM_MOUSEMOVE:

		x = LOWORD(lParam);
		y = HIWORD(lParam);
		
		for (int i = 0; i < ea->n_aeroportos; i++){
			if (x >= ea->aeroportos[i].posicao.x && x <= ea->aeroportos[i].posicao.x + 10){
				if ( y >= ea->aeroportos[i].posicao.y && y <= ea->aeroportos[i].posicao.y + 10 ){
					encontrou = TRUE;
					break;
				}
			}
		}
		
		if ( !encontrou ) {
			for (int i = 0; i < ea->n_avioes; i++){
				if (x >= ea->avioes[i].posicao.x && x <= ea->avioes[i].posicao.x + 10){
					if ( y >= ea->avioes[i].posicao.y && y <= ea->avioes[i].posicao.y + 10 ){
						wsprintf(texto, TEXT("ID_Aviao: %i\nOrigem: %s\nDestino: %s\nN Passageiros: %i"), ea->avioes[i].pid, ea->avioes[i].viagem.origem, ea->avioes[i].viagem.destino, ea->avioes[i].n_passageiros);
						_itow(ea->avioes[i].pid, recebe, 10 );  //wsprintf(recebe, TEXT("%i"), ea->avioes[i].pid);
						MessageBox(hWnd, texto, (TEXT("Aviao: %s"), recebe), MB_OK);
					}
				}
			}
		}
		break;


	case WM_PAINT:
		//WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);
		dc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);

		if (bufferDC == NULL){
			bufferDC = CreateCompatibleDC(dc);
			bufferHBIT = CreateCompatibleBitmap(dc, rect.right, rect.bottom);
			SelectObject(bufferDC, bufferHBIT);
			DeleteObject(bufferHBIT);
		}

		WaitForSingleObject(ea->H_MTX_GERAL, INFINITE);
		FillRect(bufferDC, &rect, CreateSolidBrush(RGB(255,255,255)));

		if ( ea->n_aeroportos > 0 || ea->n_avioes > 0){

			//GetClientRect(hWnd, &rect);
			//SetBkColor(dc,/*RGB(0,0,0)*/ TRANSPARENT );

			for (int i = 0; i < ea->n_avioes; i++){
				if ( ea->avioes[i].posicao.x != ea->avioes[i].viagem.pos_origem.x || ea->avioes[i].posicao.y != ea->avioes[i].viagem.pos_origem.y ){
					SetTextColor(bufferDC, RGB(255,0,255));
					rect.left = ea->avioes[i].posicao.x;
					rect.top = ea->avioes[i].posicao.y;
					if (rect.left > 950 || rect.top > 950){
						if (rect.left > 950){ rect.left = rect.left - 20; }
						else{ rect.top = rect.top - 20; }
					}
					DrawText(bufferDC, TEXT("V"), 1, &rect, DT_SINGLELINE | DT_NOCLIP);
				}
			}

			for (int i = 0; i < ea->n_aeroportos; i++){
				SetTextColor(bufferDC, RGB(0,0,0));
				rect.left = ea->aeroportos[i].posicao.x;
				rect.top = ea->aeroportos[i].posicao.y;
				if (rect.left > 950 || rect.top > 950){
					if (rect.left > 950){ rect.left = rect.left - 20; }
					else{ rect.top = rect.top - 20; }
				}
				DrawText(bufferDC, TEXT("A"), 1, &rect, DT_SINGLELINE | DT_NOCLIP);
			}

			BitBlt(dc, 0, 0, rect.right, rect.bottom, bufferDC, 0, 0, SRCCOPY);

		}
		ReleaseMutex(ea->H_MTX_GERAL);
		EndPaint(hWnd, &ps);
		//ReleaseMutex(ea->H_MTX_GERAL);

		break;

	case WM_CLOSE:
		if ( MessageBox(hWnd, TEXT("Deseja fechar o controlador Aereo?"), TEXT("Confirmação"), MB_YESNO) == IDYES ){
			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:	// Destruir a janela e terminar o programa 
				// "PostQuitMessage(Exit Status)"		
		PostQuitMessage(0);
		fechar_controlador(ea);
		break;

	default:
		//InvalidateRect(hWnd, NULL, FALSE);
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;  // break tecnicamente desnecessário por causa do return
	}
	return(0);
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
//int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	Controlo_Avioes controla_avioes;
	Controlo_Passageiros controlo_pass;
	Espaco_Aereo ea;
	ea.aeroportos = NULL;
	ea.avioes = NULL;

	ea.controla_av = &controla_avioes;
	ea.controla_pass = &controlo_pass;

	ea.max_aeroportos = -1;


    TCHAR szProgName[] = TEXT("Controlador");
    HWND hWnd; // hWnd � o handler da janela, gerado mais abaixo por CreateWindow()
    MSG lpMsg; // MSG � uma estrutura definida no Windows para as mensagens
    WNDCLASSEX wcApp; // WNDCLASSEX � uma estrutura cujos membros servem para definir as caracter�sticas da classe da janela
    
	
	wcApp.cbSize = sizeof(WNDCLASSEX); // Tamanho da estrutura WNDCLASSEX
    wcApp.hInstance = hInst; // Inst�ncia da janela actualmente exibida ("hInst" � par�metro de WinMain e vem inicializada da�)
    wcApp.lpszClassName = szProgName; // Nome da janela (neste caso = nome do programa)
    wcApp.lpfnWndProc = TrataEventos; // Endere�o da fun��o de processamento da janela ("TrataEventos")
    wcApp.style = CS_HREDRAW | CS_VREDRAW;// Estilo da janela: Fazer o redraw se for modificada horizontal ou verticalmente
    wcApp.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON_CONTROL));// �cone normal "NULL" = definido no Windows, "IDI_AP..." �cone "aplica��o"
    wcApp.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON_CONTROL));// �cone pequeno, "NULL" = definido no Windows, "IDI_INF..." �con de informa��o
    //HICON icon = LoadImage(NULL, TEXT("D:\\UNI\\SO2\\TP\\TP_SO2\\control\\favicon.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW); // cursor do rato, "NULL"=Forma definida no Windows, "IDC_ARROW" Aspeto "seta"
    wcApp.lpszMenuName =  MAKEINTRESOURCE(IDR_MENU1)  ; // Classe do menu que a janela pode ter, (NULL = n�o tem menu)
    wcApp.cbClsExtra = 0; // Livre, para uso particular
    wcApp.cbWndExtra = sizeof(Espaco_Aereo*); // Livre, para uso particular
    wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // "brush" pintura do fundo da janela. Devolvido por "GetStockObject"

	

    if (!RegisterClassEx(&wcApp)) { return(0); } // 2. Registar a classe "wcApp" no Windows


    hWnd = CreateWindow(szProgName, TEXT("Controlador Aereo"),
        WS_OVERLAPPEDWINDOW, // Estilo da janela (WS_OVERLAPPED= normal)
		/*CW_USEDEFAULT*/ 0, // Posi��o x pixels (default=� direita da �ltima)
		/*CW_USEDEFAULT*/ 0, // Posi��o y pixels (default=abaixo da �ltima)
        /*CW_USEDEFAULT*/ 1100, // Largura da janela (em pixels)
        /*CW_USEDEFAULT*/ 1100 , // Altura da janela (em pixels)
        (HWND)HWND_DESKTOP, // handle da janela pai ou HWND_DESKTOP se for a primeira, criada a partir do "desktop"
        (HMENU)NULL, // handle do menu da janela (se tiver menu)
        (HINSTANCE)hInst, // handle da inst�ncia do programa actual ("hInst" � passado num dos par�metros de WinMain()
        0); // N�o h� par�metros adicionais para a janela

	SetWindowLongPtr(hWnd, 0 , (LONG_PTR)&ea);

 	ShowWindow(hWnd, nCmdShow); // Mostrar janela - "hWnd"= handler devolvido por "CreateWindow"; "nCmdShow"= modo de exibi��o (p.e. normal/modal); � passado como par�metro de WinMain()
    UpdateWindow(hWnd); // Refrescar a janela (Windows envia � janela uma mensagem para pintar, mostrar dados, (refrescar)�

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);	// Pré-processamento da mensagem (p.e. obter código 
					  				// ASCII da tecla premida)
		DispatchMessage(&lpMsg);	// Enviar a mensagem traduzida de volta ao Windows, que
					   				// aguarda até que a possa reenviar à função de 
					   				// tratamento da janela, CALLBACK TrataEventos (abaixo)
	}

	//fechar_controlador(&ea);

    return((int)lpMsg.wParam); // Retorna sempre o par�metro wParam da estrutura lpMsg



/*			ISTO ERA DA PARTE CONSOLA DO TRABALHO

	Controlo_Avioes controla_avioes;
	Controlo_Passageiros controlo_pass;
	Espaco_Aereo ea;
	ea.aeroportos = NULL;
	ea.avioes = NULL;

	HKEY hKey;
	DWORD result;

	if ( RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\SO2"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &result) != ERROR_SUCCESS) {
		return -1;
	}

	ea.max_avioes = regista_no_registry(&hKey, TEXT("avioes"), TEXT("max_avioes"));
	ea.max_aeroportos = regista_no_registry(&hKey, TEXT("aeroporto"), TEXT("max_aeroportos"));
	if (ea.max_aeroportos == -1 || ea.max_avioes == -1 ){ return -1; }

	if (configura_tudo(&ea, &controla_avioes, &controlo_pass) == 0) {
		return -1;
	}

	interpreta_comandos(&ea);
	
	return 0;

*/
}