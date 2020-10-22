/* ---------------------------------
            Fusion-C
 Programme pour créer des fichiers dsk 
 au format 'mypalette'
 à partir d'unfichier classique .PL5

      sfranck72 - 28_10_2019
-----------------------------------*/

#include "fusion-c/header/msx_fusion.h"
#include "fusion-c/header/vdp_graph2.h"
#include "fusion-c/header/io.h"
#include <string.h>
#include <stdio.h>

// Prototypes
int FT_LoadSc5Image(char *file_name, unsigned int start_Y, char *buffer);
int FT_LoadData(char *file_name, char *buffer, int size, int skip);
void FT_errorHandler(char n, char *name);
void FT_SetName( FCB *p_fcb, const char *p_name );
void FT_AfficheTableau (char *p_tableau,char nom[],int j);

//	Variables
static FCB file;
unsigned char LDbuffer[2560]={0}; // Buffer pour les images
char mypalette[64]={0};           // Palette SC5 initialisée à 0



//	Fonctions
/* ---------------------------------
          FT_LoadSc5Image
  Charge les données d'un fichier
  à l'écran via un buffer Ram.
  L'image s'affiche à partir de la ligne Y
-----------------------------------*/ 

int FT_LoadSc5Image(char *file_name, unsigned int start_Y, char *buffer)        // Charge les données d'un fichiers
    {

        int rd=2560;

        FT_SetName( &file, file_name );
        if(fcb_open( &file ) != FCB_SUCCESS) 
        {
              FT_errorHandler(1, file_name);
              return (0);
        }

        fcb_read( &file, buffer, 7 );  // Skip 7 first bytes of the file  
        while (rd!=0)
        {
             rd = fcb_read( &file, buffer, 2560 );  // Read 20 lines of image data (128bytes per line in screen5)
             HMMC(buffer, 0,start_Y,256,20 ); // Move the buffer to VRAM. 
             start_Y=start_Y+20;
         }

return(1);
}

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
    //SetColors(15,0,80);
    
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
  //SetColors(15,6,6);
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
Exit(0);
}


/* ---------------------------------
      Affiche les valeurs d'un tableau
param :
    -nom du tableau sans les crochets (pointeur).
    -texte qu'on veut afficher pour le nom du tableau.
    -nombre de valeurs du tableau à afficher.

----------------------------------*/
void FT_AfficheTableau (char *p_tableau,char nom[],int j)
{
    int i;
    for(i=0;i<j;i++)
  {
    printf("\n\r%s [%d",nom,i );
    
    printf("] = %d",p_tableau[i]);
   }
   WaitForKey();
}


/* --------------------------------------------------------
-----------------------------------------------------------
------------------------MAIN-------------------------------
-----------------------------------------------------------
---------------------------------------------------------*/ 

void main(void) 
{

  char fichier[16]= {0};
  char ext_pl5[5] = ".pl5";
  char ext_pal[5] = ".pal";
  char ram_buff [32] = {0};
  unsigned int i, j;

 
// Programme pour creer des dsk avec le contenu du fichier mypalette
  Width(50);
  Cls();
  printf("\n  oO CREATION FICHIER PALETTE Oo");
  printf("\n\r   pour la structure 'mypalette' ");
  printf("\n\r       de la Lib FUSION-C");
  printf("\n\r      via un fichier .PL5");
  printf("\n\r==================================\n\n\r");
  printf("Nom du fichier.PL5 (Ne pas saisir l'extension)");
   printf("\n\r[ Max 8 carac ] \n\r>");
  InputString(fichier,10);
  strcat(fichier,ext_pl5);

  FT_LoadData(fichier, ram_buff, 32, 7);
  for (i=0;i<64;i+=4)
  {
    j=i/4;
    mypalette[i]=j;              // N° de palette

   if (j==0)
   {    // 1ere fois
    mypalette[i+1]=ram_buff[j]/16;    // R color
    mypalette[i+2]=ram_buff[j+1];     // G color
    mypalette[i+3]=ram_buff[j]%16;    // B color 
   }
   else
   {    // 2eme fois
    mypalette[i+1]=ram_buff[j*2]/16;    // R color
    mypalette[i+2]=ram_buff[j*2+1];       // G color
    mypalette[i+3]=ram_buff[j*2]%16;    // B color
   }
  }
  printf("\n\rchargement des donnees OK.\n");

  // Mypalette est chargée avec le fichier .PL5
  // On passe à l'enregistrement sur dsk

  printf("\n\rEntrez un nom de fichier");
  printf("\n\rpour l'enregistrement :");
  printf("\n\r[ Max 8 carac --");
  printf(" l'extension sera .pal ]\n\r>");
  InputString(fichier,10);
  strcat(fichier,ext_pal);
  Cls();
  printf("\n\rCreation de : %s",fichier);

  // Création nouveau fichier sur dsk
  FT_SetName(&file,fichier);
  fcb_create(&file);
  printf("\n\rCreation OK.");
  printf("\n\n\rCopie datas fichier.PL5 -> fichier .PAL");
  printf("\n\r[au format 'mypalette']");

  // Copie des données au format mypalette sur dsk
  fcb_write(&file,mypalette,64);
  fcb_close(&file);
  printf("\n\rCopie OK.");
  printf("\n\n\rDans FUSION-C pour charger la palette:");
  printf("\n\n\rFT_LoadData(\"nom du fichier.pal\",mypalette,64,0);");
  printf("\n\n\rSetSC5Palette((Palette *)mypalette);");  
  printf("\n\n\r[Return] pour quitter.");
  WaitForKey();
  printf("\n\r");
  Exit(0);

}
 