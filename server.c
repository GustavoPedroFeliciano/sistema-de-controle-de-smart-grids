//TP1 - REDES DE COMPUTADORES - 2023/2
//NOME: GUSTAVO PEDRO FELICIANO
//MATRÍCULA: 2020092330


#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define MSGSZ 500
#define STR_MIN 8
#define MIN_DEV_ID 0
#define MAX_DEV_ID 100

void usage(){
    printf("Chamada correta: ./server <v4/v6> <port>\n");
    exit(EXIT_FAILURE);
}

struct dispositivo{
    unsigned id;
    unsigned corrente;
    unsigned tensao;
    unsigned efic;
    unsigned pot;
};
struct dispositivo dispositivos[100];

//Funcoes pra construir mensagens de controle ERROR e OK ja em formato de string, fornecendo apenas o codigo.
void build_error_msg(char *msg_out, unsigned codigo){
    
    char *code_aux = malloc(sizeof(STR_MIN));
    sprintf(code_aux, "%02u", codigo);

    strcpy(msg_out, "ERROR ");
    strcat(msg_out, code_aux);
    strcat(msg_out, "\n");

    free(code_aux);
}
void build_ok_msg(char *msg_out, unsigned codigo){
    
    char *code_aux = malloc(sizeof(STR_MIN));
    sprintf(code_aux, "%02u", codigo);

    strcpy(msg_out, "OK ");
    strcat(msg_out, code_aux);
    strcat(msg_out, "\n");

    free(code_aux);
}

//Funcao pra testar se o dispositivo tem um id valido, retorna 1 ou 0 
unsigned is_dev_id_valid(int dev_id){
    if(dev_id >= MIN_DEV_ID && dev_id <= MAX_DEV_ID){
        return 1;
    }
    return 0;
}

