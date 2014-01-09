/*Non-Canonical Input Processing*/
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
 
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define  _XOPEN_SOURCE_EXTENDED 1
#define MAX_SIZE 1000 /*tamanho da trama*/
#define FLAG 0x7E /*flag de controlo - 0111_1110*/
 
 
struct linkLayer {
    int fileDescriptor;
    unsigned int Ns;
    unsigned int Nr;
    unsigned int timeout;
    unsigned int numTransmissions;
    unsigned char trama[MAX_SIZE]; //array para as tramas I
    unsigned char controlo[5];  //array para as tramas de supervisao
}protocolo;
 
struct termios oldtio, newtio;
char atender; // decide qual a funçao a reenviar aquando um timeout
int flag = 1; // flag que controla o alarm
int conta = 1; // contador para o número de tentativas de reenvio de uma trama
int n; // tamanho da trama I
 
extern int itrans, iretrans, otout, orej;
 
/**
*   Funçao chamada aquando o timeout
*   Dependendo do caracter atender reenvia a trama que falhou
*   Depois de ter enviado repoe a flag a 1 e incrementa o contador
*/
void atende()
{  
    if (atender == 'S') //retransmite a trama SET
    {
        printf("\nTentativa de envio da trama SET número # %d\n", conta + 1);
        enviarSET();
        printf("À espera de receber a trama UA!\n");
    }
    else if (atender == 'D') //retransmite a trama DISC
    {
        printf("\nTentativa de envio da trama DISC número # %d\n", conta + 1);
        enviarDISC();
        printf("À espera de receber a trama DISC!\n");
    }
    else if (atender == 'I') //retransmite a trama I
    {
        printf("\nTentativa de envio da trama I(Ns = %d) número # %d\n", protocolo.Ns, conta + 1);
        enviarInformacao(n + 6);
        iretrans++;
        printf("À espera de receber a trama RR(Nr = %d) ou REJ!\n", protocolo.Nr);
    }
    flag = 1;
    conta++;
    otout++;
}
 
/**
*   Permite ao utilizador definir o baudrate
*/
void definirBaudrate()
{
    unsigned int escolha;
    printf("Lista de baudrates suportados pela porta série:\n");
    printf("\t1 - 50\t\t2 - 75\t\t3 - 110\t\t4- 134\t\t5 - 150\n\t6 - 200\t\t7 - 300\t\t8 - 600\t\t9 - 1200\t10 - 1800\n\t11 - 2400\t12 - 4800\t13 - 9600\t14 - 19200\n\t15 - 38400\t16 - 57600\t17 - 115200\n");
    printf("Qual o baudrate pretendido para a porta série? (escolha o número da lista, entre 1 e 17) ");
    scanf("%u", &escolha);
    switch (escolha)
    {
        case 1 : newtio.c_cflag = B50 | CS8 | CLOCAL | CREAD;
        case 2 : newtio.c_cflag = B75 | CS8 | CLOCAL | CREAD;
        case 3 : newtio.c_cflag = B110 | CS8 | CLOCAL | CREAD;
        case 4 : newtio.c_cflag = B134 | CS8 | CLOCAL | CREAD;     
        case 5 : newtio.c_cflag = B150 | CS8 | CLOCAL | CREAD;
        case 6 : newtio.c_cflag = B200 | CS8 | CLOCAL | CREAD;
        case 7 : newtio.c_cflag = B300 | CS8 | CLOCAL | CREAD;
        case 8 : newtio.c_cflag = B600 | CS8 | CLOCAL | CREAD;
        case 9 : newtio.c_cflag = B1200 | CS8 | CLOCAL | CREAD;
        case 10 : newtio.c_cflag = B1800 | CS8 | CLOCAL | CREAD;
        case 11 : newtio.c_cflag = B2400 | CS8 | CLOCAL | CREAD;
        case 12 : newtio.c_cflag = B4800 | CS8 | CLOCAL | CREAD;
        case 13 : newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
        case 14 : newtio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
        case 15 : newtio.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
        case 16 : newtio.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
        case 17 : newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
        default : newtio.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
    }
}
/**
*   Permite ao utilizador definir o número de tentativas de retransmissao
*/
void definirTentativas()
{
    printf("Qual o número de tentativas de retransmissão? ");
    scanf("%u", &protocolo.numTransmissions);
}
 
