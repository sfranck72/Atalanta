/* ---------------------------------
|			Fusion-C            	|
|		  -- ATALANTA --  ENGLISH   |
|	sfranck72 - 18_12_2019			|
-----------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include "fusion-c/header/msx_fusion.h"
#include "fusion-c/header/vdp_graph2.h"
#include "fusion-c/header/io.h"
#include <string.h>
#include "procedures.h"						// mes procédures
#include "fusion-c/header/ayfx_player.h"	// pour les sounds
#include "fusion-c/header/vars_msxSystem.h"	// var pour ayfx_player.h
#include "fusion-c/header/pt3replayer.h"	// player PT3

//	Preprocessor------------------------------------------------
//#define ROM 					// pour gestion des password en mode ROM et non dsk
//	Constantes -------------------------------------------------
#define	TXT			PutText 	// marre d'écrire la fonction !!
#define	START_LIEU  6			// Ecran du 1er lieu chargé : cabine du commandant = 6
#define	L_ENTRY		30			// long max de saisie {entry}
#define	L_VERBE		15			// long max d'un verbe
#define	L_NOM		15			// long max d'un nom
#define	NBR_VERBE	46			// nombre de verbes dans le tableau {liste_verbe}
#define	NBR_OBJET	25			// nombre d'objets dans le tableau {liste_objet}
#define NBR_TOTAL 	34			// nombre d'actions possibles
#define NBR_LONG	26			// nombre de la liste longue des actions possibles
#define POSY		139			// 1er ligne pour afficher du texte après affichage lieu
#define AFB_SIZE 	216			// taille du fichier AFB des sounds
#define SONG_BUFFER 1500		// taille du fichier PT3 pour musiques
#define NOIR		15			// palette
#define BLANC 		14			// palette


//	Variables Globales -----------------------------------------
	static FCB file;
	//MMMtask  t;

	volatile __at (0x7A00) unsigned char dump [1024]={0}; // Buffer pour les CHR à l'adr 2E9B / lkast 7c00

	unsigned char boussole [12] ={0} ;
	//                              NNSSEEOOMMDD


	int lieu;
	int last_lieu;
	//char AllTxt [330] ={0} ;		// Buffer tous le txt du lieu
	int code_verbe;					// parser donne un code au verbe
	int code_nom;					// parser donne un code au nom
	unsigned int code_total;		// sert pour ajouter lieu+code_verbe+code_nom
	int objet;						// indice del'objet
	unsigned int execute;			// flag de sortie de boucle si action executée
	int flag_nocomprendo;			// flag de sortie de boucle si on ne comprend pas la syntaxe
	int action;						// token action
	int v1,v2;						// flag chambre du réacteur
	int chrono,ch;					// le temps écoulé



	unsigned char entry [L_ENTRY]={0};		// saisie du joueur
	int len_entry;							// longueur de la saisie
	char verbe [L_VERBE]={0};				// verbe
	char nom	[L_NOM]={0};				// nom
	char LigneDeTxt [33]={0};				// 2eme Buffer si nb_lignes>max_lignes
	unsigned char   song[SONG_BUFFER]; 		// pour stocker la musique PT3
	//unsigned char   afbdata[AFB_SIZE];		// pour stocker le bruitage


	static unsigned int count = 0;			// pour l'interrupt

	unsigned char __at(0x7E02) LDbuffer [11676];		// Buffer pour les images / last $8002
	char mypalette[64]={0};					// Palette SC5 initialisée à 0
	char fichier [12]={0} ;					// Fichier SC5
	//unsigned char lieu_char [3]= "XX";
	//unsigned char numimage [3] = "XX";
	//unsigned char lastnumimage [3] = "XX";
	//int x, y = 0;

	//int nb_lignes, max_lignes = 0;
	//unsigned char data_lieu[42]={0};		// stock le fichier pawnxx.dat
	const unsigned char * liste_verbe [NBR_VERBE]  = {"n","north","s","south","e","east",
													  "w","west","up","down",
													  "quit","take","leave",
													  "watch","save","load",
													  "inv","inventory","see",
													  "wear","remove","open",
													  "pull","push","read",
													  "press","repair",
													  "helmet","suit","apron",
													  "pilot","key","key","manual",
													  "screen","button","keyboard","green",
													  "yellow","red","lever",
													  "dial","wardrobe","bed",
													  "antenna","label"
													 };

	int                   token_verbe  [NBR_VERBE]  = { 1,1,2,2,3,3,
													    4,4,5,6,
													    7,8,9,
													    10,11,12,
													    13,13,14,
													    20,21,22,
													    23,24,25,
													    26,27,
													    50,51,52,
													    53,54,54,55,
													    60,61,61,62,
													    63,64,65,
													    66,67,68,
													    69,70
												     };


	int                   code_objet  [NBR_OBJET]  = { 0,66,61,70,51,
													   60,60,68,67,
													   68,67,50,61,
													   61,66,65,61,
													   61,60,61,53,
													   69,52,54,55
												     };

	const unsigned char * liste_objet [NBR_OBJET]  = {"NULL","a dial", "a red button", "a label", "a space suit",
													  "a white screen", "a yellow screen", "a bed", "a wardrobe" ,
													  "a bed", "a wardrobe", "a helmet", "a red button",
													  "a green button", "a dial", "a lever", "a red button",
													  "a green button", "a red button", "a yellow button", "the second pilot",
													  "a parabolic antenna", "an apron", "a key", "a manual"
													 };

	int                   lieu_objet  [NBR_OBJET]  = { 0,-1, -1, -1,  1,
													   -2, -4, -5, -5,
													   -6, -6,  6, -7,
													   -7, -8, -8, -8,
													   -8, -8, -8,  9,
													   -9,-99,-99,-99
												     };

	unsigned int liste_total [NBR_TOTAL]	= { 0,100,200,300,400,500,600,700,899,
												950,951,999,1051,1052,1053,
												1055,1060,1099,1100,1200,1300,1400,
												2050,2051,2052,2150,2151,2152,2255,
												2555,2769,6550,6551,6552
									  		  };

	int token_total [NBR_TOTAL]				= { 0,1,1,1,1,1,1,8,-2,
											    -10,-10,3,-11,-14,-12,
											    -13,-13,-4,5,6,7,9,
											    -2,-2,-2,-10,-10,-3,-15,
											    -15,-29,-10,-10,-3
									  		  };

	unsigned int liste_long [NBR_LONG]  	= { 0,1066,2570,2661,300,500,2560,
												300,300,2560,2267,2267,300,400,
												2661,2662,2664,1066,2365,2465,
												2560,2661,2662,2663,2664,1069
											  };

	int lieu_long [NBR_LONG]				= { 0,1,1,1,2,2,2,
											    3,4,4,5,6,7,7,
											    7,7,7,8,8,8,
											    8,8,8,8,8,9
											  };

	int token_long [NBR_LONG]				= { 0,16,17,18,21,20,19,
											    21,21,22,23,24,37,38,
											    25,27,26,16,35,34,
											    28,30,33,32,31,36
											  };




/* ---------------------------------
			FT_LoadData
	Load Data to a specific pointer
	size is the size of data to read
	skip represent the number of Bytes
	you want to skip from the begining of the file
	Example: skip=7 to skip 7 bytes of a MSX bin
-----------------------------------*/
int FT_LoadData(char *file_name, char *buffer, int size, int skip)
{
		InitPSG();
		FT_SetName( &file, file_name );
		if(fcb_open( &file ) != FCB_SUCCESS)
		{
					FT_errorHandler(1, file_name);
					return (0);
		}
		if (skip>0)
		{
				fcb_read( &file, buffer, skip );

		}


		fcb_read( &file, buffer, size );

		if( fcb_close( &file ) != FCB_SUCCESS )
		{
			FT_errorHandler(2, file_name);
			return (0);
		}
		//SetColors(16,0,0);
		return(0);
}

