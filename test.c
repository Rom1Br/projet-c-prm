#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <menu.h>

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
void update_screens(WINDOW *field, WINDOW *score, float cash, char prenom[80], int qty_AMZN, int qty_FB, int qty_AAPL, int qty_MSFT, int qty_GOOGL, int day, float cours_GOOGL[21]){
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
    mvwprintw(score, 2, COLS-20, "%.f USD", cash);
    mvwprintw(score, 4, 1, "------ ACTION ------ COURS DU JOUR ------ QTE POSSEDEE ------ VALEUR ------");
    mvwprintw(score, 5, 1, "Apple (AAPL)");
    mvwprintw(score, 5, COLS-10, "%d ", qty_AAPL);
    mvwprintw(score, 6, 1, "Amazon (AMZN)");
    mvwprintw(score, 6, COLS-10, "%d ", qty_AMZN);
    mvwprintw(score, 7, 1, "Facebook (FB)");
    mvwprintw(score, 7, COLS-10, "%d ", qty_FB);
    mvwprintw(score, 8, 1, "Google (GOOGL)       %.2f                    %d              %.2f", cours_GOOGL[day], qty_GOOGL, qty_GOOGL*cours_GOOGL[day]);
    mvwprintw(score, 9, 1, "Microsoft (MSFT)");
    mvwprintw(score, 9, COLS-10, "%d ", qty_MSFT);
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

int main(int argc, char *argv[]) {
  // A partir d'ici, on fixe la taille des fenêtres et on prend en charge les possibles resize du terminal
  int parent_x, parent_y, new_x, new_y;
  int score_size = 15;

  initscr();
  curs_set(1);

  // Définition des 2 fenêtres
  getmaxyx(stdscr, parent_y, parent_x);

  WINDOW *field = newwin(parent_y - score_size, parent_x, 0, 0);
  WINDOW *score = newwin(score_size, parent_x, parent_y - score_size, 0);

  draw_borders(field);
  draw_borders(score);
    
// Initialisation de nos variables
   int day, qty, qty_AMZN, qty_FB, qty_GOOGL, qty_AAPL, qty_MSFT;
   float cash, portefeuille, cours_AAPL[21], cours_AMZN[21], cours_FB[21], cours_GOOGL[21], cours_MSFT[21];
   cash = qty_AMZN = qty_FB = qty_GOOGL = qty_AAPL = qty_MSFT = 0; //Initialisation du portefeuille
   //La valeur du portefeuille sera donc cash + qty_AMZN*cours_AMZN + ...
   char prenom[80];
   int days[21] = {1, 2, 3, 6, 7, 8, 9, 10, 13, 14, 15, 16, 17, 20, 21, 22, 24, 27, 28, 29, 30};
   

//Lecture des CSV
char line[80];
int i;

i=0;
FILE* stream = fopen("GOOGL.csv", "r");
while (fgets(line, 80, stream))
{
    char* tmp = strdup(line);
    cours_GOOGL[i] = atof(getfield(tmp, 2));
    // NOTE strtok clobbers tmp
    free(tmp);
    i++;
}

i=0;
stream = fopen("AAPL.csv", "r");
while (fgets(line, 80, stream))
{
    char* tmp = strdup(line);
    cours_AAPL[i] = atof(getfield(tmp, 2));
    // NOTE strtok clobbers tmp
    free(tmp);
    i++;
}

   
// Cette boucle sert à prendre en charge le resize du terminal
  while(1) {
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
    update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);

    // refresh each window
    wrefresh(field);
    wrefresh(score);

    //Récupération du prénom
    mvwprintw(field, 1, 1, "Please enter your name :   ");
    wgetstr(field, prenom);
    update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);


    //Récupération du cash investi
    mvwprintw(field, 1, 1, "Hi %s, how much cash do you want to invest? ", prenom);
    wscanw(field, "%f", &cash);
    update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);
    wrefresh(score);

    day = 0;
    char stock[80], action[80];
    int qty;
    qty = 10;
    //Début de la boucle des jours
    while(days[day]){
        //On vérifie que la personne ne veuille pas passer au jour suivant
            mvwprintw(field, 1, 1, "Quelle action souhaitez-vous trader aujourd'hui? (%d/11)\n",days[day]);
            mvwprintw(field, 2, 1, "(Par exemple, AAPL. 'NEXT' pour passer au jour suivant)\n");
            mvwprintw(field, 3, 1, "");
            wgetstr(field, stock);
            update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);
            if(!strcmp(stock,"NEXT")){
                day++;
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
                update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);
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
                update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);
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
                update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);
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
                update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);
            }  
            update_screens(field, score, cash, prenom, qty_AMZN, qty_FB, qty_AAPL, qty_MSFT, qty_GOOGL, day, cours_GOOGL);   
    };
    // FIN BOUCLE JOURS

    
  }

  endwin();

  return 0;
}
