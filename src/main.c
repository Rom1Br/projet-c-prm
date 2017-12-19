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
    /*
     * Dessine les bords d'une "fenêtre" dans le terminal avec ncurses
     */

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


void update_screens(WINDOW *field, WINDOW *score, float cash, char *username, int qty_AAPL, int qty_AMZN, int qty_FB,
                    int qty_GOOGL, int qty_MSFT, int day,
                    float *price_AAPL, float *price_AMZN, float *price_FB, float *price_GOOGL, float *price_MSFT) {

    /*
     * Met à jour les investissements et clean les deux fenêtres
     */

    werase(field);
    draw_borders(field);
    werase(score);
    draw_borders(score);

    if (strcmp(username, "")) {
        mvwprintw(score, 1, (COLS / 2) - 8, "Investissements de %s", username);
    } else {
        mvwprintw(score, 1, (COLS / 2) - 8, "Vos investissements");
    }

    mvwprintw(score, 2, 1, "Cash :");
    mvwprintw(score, 2, COLS - 20, "%.2f USD", cash);
    mvwprintw(score, 4, 1, "------ ACTION ------ COURS DU JOUR ------ QTE POSSEDEE ------ VALEUR ------");
    mvwprintw(score, 5, 1, "Apple (AAPL)           %8.2f            %8d              %10.2f", price_AAPL[day], qty_AAPL,
              qty_AAPL * price_AAPL[day]);
    mvwprintw(score, 6, 1, "Amazon (AMZN)          %8.2f            %8d              %10.2f", price_AMZN[day], qty_AMZN,
              qty_AMZN * price_AMZN[day]);
    mvwprintw(score, 7, 1, "Facebook (FB)          %8.2f            %8d              %10.2f", price_FB[day], qty_FB,
              qty_FB * price_FB[day]);
    mvwprintw(score, 8, 1, "Google (GOOGL)         %8.2f            %8d              %10.2f", price_GOOGL[day],
              qty_GOOGL, qty_GOOGL * price_GOOGL[day]);
    mvwprintw(score, 9, 1, "Microsoft (MSFT)       %8.2f            %8d              %10.2f", price_MSFT[day], qty_MSFT,
              qty_MSFT * price_MSFT[day]);
    wrefresh(field);
    wrefresh(score);
}


const char *getfield(char *line, int num) {
    /*
     * Parse les fichiers CSV
     */

    const char *tok;
    for (tok = strtok(line, ",");
         tok && *tok;
         tok = strtok(NULL, ",\n")) {
        if (!--num)
            return tok;
    }
    return NULL;
}


int load_data(const char *file_name, float *price, int count) {
    /*
     * Charge les données des fichiers CSV
     * Renvoie 0 si ok, 1 si le fichier est introuvable et 2 si le fichier est incomplet
     */

    FILE *stream = NULL;
    int i = 0;
    char line[80];
    int status = 0;

    stream = fopen(file_name, "r");

    if (stream != NULL) {
        //MG: ignorer la ligne 1 (contient titres des colonnes)
        fgets(line, 80, stream);

        //MG: pour s'assurer qu'on ne prend que max 21 lignes (dans le cas présent)
        while (i < count && fgets(line, 80, stream))
        {
            char *tmp = strdup(line);
            price[i] = atof(getfield(tmp, 2));
            // NOTE strtok clobbers tmp
            free(tmp);
            i++;
        }
        if (i != 21) {
            status = 2;
            printf("Le fichier %s est incomplet.\n", file_name);
        }
        fclose(stream);

    } else {
        status = 1;
        printf("Le fichier %s n'a pas pu etre ouvert.\n", file_name);
    }

    return status;
}


