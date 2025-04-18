#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common.h"

#define BUFSZ 1024
#define MSGSZ 500
#define STR_MIN 8

void usage(){
    printf("Chamada correta: ./client <server IP> <port>\n");
    exit(EXIT_FAILURE);
}

unsigned process_command(char *comando, char *msg_out){
    
    //Inicialmente, token tem a primeira palavra do comando, que eh a "identificacao" dele
    char *token = strtok(comando, " ");
    
    //caso INS_REQ
    if(!strcmp(token, "install")){
        
        strcpy(msg_out, "INS_REQ"); 

        token = strtok(NULL, " ");

        if (token == NULL || strcmp(token, "param") == 0) {

        // Extrai os tokens seguintes (sensorId, cor, ten, efic_energ)
        for (int i = 0; i < 4; i++) {
            
            token = strtok(NULL, " "); // Próximo token

            if (token == NULL) {
                printf("invalid sensor\n");
                return 1;
            }

            // Concatena o token à mensagem de saída
            strcat(msg_out, " ");
            strcat(msg_out, token);
            
        }

        // Adiciona uma quebra de linha à mensagem de saída
        strcat(msg_out, "\n");
        }

        if (token == NULL || strcmp(token, "file") == 0){
            // Obter o nome do arquivo
            char *filename = strtok(NULL, " ");
            if (filename != NULL) {
                // Tentar abrir o arquivo
                FILE *file = fopen(filename, "r");
                if (file != NULL) {
                    char parametro[5];
                    for(int i = 0; i < 4; i++){
                        fgets(parametro, sizeof(parametro), file);
                        strcat(msg_out, " ");
                        strcat(msg_out, parametro);
                    }
                }
            }
        }
    }

    
    //caso REM_REQ
    else if(!strcmp(token, "remove")){
    
        strcpy(msg_out, "REM_REQ"); 

        token = strtok(NULL, " ");
        if(strcmp(token, "0") && !atoi(token))
            return 0;
        
        strcat(msg_out, " ");
        strcat(msg_out, token);
    
        strcat(msg_out, "\n");
    }

    //caso CH_REQ
    else if(!strcmp(token, "change")){

        strcpy(msg_out, "CH_REQ"); 

        token = strtok(NULL, " ");

        if (token == NULL || strcmp(token, "param") == 0) {

        // Extrai os tokens seguintes (sensorId, cor, ten, efic_energ)
        for (int i = 0; i < 4; i++) {
            token = strtok(NULL, " "); // Próximo token

            if (token == NULL) {
                printf("invalid sensor\n");
                return 1;
            }

            // Concatena o token à mensagem de saída
            strcat(msg_out, " ");
            strcat(msg_out, token);
        }

        // Adiciona uma quebra de linha à mensagem de saída
        strcat(msg_out, "\n");
        }

        if (token == NULL || strcmp(token, "file") == 0){
            // Obter o nome do arquivo
            char *filename = strtok(NULL, " ");
            if (filename != NULL) {
                // Tentar abrir o arquivo
                FILE *file = fopen(filename, "r");
                if (file != NULL) {
                    char parametro[5];
                    for(int i = 0; i < 4; i++){
                        fgets(parametro, sizeof(parametro), file);
                        strcat(msg_out, " ");
                        strcat(msg_out, parametro);
                    }
                }
            }
        }
    }

    //caso VAL_REQ || SEN_REQ
    //Para o cliente imprimir na tela a resposta a esses chamados, eles retornam 2.
    else if(!strcmp(token, "show")){
        //nesse caso, a proxima palavra do comando tem que ser VALUE/VALUES
        token = strtok(NULL, " ");
        //caso VAL_REQ
        if(!strcmp(token, "values")){
            strcpy(msg_out, "VAL_REQ");
        }
        
        //caso SEN_REQ
        else if(!strcmp(token, "value")){        
            strcpy(msg_out, "SEN_REQ");

            token = strtok(NULL, " "); //token = devId
            if(strcmp(token, "0") && !atoi(token))
                return 0; //testa se o valor eh um inteiro

            strcat(msg_out, " ");
            strcat(msg_out, token);

            strcat(msg_out, "\n");

        }
        
        else{
            return 0;
        }
    }

    //caso erro
    else{
        return 0;
    }

    return 1;
}