/* ---------------------------------
			FT_SetName

	Set the name of a file to load
			(MSX DOS)
-----------------------------------*/
void FT_SetName( FCB *p_fcb, const char *p_name )
{
	char i, j;
	memset( p_fcb, 0, sizeof(FCB) );
	for( i = 0; i < 11; i++ ) {
		p_fcb->name[i] = ' ';
	}
	for( i = 0; (i < 8) && (p_name[i] != 0) && (p_name[i] != '.'); i++ ) {
		p_fcb->name[i] =  p_name[i];
	}
	if( p_name[i] == '.' ) {
		i++;
		for( j = 0; (j < 3) && (p_name[i + j] != 0) && (p_name[i + j] != '.'); j++ ) {
			p_fcb->ext[j] =  p_name[i + j] ;
		}
	}
}


/* ---------------------------------
			FT_errorHandler

			In case of Error
-----------------------------------*/
void FT_errorHandler(char n, char *name)
{
	InitPSG();
	Screen(0);
	SetColors(15,6,6);
	switch (n)
	{
			case 1:
					Print("\n\rFAILED: fcb_open(): ");
					Print(name);
			break;

			case 2:
					Print("\n\rFAILED: fcb_close():");
					Print(name);
			break;

			case 3:
					Print("\n\rStop Kidding, run me on MSX2 !");
			break;
	}
Print("\n\r");
Exit(0);
}


/* ---------------------------------
			FT_DumpChr
			Dump des CHR
			de 2E9B à 32A0
----------------------------------*/
void FT_DumpChr   (void)
{
	unsigned int slotID;
	FT_LoadData("neutro1.alf", dump, 1024, 0);
	// lit 0xF344 <Slot address of RAM in page 3>
	slotID = Peekw(0xF344);
	// Ecrit dans F91F <SlotID of font>
	Pokew(0xF91F,slotID);
	// Ecrit dans F920 & F921 l'adresse des fonts
	//Pokew(0xF920,0x3920);

	// je l'ai positionné dans la RAM à 30Ko = $7530
	// max pour une 32Ko = $8000
	// 32Ko-1024 = $7C00

	Pokew(0xF920,0x00);
	Pokew(0xF921,0X7A);

}


/* ---------------------------------
			Init Screen 5 page 0
----------------------------------*/
void FT_Init (void)
{
	Screen(5);
	SetActivePage(0);
	SetDisplayPage(0);
	SetColors (BLANC,0,0);
	Cls();
	KeySound(0);
  	VDP60Hz();
	KeySound (0);
	InitPSG();



}

/* ---------------------------------
			Intro
----------------------------------*/
void FT_Intro (void)
{
	int fH;
	int i,y;


/* // SPLASH page
	// CHARGE palette
		StrCopy(fichier,"spidey.pal");
		FT_LoadData(fichier,mypalette,64,0);
		SetSC5Palette((Palette *)mypalette);

	// CHARGE Image
		StrCopy (fichier,"spidey.rle");
		FT_LoadData(fichier,LDbuffer,9323,0);
		RleWBToVram (&LDbuffer[0],0);

		WaitForKey();*/



 // SPLASH page
	// CHARGE palette
		StrCopy(fichier,"splash.pal");
		FT_LoadData(fichier,mypalette,64,0);
		SetSC5Palette((Palette *)mypalette);

	// CHARGE Image
		StrCopy (fichier,"splash.rle");
		FT_LoadData(fichier,LDbuffer,10467,0);
		RleWBToVram (&LDbuffer[0],0);

// test musique
	InitPSG();
	// DEBUG pour les fichiers de musique PT3
	FT_LoadData("atalanta.pt3",song, SONG_BUFFER, 0);
	PT3Init (song+99, 0);

	// bruitage	
	// InitFX();
	// FT_LoadData("atalanta.afb", afbdata, AFB_SIZE, 0);

		while (Inkey()!=32)
		  {
		    Halt();
		    DisableInterupt();
		    PT3Rout();
		    PT3Play();
		    EnableInterupt();
		  }

		//FT_Init();
// fin test musique

		//WaitForKey ();
		Cls ();

 // MANUAL page
	// CHARGE palette
		StrCopy(fichier,"manual.pal");
		FT_LoadData(fichier,mypalette,64,0);
		SetSC5Palette((Palette *)mypalette);
	// CHARGE Image
		StrCopy (fichier,"manual_e.rle");
		FT_LoadData(fichier,LDbuffer,7927,0);
		RleWBToVram (&LDbuffer[0],0);


		while (Inkey()!=32)
		  {
		    Halt();
		    DisableInterupt();
		    PT3Rout();
		    PT3Play();
		    EnableInterupt();
		  }

		//WaitForKey ();
		Cls ();
 // INTRO page
	// CHARGE Image
		StrCopy (fichier,"intro_e.rle");
		FT_LoadData(fichier,LDbuffer,8644,0);
		RleWBToVram (&LDbuffer[0],0);


		while (Inkey()!=32)
		  {
		    Halt();
		    DisableInterupt();
		    PT3Rout();
		    PT3Play();
		    EnableInterupt();
		  }

		InitPSG();
		Cls ();

/*
 // INTRO page
	RestoreSC5Palette();
	fH = Open("intro.txt",O_RDWR);
	y=0;
	for (i=1;i<22;i++)			// Nbre de lignes dans le fichier intro.txt
	{

	Read(fH,LigneDeTxt,32);    // chaque ligne fait 32 CHR
	PutText(0,y,LigneDeTxt,0); // Affiche ligne par ligne
	y+=9;
	}

	Close(fH);
	WaitForKey();
	Cls ();*/

	//FT_Init();	
	//FT_DumpChr ();	
	// Mise en page 2 de l'image SAS ouvert
	StrCopy (fichier,"lieu07O.rle");
	FT_SetName(&file,fichier);
	fcb_open (&file);
	fcb_read( &file, LDbuffer, 3500 );
	fcb_close(&file);
	// copie en page 2
	SetActivePage (2);
	RleWBToVram (&LDbuffer[0],0);
	SetActivePage (0);

}




/* ---------------------------------
			FT_SearchData

		lit les data du lieu sur dsk
		et les charges dans les var
----------------------------------*/
void FT_SearchData (int p_lieu)
{

	switch (p_lieu)
	{
	case 1 :
		StrCopy(boussole,"000000000002");
		StrCopy(LigneDeTxt,"^ COCKPIT");
		break;
	case 2 :
		StrCopy(boussole,"000300050100");
		StrCopy(LigneDeTxt,"^ NORTH CORRIDOR");
		break;
	case 3 :
		StrCopy(boussole,"020400060000");
		StrCopy(LigneDeTxt,"^ MIDDLE CORRIDOR");
		break;


	case 4 :
		StrCopy(boussole,"030000070008");
		StrCopy(LigneDeTxt,"^ SOUTHERN CORRIDOR");
		break;
	case 5 :
		StrCopy(boussole,"000002000000");
		StrCopy(LigneDeTxt,"^ SECOND PILOT'S CABIN");
		break;
	case 6 :
		StrCopy(boussole,"000003000000");
		StrCopy(LigneDeTxt,"^ YOUR CABIN");
		break;
	case 7 :
		StrCopy(boussole,"000000000000");
		StrCopy(LigneDeTxt,"^ PRESURIZED COMPARTMENT");
		break;
	case 8 :
		StrCopy(boussole,"000000000400");
		StrCopy(LigneDeTxt,"^ REACTOR ROOM");
		break;
	case 9 :
		StrCopy(boussole,"001000000000");
		StrCopy(LigneDeTxt,"^ OUTSIDE NORTH SHUTTLE");
		break;
	case 10:
		StrCopy(boussole,"091100000000");
		StrCopy(LigneDeTxt,"^ OUTSIDE SHUTTLE");
		break;
	case 11:
		StrCopy(boussole,"100007000000");
		StrCopy(LigneDeTxt,"^ OUTSIDE SOUTH SHUTTLE");
		break;
	}
	last_lieu = lieu;
}

/* ---------------------------------
			FT_ShowTxt

	 Affiche le txt
----------------------------------*/
void FT_ShowTxt(void)
{

	// BoxLine (0,0,255,211,15,0);				// cadre ecran total
	//BoxFill(65,10,190,110,5,0);				// simule l'image
	//BoxLine (60,5,195,115,15,0);				// cadre
	//Draw("C15BM59,20U16R16BM+121,+96D16L16");	//cadre double

	FT_CleanTexte ();
	TXT (0,130,"                               ",0);
	PutText(0,130,LigneDeTxt,0);		// affiche le texte du lieu

}