int save_score(const char *file_name, char *username, float score) {
    /*
     * Sauvegarde le score final
     * Renvoie 0 si ok, 1 si le fichier n'a pas pu être ouvert et 2 si une erreur d'écriture est survenue
     */

    FILE *stream = NULL;
    char line[80];
    char date[20];
    char hour[20];
    int status = 0;
    int writing_status;
    stream = fopen(file_name, "a");

    if (stream != NULL) {
        // Récupère date et heure pour inscrire dans la sauvegarde (au cas où le même joueur joue plusieurs fois)
        time_t t = time(NULL);
        strftime(date, sizeof date, "%Y/%m/%d", localtime(&t));
        strftime(hour, sizeof hour, "%H:%M:%S", localtime(&t));

        sprintf(line, "%s,%s,%s,%f\n", date, hour, username, score);
        writing_status = fputs(line, stream);

        if (writing_status == EOF) {
            printf("Erreur d'écriture dans le fichier %s.\n", file_name);
            perror("fputs()");
            status = 2;
        }
        fclose(stream);

    } else {
        status = 1;
        printf("Le fichier %s n'a pas pu etre ouvert.\n", file_name);
        perror("fopen()");
    }

    return status;
}


/*
 *  ************** MAIN *****************
 */


int main(int argc, char *argv[]) {
    // Initialisation de la taille des fenêtres (on prend en charge les possibles resize du terminal)
    int parent_x, parent_y, new_x, new_y;
    int score_size = 15;

    // Initialisation d'une variable de sortie de jeu
    int exit_flag = 0;

    // Initialisation de nos variables
    int day, qty, qty_AMZN, qty_FB, qty_GOOGL, qty_AAPL, qty_MSFT;
    float cash, cash_init, roi, price_AAPL[21], price_AMZN[21], price_FB[21], price_GOOGL[21], price_MSFT[21];

    // Initialisation du portefeuille
    //La valeur du portefeuille sera donc cash + qty_AMZN*price_AMZN + ...
    cash = cash_init = roi = qty_AMZN = qty_FB = qty_GOOGL = qty_AAPL = qty_MSFT = 0;

    // Initialisation pour ne pas afficher de caractère bizarres avant de demander le prénom
    char username[80] = "";

    int days[TAILLE] = {1, 2, 3, 6, 7, 8, 9, 10, 13, 14, 15, 16, 17, 20, 21, 22, 24, 27, 28, 29, 30};

    char stock[80], action[80];

    day = 0;

    // Vérification de l'intégrité des fichiers de données
    if (load_data("data/AAPL.csv", price_AAPL, TAILLE) != 0) {
        exit(1);
    }
    if (load_data("data/AMZN.csv", price_AMZN, TAILLE) != 0) {
        exit(1);
    }
    if (load_data("data/FB.csv", price_FB, TAILLE) != 0) {
        exit(1);
    }
    if (load_data("data/GOOGL.csv", price_GOOGL, TAILLE) != 0) {
        exit(1);
    }
    if (load_data("data/MSFT.csv", price_MSFT, TAILLE) != 0) {
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

    // Initialisation des deux fenêtres

    update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                   price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);

    // Refresh les fenêtres
    wrefresh(field);
    wrefresh(score);

    // Récupération du prénom
    mvwprintw(field, 1, 1, "Entrez votre nom d'utilisateur :   ");
    wgetstr(field, username);

    update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                   price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);

    // Récupération du cash investi
    mvwprintw(field, 1, 1, "Bonjour %s, quel est le montant de départ que vous souhaitez investir ? ", username);
    wscanw(field, "%f", &cash_init);

    // MG: Pour prévoir calcul ROI final
    cash = cash_init;
    update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                   price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);
    wrefresh(score);

    day = 0;
    qty = 10;

    // Début de la boucle des jours

    while (day < TAILLE) {
        // On sort automatiquement après le 30/11
        // On vérifie que la personne ne veuille pas passer au jour suivant

        mvwprintw(field, 1, 1, "Quelle action souhaitez-vous trader aujourd'hui? (%d/11)\n", days[day]);
        mvwprintw(field, 2, 1, "(AAPL, AMZN, FB, GOOGL ou MSFT. 'NEXT' = jour suivant, 'EXIT' = sortir)\n");
        mvwprintw(field, 3, 1, "");
        wgetstr(field, stock);

        update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                       price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);

        // On sort sur le mot EXIT pour finir le jeu avant le 30/11
        if (!strcmp(stock, "EXIT")) {
            exit_flag = 1;
            break;

        } else if (!strcmp(stock, "NEXT")) {
            day++;
            // On sort automatiquement à la fin du tableau des jours
            if (day >= TAILLE) {
                exit_flag = 1;
                break;
            }

        } else if (!strcmp(stock, "AAPL")) {
            mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) APPLE. Que souhaitez-vous faire ?", qty_AAPL);
            mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34) Cours du jour : %.2f", price_AAPL[0]);
            mvwprintw(field, 3, 1, "");
            wscanw(field, "%s %d", &action, &qty);
            if (!strcmp(action, "SELL")) {
                qty_AAPL -= qty;
                cash += qty * price_AAPL[day];
            } else if (!strcmp(action, "BUY")) {
                qty_AAPL += qty;
                cash -= qty * price_AAPL[day];
            }

        } else if (!strcmp(stock, "GOOGL")) {
            mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) GOOGLE. Que souhaitez-vous faire ?", qty_GOOGL);
            mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34)");
            mvwprintw(field, 3, 1, "");
            wscanw(field, "%s %d", &action, &qty);
            if (!strcmp(action, "SELL")) {
                qty_GOOGL -= qty;
                cash += qty * price_GOOGL[day];
            } else if (!strcmp(action, "BUY")) {
                qty_GOOGL += qty;
                cash -= qty * price_GOOGL[day];
            }

        } else if (!strcmp(stock, "AMZN")) {
            mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) AMAZON. Que souhaitez-vous faire ?", qty_AMZN);
            mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34)");
            mvwprintw(field, 3, 1, "");
            wscanw(field, "%s %d", &action, &qty);
            update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                           price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);
            if (!strcmp(action, "SELL")) {
                qty_AMZN -= qty;
                cash += qty * price_AMZN[day];
            } else if (!strcmp(action, "BUY")) {
                qty_AMZN += qty;
                cash -= qty * price_AMZN[day];
            }

        } else if (!strcmp(stock, "FB")) {
            mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) FACEBOOK. Que souhaitez-vous faire ?", qty_FB);
            mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34)");
            mvwprintw(field, 3, 1, "");
            wscanw(field, "%s %d", &action, &qty);
            update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                           price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);
            if (!strcmp(action, "SELL")) {
                qty_FB -= qty;
                cash += qty * price_FB[day];
            } else if (!strcmp(action, "BUY")) {
                qty_FB += qty;
                cash -= qty * price_FB[day];
            }

        } else if (!strcmp(stock, "MSFT")) {
            mvwprintw(field, 1, 1, "Vous avez actuellement %d action(s) MICROSOFT. Que souhaitez-vous faire ?",
                      qty_MSFT);
            mvwprintw(field, 2, 1, "(Ex: BUY 17 / SELL 34)");
            mvwprintw(field, 3, 1, "");
            wscanw(field, "%s %d", &action, &qty);
            update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                           price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);
            if (!strcmp(action, "SELL")) {
                qty_MSFT -= qty;
                cash += qty * price_MSFT[day];
            } else if (!strcmp(action, "BUY")) {
                qty_MSFT += qty;
                cash -= qty * price_MSFT[day];
            }

        } else {
            // En cas d'entrée inconnue, une erreur s'affiche pour deux secondes
            mvwprintw(field, 4, 1, "ERREUR SAISIE INCONNUE : Vous ne pouvez pas trader cette action.");
            wrefresh(field);
            sleep(2);
            update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                           price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);
        }

        update_screens(field, score, cash, username, qty_AAPL, qty_AMZN, qty_FB, qty_GOOGL, qty_MSFT, day,
                       price_AAPL, price_AMZN, price_FB, price_GOOGL, price_MSFT);
    };

    // Fin du jeu
    endwin();

    roi = ((qty_AAPL * price_AAPL[TAILLE - 1]
            + qty_AMZN * price_AMZN[TAILLE - 1]
            + qty_FB * price_FB[TAILLE - 1]
            + qty_GOOGL * price_GOOGL[TAILLE - 1]
            + qty_MSFT * price_MSFT[TAILLE - 1]
            + cash
            - cash_init) / cash_init) * 100; //on multiplie par 100 pour l'afficher en %

    // Affiche les résultats et sauvegarde le score
    printf("Game Over\n\n");
    printf("Votre score final (ROI): %f %%\n", roi);
    printf("Bonne journée !\n");
    save_score("score.csv", username, roi);

    return 0;
}