//Funcao que trata a mensagem que chega do cliente e retorna a mensagem de controle (OK ou ERROR) ou de RESPONSE para o cliente na variavel msg_out
void process_request(char *request, struct dispositivo dispositivos[], char *response){

    char *token = strtok(request, " "); //token = type
    unsigned req_type = parse_msg_type(token);
    
    int dev_id = 0;
    int iA;
    int V;
    int porcento;
    int W;

    switch (req_type){
        case INS_REQ:
            // INS_REQ <devId> <corrente> <tensao> <eficiência energética>
            
            token = strtok(NULL, " "); //token = devId
            dev_id = atoi(token);
            token = strtok(NULL, " "); //token = corrente
            iA = atoi(token);
            token = strtok(NULL, " "); //token = tensao
            V = atoi(token);
            token = strtok(NULL, " "); //token = eficiencia
            porcento = atoi(token);

            //Primeiro testa se o id do dispositivo eh invalido
            if(!is_dev_id_valid(dev_id)){
                build_error_msg(response, 3);
                return;
            }

            if(dispositivos[dev_id - 1].id == dev_id && dispositivos[dev_id - 1].pot != 0){
                build_error_msg(response, 4);
                return;
            }

            if(iA > 10 || iA < 0){
                build_error_msg(response, 3);
                return;
            }

            if(V > 150 || V < 0){
                build_error_msg(response, 3);
                return;
            }

            if(porcento > 100 || porcento < 0){
                build_error_msg(response, 3);
                return;
            }

            W = V * iA;

            //Instala o dispositivo 
            dispositivos[dev_id - 1].id = dev_id;
            dispositivos[dev_id - 1].corrente = iA;
            dispositivos[dev_id - 1].tensao = V;
            dispositivos[dev_id - 1].efic = porcento;
            dispositivos[dev_id - 1].pot = W;

            //Resposta de sucesso
            build_ok_msg(response, 1); 
        break;

        case REM_REQ:
            // REM_REQ <devId> 
            token = strtok(NULL, " "); //token = devId
            dev_id = atoi(token);
      
            //Primeiro testa se o id do dispositivo eh invalido
            if(!is_dev_id_valid(dev_id)){
                build_error_msg(response, 3); 
                return;
            }

            //testa se o dispositivo estah instalado
            if(dispositivos[dev_id - 1].efic == 0 && dispositivos[dev_id - 1].pot == 0){
                build_error_msg(response, 1);
                return;
            }
            
            //se estiver instalado, desinstala (zerando todos os seus atributos no vetor do DB)
            dispositivos[dev_id - 1].id = 0;
            dispositivos[dev_id - 1].corrente = 0;
            dispositivos[dev_id - 1].tensao = 0;
            dispositivos[dev_id - 1].efic = 0;
            dispositivos[dev_id - 1].pot = 0;

            //Resposta de sucesso
            build_ok_msg(response, 2); 

        break;

        case CH_REQ:
            // CH_REQ <devId> <corrente> <tensao> <eficiência energética>
            token = strtok(NULL, " "); //token = devId
            dev_id = atoi(token);
            token = strtok(NULL, " "); //token = ligado
            iA = atoi(token);
            token = strtok(NULL, " "); //token = dado
            V = atoi(token);
            token = strtok(NULL, " "); //token = dado
            porcento = atoi(token);

            //Primeiro testa se o id do dispositivo eh invalido
            if(!is_dev_id_valid(dev_id)){
                build_error_msg(response, 3); 
                return;
            }
            
            //Testa se o dispositivo estah instalado
            if(dispositivos[dev_id - 1].efic == 0 && dispositivos[dev_id - 1].pot == 0){
                build_error_msg(response, 1);
                return;
            }

            if(iA > 10 || iA < 0){
                build_error_msg(response, 3);
                return;
            }

            if(V > 150 || V < 0){
                build_error_msg(response, 3);
                return;
            }

            if(porcento > 100 || porcento < 0){
                build_error_msg(response, 3);
                return;
            }

            W = V * iA;

            //se estiver instalado, muda seu estado
            dispositivos[dev_id - 1].id = dev_id;
            dispositivos[dev_id - 1].corrente = iA;
            dispositivos[dev_id - 1].tensao = V;
            dispositivos[dev_id - 1].efic = porcento;
            dispositivos[dev_id - 1].pot = W;
            
            //resposta de sucesso
            build_ok_msg(response, 3);
            
        break;

        case SEN_REQ:
            // SEN_REQ <devId> 
            token = strtok(NULL, " "); //token = devId
            dev_id = atoi(token);

            //Primeiro testa se o id do dispositivo eh invalido
            if(!is_dev_id_valid(dev_id)){
                build_error_msg(response, 3); 
                return;
            }

            //Testa se o dispositivo estah instalado
            if(dispositivos[dev_id - 1].efic == 0 && dispositivos[dev_id - 1].pot == 0){
                build_error_msg(response, 1);
                return;
            }

            //Constroi a resposta com os dados como uma string dinamicamente
            strcpy(response, "SEN_RES");

            char *aux = malloc(STR_MIN);
            //Consulta o DB e insere as infos na string da msg de resposta 
            sprintf(aux, " %d", dispositivos[dev_id - 1].id);
            strcat(response, aux);
            
            sprintf(aux, " %d", dispositivos[dev_id - 1].pot);
            strcat(response, aux);

            sprintf(aux, " %d", dispositivos[dev_id - 1].efic);
            strcat(response, aux);

            strcat(response, "\n");
            
            free(aux);
            puts(response);
        break;

        case VAL_REQ:
            
            strcpy(response, "VAL_RES");
            
            char aux2[STR_MIN];
            unsigned dev_count = 0;
            //Percorre todos os dispositivos do vetor
            for (int i = MIN_DEV_ID; i <= MAX_DEV_ID; i++){
                if(dispositivos[i - 1].id == i && dispositivos[i - 1].pot != 0){
                    //significa que o dispositivo i estah instalado
                    /*sprintf(aux2, " %d", i);
                    strcat(response, aux2); */

                    sprintf(aux2, " %d", dispositivos[i - 1].id);
                    strcat(response, aux2); 

                    sprintf(aux2, " %d", dispositivos[i - 1].pot);
                    strcat(response, aux2); 

                    sprintf(aux2, " %d", dispositivos[i - 1].efic);
                    strcat(response, aux2);

                    dev_count++;
                }
            }
            strcat(response, "\n");

            //testa se nao tem nenhum dispositivo instalado
            if(!dev_count){
                build_error_msg(response, 2); 
                return;
            }

        break;
        
        default:

        break;
    }
}

