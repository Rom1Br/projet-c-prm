// MG: 
// PDCURSES 1 pour compilation sous Windows avec librairie pdcurses.lib et execution avec pdcurses.dll
// PDCURSES 0 pour fonctionnement sous MACC

#define PDCURSES 1 

#if PDCURSES
#include <curses.h>
#else
#include <ncurses.h>
#include <menu.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define TAILLE 21 //MG: pour définir la taille du tableau days et arrêter la boucle après le 30/11


void draw_borders(WINDOW *screen) {
  int x, y, i;

  getmaxyx(screen, y, x);

  // 4 corners
  mvwprintw(screen, 0, 0, "+");
  mvwprintw(screen, y - 1, 0, "+");
  mvwprintw(screen, 0, x - 1, "+");
  mvwprintw(screen, y - 1, x - 1, "+");

  // sides
  for (i = 1; i < (y - 1); i++) {
    mvwprintw(screen, i, 0, "|");
    mvwprintw(screen, i, x - 1, "|");
  }

  // top and bottom
  for (i = 1; i < (x - 1); i++) {
    mvwprintw(screen, 0, i, "-");
    mvwprintw(screen, y - 1, i, "-");
  }
}

// Petite fonction faite maison pour la MAJ des investissements, et cleaner les deux fenêtres
void update_screens(WINDOW *field, WINDOW *score, float cash, char *prenom, int qty_AAPL, int qty_AMZN, int qty_FB, int qty_GOOGL, int qty_MSFT,  int day, 
    float * cours_AAPL, float * cours_AMZN, float * cours_FB, float * cours_GOOGL, float * cours_MSFT){

    werase(field);
    draw_borders(field);
    werase(score);
    draw_borders(score);
    
    if(strcmp(prenom,"")){
        mvwprintw(score, 1, (COLS/2)-8, "%s's Investments", prenom);
    }
    else{
        mvwprintw(score, 1, (COLS/2)-8, "Your Investments");
    }

    mvwprintw(score, 2, 1, "Cash :");
    mvwprintw(score, 2, COLS-20, "%.2f USD", cash);
    mvwprintw(score, 4, 1, "------ ACTION ------ COURS DU JOUR ------ QTE POSSEDEE ------ VALEUR ------");
    mvwprintw(score, 5, 1, "Apple (AAPL)           %8.2f            %8d              %10.2f", cours_AAPL[day], qty_AAPL, qty_AAPL*cours_AAPL[day]);
    mvwprintw(score, 6, 1, "Amazon (AMZN)          %8.2f            %8d              %10.2f", cours_AMZN[day], qty_AMZN, qty_AMZN*cours_AMZN[day]);
    mvwprintw(score, 7, 1, "Facebook (FB)          %8.2f            %8d              %10.2f", cours_FB[day], qty_FB, qty_FB*cours_FB[day]);
    mvwprintw(score, 8, 1, "Google (GOOGL)         %8.2f            %8d              %10.2f", cours_GOOGL[day], qty_GOOGL, qty_GOOGL*cours_GOOGL[day]);
    mvwprintw(score, 9, 1, "Microsoft (MSFT)       %8.2f            %8d              %10.2f", cours_MSFT[day], qty_MSFT, qty_MSFT*cours_MSFT[day]);
    wrefresh(field);
    wrefresh(score);
}

//Fonction pour parser les CSV
const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}
//Fin fonction pour parser

//MG: fonction qui charge les données des fichiers CSV: renvoie 0 si OK, 1 si fichier introuvable, 2 si fichier incomplet
int chargement_data(const char *nom_fichier,float *cours,int count){ 
FILE *stream = NULL;
int i=0;
char line[80];
int r=0;
stream = fopen(nom_fichier, "r");
    if (stream != NULL)
    {
        fgets(line, 80, stream); //MG: ignorer la ligne 1 (contient titres des colonnes)
        while (i<count && fgets(line, 80, stream)) //MG: pour s'assurer qu'on ne prend que max 21 lignes (dans le cas présent)
        {
            char* tmp = strdup(line);
            cours[i] = atof(getfield(tmp, 2));
            // NOTE strtok clobbers tmp
            free(tmp);
            i++;
        }
        if(i!=21){
        r=2;
        printf("Le fichier %s est incomplet\n",nom_fichier);
        }
    fclose(stream);
    } 
    else {
        r=1;
        printf("Le fichier %s n a pas pu etre ouvert\n",nom_fichier);
    }
return r;  
}

