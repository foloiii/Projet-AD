#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

int fOpenLE(string name);

int main()
{
    string name = "";
    int choix;
    cout << "\t -- Projet Aide a la decision -- \n\n1)Importer un Graph\n2)Creer un Graph" << endl;
    cin >> choix;
    switch(choix){
        case 1: // Importer un graph
            system("cls");
            cout << "-> Nom du fichier contenant le graphe (sans l'extension) :" << endl;
            cin >> name;
            fOpenLE(name);
            break;
        case 2: // Cr�er un graph

            break;
        default:

            break;
    }
    return 0;
}

int fOpenLE(string name){

     string mon_fichier = name + ".txt";  // je stocke dans la cha�ne mon_fichier le nom du fichier � ouvrir
     int nombre;
     cerr << " --> int fOpenLE(string name) \nOuverture de " + mon_fichier + " en lecture + ecriture ..." << endl;
     ifstream fichier(mon_fichier.c_str(), ios::in);
     if(fichier)  // si l'ouverture a r�ussi
     {
            cerr << "Ouverture reussie !\n" << endl;
          // instructions

            char caractere;  // notre variable o� sera stock� le caract�re
            //int nbr;
            //getline(fichier,nbr);
            while(caractere != 'e'){
                fichier.get(caractere);  // on lit un caract�re et on le stocke dans caractere


                cout << caractere;  // on l'affiche
            }
            cin >> caractere;
            //nombre = (int) caractere;
            //fichier >> nombre;

            //cout << "Le fichier contient " + nombre << endl;


            fichier.close();  // je referme le fichier
     }
     else  // sinon
            cerr << "Erreur � l'ouverture !" << endl;

     return 0;
}