/* ---------------------------------
			FT_ShowImage

		Charge l'image du HUB et la palette
----------------------------------*/
void FT_ShowImage(void)
{
	HideDisplay ();
// CHARGE palette
	StrCopy(fichier,"tab.pal");
	FT_LoadData(fichier,mypalette,64,0);
	SetSC5Palette((Palette *)mypalette);

// CHARGE Image
	StrCopy (fichier,"tab.rle");
	FT_LoadData(fichier,LDbuffer,3994,0);
/*	FT_SetName(&file,fichier);
	fcb_open (&file);
	fcb_read( &file, LDbuffer, 3994 );	// taille du HUB compressé RLEWB
	fcb_close(&file);*/
	RleWBToVram (&LDbuffer[0],0);
	FT_CleanTexte ();					// pour enlever le bas de la page
	ShowDisplay ();
}

/* ---------------------------------
			FT_ShowLieu

		Charge l'image seulement
----------------------------------*/
void FT_ShowLieu(void)
{

	char numlieu [2]={0};

// CHARGE Image
	StrCopy (fichier,"lieu");
	if (lieu<10)
	{
		StrConcat (fichier,"0");
	}

	Itoa (lieu,numlieu,10);
	StrConcat (fichier,numlieu);
	// routine RLEWB
	StrConcat (fichier,".rle");

	FT_SetName(&file,fichier);
	fcb_open (&file);
	fcb_read( &file, LDbuffer, 11676 );
	fcb_close(&file);
	// page 1
	SetActivePage (1);
	RleWBToVram (&LDbuffer[0],0);
	// copy page1 vers page0 image centre de 176x84
	SetActivePage (0);
	LMMM (40,85+256,40,23,176,84,0);
	// cas du SAS ouvert après un LOAD
	if (lieu==7 && v2==1)
		{
			LMMM (40,256+256,40,23,176,84,0);
		}
}


/* ---------------------------------
		FT_LoadTxt

	Charge txt disk to RAM
----------------------------------*/
/*void FT_LoadTxt(void)
{
	int fH;

	for (fH = 0; fH < 330; ++fH)
	{
		AllTxt [fH] = 0;		// reinit du buffer txt en RAM
	}


	 StrCopy (fichier,"pawn");
	 StrConcat (fichier,lieu_char);
	 StrConcat (fichier,".txt");
	 fH = Open(fichier,O_RDONLY);
	 // Charge en RAM tous le txt du lieu
	 Read(fH,AllTxt,nb_lignes*33);
	 Close (fH);
}


*/



/* ---------------------------------
			FT_Entry

		Gère l'input
----------------------------------*/
void FT_Entry (void)
{
	//unsigned char entree [30];
	unsigned char ch;
	int i=0;

	StrCopy (nom,"               ");
	StrCopy (verbe,"                ");
	StrCopy(entry,"                              ");	// réinit saisie
	len_entry = 0;						// réinit de la longueur de saisie

	BoxFill (0,200,255,211,0,0);		// Nettoie la zone de saisie
	PutText(0,200,">_",0);				// Indique la ligne de saisie

	while (i<30)						// saisie max = 30
	{
		ch = WaitKey();
		if (ch==8 && i>0)				// si backstroke et pas le 1er chr alors backstroke
		{
			entry[i] = ' ';
			PutText(i*8,200,"_ ",0);
			i-=1;
		}
		else if (ch!=8 && ch!=13)		// si pas backstroke nin return alors on stocke la saisie
		{
			if (ch>=65 && ch<=90)		// passe en minuscule les majuscules
			{
				ch+=32;
			}
			entry[i] = ch;
			PutText(8,200,entry,0);
			PutText(8*i+16,200,"_",0);
			i+=1;
		}
		if (ch==13)						// si return on arrête tout
		{
			break;
		}
	}

	len_entry = i;						// longueur de saisie {len_entry} pour le parser
}

/* ---------------------------------
			FT_Parser

		Sépare le verbe et le nom
----------------------------------*/
void FT_Parser (void)
{
	int space=0;						// réinit position ESPACE
/*	StrCopy(verbe,"               ");	// réinit le verbe
	StrCopy(nom,"               ");		// réinit le nom*/

	StrLeftTrim (entry);				// enlève les éventuels espaces au début de la chaine
	space = StrChr(entry,32);			// position d'ESPACE {32 en ASCII} dans la string entry

	if (space == -1)					// si pas d'espace {=-1}
	{
		StrCopy(verbe,entry);			// verbe = saisie totale
	}
	else
	{
		NStrCopy (verbe,entry,space);	// sinon ESPACE et sépare 'verbe' et 'nom'
		NStrCopy (nom,entry+space+1,len_entry-space);
		StrRightTrim (nom);				// enlève les éventuels espaces à la fin de 'nom'
	}
}

/* ---------------------------------
				 FT_Verbe
	 Gère le verbe et associe le code_verbe
----------------------------------*/
void FT_Verbe (void)
{
	int i, resultat, len;


	for (i = 0; i < NBR_VERBE; ++i)
	{
		len = StrLen(verbe);
		resultat = MemCompare(liste_verbe [i],verbe,len);	// compare 2 strings, si = renvoie 0

			if (resultat==0 && StrLen(liste_verbe[i])==len)	// si le verbe est dans la liste...
			{
				code_verbe = token_verbe [i];				// Associe le code au verbe
				return ;
			}
	}
	flag_nocomprendo = 1;
	code_verbe = 0;
}

/* ---------------------------------
			FT_Nom
		Gère le nom et associe code_nom
----------------------------------*/
void FT_Nom(void)
{
	int i, resultat, len;

/*	if (flag_nocomprendo!=0)
		{
			return;
		}*/
	for (i = 0; i < NBR_VERBE; ++i)
	{
		len = StrLen(nom);
		resultat = MemCompare(liste_verbe [i],nom,len);		// compare 2 strings, si = renvoie 0

			if (resultat==0 && StrLen(liste_verbe[i])==len)	// si le verbe est dans la liste...
			{
				code_nom = token_verbe [i];					// Associe le code au verbe
				return ;
			}
	}

	code_nom = 0;
	if (len!=0)
	{
		flag_nocomprendo = 1;
	}


}

/* ---------------------------------
			FT_Objet
		récupère le N° de l'objet
----------------------------------*/
void FT_Objet (void)
{

	int i;

	if (code_nom!=0)						// s'il y a un nom
	{
		objet=0;							// remise à 0 de l'indice objet
		for (i = 1; i < NBR_OBJET; ++i)
		{
			if (code_nom==code_objet[i])
			{
				if (abs(lieu_objet[i])==lieu || lieu_objet[i]==0)
				{
					objet = i;
				}
			}
		}
	}
}

/* ---------------------------------
			FT_Total
	code_total =  lieu + verbe + nom
----------------------------------*/
void FT_Total(void)
{
		code_total =(code_verbe*100)+code_nom;
		FT_SearchLong ();
		if (execute ==0)
		{
		code_total =(code_verbe*100)+code_nom;
		FT_SearchAction ();
		}
		if (execute ==0)
		{
		code_total =(code_verbe*100)+99;
		FT_SearchAction ();
		}
}

/* ---------------------------------
			FT_SearchLong
cherche 1ere version code_total :
lieu_long liste_long & token long
----------------------------------*/
void FT_SearchLong (void)
{
	int i;

	for (i = 1; i < NBR_LONG; ++i)
	{
		if (code_total==liste_long[i] && lieu==lieu_long[i])
		{
			action = token_long[i];
			execute = 1;
		}
	}
}


/* ---------------------------------
			FT_SearchAction
cherche les 3 versions de code_total :
FT_TotalA, FT_TotalB & FT_TotalC
----------------------------------*/
void FT_SearchAction (void)
{
	int i;

	for (i = 1; i < NBR_TOTAL; ++i)
	{
		if (code_total==liste_total[i])
		{
			action = token_total[i];
			execute = 1;
		}
	}
}

