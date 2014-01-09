#include <stdio.h>
#include <stdlib.h>
#include <string.h>
  
#include "protocolo_emissor.c"
  
int itrans = 0, iretrans = 0, otout = 0, orej = 0;
  
//Estrutura de dados Aplicação
struct applicationLayer {
    int fileDescriptor;
    char file[MAX_SIZE];
    int fileSize;
    int tamanho;
    int numTrans;
}aplicacao;
  
/**
 * Função que define qual o ficheiro a ser enviado
 */
void definirFicheiro()
{
    printf("Qual o ficheiro que pretende transmitir? ");
    scanf("%s", aplicacao.file);
}
  
/**
 * Função que define o tamanho do pacote de dados
 */
void definirTamanhoDados()
{
    printf("Qual a quantidade de dados que pretende enviar em cada transmissão? ");
    scanf("%d", &(aplicacao.tamanho));  
}
  
/**
 * Função que pede ao utilizador se pretende usar valore por defeito ou nao
 */
void definicoesEmissor()
{
    int escolha;
    printf("Pretende utilizar as definições automáticas para a aplicação emissor? (Introduza o número: 1 - sim ou 2 - não) ");
    scanf("%d", &escolha);
    if (escolha == 2)
    {
        definirFicheiro();
        definirTamanhoDados();
    }
    else
    {
        strcpy(aplicacao.file, "pinguim.gif");
        aplicacao.tamanho = 100;
    }
}
  
/**
*   Determina o tamanho de um ficheiro em bytes
*/
  
void tamanhoFicheiro()
{
    FILE *fp;
    if((fp = fopen(aplicacao.file, "rb")) == NULL)
        printf("\nNão foi possível abrir o ficheiro %s para determinar o seu tamanho!\n", aplicacao.file);
    fseek(fp, 0L, SEEK_END);
    aplicacao.fileSize = ftell(fp);
    fclose(fp);
}
  
/**
*   funçao que cria um pacote de controlo e envia-o com llwrite()
*   @param controlo array a ser enviado
*   @param C campo de controlo, 0x01 ou 0x03 se trama I inicial ou final respectivamente
*   @return 1 se enviou correctamente e -1 se nao
*/
int pacoteControlo(unsigned char * controlo, int C)
{
    controlo[0] = C; //campo de controlo
    controlo[1] = 0x00; //T1
    controlo[2] = 0x02; //L1
    controlo[3] = 0x2A; //V1
    controlo[4] = 0xD8;
    controlo[5] = 0x01; //T2
    //controlo[] = (unsigned char) strlen(aplicacao.file);
    controlo[6] = 0x0B; //L2 -> pinguim.gif - 11 caracteres
    controlo[7] = 0x70; //v2 -> ASCII (p)
    /*unsigned char temp[strlen(aplicacao.file)];   
    charToAscii(aplicacao.file, temp)   
    strcpy(&controlo[6], temp);*/
    controlo[8] = 0x69; //ASCII (i)
    controlo[9] = 0x6E; //ASCII (n)
    controlo[10] = 0x67; //ASCII (g)
    controlo[11] = 0x75; //ASCII (u)
    controlo[12] = 0x69; //ASCII (i)
    controlo[13] = 0x6D; //ASCII (m)
    controlo[14] = 0x2E; //ASCII (.)
    controlo[15] = 0x67; //ASCII (g)
    controlo[16] = 0x69; //ASCII (i)
    controlo[17] = 0x66; //ASCII (f)
    return llwrite(aplicacao.fileDescriptor, controlo, 18);
}
  
/**
*   funçao que cria um pacote de dado e envia-o com llwrite()
*   @param caracteres array a ser enviado
*   @param sequencia número de sequencia da trama I
*   @return 1 se enviou correctamente e -1 se nao
*/
int pacoteDados(unsigned char * caracteres, int sequencia)
{
    unsigned char dados[4 + aplicacao.tamanho];
    int i;
    dados[0] = 0x00; //campo de controlo -> dados
    dados[1] = sequencia; //número de sequência
    dados[2] = aplicacao.tamanho / 256; //L2
    dados[3] = aplicacao.tamanho % 256; //L1
    for (i = 0 ; i < aplicacao.tamanho ; i++)
    {
        dados[4 + i] = caracteres[i];
    }
    return llwrite(aplicacao.fileDescriptor, dados, 4 + aplicacao.tamanho);
}
  