/**
*   Permite ao utilizador definir o timeout
*/
void definirTimeout()
{
    printf("Qual o timeout entre retransmissões? ");
    scanf("%d", &protocolo.timeout);
}
 
/**
*   Permite ao utilizador configurar o protocolo   
*/
void definicoes()
{
    int escolha;
    printf("Pretende utilizar as definições automáticas para o protocolo? (Introduza o número: 1 - sim ou 2 - não) ");
    scanf("%d", &escolha);
    if (escolha == 2)
    {
        definirBaudrate();
        definirTentativas();
        definirTimeout();
    }
    else
    {
        newtio.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
        protocolo.numTransmissions = 5;
        protocolo.timeout = 3;
    }
}
//Funções de envio das tramas
/**
 * Função que envia dados de supervisão e verifica se foi ou não bem sucedido o envio
 * @return 1 se o envio for bem sucedido e 0 caso seja mal sucedido
 */
int enviarSupervisao(char tipo[])
{
    if (write(protocolo.fileDescriptor, protocolo.trama, 5) == 5) //retorna o número de caracteres escritos
    {
        printf("Envio bem sucedido da trama %s!\n", tipo);
        return 1; //envio bem sucedido
    }
    else
        return 0; //envio mal sucedido
}
 
/**
 * Função que envia tramas de informaçao e verifica se foi ou não bem sucedido o envio
 * @return 1 se o envio for bem sucedido e 0 caso seja mal sucedido
 */
int enviarInformacao(int tamanho)
{
    if (write(protocolo.fileDescriptor, protocolo.trama, tamanho) == tamanho) //devem ser escritos tamanho bytes
    {
        printf("Envio bem sucedido da trama I (Ns = %d)!\n", protocolo.Ns);
        return 1; //envio bem sucedido
    }
    else
        return 0; //envio mal sucedido
}
 
//Funções de envio dos vários tipos de tramas
/**
 * Envia a trama de set up (SET) com a seguinte constituição:<br>
 * > tipo de trama - "SET"<br>
 * > campo de endereço (A) - 0x03 (0000_0011)<br>
 * > campo de controlo (C) - 0x03 (0000_0011)<br>
 * > campo de supervisão (BCC) - A xor C<br>
 * > trama SET - (F, A, C, BCC, F)
 */
int enviarSET()
{
    protocolo.trama[0] = FLAG;
    protocolo.trama[1] = 0x03;
    protocolo.trama[2] = 0x03;
    protocolo.trama[3] = 0x03 ^ 0x03;
    protocolo.trama[4] = FLAG;
    return enviarSupervisao("SET"); //chama a função que envia a trama
}
 
/**
 * Envia a trama disconnect (DISC) com a seguinte constituição:<br>
 * > tipo de trama - "DISC"<br>
 * > campo de endereço (A) - 0x01 (0000_0001)<br>
 * > campo de controlo (C) - 0x0B (0000_1011)<br>
 * > campo de supervisão (BCC) - A xor C<br>
 * > trama SET - (F, A, C, BCC, F)
 */
int enviarDISC()
{
    //unsigned char DISC[5] = {FLAG, 0x01, 0x0B, 0x01 ^ 0x0B, FLAG};
    protocolo.trama[0] = FLAG;
    protocolo.trama[1] = 0x01;
    protocolo.trama[2] = 0x0B;
    protocolo.trama[3] = 0x01 ^ 0x0B;
    protocolo.trama[4] = FLAG;
    return enviarSupervisao("DISC"); //chama a função que envia a trama
}
 