//MG: fonction pour sauvegarder le score (grosse surprise vu le nom...)
int save_score(const char *nom_fichier,char *prenom,float score){ 
FILE *stream = NULL;
char line[80];
char date[20];
char heure[20];
int r=0;
int rc;
stream = fopen(nom_fichier, "a"); 
    if (stream != NULL)
    {
        //récupérer date et heure pour inscrire dans la sauvegarde (au cas où le même joueur joue plusieurs fois)
        time_t t = time(NULL);

        strftime(date, sizeof date, "%Y/%m/%d", localtime(&t));
        strftime(heure, sizeof heure, "%H:%M:%S",  localtime(&t));
        // fin récupération date et heure

        sprintf(line, "%s,%s,%s,%f\n",date,heure,prenom,score); 
        rc = fputs(line,stream);
        if (rc == EOF){
            printf("Erreur d ecriture dans le fichier %s\n",nom_fichier);
            perror("fputs()");
            r=2; 
            }
    fclose(stream);
    } 
    else {
        r=1;
        printf("Le fichier %s n a pas pu etre ouvert\n",nom_fichier);
        perror("fopen()");
    }
return r;  
}
///////////////////////////////////////////////////////Main//////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  // A partir d'ici, on fixe la taille des fenêtres et on prend en charge les possibles resize du terminal
  int parent_x, parent_y, new_x, new_y;
  int score_size = 15;
  int sortie = 0; //MG: pour pouvoir demander la sortie du jeu

    // Initialisation de nos variables
   int day, qty, qty_AMZN, qty_FB, qty_GOOGL, qty_AAPL, qty_MSFT;
   float cash, cash_init, roi, cours_AAPL[21], cours_AMZN[21], cours_FB[21], cours_GOOGL[21], cours_MSFT[21];
   cash = cash_init = roi = qty_AMZN = qty_FB = qty_GOOGL = qty_AAPL = qty_MSFT = 0; //Initialisation du portefeuille
   //La valeur du portefeuille sera donc cash + qty_AMZN*cours_AMZN + ...
   char prenom[80]=""; //MG: initialiser la variable pour ne pas afficher des caractères bizarres avant de demander le prénom
   int days[TAILLE] = {1, 2, 3, 6, 7, 8, 9, 10, 13, 14, 15, 16, 17, 20, 21, 22, 24, 27, 28, 29, 30};

    char stock[80], action[80];
    
    day=0;
  
    if(chargement_data("AAPL.csv",cours_AAPL,TAILLE)!=0){
        exit(1);
        }
    if(chargement_data("AMZN.csv",cours_AMZN,TAILLE)!=0){
        exit(1);
        }
    if(chargement_data("FB.csv",cours_FB,TAILLE)!=0){
        exit(1);
        }
    if(chargement_data("GOOGL.csv",cours_GOOGL,TAILLE)!=0){
        exit(1);
        }
    if(chargement_data("MSFT.csv",cours_MSFT,TAILLE)!=0){
        exit(1);
        }

  initscr();
  curs_set(1);

  // Définition des 2 fenêtres

  getmaxyx(stdscr, parent_y, parent_x);
  WINDOW *field = newwin(parent_y - score_size, parent_x, 0, 0);
  WINDOW *score = newwin(score_size, parent_x, parent_y - score_size, 0);
  draw_borders(field);
  draw_borders(score);