/* ---------------------------------
			FT_VerifObjet
Avant d'executer l'action :
- vérif si objet existe dans lieu
- passe action en +
----------------------------------*/
void FT_BeforeExecute (void)
{

	if (action<0 && code_nom!=0)
	{
		if (objet==0)					// si indice objet n'est pas dans un lieu
		{
			FT_Message ("Nothing here");	// c'est que l'objet n'est pas dans le lieu ou possédé
			execute=0;					// pour éviter le lanceemnt des actions
			return;						// alors on quitte
		}

	}
	if (action<0)
		{
			action*=-1;			// on passe l'action en positif
		}
}
/* ---------------------------------
			FT_Execute
		Execute l'action de 0 à 9
----------------------------------*/
void FT_Execute (void)
{
	char joy;
	int i;					// pour le bruitage du case 8
	switch (action)
	{
		case 0 :
		break;

		// DIRECTIONS {100 à 600}
		case 1 :
			FT_Case1 ();
			break;

		// PRENDRE {899}
		case 2 :
			FT_Case2 ();
			break;

		// DEPOSE {999}
		case 3 :
			FT_Case3 ();
			break;

		// REGARDER {1099}
		case 4 :
			FT_Message ("nothing special to see"); // cas par défaut
			break;

		// SAVE {1100}
		case 5 :
			FT_Save ();
			break;

		// LOAD {1200}
		case 6 :
			FT_Load ();
			break;

		// INVENTAIRE {1300}
		case 7 :
			FT_CleanTexte ();
			TXT(0,POSY+9,"you",0);
			TXT(0,POSY+18,"get :",0);
			FT_Inventaire (0);					// cherche les objets = 0
		break;

		// QUIT {700}
		case 8 :

	// DEBUG Test de sounds AFX
	PlayFX(2);
	for ( i = 0; i < 10; ++i)
	{
	FT_Wait(1);
	FT_CheckFX();
	}
	// DEBUG Test de sounds AFX

		FT_Quit ();
		break;

		// VOIR {1400}
		case 9 :
		FT_CleanTexte ();
		TXT (0,POSY+9,"there's:",0);			// pour lui montrer les nouveaux objets
		FT_Inventaire (lieu);
		// pour lui montrer les ecrans
		KillKeyBuffer ();
		FT_Message("Cursors and [ESC] to quit");
		//TXT (219,POSY+45,"[ESC]",0);	// info pour quitter le mode VOIR
		while (Inkey()!=27)  // tant que ce n'est pas ESC
		{
			joy=JoystickRead (0);
			if (joy==7)
				{
				// GAUCHE
				LMMM (40,256,40,23,176,84,0);
				}

			if (joy==1)
				{
				// HAUT = CENTRE
				LMMM (40,85+256,40,23,176,84,0);
				}
			if (joy==1 && lieu==7 && v2==1)
				{
				// HAUT = CENTRE  cas particulier SAS ouvert dans compartiment étanche
				LMMM (40,256+256,40,23,176,84,0);
				}

			if (joy==3)
				{
				// DROITE
				LMMM (40,170+256,40,23,176,84,0);
				}
		}
		KillKeyBuffer ();
		chrono++;
		break;
	}
}
/* ---------------------------------
			FT_Save
		sauvegarde en mode dsk
		ou
		donne le password en mode ROM
----------------------------------*/
void FT_Save (void)
{
	int i;
	char buff_nb [4];					// buffer pour le SAVE
	char buff_save [90];

#ifdef ROM
	// systeme ROM
	if (lieu>=9)
	{
		FT_CleanTexte ();
		TXT (0,POSY+9,"backups in space",0);
		TXT (0,POSY+18,"are not allowed.",0);
	}
	else
	{

		// réinit du buffer
		for (i = 0; i < 90; ++i)				// réinit du buffer
		{
			buff_save[i]=0;
		}
		// password = lieu+9|100-chrono|v1|v2
		lieu+=9;
		Itoa(lieu,buff_nb,10);				// passer le nbre en char
		StrConcat (buff_save,buff_nb);		// copier le char dans la chaine
		lieu-=9;
		chrono = 100 - chrono;
			if (chrono<10)
			{
				StrConcat(buff_save,"0");
			}
		Itoa(chrono,buff_nb,10);			// passer le nbre en char
		StrConcat (buff_save,buff_nb);		// copier le char dans la chaine
		chrono = 100 - chrono;
		Itoa(v1,buff_nb,10);				// passer le nbre en char
		StrConcat (buff_save,buff_nb);		// copier le char dans la chaine
		Itoa(v2,buff_nb,10);				// passer le nbre en char
		StrConcat (buff_save,buff_nb);
		// affichage password
		FT_CleanTexte ();
		TXT (0,POSY+9,"Password :",0);
		TXT (90,POSY+9,buff_save,0);
	}
#endif

#ifndef ROM
	//systeme dsk
	int fh;
	FT_Message ("backup in progress...");

	for (i = 0; i < 90; ++i)				// réinit du buffer
		{
			buff_save[i]=0;
		}

	// OBJETS
	for (i = 1; i < NBR_OBJET; ++i)
		{
			Itoa(lieu_objet[i],buff_nb,10);		// passer le nbre en char
			StrConcat (buff_save,buff_nb);		// copier le char dans la chaine
			StrConcat (buff_save,",");
		}

	// LIEU,CHRONO,V1,V2

		Itoa(lieu,buff_nb,10);				// passer le nbre en char
		StrConcat (buff_save,buff_nb);		// copier le char dans la chaine
		StrConcat (buff_save,",");
		Itoa(chrono,buff_nb,10);			// passer le nbre en char
		StrConcat (buff_save,buff_nb);		// copier le char dans la chaine
		StrConcat (buff_save,",");
		Itoa(v1,buff_nb,10);				// passer le nbre en char
		StrConcat (buff_save,buff_nb);		// copier le char dans la chaine
		StrConcat (buff_save,",");
		Itoa(v2,buff_nb,10);				// passer le nbre en char
		StrConcat (buff_save,buff_nb);		// copier le char dans la chaine
		StrConcat (buff_save,",");


	// calcule la longueur du txt à sauver
	i=StrLen(buff_save);
	//TXT(0,130,buff_save,0);					// DEBUG
	// on sauvegarde
	fh=Open("astro.txt",O_WRONLY);
	Write(fh,buff_save,i);
	Close(fh);
	FT_Message ("SAVE OK");
	KillKeyBuffer ();
#endif

}
/* ---------------------------------
			FT_Load
		Charge une sauvegarde en mode dsk
		ou
		demande saisie password en mode ROM
----------------------------------*/
void FT_Load (void)
{
	int i;
	char buff_nb [4];
	unsigned char password_chr = 0 ;	// buffer pour le LOAD
	char buff_save [90];


#ifdef ROM
	// systeme ROM
	// réinit du buffer
	if (lieu>=9)
	{
		FT_CleanTexte ();
		TXT (0,POSY+9,"Loadings in space",0);
		TXT (0,POSY+18,"are not allowed.",0);
	}
	else
	{
	for (i = 0; i < 90; ++i)
		{
			buff_save[i]=0;
		}
	// saisie du password
	FT_CleanTexte ();
	TXT (0,POSY+9,"Password :",0);
	TXT (90,POSY+9,"_",0);
	for (i = 0; i < 6; ++i)
		{

			while (password_chr<48 || password_chr>57)
			{
				password_chr = WaitKey ();
			}
			password_chr -= 48;

			Itoa (password_chr,buff_nb,10);
			StrConcat (buff_save,buff_nb);
			password_chr = 0;
			TXT(90+(9*i),POSY+9,buff_nb,0);
				if (i<5)
				{
					TXT (99+(9*i),POSY+9,"_",0);
				}
		}

	// verif password

		NStrCopy (buff_nb,buff_save,2);		// récupère le lieu
		i = atoi(buff_nb); 					// passe le chr en int
		i-=9;								// enlève les 9 du code
		if (i>11)
			{
				return;
			}
		NStrCopy (buff_nb,buff_save+4,1);	// récupère v1	réacteur
		i = atoi(buff_nb); 				 	// passe le chr en int
		if (i>2)
			{
				return;
			}
		NStrCopy (buff_nb,buff_save+5,1);	// récupère v2	SAS
		i = atoi(buff_nb); 				 	// passe le chr en int
		if (i>1)
			{
				return;
			}

		NStrCopy (buff_nb,buff_save,2);		// récupère le lieu
		i = atoi(buff_nb); 					// passe le chr en int
		i-=9;								// enlève les 9 du code
		lieu = i;

		NStrCopy (buff_nb,buff_save+2,2);	// récupère le chrono
		i = atoi(buff_nb); 				 	// passe le chr en int
		i = 100 - i;
		chrono = i;

		NStrCopy (buff_nb,buff_save+4,1);	// récupère v1	réacteur
		i = atoi(buff_nb); 				 	// passe le chr en int
		v1 = i;


		NStrCopy (buff_nb,buff_save+5,1);	// récupère v2	SAS
		i = atoi(buff_nb); 				 	// passe le chr en int
		v2 = i;

	FT_Message("Password OK");
	}
#endif


#ifndef ROM
	//systeme dsk

	int fh,virgule,inc;

	FT_Message ("Loading in progress...");
	for (i = 0; i < 90; ++i)				// réinit du buffer
	{
		buff_save[i]=0;
	}

	// Chargement dans le buffer
	fh=Open("astro.txt",O_RDONLY);
	Read(fh,buff_save,90);
	Close(fh);

	//parser
	inc=0;
	virgule=0;
	// MAJ du tableau lieu_objet
	for (i = 1; i < NBR_OBJET; ++i)
	{
	if (i==1)
	{
		virgule = StrChr(buff_save+inc,44);		// position de virgule {44 en ASCII} dans buff_save
		NStrCopy (buff_nb,buff_save,virgule);

	}
	else
	{
		virgule = StrChr(buff_save+inc,44);
		NStrCopy (buff_nb,buff_save+inc,virgule);
	}

	inc+=virgule+1;
	fh=atoi(buff_nb);
	lieu_objet[i]=fh;
	}
	// MAJ LIEU,CHRONO,V1,V2
	for (i = 0; i < 4; ++i)
	{
	virgule = StrChr (buff_save+inc,44);
	NStrCopy (buff_nb,buff_save+inc,virgule);
	inc+=virgule+1;
	fh=atoi(buff_nb);

		switch (i)
		{
			case 0 :
			lieu = fh;
			break;

			case 1 :
			chrono = fh;
			break;

			case 2 :
			v1 = fh;
			break;

			case 3 :
			v2 = fh;
			break;
		}
	}
#endif
}


