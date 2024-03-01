// Pedro Antônio da Silva | 194828
// Grégory Fortunato | 179772

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>

int CLI, PT, PBP;
float PAT;
int MINATEN, MAXATEN;

int clientesTatuador = 0;
int clientesPiercer = 0;
int clientesAtendidosSemEsperar = 0;
int clientesNaoAtendidos = 0;

int tempoEsperaTatuador = 0;
int tempoEsperaPiercer = 0;
int fila_puf_tatuador = 0;
int fila_puf_piercer = 0;

typedef struct
{
    int id;
    int tempoAtendimento;
} InfoClientes;

// informações individuais de cada cliente
InfoClientes *infoCliTatuador;
InfoClientes *infoCliPiercer;

// declarando semáforos
sem_t sem_tatuador;
sem_t sem_piercer;

void *atendimentoTatuador(void *arg)
{
    int proximo = 1;
    while (proximo <= CLI)
    {
        sem_wait(&sem_tatuador);

        InfoClientes cli = infoCliTatuador[proximo]; // acesso a infoCliTatuador global

        printf("[TATUADOR] Tatuador INICIOU o atendimento do cliente %d. Tempo de atendimento %d\n", cli.id, cli.tempoAtendimento);
        usleep(cli.tempoAtendimento * 5000);
        printf("[TATUADOR] Tatuador FINALIZOU o atendimento do cliente %d.\n", cli.id);
        usleep(100);

        fila_puf_tatuador--;
        proximo++;
    }
    return NULL;
}

void *atendimentoPiercer(void *arg)
{
    int proximo = 1;
    while (proximo <= CLI)
    {
        sem_wait(&sem_piercer);

        InfoClientes cli = infoCliPiercer[proximo]; // acesso a infoCliPiercer global

        printf("[PIERCING] Body piercer INICIOU o atendimento do cliente %d. Tempo de atendimento: %d.\n", cli.id, cli.tempoAtendimento);
        usleep(cli.tempoAtendimento * 5000);
        printf("[PIERCING] Body piercer FINALIZOU o atendimento do cliente %d.\n", cli.id);
        usleep(100);

        fila_puf_piercer--;
        proximo++;
    }
    return NULL;
}