int roundUp()
{
    double num;
    num = aplicacao.fileSize / aplicacao.tamanho;
    if (aplicacao.fileSize % aplicacao.tamanho != 0)
        return (int) (num + 1);
    else
        return (int) num;
}
  
void registoOcorrencias()
{
    printf("\nRegisto de Ocorrências\n");
    printf("\tTramas de Informação:\n\t\t- transmitidas: %d\n\t\t- retransmitidas: %d\n", itrans, iretrans);
    printf("\tNúmero de ocorrências de time-out: %d\n", otout);
    printf("\tNúmero de tramas REJ recebidas: %d\n", orej);
}
  
void main()
{
    int i, nsequencia = 1, fim = 0;
    FILE *fp;
    printf("**************************************** EMISSOR ****************************************\n");
    definicoesEmissor();
    tamanhoFicheiro();
    unsigned char controlo[18], dados[aplicacao.tamanho]; 
    if((fp = fopen(aplicacao.file, "rb")) == NULL)
    {
        printf("\n\\t!Não foi possível abrir o ficheiro %s para leitura binária!\n", aplicacao.file);
        exit(-1);
    }
    else
    {       
        if(fread(dados, 1, aplicacao.tamanho, fp) < 1)
        {
            printf("\t!Não foi possível ler todos os caracteres presentes no ficheiro %s!\n", aplicacao.file);
        }
        fseek(fp, aplicacao.tamanho, 0);
    }
    if ((aplicacao.fileDescriptor = llopen("/dev/ttyS4")) == -1)
    {
        printf("\t!Não foi possível estabelecer a ligação da porta série (RS232)!\n");
        printf("\t!A terminar a ligação ...!\n");
        exit(-1);
    }
    else
    {
        printf("-> Fim da Fase de Estabelecimento <-\n");
        printf("Estabelecida a ligação da porta série (RS232)\n");
        printf("\n-> Início da Fase de Transferência de Dados <-");
        if (pacoteControlo(controlo, 1) == -1)
        {
            printf("\t!Não foi possivel enviar a trama de informação com o pacote de controlo start!\n");
            printf("\t!A terminar a ligação ...!\n");
            exit(-1);
        }
        else
        {
            aplicacao.numTrans = roundUp();
            while(!fim)
            {
                  
                printf("!Envio da trama de informação com o pacote de dados #%d!\n", nsequencia);
                  
                if(nsequencia==aplicacao.numTrans)
                {
                    aplicacao.tamanho = fread(dados, 1, aplicacao.tamanho, fp);
                    fim = 1;
                }
                  
                if (pacoteDados(dados, nsequencia) == -1)
                {
                    printf("\t!Não foi possivel enviar a trama de informação com o pacote de dados #%d!\n", nsequencia);
                    printf("\t!A terminar a ligação ...!\n");
                    exit(-1);
                }               
                else
                {
                    aplicacao.tamanho = fread(dados, 1, aplicacao.tamanho, fp);
                    nsequencia++;
                    fseek(fp, aplicacao.tamanho*nsequencia, 0);
                }
            }
            printf("-> Fim da Fase de Transferência de Dados <-\n");
            if (pacoteControlo(controlo, 3) == -1)
            {
                printf("\t!Não foi possivel enviar a trama de informação com o pacote de controlo end!\n");
                printf("\t!A terminar a ligação ...!\n");
                exit(-1);
            }
            else
            {
                printf("\n-> Início da Fase de Terminação <-");
                if (llclose(aplicacao.fileDescriptor) == -1)
                {
                    printf("\t!Não foi possivel terminar a ligação da porta série (RS232)!\n");
                    printf("\t!A terminar a ligação ...!\n");
                    exit(-1);
                }
                else
                {
                    printf("Terminada a ligação da porta série.\n");
                    printf("-> Fim da Fase de Terminação <-\n");
                }
            }
        }
    }
    registoOcorrencias();
}