/* ---------------------------------
			FT_Execute1
		Execute l'action 10 à 19
		on a enlevé 10 à action
		et on refait un switch
----------------------------------*/
void FT_Execute1 (void)
{

	switch (action)
		{
		// ENLEVER {0950,0951,2150,2151} Gestion des vêtements
		case 10 :
			FT_Case10 ();
		break;

		// REGARDER COMBINAISON {1051}
		case 11 :
			FT_CleanTexte ();
			TXT (0,POSY,"This is your suit",0);
			TXT (0,POSY+9,"for the spacewalk",0);
		break;

		// REGARDER LE SECOND PILOTE {1053}
		case 12 :
			FT_Case12 ();
		break;

		// REGARDER ECRAN OU MANUEL {1060,1055}
		case 13 :
			FT_Message ("Isn't it better to read it?");
		break;

		// REGARDER TABLIER {1052}
		case 14 :
			FT_CleanTexte ();
			TXT (0,POSY,"It's very heavy.",0);
			TXT (0,POSY+9,"Probably lead-treated.",0);
		break;
		// LIRE OUVRIR MANUEL {2555,2255}
		case 15 :
			FT_Case15 ();
		break;

		// COKPIT : REGARDER CADRAN {011066}
		case 16 :
			FT_Case16 ();
		break;

		// COKPIT : LIRE ETIQUETTE {012570}
		case 17 :
			FT_CleanTexte ();
			TXT (0,POSY,    "        GALACTIC S.O.S ",0);
			TXT (0,POSY+9,  "      Push the keyboard",0);
			TXT (0,POSY+18, "    only in case of emergency.",0);
			TXT (0,POSY+27, "   Any abuse will be punished.",0);
		break;

		// COKPIT : APPUYER BOUTON {012661}
		case 18 :
			FT_CleanTexte ();
			TXT (0,POSY,    "Indicator light comes on briefly",0);
			FT_Message ("FAULTY EXTERNAL ANTENNA");
		break;

		// COULOIR HAUT : LIRE ECRAN {022560}
		case 19 :
			FT_Case19 ();
		break;
		}
}

/* ---------------------------------
			FT_Execute2
		Execute l'action 20 à 29
----------------------------------*/
void FT_Execute2 (void)
{

	switch (action)
		{
		// COULOIR HAUT : MONTER {020500}
		case 20 :
			FT_Case20 ();
		break;

		// COULOIR : EST {020300,030300,040300}
		case 21 :
			FT_CleanTexte ();
			TXT (0,POSY,    "Better not to wake",0);
			TXT (0,POSY+9,  "the passengers,they might",0);
			TXT (0,POSY+18, "panic when getting up.",0);
		break;

		// COULOIR BAS : LIRE ECRAN {042560}
		case 22 :
			FT_Case22 ();
		break;

		// CABINE SECOND : OUVRIR ARMOIRE {052267}
		case 23 :
			FT_Case23 ();
		break;

		// CABINE DU COMMANDANT : OUVRIR ARMOIRE {062267}
		case 24 :
			FT_Message ("It's empty");
		break;
		//  COMP ETANCHE : APPUYER BOUTON {072661}
		case 25 :
			FT_Case25 ();
		break;

		// COMP ETANCHE : APPUYER ROUGE {072664}
		case 26 :
			FT_Case26 ();
		break;

		// COMP ETANCHE : APPUYER VERT {072662}
		case 27 :
			FT_Case27 ();
		break;

		// SALLE REACTEUR : LIRE ECRAN  {082560}
		case 28 :
			FT_Case28 ();
		break;

		// REPARER ANTENNE {2769}
		case 29 :
			FT_CleanTexte ();
			TXT (0,POSY,   "Only the second pilot would",0);
			TXT (0,POSY+9, "be able to repair it.",0);
		break;
		}
}

/* ---------------------------------
			FT_Execute3
		Execute l'action 30 à 39
----------------------------------*/
void FT_Execute3 (void)
{
	switch (action)
		{
		// SALLE REACTEUR : APPUYER BOUTON  {082661}
		case 30 :
			FT_Case30 ();
		break;

		// SALLE REACTEUR : APPUYER ROUGE  {082664}
		case 31 :
			FT_Case31 ();
		break;

		// SALLE REACTEUR : APPUYER JAUNE  {082663}
		case 32 :
			FT_Case32 ();
		break;

		// SALLE REACTEUR : APPUYER VERT  {082662}
		case 33 :
			FT_Case33 ();
		break;

		// SALLE REACTEUR : POUSSER LEVIER {082465}
		case 34 :
			FT_Case34 ();
		break;

		// SALLE REACTEUR : TIRER LEVIER {082365}
		case 35 :
			FT_Case35 ();
		break;

		// EXTERIEUR : REGARDER ANTENNE {091069}
		case 36 :
			FT_Case36 ();
		break;

		// COMP ETANCHE :EST   {070300}
		case 37 :
			if (v2==1)		// sas ouvert impossible de changer de pièce
			{
				action = 1;	// on renvoit vers les directions qui vont bloquer
				FT_Execute ();
				break;
			}
			lieu=4;			// sas fermé on peut aller à l'EST donc lieu 4
		break;

		// COMP ETANCHE : OUEST  {070400}
		case 38 :
			if (v2==1)		// sas ouvert on peut aller dans l'espace
			{
				lieu=11;	// ok
				break;
			}
			action = 1;		// on renvoie vers les directions qui vont bloquer
			FT_Execute ();
		break;

		}
}

/* ---------------------------------
			FT_CASE
----------------------------------*/

// DIRECTIONS {100 à 600}
void FT_Case1 (void)
{
	int pos;
	unsigned char direction[2];

	pos = (code_verbe*2)-2;				// position dans la chaine boussole
	MemCopy (direction,boussole+pos,2);	// on récupère les 2 chr
	pos=atoi (direction);				// les 2 chr en int
	if (pos==0)							// si 0 impossible
	{
		FT_Message("Impossible direction");
		return;
	}
	else
	{
		lieu = pos;						// sinon lieu = les 2 chr
	}
}