//argv[1] -> familia IP
//argv[2] -> porta do processo
int main(int argc, char **argv){
    // Garantia de que o programa foi inicializado corretamente
    if(argc < 3){
        usage();
    }

    // ------------- DB DO SERVER ------------- //
    
    struct dispositivo database[MAX_DEV_ID];
    //inicializa o banco de dados com os ids 0
    for(int j = MIN_DEV_ID - 1; j <= MAX_DEV_ID - 1; j++){
        database[j].id = 0;
        database[j].corrente = 0;
        database[j].tensao = 0;
        database[j].efic = 0;
        database[j].pot = 0;
        
    }
    
    // ------------- CONEXAO COM CLIENTE ------------- //
    // --- DEFINICAO DE v4|v6 E PORTA --- //
    // Salva a familia IP e a porta na variavel storage que eh do tipo POSIX certo pra guardar endereco
    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage();
    }

    // --- INICIALIZACAO DO SOCKET --- //
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("erro ao inicializar socket");
    }

    //Essa eh apenas uma opcao pra reutilizar a mesma porta em duas execucoes consecutivas sem ter que esperar
    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    // Salva o endereco de storage em addr, que eh do tipo correto (addr_storage nao eh suportado)
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    
    // O bind associa('liga') o socket a uma porta do SO 
    // Recebe o socket e o addr (v4|v6 + porta)
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("erro no bind");
    }

    // --- ESPERAR CONEXAO DO CLIENTE --- //
    // O listen indica que o socket passado eh passivo, i.e., que vai esperar requisicao de conexao 
    // Recebe o socket e o maximo de conexoes pendentes possiveis para aquele socket
    if (0 != listen(s, 10)) {
        logexit("erro no listen");
    }
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("ouvindo na porta %s\n", addrstr);

    // --- ACEITAR CONEXAO DO CLIENTE --- //
    struct sockaddr_storage cstorage;
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    // A funcao accept aceita a conexao pelo socket s e cria OUTRO socket pra comunicar com o cliente 
    // Recebe o socket de conexao, caddr pra salvar o endereco do cliente e caddrlen pro tamanho desse endereco
    int client_sock = accept(s, caddr, &caddrlen);
    if (client_sock == -1) {
        logexit("erro ao conectar com o cliente");
    }

    // So transforma o endereco de sockaddr pra string
    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    while (1) {
        // ------------- TROCA DE MENSAGENS ------------- //

        // --- RECEBIMENTO DA MENSAGEM DO CLIENTE (request) --- //
        //Recebe msg em formato de [string] e salva no numa string
        char req_msg[MSGSZ];
        memset(req_msg, 0, MSGSZ);
        size_t count = recv(client_sock, req_msg, MSGSZ - 1, 0);
        if(count == 0){
            //Se nao receber nada, significa que a conexão foi fechada
            close(client_sock); // Encerra a conexao com aquele cliente
            break;
        }
        printf("%s", req_msg); //Imprime a msg na tela
        
        // --- ACAO E CONSTRUCAO DA MENSAGEM DE RESPOSTA --- //
        //Processa a mensagem e guarda a mensagem de resposta ao cliente numa string
        char *res_msg = malloc(MSGSZ);
        process_request(req_msg, database, res_msg);

        // --- ENVIO DA MENSAGEM (response) --- //
        count = send(client_sock, res_msg, strlen(res_msg) + 1, 0);
        if (count != strlen(res_msg) + 1) {
            logexit("erro ao enviar mensagem de resposta");
        }

        free(res_msg);
        res_msg = NULL;
    }

    exit(EXIT_SUCCESS);
}