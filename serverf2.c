//gcc serverf2.c -o serverf2 -lpthread gia compile
//aparaithta include gia to programma
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/un.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#define SHM_SIZE_PARAGGELIA 500*sizeof(struct struct_par)//megethos ths domhs paragellia sthn  shared memory 
#define SHM_KEY_PARAGGELIA 47834790//kleidi ths shared memory (to ID ths dhladh)
#define SEM_NAME "my_sem"//onoma semaforou pitsas
#define SEM_NAMEP "psistis"//onoma semaforou psisti
#define SEM_NAMED "dianomeas"//onoma semaforou dianomea
#define SOCK_PATH "my_path"//path tou socket mas
#define LISTENQ 45 //oura aitisewn pou tha dexetai o server mas.Mexri 20 aithseis
#define SPECIAL_TIME 150000 //xronos psisimatos mias pitsas special
#define PEPPERONI_TIME 120000 //xronos psisimatos mias pitsas pepperoni
#define MARGARITA_TIME 100000 //xronos psisimatos mias pitsas margarita
#define MAKRIA_TIME 100000 //xronos pou xreiazetai mia paraggelia na paei an einai makria o pelaths
#define KONTA_TIME 50000 ////xronos pou xreiazetai mia paraggelia na paei an einai konta o pelaths

struct struct_par//Domh typou struct_par pou periexei thn paraggelia pou stelnetai apo ton client
{
  int arithmos_special; //plithos special.Apo 0-3(tha elegxetai apo ton client)
  int arithmos_pepperoni; //plithos pepperoni
  int arithmos_margarita; //plithos margarita
  int apostash; //apostash ths paraggelias (0 gia kontino pelath kai 1 gia makrino).Tha elegxetai apo ton client
  int no;//to noumero ths kathe paraggelias
}paraggelia; //dhmiourgia mias metavlitis typou struct struct_par gia tis paraggelies mas



int sockfd,newsockfd,serverlength,clientlength;// sockfd = arxiko socket pou syndeei client me server
                                   //newsockfd = socket pou dhmiourgeitai gia na eksipiretisei o server
                                   //ton client
                                   //serverlength = mhkos dieuthinsewn tou server mas sthn mnhmh
                                   //clientlength = mhkos dieuthinsewn tou client sthn mnhmh

struct sockaddr_un serveraddress,clientaddress;//domh me dieuthinseis tou server kai tou client

int shm_id_paraggelia; //metavliti pou periexei to onoma ths shared memory

size_t shm_size_paraggelia = SHM_SIZE_PARAGGELIA;//megethos shared memory typou size_t

key_t shm_key_paraggelia = SHM_KEY_PARAGGELIA;//kleidi gia th shared memory

struct struct_par *ptr_paraggelia;//orisame enan pointer typou struct_par gia na deixnei sth domh mas
				  //sthn koinh mnhmh


sem_t *my_sem;// semaforos pitsas
sem_t *psistis;//semaforos psisti
sem_t *dianomeas;//semaforos dianomea

char buff2[256]="We are sorry that your order came late..Have a free cola..";//pinakas me to mhnyma oti
//o pelaths prepei na parei coca cola.To vazoume se ena buff2 gia na einai pio syntomes oi entoles
char buff3[256]="Here is your order sir..";//Omoiws me to buff2

int *ptr2_paraggelia;//deikths pou deixnei monima sthn 1h thesi ths shared memory
                    //voithaei gia na ginei argotera to dettach

void sig_zombiekiller(int signo)//Synarthsh pou skotonei tis zombie proccesses
{
  int stat;
  pid_t pid;//o pateras pou exei kanei fork() meta prepei na kalesei thn wait etsi wste h child
  while((pid=waitpid(-1,&stat,WNOHANG))>0) //proccess na mhn desmeuei ypologistikous porous enw exei
  {					   //teleiwsei th douleia ths

	printf("\nchild %d was terminated\n\n",pid);
  }
}