// PRENDRE {899}
void FT_Case2 (void)
{
if (lieu_objet[objet]==0 && objet!=0)			// A_t_on déjà l'objet ?
		{
			FT_Message("You already have it");
			return;
		}
	else if (lieu_objet[objet]==0 && objet==0)	// verbe sans nom
		{
			FT_Message("Designate an object");
			return;
		}
	else if (lieu_objet[objet]<0)			// objet négatif pas prenable
		{
			FT_Message("It's not possible");
			return;
		}
	else if (objet==4 && lieu_objet[22]==0)	// si on veut mettre la combinaison alors qu'on a le tablier
		{
			FT_Message ("Take off the apron first");
			return;
		}
	else lieu_objet[objet]=0;				// on prend l'objet {0 dans le tableau}

	if (objet==4 || objet == 9 || objet==11)	// si ce sont des habits on dit qu'on les enfile
		{
			FT_Message ("You put it on");
		}
	else
		{
			FT_Message ("Done");
		}
}

// DEPOSE {999}
void FT_Case3 (void)
{
	if (objet==0 || lieu_objet[objet]!=0)		// si on a pas l'objet
	{
		FT_Message ("You don't have it");
		return;
	}
	if  (lieu<9)								// si on est à l'intérieur
	{
		lieu_objet[objet]=lieu;
		FT_Message ("Done");
		return;
	}
	lieu_objet[objet]=-99;						// sinon on est dans l'espace
	FT_Message ("He got lost in space");
}

// ENLEVER {0950,0951,2150,2151} Gestion des vêtements
void FT_Case10 (void)
{
	if (lieu_objet[objet]!=0)				// si on a pas l'objet
	{
		FT_Message ("You don't have it");
		return;
	}
	if (lieu>=9 || (lieu==7 && v2==1))		// dans l'espace
	{
		FT_Message("Air, air!!! Aaagh!!!");
		FT_Kill();
		return;
	}
	if  (lieu<9)							// si on est à l'intérieur
	{
		lieu_objet[objet]=lieu;
		FT_Message ("Done");
		return;
	}
	lieu_objet[objet]=-99;						// si on est à l'extérieur
	FT_Message ("He got lost in space");
}

// REGARDER LE SECOND PILOTE {1053}
void FT_Case12 (void)
{
	FT_CleanTexte ();
	TXT (0,POSY,  "The second lost consciousness",0);
	TXT (0,POSY+9,"and his helmet is split.",0);
	TXT (0,POSY+18,"He must have been struck by",0);
	TXT (0,POSY+27,"a small meteorite during",0);
	TXT (0,POSY+36,"antenna repair.",0);
	TXT (0,POSY+45,"Luckily he's alive.",0);
}

// LIRE OUVRIR MANUEL {2555,2255}
void FT_Case15 (void)
{
	if (lieu_objet[24]==lieu)		// si on a pas l'objet mais qu'il est dans le lieu
	{
		FT_Message ("Take it first");
		return;
	}
	FT_CleanTexte ();
	TXT (0,POSY,   "      POSITRONIC REACTOR",0);
	TXT (0,POSY+9, "          USER MANUAL",0);
	TXT (0,POSY+18,"To activate the reactor :",0);
	TXT (0,POSY+27,"- Pull the lever, press in",0);
	TXT (0,POSY+36,"order green, yellow and red",0);
	TXT (0,POSY+45,"buttons.",0);
	FT_EcranSuivant ();
	FT_CleanTexte ();
	TXT (0,POSY+9,"To desactivate the reactor..",0);
	TXT (0,POSY+18,"        .....",0);
	TXT (0,POSY+27,"Ouch ! The page is torn off !!",0);
}

// REGARDER CADRAN {011066}
void FT_Case16 (void)
{
			Itoa(840-(chrono*4),LigneDeTxt,10);
			FT_CleanTexte ();
			TXT (0,POSY,    "      REACTOR TEMPERATURE",0);
			TXT (0,POSY+9, "Indicated",0);
			TXT (81,POSY+9,LigneDeTxt,0);
			TXT (111,POSY+9, "degrees.",0);
			TXT (0,POSY+18, "It's increasing rapidly,there's",0);
			TXT (0,POSY+27, "a red mark at 800 degrees!",0);
}

// LIRE ECRAN LIEU 2 {022560}
void FT_Case19 (void)
{
	FT_CleanTexte ();
	TXT (0,POSY,    "You are at the foot of a ladder,",0);
	TXT (0,POSY+9,  "It is written on the screen :",0);
	TXT (0,POSY+27, "       RESERVED ENTRY",0);
	TXT (0,POSY+36, "    TO ON-BOARD PERSONNEL",0);
}

// MONTER QUAND ON EST AU LIEU 2 {020500}
void FT_Case20 (void)
{
	if (lieu_objet[20]==9)		// Aide si on a pas trouvé le second
	{
		FT_CleanTexte ();
		TXT (0,POSY,    "     [...Though...]",0);
		TXT (0,POSY+9, "If the second pilot was there,",0);
		TXT (0,POSY+18, "he is the only one capable",0);
		TXT (0,POSY+27, "of handling technical problems.",0);
		FT_EcranSuivant ();
	}

	lieu = 1;
}

// LIRE ECRAN LIEU 4 {042560}
void FT_Case22 (void)
{
	FT_CleanTexte ();
	TXT (0,POSY,    "It's located in the west,",0);
	TXT (0,POSY+9,  "It is written:",0);
	TXT (0,POSY+27, "           WARNING",0);
	TXT (0,POSY+36, "    DEPRESSURIZED COCKPIT",0);
}

// OUVRIR ARMOIRE SECOND {052267}
void FT_Case23 (void)
{
	if (lieu_objet[24]!=-99)			// si le manuel a déjà été pris on arrête
	{
		FT_Message("Already done");
		return;
	}
	if (lieu_objet[23]!=0)				// si on a pas la clef on arrête
	{
		FT_Message("It is locked");
		return;
	}
	FT_Message("Done");			//sinon ok
	lieu_objet[24]=lieu;				// on fait apparaitre le manuel
	lieu_objet[22]=lieu;				// on fait apparaitre le tablier
	FT_CleanTexte ();					// on fait un inventaire du lieu
}

// APPUYER BOUTON COMP ETANCHE {072661}
void FT_Case25 (void)
{
	FT_CleanTexte ();
	TXT (0,POSY+9,   "Be more specific :",0);
	TXT (0,POSY+18, " - \"press red\"",0);
	TXT (0,POSY+27, "or",0);
	TXT (0,POSY+36, " - \"press green\"",0);
}

// APPUYER ROUGE POUR OUVRIR SAS {072664}
void FT_Case26 (void)
{
	int i,pos;

	if (v2==1)		// si c'est déjà ouvert alors juste un clic
	{
	FT_Message("Clic");
	LMMM (40,256+256,40,23,176,84,0);
	return;
	}
	FT_CleanTexte ();								// sinon on ouvre
	TXT (0,POSY+9,  "The EAST door closes.",0);
	TXT (0,POSY+18, "The WEST door opens onto",0);
	TXT (0,POSY+27, "space, the air is evacuated",0);
	TXT (0,POSY+36, "in a whistle...",0);
	LMMM (40,256+256,40,23,176,84,0);
	if (lieu_objet[4]!=0 || lieu_objet[11]!=0)		// si on a pas le casque ou combi on meurt
	{
		FT_Message("Aaaargh !!!");
		FT_Kill ();
	}
	pos=0;
	for (i = 1; i < NBR_OBJET ; ++i)		// si des objets sont dans le lieu
	{
		if (lieu_objet[i]==lieu)     		// ils partent dans l'espace -99
		{
			FT_EcranSuivant ();
			FT_CleanTexte ();
			TXT (10,POSY,"Objects lost in space :",0);
			TXT (0,(POSY+9+pos),liste_objet[i],0);
			lieu_objet[i]=-99;
			pos+=9;
		}
	}
	v2=1;			// flag de sas ouvert
	LMMM (40,256+256,40,23,176,84,0);
}