//Transforma uma mensagem OK, ERROR em um aviso no terminal do cliente
void process_res_msg(char *msg_in, char *str_out, char *req_msg){
    char *token = strtok(msg_in, " "); //token = type
    int type = parse_msg_type(token);
    unsigned code = 0;
    char *pot_aux = malloc(STR_MIN);
    char *efic_aux = malloc(STR_MIN);
    char *dev_aux = malloc(STR_MIN);
    int i = 0;
    int count = 0;

    switch (type){

        case ERROR:
            token = strtok(NULL, " "); //token = codigo do erro
            code = atoi(token);

            switch (code){
                case 1:
                    strcpy(str_out,"sensor not installed\n");
                break;

                case 2:
                    strcpy(str_out,"no sensors\n");
                break;

                case 3:
                    strcpy(str_out,"invalid sensor\n");
                break;

                case 4:
                    strcpy(str_out,"sensor already exists\n");
                break;

                default:
                    
                break;
            }
        break;

        case OK:
            token = strtok(NULL, " "); //token = codigo do erro
            code = atoi(token);

            switch (code){

                case 1:
                    strcpy(str_out,"successful installation\n");
                break;

                case 2:
                    strcpy(str_out,"successful removal\n");
                break;

                case 3:
                    strcpy(str_out,"successful change\n");
                break;

                default:
                    
                break;
            }
        break;

        case SEN_RES:
            // Desmonta o SEN_RES
            token = strtok(NULL, " ");
            token = strtok(NULL, " "); 
            strcpy(pot_aux, token);
            token = strtok(NULL, " "); 
            strcpy(efic_aux, token);

            token = strtok(req_msg, " "); //token = DEV_REQ

            token = strtok(NULL, " "); //token = dev_id
            strcpy(dev_aux, token);
            dev_aux = strtok(dev_aux, "\n");
            
            strcpy(str_out, "sensor ");
            strcat(str_out, dev_aux);
            strcat(str_out, ": ");
            strcat(str_out, pot_aux);
            strcat(str_out, " ");
            strcat(str_out, efic_aux);
            strcat(str_out, "\n");

        break;

        case VAL_RES:
            // Desmonta o VAL_RES
            //percorre toda a mensagem e conta quanto espacos ela tem
            while (msg_in[i] != '\n'){
                
                if(msg_in[i] ==  0 || msg_in[i] == 32){
                    count++;
                   
                }
                i++;
            }

            char aux[MSGSZ] = "";
            for(int j = 0; j < count/3; j++){
                //token = strtok(NULL, " ");
                token = strtok(NULL, " "); //token = dev[i]
                strcat(aux, token);
                strcat(aux, " (");

                token = strtok(NULL, " "); //token = potencia
                strcat(aux, token);
                strcat(aux, " ");
                
                token = strtok(NULL, " "); //token = eficiencia
                if(j == (count/3) - 1){
                    token = strtok(token, "\n");
                }
                strcat(aux, token);
                strcat(aux, ") ");
            }
            
            token = strtok(req_msg, " "); 
            
            strcpy(str_out, "sensors");
            strcat(str_out, ": ");

            strcat(str_out, aux);
            strcat(str_out, "\n");

        break;

        default:
            break;
    }

    free(dev_aux);
    dev_aux = NULL;
    free(pot_aux);
    pot_aux = NULL;
    free(efic_aux);
    efic_aux = NULL;
}

//argv[1] -> IP do servidor
//argv[2] -> porta do processo
int main(int argc, char **argv){
    // Garantia de que o programa foi inicializado corretamente
    if(argc < 3){
        usage();
    }
    
    // ------------- CONEXAO COM SERVIDOR ------------- //

    //--- DEFINICAO DE IP E PORTA ---//
    // sockaddr_storage eh a estrutura de dados do POSIX que permite guardar IPv4 e IPv6, e tambem guarda a porta
    // A funcao addrparse vai salvar a o IP e a porta (que sao recebidas como string) na variavel storage
    struct sockaddr_storage storage;
    if(0 != addrparse(argv[1], argv[2], &storage)){
        usage();
    }

    //--- INICIALIZACAO DO SOCKET ---//
    //Tanto IPv4 quanto IPv6 a partir do endereco que estah salvo em storage
    //O argumento ss_family guarda a constante AF_INET (v4) ou AF_INET6 (v6)
    int s = socket(storage.ss_family, SOCK_STREAM, 0);
    if(s == -1){
        logexit("erro ao incializar socket");
    }

    //--- ESTABELECENDO CONEXAO COM SERVIDOR --//
    //Salva o endereco de storage em addr, que eh do tipo correto (addr_storage nao eh suportado)
    //Usa a funcao connect passando o
    //      - socket local (s) 
    //      - porta e IP do servidor de destino (addr)
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if(connect(s, addr, sizeof(storage)) != 0){
        logexit("erro ao conectar com servidor");
    }

    //Transforma addr (que tem IP e porta) de volta pra string e printa
    char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

    printf("conectado a %s\n", addrstr);

    // ------------- TROCA DE MENSAGENS ------------- //
    unsigned total = 0;
    while(1){
        // --- RECEBE O COMANDO --- // 
        char *buf = malloc(BUFSZ);
        memset(buf, 0, BUFSZ-1);
        fgets(buf, BUFSZ-1, stdin);
        buf = strtok(buf, "\n"); //desconsidera o enter que se da ao acabar de digitar o comando

        // --- CONSTROI A MENSAGEM DE REQUISICAO --- //
        //msg_buf guarda a mensagem que vai ser enviada ao server
        char *msg_buf = malloc(MSGSZ);
        unsigned correto = process_command(buf, msg_buf);
        if(!correto)
            break;  //se receber mensagem com algum erro, sai do loop
        if(!strcmp(buf, "kill"))
            break;  //se receber o comando kill, sai do loop

        // --- ENVIA A MENSAGEM DE REQUISICAO --- // 
        size_t count = send(s, msg_buf, strlen(msg_buf)+1, 0); //envia como string
        if (count != strlen(msg_buf)+1) {
            logexit("send");
        }

        // --- RECEBE MENSAGEM DE RESPOSTA  --- //
        //O recv salva o que é recebido byte a byte o e retorna o numero de bytes recebido
        char *buf_res = malloc(MSGSZ);
        count = recv(s, buf_res + total, BUFSZ - total, 0);
        if(count == 0){
            //Se nao receber nada, significa que a conexão foi fechada
            break;
        }
        total += count; //Desloca o buffer pra receber o proximo byte

        // --- TRATA MENSAGEM DE RESPOSTA E IMPRIME AVISO NA TELA --- //
        char *warn = malloc(BUFSZ);
        process_res_msg(buf_res, warn, msg_buf);
        
        printf("%s", warn);

        // --- LIBERA A MEMORIA ---//
        free(buf);
        buf = NULL;
        free(msg_buf); //desaloca o espaco da msg
        msg_buf = NULL;
        free(buf_res);
        buf_res = NULL;
        free(warn);
        warn = NULL;

        total = 0;
    }
    //Ao sair do loop, fecha o socket e finaliza a conexao
    close(s);

    //Termina o programa
    exit(EXIT_SUCCESS);
}