void father_wait(void)//synarthsh h opoia kaleitai gia na kanei wait o pateras mexri na teleiwsoun ta
{		     //paidia tou gia na mhn proxwrhsei parakatw ston kwdika kai steilei thn paraggelia
	 	     //prin psithoun oi pitses
	 int *stat2;
 	 pid_t pid2;
 	 while((pid2=waitpid(-1,stat2,0))!=-1)//to -1 dhlwnei pws o pateras tha perimenei ola ta paidia
 	 {				      //na termatisoun prin synexisei ston kwdika.To 0 sthn
					      //waitpid dhlwnei pws den exoume thn WNOHANG me thn opoia
					      //otan den exei termatisei kanena paidi tote o pateras
					      //termatizei kai den perimenei.
        	printf("\nfather %d waited for the pizza!\n\n",pid2);
  	 }


}

void give_coca_cola(int x) //synarthsh h opoia kaleitai otan mia paraggelia exei kathisterisei
{			//kai stelnei ston client mhnyma oti phre coca-cola
	signal(SIGALRM,give_coca_cola);//anadromh gia na ekteleitai me kathe signal h catch_delay
	write(newsockfd,buff2,sizeof(buff2));
}

void sig_exit(int signal)//synarthsh pou kaleite apo to xrhsth otan pathsei CTRL-C kai teleiwnei
{			 // to programma

	int newsockfd;
	sem_t *my_sem;
	sem_t *psistis;
	sem_t *dianomeas;
	close(newsockfd);   //kleinei genika to programma,olous tous semaforous kai kanei dettach
	sem_close(my_sem);  //thn mnhmh mas gia na teleiwsoun ola
	sem_close(psistis);
	sem_close(dianomeas);
	sem_unlink(SEM_NAME);
	sem_unlink(SEM_NAMEP);
	sem_unlink(SEM_NAMED);
	if((int)shmdt(ptr2_paraggelia)==-1) //casting se int kai elegxos an egine apokollisi
	{
		printf("Couldn't dettach from shared memory\n");
		exit(1);
	}
	shmctl(shm_id_paraggelia,IPC_RMID,NULL);
	exit(0);
}