// APPUYER BOUTON VERT COMP ETANCHE {072662}
void FT_Case27 (void)
{
	if (v2==0)		// si c'est déjà fermé alors juste un clic
	{
	FT_Message("Clic");
	LMMM (40,85+256,40,23,176,84,0);
	return;
	}
	FT_CleanTexte ();								// sinon on ferme
	TXT (0,POSY+9,  "The WEST door closes.",0);
	TXT (0,POSY+18, "The EAST door opens onto",0);
	TXT (0,POSY+27, "the corridor.The compartment",0);
	TXT (0,POSY+36, "oxygen system is on.",0);
	LMMM (40,85+256,40,23,176,84,0);
	v2=0;											// flag de la fermeture du sas
	if (lieu_objet[20]!=0 || lieu_objet[23]!=-99)	// si le second ou la clef ne sont pas là
	{
		return;										// on quitte
	}
	FT_EcranSuivant ();
	FT_CleanTexte ();
	TXT (0,POSY+9,  "the pilot is breathing.",0);
	TXT (0,POSY+18, "He remembers the situation",0);
	TXT (0,POSY+27, "and says:Quick! At the reactor!",0);
	TXT (0,POSY+36, "Here's the key of my...",0);
	TXT (0,POSY+45, "He loses consciousness.",0);

	lieu_objet[23]=lieu;		// on met la clef dans le lieu
}

// LIRE ECRAN SALLE REACTEUR {082560}
void FT_Case28 (void)
{
	FT_CleanTexte ();
	TXT (0,POSY+9,  "RED ALERT !",0);
	TXT (0,POSY+18, "Always have the recator manual",0);
	TXT (0,POSY+27, "with you.",0);
}

// APPUYER BOUTON SALLE REACTEUR {082661}
void FT_Case30 (void)
{
	FT_CleanTexte ();
	TXT (0,POSY,   "Be more specific :",0);
	TXT (0,POSY+9, " - \"press red\"",0);
	TXT (0,POSY+18, "or",0);
	TXT (0,POSY+27, " - \"press green\"",0);
	TXT (0,POSY+36, "or",0);
	TXT (0,POSY+45, " - \"press yellow\"",0);
}
// SALLE REACTEUR : APPUYER ROUGE  {082664}
void FT_Case31 (void)
{
	FT_Message ("CLICK");
	if (v1!=0)						// si le flag est déjà incrémenté
	{
		v1=0;						// alors on perd du  temps
		chrono=abs(chrono/2);
	}

	FT_CleanTexte ();
	TXT (0,POSY,    "a pipe loses slightly through",0);
	TXT (0,POSY+9,  "a joint (surely by excess",0);
	TXT (0,POSY+18, "pressure). Drops are falling",0);
	TXT (0,POSY+27, "on the control screen near you.",0);
	v1=1;							// le flag est incrémenté
}

	// SALLE REACTEUR : APPUYER JAUNE  {082663}
void FT_Case32 (void)
{
	FT_Message ("CLICK");
	if (v1!=1)					// si on pas appuyer le rouge d'abord
	{
		v1=0;					// alors on perd du  temps
		chrono=abs(chrono/2);
	}
	v1=2;						// le flag est incrémenté
	if(lieu_objet[22]!=0)		// si on a pas le tablier
	{
		FT_CleanTexte ();
		TXT (0,POSY,    "You don't feel good",0);	// alerte
	}
}

	// SALLE REACTEUR : APPUYER VERT  {082662}
void FT_Case33 (void)
{
	FT_Message ("CLICK");
	if (v1!=2)					// si on pas appuyer le rouge & jaune d'abord
	{
		v1=0;					// alors on perd du  temps
		chrono=abs(chrono/2);
	}
	v1=3;						// le flag est incrémenté
	if(lieu_objet[22]!=0)		// si on a pas le tablier
	{
		FT_CleanTexte ();
	TXT (0,POSY,    "You just absorbed too much",0);
	TXT (0,POSY+9,  "radiations. You are very sick",0);
	TXT (0,POSY+18, "and you lose consciousness.",0);

	FT_EcranSuivant ();
	FT_Kill ();					// on perd
	}
}

	// SALLE REACTEUR : POUSSER LEVIER {082465}
void FT_Case34 (void)
{
	FT_Message ("CLANK");
	if (v1!=3)					// si on pas appuyer le rouge & jaune d'abord
	{
		v1=0;					// alors on perd du  temps
		chrono=abs(chrono/2);
		return;
	}
	FT_Win ();					// sinon on gagne
}

	// SALLE REACTEUR : TIRER LEVIER {082365}
void FT_Case35 (void)
{
	FT_Message ("CLUNK");
	// alors on perd du  temps
	chrono=abs(chrono/2);
}

	// EXTERIEUR : REGARDER ANTENNE {091069}
void FT_Case36 (void)
{
	FT_CleanTexte ();
	TXT (0,POSY,    "It seems to be damaged by the",0);
	TXT (0,POSY+9,  "impact of a large meteorite.",0);
}

/* ---------------------------------
			FT_Inventaire
	Gère  :  - l'inventaire
	         - ce que l'on voit dans les pièces
----------------------------------*/
void FT_Inventaire(int besoin)
{
	int i,pos,test;

			test=0;
			pos = POSY;							// 1er ligne dispo pour affichage texte
			for (i = 1; i < NBR_OBJET; ++i)		// on liste le tableau objet si = besoin (0 ou lieu)
			{
				if (abs(lieu_objet[i]) == besoin)	// absolu pour lister les objets non prenables
				{
					if (pos==193)				// fin de la page
					{
						TXT (246,pos,"~",0);	// pour passer au texte suivant si trop d'objets
						WaitKey ();
						FT_CleanTexte();
						pos=POSY;
					}
					TXT(64,pos,liste_objet[i],0);
					pos+=9;
					test=1;						// si on a trouvé un objet on active le flag de test
				}
			}
			if (test==0)						// si le flag test n'a pas été activé alors pas d'objets
			{
				TXT(64,POSY+9,"nothing",0);
			}
}


/* ---------------------------------
			FT_Quit
	Gère verbe = Quit
----------------------------------*/
void FT_Quit(void)
{
	// Retour au MSX-DOS
	RestoreSC5Palette();
	SetColors (15,0,0);
	Screen(0);
	Exit(0);
}

/* ---------------------------------
			FT_Temps
	Gère le chrono
----------------------------------*/
void FT_Temps(void)
{

	char temps [3];	// pour chronos affichage

	chrono-=1;
	if (chrono<0)
		{
			chrono = 0;
		}
	TXT (40,0,"chronos",0);
	TXT (160,2,"  ",0);
	Itoa (chrono,temps,10);
	TXT (160,2,temps,0);


	if (chrono<=10 && chrono>5 && ch==0)
	{
		FT_CleanTexte ();
		TXT(0,POSY+9,"An alarm signal starts !",0);
		FT_EcranSuivant ();
		ch=1;
	}
	if (chrono<=5 && chrono>2 && ch==1)
	{
		FT_CleanTexte ();
		TXT(0,POSY+9,"The temperature of the shuttle",0);
		TXT(0,POSY+18,"becomes unbearable !",0);
		FT_EcranSuivant ();
		ch=2;
	}
	if (chrono<=2 && ch==2)
	{
		FT_CleanTexte ();
		TXT(0,POSY+9,"The shuttle is shaken",0);
		TXT(0,POSY+18,"by vibrations !",0);
		FT_EcranSuivant ();
		ch=3;
	}
	if (chrono>0)
	{
		return;
	}
	FT_CleanTexte();
	BoxFill (0,POSY-9,255,212,NOIR,0);				// enlève le nom de la pièce
	TXT(0,POSY,"         TOO LATE !",0);
	TXT(0,POSY+9,"    The reactor is HS",0);
	TXT(0,POSY+18," The shuttle is disintegrated",0);
	TXT(0,POSY+36," It's surprising, the silence",0);
	TXT(0,POSY+45,"  of explosions in space...",0);
	FT_EcranSuivant ();
	FT_Kill ();
}

