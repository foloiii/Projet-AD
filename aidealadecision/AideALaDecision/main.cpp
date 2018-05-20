#include <iostream>
#include <sstream>
#include "decision.hpp"

using namespace std;

int main(int argc, const char * argv[]) {

    int val;
    string choix;

    do {

        cout<< "Nom du fichier : ";
        cin>> choix;

        Decision d(choix);

        do {

            cout<< endl << "Quel methode voulez vous executer : " << endl << "1 : Methode naive" << endl << "2 : Reduction de domaines" << endl << "3 : Variable la plus contrainte" << endl << "4 : Coherence d arete"<<endl;
            cin>> choix;

            istringstream iss(choix);
            iss >> val;

            Finale valFin;
            Noeu element;
            valFin.test = true;
            element.variable = -1;
            d.StatIni();

            switch (val) {
                case 1:
                    cout<< endl << endl << "-----Methode naive-----" <<endl;
                    valFin = d.MethodeNaif(element, valFin);
                    d.afficherEtatFin(valFin);
                    break;

                case 2:
                    cout<< endl << endl << "-----Methode reduction de domaine-----" <<endl;
                    valFin = d.MethodeReductionDomaine(element, valFin);
                    d.afficherEtatFin(valFin);
                    break;

                case 3:
                    cout<< endl << endl << "-----Methode variable la plus contrainte-----" <<endl;
                    valFin = d.MethodeOptimisation(element, valFin);
                    d.afficherEtatFin(valFin);
                    break;

                case 4:
                    cout<< endl << endl << "-----Coherence d arete-----" <<endl;
                    valFin = d.MethodeCohe(element, valFin);
                    d.afficherEtatFin(valFin);
                    break;

                default:
                    cout<< endl << endl << "Choix inconnu" << endl;
                    break;
            }

            cout<< endl << endl << "Voulez-vous changer de methode de calcul ? (o/oui/n/non) : ";
            cin >> choix;

        } while (choix == "o" || choix == "O" || choix == "oui" || choix == "OUI");


        cout<< endl << endl << "Voulez-vous charger un autre fichier ? (o/oui/n/non) : ";
        cin >> choix;

    } while (choix == "o" || choix == "O" || choix == "oui" || choix == "OUI");

    return 0;
}
