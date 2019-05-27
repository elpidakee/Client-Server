//aparaithta include
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
#include <strings.h>
#include <string.h>

#define SOCK_PATH "my_path"//onoma tou socket mas
#define SHM_KEY_PARAGGELIA 47834790//to ID ths shared memory
#define SHM_SIZE_PARAGGELIA 800*sizeof(struct struct_par) //megethos ths shared memory
#define LISTENQ 20 //oura aithsewn pou mporei na dexthei o server
#define SEM_NAME "my_sem" //onoma semaforou pitswn
#define SEM_NAMEP "psistis" //onoma semaforou psisti
#define SEM_NAMED "dianomeas" //onoma semaforou dianomea

struct struct_par//domh typou struct_par me tis metavlites pou tha xrhsimopoihsoume
{
	int arithmos_special;
	int arithmos_pepperoni;
	int arithmos_margarita;
	int apostash;
}paraggelia; //dhmiourgia metavlitis typou struct struct_par

struct struct_par *ptr_paraggelia; //orismos deikth typou struct_par pou tha deixnei sthn arxh ths domhs

int main(int argc,char *argv[])
{
	char buff[256];//buffer gia thn read
	char buff2[256];//auto pou parthike
	int sum = 3;//synolo maximum pitswn
	int ar_sp,ar_pe,ar_ma; //voithitikes metavlites pou deixnoun to plithos kathe pitsas stis
			       // epiloges tou xristi
	int distance; //voithitiki metavliti gia thn apostash tou pelath
	int sockcl,serverlength; //orismos metavlitis tou socket tou client
				 //serverlength = mhkos dieuthinsis server

	struct sockaddr_un serveraddress; //domh me dieuthinsi tou server

	//DHMIOURGIA TOU sockcl

	sockcl = socket(AF_UNIX,SOCK_STREAM,0);

	//elegxos dhmiourgias

	if(sockcl == -1)
	{
		perror("SOCKET");
		exit(1);
	}

	serveraddress.sun_family = AF_UNIX; //orismos typou oikogeneias socket.Vazoume AF_UNIX giati
					    // client kai server einai sto idio mhxanhma
	strcpy(serveraddress.sun_path,SOCK_PATH);//antigrafh dieuthinsis tou socket tou client ston server

	serverlength = strlen(serveraddress.sun_path) + sizeof(serveraddress.sun_family); //synoliko
											  //mhkos
											  //dieuthinsis
											  //tou server

	//SYNDESH TOU SOCKET ME TON SERVER KAI ELEGXOS

	if(connect(sockcl,(struct sockaddr *)&serveraddress,serverlength) == -1)
	{
		perror("CONNECT");
		exit(1);
	}
	else
	{
		printf("\nCLIENT CONNECTED\n");
	}

	read(sockcl,buff2,sizeof(buff2));//lipsi tou periexomenou tou buff2 gia na ginei parakatw h
					 //sygkrish gia to an tha lavei o client to mhnyma oti h
					 //paraggelia tou eftase kathisterimenh

	if(argc==1)//An to plithos twn orismatwn einai 1,tote to programma
	{	   //tha lavei input apo to xrhsth apo to menu

		//DOMH EPILOGWN--MENU XRHSTH

		printf("\nWELCOME TO OUR PIZZA HOUSE!!MAKE YOUR ORDER FROM THE MENU BELOW!\n\n");//kalwsorisma

		printf("You can order at maximum 3 pizzas...\n\n");

		printf("How many special pizzas do you want?Enter from 0 to 3.\n");//pairnei ton arithmo twn special
		do{
			scanf("%d",&ar_sp);
		}while(ar_sp<0 || ar_sp>3);
		sum=sum-ar_sp;
		printf("How many pepperoni pizzas do you want?Enter from 0 to %d.\n",sum);
		do{
			scanf("%d",&ar_pe);
		}while(ar_pe<0 || ar_pe>sum);
		sum=sum-ar_pe;
		printf("How many margarita pizzas do you want?Enter from 0 to %d.\n",sum);
		do{
			scanf("%d",&ar_ma);
		}while(ar_ma<0 || ar_ma>sum);
		sum=sum-ar_ma;
		if(sum==3)
		{
			printf("Try calling us when you DO feel hungry.Thank you..\n\n");

		}
		else
		{
			do//plhktrologhsh apo to xrhsth me elegxo gia thn apostash tou
			{
				printf("Type your distance.Type 1 if you are far or 0 if you are near.\n");
				scanf("%d", &distance);
			}while(distance!=0 && distance!=1);
			printf("Thank you for choosing us!Your order will be ready soon...or not...\n\n");
		}
	}
	else//perasma sto programma ta orismata apo to script
	{
		ar_sp = atoi(argv[1]);//ksekiname apo to to 1 kai oxi apo to mhden,giati sto argv[0]
		ar_pe = atoi(argv[2]);//periexetai to onoma tou programmatos
		ar_ma = atoi(argv[3]);
		distance = atoi(argv[4]);
	}
	//apostolh-elegxos apostolhs paraggelias ston server.Stelnoume thn kathe metavliti ksexwrista
	//Prwta stelnoume ton arithmo twn special,meta twn pepperoni,meta twn margarita kai meta thn
	//apostash tou pelath

	if(send(sockcl,&ar_sp,sizeof(ar_sp),0) == -1)
	{
		perror("\nSPECIAL ERROR SEND\n");
		exit(1);
	}
	//else
	//{
	//	printf("\nSPECIAL SENDING MESSAGE SUCCESS\n");
	//}
	if(send(sockcl,&ar_pe,sizeof(ar_pe),0) == -1)
        {
                perror("\nPEPPERONI ERROR SEND\n");
                exit(1);
        }
	if(send(sockcl,&ar_ma,sizeof(ar_ma),0) == -1)
        {
                perror("ERROR SEND");
                exit(1);
        }
	if(send(sockcl,&distance,sizeof(distance),0) == -1)
        {
                perror("\nMARGARITA ERROR SEND\n");
                exit(1);
        }



	//LIPSI PARAGGELIAS APO TON SERVER(PIZZARIA)
	if((ar_sp+ar_pe+ar_ma)!=0)
	{
		do//elegxos an o pelaths tha parei coca cola h oxi.An to read pou tha diavasei den einai to
	  	//string pou leei gia thn coca cola,shmainei oti h paraggelia den einai kathisterimeni,opote
	  	//den tha ksanakanei read.An apo thn allh to 1o string pou kanei read einai auto me thn
	  	//coca cola,tote tha prepei na kanei kai allo ena read gia na parei kai to mhnyma pou
	  	//dinei kai thn paraggelia ston pelath
		{
			read(sockcl,buff,sizeof(buff));//diavazei thn paraggelia
			printf("\n%s\n",buff);//typwnei thn paraggelia pou katafthanei apo ton server
			printf("\n");
		}while(!strcmp(buff,buff2));//oso ta buff kai buff2 den einai idia,tha kanei read mexri na
	}			    	    //lavei kai thn paraggelia


	close(sockcl);//kleisimo tou socket tou client

}//kleisimo ths main
