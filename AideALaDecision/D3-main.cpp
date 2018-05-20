#include <iostream>
#include <sstream>
#include "D3-psc.hpp"

using namespace std;

int main(int argc, const char * argv[]) {

    int val;
    string choix;

    do {
        cout<< "\t-- Aide a la decision --\n\t  -- Resolution PSC --\n\n";
        cout<< "Nom du fichier : ";
        cin>> choix;

        PSC d(choix);

        do {

            cout<< endl << "Menu :" << endl << "Etape 1 - Resolution triviale" << endl << "Etape 2 - Reduction des domaines de valeurs" << endl << "Etape 3 - Variable la plus contrainte" << endl << "Etape 4 - Coherence d'arete"<<endl;
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
                    cout<< endl << endl << "-> METHODE TRIVIALE" <<endl;
                    valFin = d.MethodeNaif(element, valFin);
                    d.afficherEtatFin(valFin);

                    break;

                case 2:
                    cout<< endl << endl << "-> REDUCTION DE DOMAINES DE VALEURS" <<endl;
                    valFin = d.MethodeReductionDomaine(element, valFin);
                    d.afficherEtatFin(valFin);
                    break;

                case 3:
                    cout<< endl << endl << "-> VARIABLE LA PLUS CONTRAINTE" <<endl;
                    valFin = d.MethodeOptimisation(element, valFin);
                    d.afficherEtatFin(valFin);
                    break;

                case 4:
                    cout<< endl << endl << "-> COHERENCE D'ARETE" <<endl;
                    valFin = d.MethodeCohe(element, valFin);
                    d.afficherEtatFin(valFin);
                    break;

                default:
                    cout<< endl << endl << "Choix incorrect" << endl;
                    break;
            }

            cout<< endl << endl << "Continuer ?";
            cin >> choix;

        } while (choix == "o" || choix == "O" || choix == "oui" || choix == "OUI");


        cout<< endl << endl << "Charger un autre fichier ?";
        cin >> choix;

    } while (choix == "o" || choix == "O" || choix == "oui" || choix == "OUI");

    return 0;
}