int main()
{

        signal(SIGCHLD, sig_zombiekiller);//signals pou kalountai mesa sth main gia na apofygoun
					  //zombie processes
        signal(SIGINT, sig_exit);//signal pou tha ginetai termatismos tou server kai apodesmeush
				 // ypologistikwn porwn


        //CREATION OF THE SHARED MEMORY-ME TO IPC_CREAT DHMIOURGOUME THN KOINH MNHMH APO TO MHDEN

        shm_id_paraggelia = shmget(shm_key_paraggelia,shm_size_paraggelia,0600|IPC_CREAT);//ta dikaiwmata
											  //einai 0600

        //ELEGXOS DHMIOURGIAS SHARED MEMORY

        if(shm_id_paraggelia<0)
        {
                printf("Sorry.Creation of shared memory failed!\n"); 
                exit(1);
        }

        //PROSKOLLISI THS DIERGASIAS-PATERA STH SHARED MEMORY

        ptr_paraggelia = (struct struct_par*)shmat(shm_id_paraggelia,NULL,0); //Egine casting giati to
									      // ptr_paraggelia einai
									      // deikths typou struct_par
									      // kai h shmat prepei na
									      // epistrepsei timh integer
	ptr2_paraggelia = (int *)ptr_paraggelia;//deixnei MONIMA sthn 1h thesi ths shared memory.
					        //Tha voithisei argotera sta dettach ths mnhmhs

	ptr_paraggelia = malloc(500*(sizeof(struct struct_par)));//dynamikh desmeysh mnhmhs
                                                                //desmeoume 500 theseis mnhmhs epi
                                                                //oso megethos pianei mia domh mas
                                                                //o ptr_paraggelia deixnei sthn arxh
                                                                //ths kathe struct sth shared memory
        ptr_paraggelia[0].no = 0;//orizoume to noumero ths 1hs paraggelias mhden.Tha xreiastei argotera
				//gia na ksexwrizoume se poia domh sthn koinh mnhmh tha apothikeuoume
				//ta dedomena pou stelnei o client analoga me thn paraggelia


	//ELEGXOS GIA TO ATTACH

        if(ptr_paraggelia == (struct struct_par *)-1)//to cast egine gia na epistrafei typos struct_par
        {
                printf("Sorry.Attachment Failed!\n");
                exit(1);
        }

	//PROSKOLLISI METAVLITIS psisti STH SHARED MEMORY-ELEGXOS

	psistis = (sem_t*)shmat(shm_id_paraggelia, NULL,0);//o psistis einai semaforos opote kaname cast
	if(psistis ==(sem_t*) -1)			   //gia na epistafei swsth timh
	{
		printf("Attachement of 'psistis' failed\n");
		exit(1);
	}

	//PROSKOLLISI METAVLITIS dianomeas STH SHARED MEMORY-ELEGXOS
	dianomeas =(sem_t*)shmat(shm_id_paraggelia,NULL,0);//omoia me ton psisti
	if(dianomeas ==(sem_t*) -1)
	{
		printf("Attachement of 'dianomeas' failed\n");
		exit(1);
	}


        //DHMIOURGIA SEMAPHORES PITSWN-ELEGXOS
	my_sem = sem_open(SEM_NAME,O_CREAT | O_RDWR,S_IRUSR | S_IWUSR,1);//prin thn klhsh fork
	if(my_sem == SEM_FAILED)					//ton arxhkopoioume se 1
	{
		printf("Sorry.Pizza's semaphore failed\n");
		exit(1);
	}

	//DHMIOURGIA SEMAPHORE THS METAVLITIS psistis-ELEGXOS
	psistis = sem_open(SEM_NAMEP,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR,9);//Ton arxikopoioume se 9 gia na
	if(psistis == SEM_FAILED)				       //mporoun na dhmiourgountai mexri
	{							      //kai 10 psistes tautoxrona,kai
		printf("Sorry.Cooker's semaphore failed\n");	      //molis parei timh -1 tote kanei
		exit(1);					      //block gia na mhn ftiaxtoun alloi
	}

	//DHMIOURGIA SEMAPHORES dianomeas-ELEGXOS
	dianomeas = sem_open(SEM_NAMED,O_CREAT | O_RDWR,S_IRUSR | S_IWUSR,9);//Omoiws me ton psisti
	if(dianomeas == SEM_FAILED)					     //arxikopioish se 9 gia ton
	{								     //idio logo
		printf("Sorry.Delivery's semaphore failed\n");
		exit(1);
	}

	//DHMIOURGIA sockfd TOU SERVER-ELEGXOS

	sockfd = socket(AF_UNIX,SOCK_STREAM,0);//AF_UNIX giati server kai client einai sto idio mhxanhma
	if(sockfd==-1)				//SOCK_STREAM einai o tropos apostolhs mhnymatwn
	{
		perror("SOCKET");
		exit(1);
	}

	serveraddress.sun_family = AF_UNIX; //orismos tou typou oikogeneias socket.Xrhsimopoioume AF_UNIX
					    // giati vriskontai sto idio mhxanhma o client kai o server
	strcpy(serveraddress.sun_path , SOCK_PATH); //dhlwsh gia to onoma tou socket
	unlink(serveraddress.sun_path);//diagrafh opoiudhpote prohgoumenou socket me to idio onoma

	serverlength = strlen(serveraddress.sun_path) + sizeof(serveraddress.sun_family);//mhkos address
											 //tou server

	//Syndesh sockfd me local port leitourgikou kai elegxos an egine h syndesh gia na kserei
	//o kernel poio local port tou leitourgikou na antistoixisei me to sockfd

	if(bind(sockfd,(struct sockaddr *)&serveraddress ,serverlength)==-1)
	{
		perror("BIND");
		exit(1);
	}

	//Entolh listen gia na perimenei o server mexri LISTENQ aithseis apo to server

	if(listen(sockfd,LISTENQ) == -1)
	{
		perror("LISTEN");
		exit(1);
	}


	for(;;)//anoigma server gia infinite loop
	{
		printf("\nSERVER OPENED....\n");

		//APODOXH tou newsockfd gia ton client.To newsockfd tha dexetai ta mhnymata tou client

		clientlength = sizeof(clientaddress);//megethos dieuthinsewn tou client (newsockfd)

		newsockfd = accept(sockfd,(struct sockaddr *)&clientaddress,&clientlength);

		//elegxos an egine h apodoxh
		if(newsockfd == -1)
		{
			perror("ACCEPT");
			continue;
		}

		printf("SERVER CONNECTED!\n");//O server einai syndedemenos kai mpoorei na lavei pleon
					      //mhnymata kai plhrofories apo ton client

		write(newsockfd,buff2,sizeof(buff2));//stelnei ston client to buff2 to opoio periexei to
						     //mhnyma oti prepei na parei coca cola apla gia na
						    //to apothikeusei wste pio meta na to sygkrinei o
					            //client me to buff pou tha steilei o server
						    //vlepe client gia na katalaveis

		//DHMIOURGIA PAIDIWN


		if(fork()==0)//fork gia kathe paraggelia
		{
			//Voithitikes metavlites pou mas deixnoun ton arithmo twn pitswn,ths paraggelias
			//kai thn apostash tou pelath ta opoia stelnontai apo ton client
			int ar_sp;
			int ar_pe;
			int ar_ma;
			int distance;
			int n;//noumero paraggelias
			close(sockfd);//kleisimo tou socket gia ta paidia gia na mhn exei th dieythinsi
				      //synexeia in use

			//DHMIOURGIA KOINHS MNHMHS GIA TA PAIDIA WSTE NA KSEROUN POU DIAVAZOUN

			shm_id_paraggelia = shmget(shm_key_paraggelia,shm_size_paraggelia,0600);

        		//Elegxos dhmiourgias shared memory

		        if(shm_id_paraggelia<0)
        		{
                		printf("Sorry.Creation of shared memory failed!\n");
                		exit(1);
        		}

        		//proskollisi diergasias-paidiou sthn shared memory

		        ptr_paraggelia = (struct struct_par*)shmat(shm_id_paraggelia,NULL,0);

        		//elegxos an egine to attach

		        if(ptr_paraggelia == (struct struct_par *)-1)//to cast egine gia na epistrafei
			{					     // typos struct_par

                		printf("Sorry.Attachment Failed!\n");
                		exit(1);
        		}



			//LIPSI DEDOMENWN APO TON CLIENT-ELEGXOI STA recv

			if(recv(newsockfd,&ar_sp,sizeof(ar_sp),0)==-1)//lipsi tou arithmou twn special
			{
				perror("RECEIVE");
			}
			if(recv(newsockfd,&ar_pe,sizeof(ar_pe),0)==-1)//lipsi tou arithmou twn pepperoni
                        {
                                perror("RECEIVE");
                        }
			if(recv(newsockfd,&ar_ma,sizeof(ar_ma),0)==-1)//lipsi arithmou gia margarita
                        {
                                perror("RECEIVE");
                        }
			if(recv(newsockfd,&distance,sizeof(distance),0)==-1)//lipsi apostashs tou pelath
                        {
                                perror("RECEIVE");
                        }

			if((ar_sp+ar_pe+ar_ma)==0)//elegxos an o xrhsths evale mhden pitses,to programma
				exit(0);	  //na mhn traksei parakatw

			n=ptr_paraggelia[0].no+1;//efoson o deikths kata vathos einai enas pinakas me
						//deiktes,tote auksanoume sthn 1h domh mas sthn koinh
						//mnhmh thn metavliti no kata 1,kathe fora pou o server
						//kanei receive mia paraggelia.Etsi vazontas thn timh
						//auth sthn metavliti n,mporoume parakatw na kataxwroume
						//ta dedomena pou stelnei o client se diaforetikh domh
						//kathe fora mesa sthn koinh mas mnhmh
						//Thn n thn eixame arxikopoihsei pio panw se 0,ara otan
						//erthei h 1h paraggelia tha parei thn timh 1

			ptr_paraggelia[n].arithmos_special=ar_sp;//kataxwrhsh sthn domh n ton arithmo
								//twn special pou thelei o client
			ptr_paraggelia[n].arithmos_pepperoni=ar_pe;//omoiws vazoume tis pepperoni
			ptr_paraggelia[n].arithmos_margarita=ar_ma;//omoiws kai tis margarita
			ptr_paraggelia[n].apostash=distance;//ta idia gia thn apostash tou pelath

			signal(SIGALRM,give_coca_cola);//shma pou tha pianei to sigalrm pou stelnei h
						       //ualarm kai tha dinei coca cola ston client
			ualarm(500000,500000);//arxh xronometrou gia na stelnei sigalrm kathe 500ms


			//while : oso yparxoun pitses se mia paraggelia trexei tous psistes
			//h while termatizei molis psithoun oles oi pitses tis paraggelias
			while(((ptr_paraggelia[n].arithmos_special)+(ptr_paraggelia[n].arithmos_pepperoni)+(ptr_paraggelia[n].arithmos_margarita))>0)
			{

				sem_wait(psistis);//o semaforos tou psisti meiwnetai kata 1
						  //Molis ginei arnhtikos(otan tha exoume 10 psistes
						  //na douleuoun) , tote tha kleidwnei ton parakatw
						  //kwdika ston opoio psinontai oi pitses
						  //Ton xreiazomaste gia na prostateuetai h metavliti
						  //koinhs prospelashs 'psistis'


				if(ptr_paraggelia[n].arithmos_special>0)//elegxos an sthn paraggelia pou
				{					//vriskomaste(sthn n)exoume pizza
									//special gia na psisoume

					sem_wait(my_sem);//kleidwma semaforou ths pitsas etsi wste an
							// psinontai parapanw apo mia special taytoxrona
							// na mhn prospelaunetai h metavliti
							// arithmos_special sthn koinh mnhmh mesa sth
							// struct apo polles diergasies

					ptr_paraggelia[n].arithmos_special--;//meiwnoume ton arithmo twn
									     // special pou exei h
									     // paraggelia n kata 1
					//printf("%d\n",ptr_paraggelia[1].arithmos_special);

					sem_post(my_sem);//apodesmeusi semaforou pitsas gia allh pizza
							 // special pou prepei na psithei

					if(fork()==0)//dhmiourgia psisti pou tha psisei thn pitsa
					{
						usleep(SPECIAL_TIME);//psinoume thn special gia xrono
								     //SPECIAL_TIME = 150000 ms
						printf("special ready\n");
						sem_post(psistis);//apodesmeuoume thn metavliti psistis
								  //afou kalesoume thn usleep kai h
								  //etsi wste na mporei kai allh
								  //diergasia na mpei sthn if gia na
								  //psithei allh pitsa


						exit(0);//gnwstopoiei oti h diergasia teleiwse to psisimo
					}
				}
				else if(ptr_paraggelia[n].arithmos_pepperoni>0)//omoia leitourgia me thn
				{					       //special parapanw
					sem_wait(my_sem);
                                        ptr_paraggelia[n].arithmos_pepperoni--;
                                        sem_post(my_sem);
					if(fork()==0)
					{
                                                usleep(PEPPERONI_TIME);
						printf("pepperoni ready\n");
                                                sem_post(psistis);
                                                exit(0);
					}
				}
				else if(ptr_paraggelia[n].arithmos_margarita>0)//omoia me ta prohgoumena
				{
					sem_wait(my_sem);
                                        ptr_paraggelia[n].arithmos_margarita--;
                                        sem_post(my_sem);
					if(fork()==0)
					{
                                                usleep(MARGARITA_TIME);
						printf("margarita ready\n");
                                                sem_post(psistis);
                                                exit(0);
					}
				}

			}//kleisimo while


			father_wait();//klhsh ths father_wait() etsi wste o pateras na perimenei tis
				     //pitses na psithoun prwta prin katevei kai ektelesei ton kwdika
				     //gia thn paradosh ths paraggelias




			//DELIVERAS
			sem_wait(dianomeas);//desmeusi semaforou dianomea etsi wste otan exoume 10
					    //dianomeis na douleuoun parallhla to epomeno
					    //sem_wait(dianomeas) na kanei ton semaforo iso me -1 etsi
					    //wste na kanei block se olous tous ypoloipous
			if(ptr_paraggelia[n].apostash==1)//elegxos an h apostash tou pelath einai kontinh
							 // h makrinh (0 gia konta kai 1 gia makria)
			{
				usleep(MAKRIA_TIME);//analoga me thn apostash o deliveras kanei
						    //diaforetiko xrono paradosis.Auto lynetai me ena
						    //sleep sthn kathe periptwsi kai amesws meta to
						    //sleep stelei ston client h paraggelia
			}
			else
			{
				usleep(KONTA_TIME);
			}
			sem_post(dianomeas);//apodesmeusi semaforou tou dianomea gia na mporoun kai alles
					    //paraggelies na paradwthoun


			write(newsockfd ,buff3 ,sizeof(buff3));//paradosi paraggelia.To buff3 periexei
							      //to mhnyma pou leei ston client oti h
							      //paraggelia paradwthike
			signal(SIGALRM,SIG_IGN);//ignore to shma pou stelnei h ualarm gia na mhn dwsei
						//coca cola se mh kathisterimeni paraggelia

			//KLEISIMO SEMAPHORES TWN PAIDIWN

			sem_close(my_sem);//kleisimo semaforou pitswn
			sem_unlink(SEM_NAME);//diagrafh semaphore twn pitswn

			//APOKOLLISI MNHMHS GIA TA PAIDIA

			if((int)shmdt(ptr2_paraggelia)==-1)//casting sto return tou shmdt gia na elegxthei me to -1
			{
				printf("Couldn't dettach from shared memory\n");
				exit(1);
			}


			close(newsockfd);//kleinoume to socket gia ta paidia
			exit(0);

		}//telos ths fork gia ton patera

	}//telos infinite loop tou server

	// KLEISIMO SEMAFORES KAI APOKOLISI MNHMHS GIA TON PATERA

	sem_close(my_sem);//kleisimo semaforou pitswn
	sem_close(dianomeas);//kleisimo semaforou tou dianomea
	sem_close(psistis);//kleisimo semaforou tou psisti
	sem_unlink(SEM_NAME);//diagrafh semaforou
	sem_unlink(SEM_NAMED);//diagrafh semafore
	sem_unlink(SEM_NAMEP);//diagrafh semafore

	//APOKOLLISI KOINHS MNHMHS
	if((int)shmdt(ptr2_paraggelia)==-1)
        {
        	printf("Couldn't dettach from shared memory\n");
        	exit(1);
        }

	//APOKOLLISI METAVLITIS psistis

	if((int)shmdt(psistis)==-1)//casting kai elegxos
	{
		printf("Couldn't dettach variable 'psistis' from shared memory\n");
		exit(1);
	}

	 //APOKOLLISI METAVLITIS dianomeas

	 if((int)shmdt(dianomeas)==-1)//casting kai elegxos
         {
                printf("Couldn't dettach variable 'dianomeas' from shared memory\n");
                exit(1);
         }


	//DIAGRASFH KOINHS MNHMHS

	shmctl(shm_id_paraggelia,IPC_RMID ,NULL);

	exit(0);

}//telos main