// Cette boucle sert à prendre en charge le resize du terminal --> MG: chez moi le resize ne marchait pas, mais est-ce très important ?

  //while(sortie!=1) { 
    getmaxyx(stdscr, new_y, new_x);
    if (new_y != parent_y || new_x != parent_x) {
      parent_x = new_x;
      parent_y = new_y;
      wresize(field, new_y - score_size, new_x);
      wresize(score, score_size, new_x);
      mvwin(score, new_y - score_size, 0);
      wclear(stdscr);
      wclear(field);
      wclear(score);
      draw_borders(field);
      draw_borders(score);
    }

    // Initialisation des 2 fenêtres
    update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
        cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);

    // refresh each window
    wrefresh(field);
    wrefresh(score);

    //Récupération du prénom
    mvwprintw(field, 1, 1, "Please enter your name :   ");
    wgetstr(field, prenom);
    update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
        cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);

    //Récupération du cash investi
    mvwprintw(field, 1, 1, "Hi %s, how much cash do you want to invest? ", prenom);
    wscanw(field, "%f", &cash_init);
    cash = cash_init; // MG: Pour prévoir calcul ROI final
    update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
        cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);
    wrefresh(score);

    day = 0;
    qty = 10;
    //Début de la boucle des jours
    while(day<TAILLE){ //MG: changement de la condition de sortie, sinon on ne sortait pas après le 30/11
        //On vérifie que la personne ne veuille pas passer au jour suivant
            mvwprintw(field, 1, 1, "Quelle action souhaitez-vous trader aujourd'hui? (%d/11)\n",days[day]);
            mvwprintw(field, 2, 1, "(AAPL, AMZN, FB, GOOGL ou MSFT. 'NEXT' = jour suivant, 'EXIT' = sortir)\n");
            mvwprintw(field, 3, 1, "");
            wgetstr(field, stock);
            update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
                cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);
            //MG: sortie sur le mot EXIT pour finir le jeu avant le 30/11 (ou tester le programme sans avoir à taper 20 fois 'NEXT'...)
            if(!strcmp(stock,"EXIT")){
                sortie = 1; 
                break; 
            }
            else if(!strcmp(stock,"NEXT")){
                day++;
            //MG: sortie automatique sur fin du tableau des jours
                if(day>=TAILLE){ 
                    sortie =1;
                    break;
                }
            }
            else if(!strcmp(stock,"AAPL")){
                mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) APPLE. Que souhaitez-vous faire?", qty_AAPL);
                mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34) Cours du jour : %.2f", cours_AAPL[0]);
                mvwprintw(field, 3, 1, "");
                wscanw(field, "%s %d", &action, &qty);
                if(!strcmp(action,"SELL")){
                    qty_AAPL -= qty;
                    cash += qty*cours_AAPL[day];
                }
                else if(!strcmp(action,"BUY")){
                    qty_AAPL += qty;
                    cash -= qty*cours_AAPL[day];
                }
            }
            else if(!strcmp(stock,"GOOGL")){
                mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) GOOGLE. Que souhaitez-vous faire?", qty_GOOGL);
                mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34)");
                mvwprintw(field, 3, 1, "");
                wscanw(field, "%s %d", &action, &qty);
                if(!strcmp(action,"SELL")){
                    qty_GOOGL -= qty;
                    cash += qty*cours_GOOGL[day];
                }
                else if(!strcmp(action,"BUY")){
                    qty_GOOGL += qty;
                    cash -= qty*cours_GOOGL[day];
                }
            }
            else if(!strcmp(stock,"AMZN")){
                mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) AMAZON. Que souhaitez-vous faire?", qty_AMZN);
                mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34)");
                mvwprintw(field, 3, 1, "");
                wscanw(field, "%s %d", &action, &qty);
                update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
                    cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);
                if(!strcmp(action,"SELL")){
                    qty_AMZN -= qty;
                    cash+= qty*cours_AMZN[day];
                }
                else if(!strcmp(action,"BUY")){
                    qty_AMZN += qty;
                    cash-= qty*cours_AMZN[day];
                }
            }
            else if(!strcmp(stock,"FB")){
                mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) FACEBOOK. Que souhaitez-vous faire?", qty_FB);
                mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34)");
                mvwprintw(field, 3, 1, "");
                wscanw(field, "%s %d", &action, &qty);
                update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
                    cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);
                if(!strcmp(action,"SELL")){
                    qty_FB -= qty;
                    cash+= qty*cours_FB[day];
                }
                else if(!strcmp(action,"BUY")){
                    qty_FB += qty;
                    cash-= qty*cours_FB[day];
                }
            } 
            else if(!strcmp(stock,"MSFT")){
                mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) MICROSOFT. Que souhaitez-vous faire?", qty_MSFT);
                mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34)");
                mvwprintw(field, 3, 1, "");
                wscanw(field, "%s %d", &action, &qty);
                update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
                    cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);
                if(!strcmp(action,"SELL")){
                    qty_MSFT -= qty;
                    cash+= qty*cours_MSFT[day];
                }
                else if(!strcmp(action,"BUY")){
                    qty_MSFT += qty;
                    cash-= qty*cours_MSFT[day];
                }
            } 
            else {
                mvwprintw(field, 4, 1, "ERREUR : Vous ne pouvez pas trader cette action.");
                wrefresh(field);
                sleep(2);
                update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
                    cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);
            }  
            update_screens(field, score, cash, prenom, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day, 
                cours_AAPL, cours_AMZN, cours_FB, cours_GOOGL, cours_MSFT);  
    };
    // FIN BOUCLE JOURS
    
  //}
  endwin();

roi = ((qty_AAPL*cours_AAPL[TAILLE-1]
        +qty_AMZN*cours_AMZN[TAILLE-1]
        +qty_FB*cours_FB[TAILLE-1]
        +qty_GOOGL*cours_GOOGL[TAILLE-1]
        +qty_MSFT*cours_MSFT[TAILLE-1]
        +cash 
        -cash_init)/cash_init)*100; //on multiplie par 100 pour l'afficher en %
printf("Game Over\n\n");
printf("Votre score final (ROI): %f %%\n",roi);
printf("Bonne journee !");
save_score("score.csv",prenom,roi);

return 0;
}