/**
 * Envia a trama unnumbered acknowledgment (UA) com a seguinte constituição:<br>
 * > tipo de trama - "UA"<br>
 * > campo de endereço (A) - 0x01 (0000_0001)<br>
 * > campo de controlo (C) - 0x07 (0000_0111)<br>
 * > campo de supervisão (BCC) - A xor C<br>
 * > trama SET - (F, A, C, BCC, F)
 */
int enviarUA()
{
    //unsigned char UA[5] = {FLAG, 0x01, 0x07, 0x01 ^ 0x07, FLAG};
    protocolo.trama[0] = FLAG;
    protocolo.trama[1] = 0x01;
    protocolo.trama[2] = 0x07;
    protocolo.trama[3] = 0x01 ^ 0x07;
    protocolo.trama[4] = FLAG;
    return enviarSupervisao("UA"); //chama a função que envia a trama
}
 
/**
 * Função de recepção de trama UA
 * @return 1 se bem sucedido e 0 se não
 */
int receberUA()
{
    if((read(protocolo.fileDescriptor, protocolo.controlo, 5)) == 5)
    {
        if(protocolo.controlo[0] == FLAG)
        {
            if(protocolo.controlo[1] == 0x03)
            {
                if(protocolo.controlo[2] == 0x07)
                {
                    if(protocolo.controlo[3] == 0x03 ^ 0x07)
                    {
                        if(protocolo.controlo[4] == FLAG)
                        {
                            printf("Recebida a trama UA com sucesso!\n");
                            return 1;
                        }
                        else
                            return 0;
                    }
                    else
                        return 0;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else
            return 0;
    }
    else
        return 0;
}
 
/**
 * Função que recebe trama RR ou REJ
 * @return 1 ou 2 se bem sucedido, caso receba RR ou REJ, respectivamente e 0 se não
 */
int receberRRouREJ()
{
    unsigned char Crr, Crej;
    if(protocolo.Nr)
    {
        Crr = 0x21;
        Crej = 0x05;
    }
    else
    {
        Crr = 0x01;
        Crej = 0x25;
    }
    if((read(protocolo.fileDescriptor, protocolo.controlo, 5)) == 5)
    {  
        if(protocolo.controlo[0] == FLAG)
        {
            if(protocolo.controlo[1] == 0x03)
            {
                if(protocolo.controlo[2] == Crr)
                {
                    if(protocolo.controlo[3] == 0x03 ^ Crr)
                    {
                        if(protocolo.controlo[4] == FLAG)
                        {
                            printf("Recebida a trama RR(Nr = %d) com sucesso!\n", protocolo.Nr);
                            return 1;
                        }
                        else
                            return 0;
                    }
                    else
                        return 0;
                }
                else if (protocolo.controlo[2] == Crej)
                {
                    if(protocolo.controlo[3] == 0x03 ^ Crej)
                    {
                        if(protocolo.controlo[4] == FLAG)
                        {
                            printf("Recebida a trama REJ(Nr = %d) com sucesso!\n", protocolo.Nr);
                            return 2;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                        return 0;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else
            return 0;
    }
    else
        return 0;
}
 
/**
 * Função de recepção de trama UA
 * @return 1 se bem sucedido e 0 se não
 */
int receberDISC()
{
    if((read(protocolo.fileDescriptor, protocolo.controlo, 5)) == 5)
    {
        if(protocolo.controlo[0] == FLAG)
        {
            if(protocolo.controlo[1] == 0x01)
            {
                if(protocolo.controlo[2] == 0x0B)
                {
                    if(protocolo.controlo[3] == 0x01 ^ 0x0B)
                    {
                        if(protocolo.controlo[4] == FLAG)
                        {
                            printf("Recebida a trama DISC com sucesso!\n");
                            return 1;
                        }
                        else
                            return 0;
                    }
                    else
                        return 0;
                }
                else
                    return 0;
            }
            else
                return 0;
        }
        else
            return 0;
    }
    else
    {   return 0;}
}
 
/**
*   Configura a porta série, envia a trama SET, espera pela trama UA
*   @param porta porta a ser configurada
*   @return 1 se bem sucedido e -1 se nao
*/
int llopen(char * porta)
{
    protocolo.fileDescriptor = open(porta, O_RDWR | O_NOCTTY );
    if (protocolo.fileDescriptor < 0)
    {
        perror(porta);
        return -1;
    }
    if (tcgetattr(protocolo.fileDescriptor, &oldtio) == -1) /* save current port settings */
    {
        perror("tcgetattr");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    definicoes();
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;
    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;   /* blocking read until 5 chars received */
  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
  */
    tcflush(protocolo.fileDescriptor, TCIOFLUSH);
    if (tcsetattr(protocolo.fileDescriptor,TCSANOW,&newtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }
    //Fase de estabelecimento
    //verifica o envio da trama I
    printf("\n-> Início da Fase de Estabelecimento <- \nTentativa de envio da trama SET número # 1\n");
    (void) signal(SIGALRM, atende); //instala rotina que atende interrupção por tempo para a trama SET
    if (enviarSET())
    {
        printf("À espera de receber a trama UA!\n");
        while(conta < protocolo.numTransmissions)
        {
            if (flag)
            {
                atender = 'S'; //atender interrupção para a trama SET
                alarm(protocolo.timeout); //activa alarme de 3s
                flag = 0;
            }
            if (receberUA())
            {
                alarm(0);
                flag = 1;
                conta = 1; //reinicia a contagem das tentativas de envio
                protocolo.Nr = 1;
                return protocolo.fileDescriptor;
            }
        }
        if (conta == protocolo.numTransmissions)
        {
            printf("\t!Falha na recepção da trama UA!\n");
            tcflush(protocolo.fileDescriptor, TCIOFLUSH);
            close(porta);
            return -1;
        }
    }
    else
    {
        printf("\t!Erro no envio da trama SET!\n");
        return -1;
    }
}
 
/**
*   Conta a quantidade de octetos 0x7E e 0x7D num array
*   @param x array a ser analizado
*   @param tamanho tamanho do array a ser analizado
*   @return retorna o número de octetos que foram encontrados
*/
int numOctetos(unsigned char x[], int tamanho)
{
    int i, contar = 0;
    for(i = 0 ; i <= tamanho + 1 ; i++)
    {
        if(x[i] == 0x7E || x[i] == 0x7D)
            contar++;
    }
    return contar;
}
 
/**
*   Realiza o stuffing do array desejado
*   @param temp array a ser analizado
*   @param v array onde será guardado o temp stuffed
*   @param contar tamanho do array v
*/
void stuffing(unsigned char temp[], unsigned char v[], int contar)
{
    int i, j;
    for(i = 0, j = 0 ; i < contar ; i++)
    {
        if(temp[i] == 0x7E)
        {
            v[j] = 0x7D;
            j++;
            v[j] = 0x5E;
            j++;
        }
        else if(temp[i] == 0x7D)
        {
            v[j] = 0x7D;
            j++;
            v[j] = 0x5D;
            j++;
        }
        else
        {
            v[j] = temp[i];
            j++;
        }
    }
}
 
/**
*   Envia pela porta série as tramas de informaçao e espera pelas tramas de supervisao
*   @param fd descriptor da porta série
*   @param buffer array a ser transmitido pela porta
*   @param lenght tamanho do array a ser enviado
*   @return 1 quando recebe RR e -1 quando falha o envio de uma trama I
*/
int llwrite(int fd, unsigned char * buffer, int lenght)
{
    printf("\nTentativa de envio da trama I(Ns = %d) número # 1\n", protocolo.Ns);
    itrans++;
    n = numOctetos(buffer, lenght) + lenght;
    unsigned char stuffed[n];
    stuffing(buffer, stuffed, n);  
    protocolo.trama[0] = FLAG;
    protocolo.trama[1] = 0x03;
    if(protocolo.Ns)
        protocolo.trama[2] = 0x02;
    else
        protocolo.trama[2] = 0x00;
    protocolo.trama[3] = 0x03 ^ protocolo.trama[2];
    int i, r;
    for(i = 0 ; i < n ; i++)
        protocolo.trama[4 + i] = stuffed[i];
    unsigned char BCC2;
    for(i = 0 ; i < lenght ; i++)
        BCC2 ^= buffer[i];
    protocolo.trama[4 + n] = BCC2;
    protocolo.trama[4 + n + 1] = FLAG;
    //verifica o envio da trama I
    if(enviarInformacao(n + 6)) //sendo bem enviada passa a transmitir nova trama I
    {
        printf("À espera de receber a trama RR(Nr = %d) ou REJ!\n", protocolo.Nr);
        while(conta < protocolo.numTransmissions)
        {
            if (flag)
            {
                atender = 'I'; //atender interrupção para a trama I
                alarm(protocolo.timeout); // activa alarme de 3s
                flag = 0;
            }
            r = receberRRouREJ(protocolo.Nr);
            if (r == 1)
            {
                protocolo.Ns = !protocolo.Ns;
                protocolo.Nr = !protocolo.Nr;
                alarm(0);
                flag = 1;
                conta = 1; //reinicia a contagem das tentativas de envio
                return 1;
            }
            else if (r == 2)
            {
                alarm(0);
                flag = 1;
                conta = 1; //reinicia a contagem das tentativas de envio
                orej++;
                enviarInformacao(n + 6);
                continue;
            }
        }
        if (conta == protocolo.numTransmissions)
        {
            printf("\t!Falha na recepção da trama: RR(Nr = %d) ou REJ!\n", protocolo.Nr);
            tcflush(protocolo.fileDescriptor, TCIOFLUSH);
            close(protocolo.fileDescriptor);
            return -1;
        }
    }
    else
    {
        printf("\t!Erro no envio da trama I (Ns = %d)!\n", protocolo.Ns);
        return -1;
    }
}
 
/**
*   Termina as ligaçoes e fecha a porta <br>
*   Envia a trama DISC, espera pela trama DISC do receptor e envia a resposta UA
*   @param porta descriptor da porta série
*   @return 1 se ocorrer tudo bem e -1 se algo falhar
*/
int llclose(int porta)
{
    if (tcsetattr(porta, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        return -1;
    }
    else
    {
        //Fase de terminação
        printf("\nTentativa de envio da trama DISC número # 1\n");
        //verifica o envio da trama DISC
        if (enviarDISC()) //envia a trama DISC
        {
            printf("À espera de receber a trama DISC!\n");
            while (conta < protocolo.numTransmissions)
            {
                if (flag)
                {
                    atender = 'D'; //atender interrupção para a trama DISC
                    alarm(protocolo.timeout); // activa alarme de 3s
                    flag = 0;
                }
                if (receberDISC())
                {
                    alarm(0);
                    flag = 1;
                    conta = 1; //reinicia a contagem das tentativas de envio
                    break;
                }
            }
            if (conta == protocolo.numTransmissions)
            {
                printf("\t!Falha na recepção da trama DISC!\n");
                tcflush(protocolo.fileDescriptor, TCIOFLUSH);
                close(porta);
                return -1;
            }
        }
        else
        {
            printf("\t!Erro no envio da trama DISC!\n");
            return -1;
        }
        if(enviarUA())
        {
            tcflush(protocolo.fileDescriptor, TCIOFLUSH);
            close(porta);
            return 1;
        }
        else
        {
            printf("\t!Erro no envio da trama UA!\n");
            return -1;
        }
         
    }
}