void *recepcaoCliente(void *arg)
{
    infoCliTatuador = malloc(CLI * sizeof(InfoClientes));
    infoCliPiercer = malloc(CLI * sizeof(InfoClientes));

    while (clientesTatuador + clientesPiercer < CLI * 2)
    {
        usleep(1000000); // Tempo entre a chegada dos clientes

        int chegada_clientes = (rand() % 10) + 1; // Chegada de clientes

        for (int i = 0; i < chegada_clientes; i++)
        {

            double aux = ((double)rand() / RAND_MAX); // Escolher serviço 0 até 1

            if (aux <= PAT && clientesTatuador < CLI) // Cliente para tatuador
            {
                if (fila_puf_tatuador <= PT)
                {
                    tempoEsperaTatuador = (rand() % (MAXATEN - MINATEN + 1)) + MINATEN;
                    clientesTatuador++;
                    infoCliTatuador = realloc(infoCliTatuador, (clientesTatuador + 1) * sizeof(InfoClientes)); // add informações ao array

                    if (fila_puf_tatuador == 0)
                    {
                        printf("[GERADOR] Cliente %d de tatuagem chegou e será atendido sem esperar. Tempo de atendimento %d.\n", clientesTatuador, tempoEsperaTatuador);
                        usleep(100);
                        clientesAtendidosSemEsperar++;
                    }
                    else
                    {
                        printf("[GERADOR] Cliente %d de tatuagem chegou e foi sentar em puff de espera %d. Tempo de espera: %d.\n", clientesTatuador, fila_puf_tatuador, tempoEsperaTatuador);
                        usleep(100);
                    }
                    fila_puf_tatuador++;

                    infoCliTatuador[clientesTatuador].id = clientesTatuador;
                    infoCliTatuador[clientesTatuador].tempoAtendimento = tempoEsperaTatuador;

                    sem_post(&sem_tatuador);
                }
                else
                {
                    printf("[GERADOR] Cliente de tatuagem chegou e foi embora sem atendimento\n");
                    usleep(100);
                    clientesNaoAtendidos++;
                }
            }
            else if (aux > PAT && clientesPiercer < CLI) // Cliente para body piercer
            {
                if (fila_puf_piercer <= PBP)
                {
                    tempoEsperaPiercer = (rand() % (MAXATEN - MINATEN + 1)) + MINATEN;
                    clientesPiercer++;
                    infoCliPiercer = realloc(infoCliPiercer, (clientesPiercer + 1) * sizeof(InfoClientes)); // add informações ao array

                    if (fila_puf_piercer == 0)
                    {
                        printf("[GERADOR] Cliente %d de body piercing chegou e será atendido sem esperar.Tempo de atendimento %d\n", clientesPiercer, tempoEsperaPiercer);
                        usleep(100);
                        clientesAtendidosSemEsperar++;
                    }
                    else
                    {
                        printf("[GERADOR] Cliente %d de body piercing chegou e foi sentar em puff de espera %d. Tempo de espera: %d.\n", clientesPiercer, fila_puf_piercer, tempoEsperaPiercer);
                        usleep(100);
                    }
                    fila_puf_piercer++;

                    infoCliPiercer[clientesPiercer].id = clientesPiercer;
                    infoCliPiercer[clientesPiercer].tempoAtendimento = tempoEsperaPiercer;

                    sem_post(&sem_piercer);
                }
                else
                {
                    printf("[GERADOR] Cliente de body piercing chegou e foi embora sem atendimento\n");
                    usleep(100);
                    clientesNaoAtendidos++;
                }
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{

    CLI = atoi(argv[1]);
    PT = atoi(argv[2]);
    PBP = atoi(argv[3]);
    PAT = atof(argv[4]);
    MINATEN = atoi(argv[5]);
    MAXATEN = atoi(argv[6]);

    // Inserção inválida
    if (CLI < 10 || CLI > 100 ||
        PT < 0 || PT > 10 ||
        PBP < 0 || PBP > 10 ||
        PAT < 0 || PAT > 0.99 ||
        MINATEN < 0 || MINATEN > 100 ||
        MAXATEN < MINATEN || MAXATEN > 500)
    {
        printf("VALORES DE ENTRADA INVÁLIDOS.\n");
        return 1;
    }

    infoCliPiercer = NULL;
    infoCliTatuador = NULL;

    srand(time(NULL));

    // Inicializando semáforos
    sem_init(&sem_tatuador, 0, 0);
    sem_init(&sem_piercer, 0, 0);

    pthread_t th_tatuador, th_piercer, th_chegadaClientes;
    pthread_create(&th_tatuador, NULL, atendimentoTatuador, NULL);
    printf("[TATUADOR] Tatuador disponível para atender ...\n");
    pthread_create(&th_piercer, NULL, atendimentoPiercer, NULL);
    printf("[PIERCING] Body piercer disponível para atender ...\n");
    pthread_create(&th_chegadaClientes, NULL, recepcaoCliente, NULL);

    pthread_join(th_tatuador, NULL);
    printf("[TATUADOR] Thread Finalizou\n");
    pthread_join(th_piercer, NULL);
    printf("[PIERCING] Thread Finalizou\n");
    pthread_join(th_chegadaClientes, NULL);
    printf("[GERADOR] Thread Finalizou\n");

    free(infoCliPiercer);
    free(infoCliTatuador);
    sem_destroy(&sem_tatuador);
    sem_destroy(&sem_piercer);

    // Análise
    printf("-----------------------------------------------------------------\n");
    printf("[MAIN] Clientes que foram atendidos (total): %d\n", (clientesTatuador + clientesPiercer));
    printf("[MAIN] Clientes atendidos pelo tatuador: %d\n", clientesTatuador);
    printf("[MAIN] Clientes atendidos pelo body piercer: %d\n", clientesPiercer);
    printf("[MAIN] Clientes que foram atendidos sem esperar: %d\n", clientesAtendidosSemEsperar);
    printf("[MAIN] Clientes que foram embora, sem serem atendidos: %d\n", clientesNaoAtendidos);
    printf("-----------------------------------------------------------------\n");
    return 0;
}