/* ---------------------------------
			FT_Kill
			Perdu
----------------------------------*/
void FT_Kill (void)
{
	KillKeyBuffer();
	FT_CleanTexte ();
	BoxFill (0,POSY-9,255,212,NOIR,0);				// enlève le nom de la pièce
	TXT(0,POSY,   "   MESSAGE FROM THE GALAXY",0);
	TXT(0,POSY+9, "   Tragedy close to VEGA",0);
	TXT(0,POSY+18,"   Space shuttle ATALANTA",0);
	TXT(0,POSY+27," in service with 250 passengers",0);
	TXT(0,POSY+36," on board, was destroyed by",0);

	FT_EcranSuivant();
	FT_CleanTexte ();
	TXT(0,POSY,   "a violent explosion caused",0);
	TXT(0,POSY+9, "it seems from inexperience",0);
	TXT(0,POSY+18,"of his commander (according to",0);
	TXT(0,POSY+27,"rumors collected by our",0);
	TXT(0,POSY+36,"correspondent).",0);

	FT_EcranSuivant();
	FT_CleanTexte ();
	TXT(0,POSY,   "Who is responsible for this",0);
	TXT(0,POSY+9, "disaster in galactic history",0);
	TXT(0,POSY+18,"will see his memory forever",0);
	TXT(0,POSY+27,"marked by tragedy...",0);
	TXT(0,POSY+45,"           [ END ]",0);
	FT_EcranSuivant();

	FT_Quit();
}

/* ---------------------------------
			FT_Win
			Gagné
----------------------------------*/
void FT_Win (void)
{
	KillKeyBuffer();
	FT_CleanTexte ();
	BoxFill (0,POSY-9,255,212,NOIR,0);				// enlève le nom de la pièce
	TXT(0,POSY,   "    MESSAGE FROM THE GALAXY",0);
	TXT(0,POSY+9, "     A commander saves",0);
	TXT(0,POSY+18,"       space shuttle !",0);
	TXT(0,POSY+27,"The famous ATALANTA shuttle",0);
	TXT(0,POSY+36,"in service with 250 passengers",0);
	TXT(0,POSY+45," on board, was saved from",0);


	FT_EcranSuivant();
		FT_CleanTexte ();
	TXT(0,POSY,   "scheduled destruction,",0);
	TXT(0,POSY+9, "thanks to the coolness of",0);
	TXT(0,POSY+18,"his brave commander who",0);
	TXT(0,POSY+27,"managed to deactivate",0);
	TXT(0,POSY+36,"the blocked reactor.",0);


	FT_EcranSuivant();
		FT_CleanTexte ();
	TXT(0,POSY,   "His name will forever be",0);
	TXT(0,POSY+9, "etched in the memories",0);
	TXT(0,POSY+18,"of galactic history !!",0);
	TXT(0,POSY+36,"       [ END ]",0);
	TXT(0,POSY+45,"  [c] sfranck72-2020",0);
	FT_EcranSuivant();

	FT_Quit();
}


/* ---------------------------------
			FT_Message
	Gère un message en bas  + clignotement
----------------------------------*/
void FT_Message (unsigned char *texte)
{
	BoxFill (0,197,255,212,NOIR,0);	// nettoie la zone texte
	BoxLine (0,197,254,210,BLANC,0);	// cadre blanc pour le message
	TXT (8,200,texte,0);
	FT_Wait (100);					// tempo 10
	BoxFill (0,197,255,212,NOIR,0);	// nettoie la zone texte
}

/* ---------------------------------
			FT_Wait
			tempo
----------------------------------*/
void FT_Wait (int temps)
{
	int cpt;
	cpt=0;				//compteur
	while (cpt<=temps)	// tempo
	    {
			EnableInterupt()
			Halt ();
	       	cpt ++;
	    }
}

/* ---------------------------------
			FT_CleanTexte
		nettoie la zone texte
----------------------------------*/
void FT_CleanTexte (void)
{
BoxFill (0,POSY,255,212,NOIR,0);	// nettoie la zone texte
}

/* ---------------------------------
			FT_EcranSuivant
		- attend une touche
		- affiche la flèche bas
----------------------------------*/
void FT_EcranSuivant (void)
{
	TXT (246,POSY+45,"~",0);	// pour passer au texte suivant
	WaitKey ();
	TXT (246,POSY+45," ",0);
}

/* ---------------------------------
            FT_CheckFX
     Check if Playing Sound FX
          must be updated
-----------------------------------*/
void FT_CheckFX (void)
{
/* if (TestFX()==1)
 {
      if (JIFFY!=0)
      {
       JIFFY=0;
        UpdateFX();
      }
  }*/
}

/* ---------------------------------
			FT_Debug
			debug
----------------------------------*/
void FT_Debug (void)
{
	char test [10];	// pour debug lieu

/*		Itoa (code_verbe,test,10);		// DEBUG verbe
		TXT (0,0,"verbe  ",0);
		TXT (40,0,test,0);
		Itoa (code_nom,test,10);		// DEBUG nom
		TXT (0,9,"nom    ",0);
		TXT (40,9,test,0);
		Itoa (objet,test,10);			// DEBUG indice objet
		TXT (0,18,"objet  ",0);
		TXT (40,18,test,0);
		Itoa(action,test,10);			// DEBUG action
		TXT (0,27,"acti   ",0);
		TXT (31,27,test,0);
		Itoa(flag_nocomprendo,test,10);	// DEBUG No_comprendo
		TXT (0,36,"nocom  ",0);
		TXT (48,36,test,0);
		Itoa(execute,test,10);			// DEBUG execute
		TXT (0,45,"exec   ",0);
		TXT (48,45,test,0);
		TXT (200,0,"lieu   ",0);		// DEBUG lieu
		Itoa (lieu,test,10);
		TXT (232,0,test,0);
		Itoa (code_total,test,10);		// DEBUG Code_total
		TXT (0,121,"total          ",0);
		TXT (50,121,test,0);*/

		TXT (37,0,"chronos:",0);			// DEBUG chrono
		TXT (160,2,"  ",0);
		Itoa (chrono,test,10);
		TXT (160,2,test,0);
/*		TXT (200,0,"lieu   ",0);		// DEBUG lieu
		Itoa (lieu,test,10);
		TXT (232,0,test,0);
*/
}

/*-------------------------------------------------------

					MAIN

------------------------------------------------------- */

void main(void)
{

	//int i;


	// Init Screen 5
	FT_Init ();

	// Charge l'intro
	FT_Intro();

	// Charge les CHR
	FT_DumpChr();

	// Gestion des variables
	last_lieu = 0;			// dernier lieu visité
	lieu = START_LIEU;		// Lieu de départ
	chrono=100;				// Gestion du temps
	ch=0;
	v1=1;					// Gestion flag salle de réacteur
	v2=0;					// Gestion SAS ouvert/fermé
	FT_ShowImage ();		// HUB

	while(1)
	{

		len_entry = 0;
		code_verbe = 0;
		code_nom   = 0;
		code_total =0;
		flag_nocomprendo = 0;
		objet = 0;
		execute = 0;			// flag pour sortir de la boucle si on a executé une action dans B,D ou E
		action = 0;
		FT_Temps ();			// maj du temps

		if (last_lieu!=lieu)
		{
		FT_SearchData (lieu);
		FT_ShowLieu();
	 	FT_ShowTxt ();
		}

		FT_Entry ();
		FT_Parser ();
		FT_Verbe ();
		FT_Nom ();

		if (flag_nocomprendo==0)
			{
			FT_Objet ();
			FT_Total ();
			FT_BeforeExecute ();
			}

		if (flag_nocomprendo==0 && execute==1)
		{

			if (action<10)
			{
				FT_Execute  ();					// action de 0 à 9
			}

			 if (action>=10 && action<20)
			{
				FT_Execute1 ();					// action de 10 à 19
			}
			 if (action>=20 && action<30)
			{
				FT_Execute2 ();					// action de 20 à 29
			}
			if (action>=30)
			{
				FT_Execute3 ();					// action de 30 à 39
			}
		}

		if (flag_nocomprendo==1)
		{
			FT_Message("No Comprendo");
			flag_nocomprendo==0;
		}





	//FT_Debug ();


	} 	// fin du while (1)
}		// Fin du MAIN
