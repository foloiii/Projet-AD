//
//  main.cpp
//  AideALaDecision
//
//  Created by ROBERT Bastien on 14/04/2017.
//  Copyright © 2017 ROBERT Bastien. All rights reserved.
//

#include <iostream>
#include <sstream>
#include "decision.hpp"


using namespace std;

int main(int argc, const char * argv[]) {
    
    string response;
    int value;
    
    cout<< "--------------------------------\n--------------------------------\nAide a la decision - mini projet\n--------------------------------\n--------------------------------" << endl << endl << endl;
    
    do {
    
        cout<< "Nom du fichier (en relatif ou absolu) : ";
        cin>> response;
        
        Decision d(response);
        
        do {
        
            cout<< endl << "Quel methode voulez vous executer : " << endl << "1 : Methode naive" << endl << "2 : Reduction de domaines" << endl << "3 : Variable la plus contrainte" << endl << "4 : Coherence d arete"<<endl;
            cin>> response;
            
            istringstream iss(response);
            iss >> value;
            
            Final values;
            Node element;
            values.check = true;
            element.variable = -1;
            d.InitStatistics();
            
            switch (value) {
                case 1:
                    cout<< endl << endl << "-----Methode naive-----" <<endl;
                    values = d.naiveMethod(element, values);
                    d.showFinalState(values);
                    break;
                    
                case 2:
                    cout<< endl << endl << "-----Methode reduction de domaine-----" <<endl;
                    values = d.ReductionDomainMethod(element, values);
                    d.showFinalState(values);
                    break;
                    
                case 3:
                    cout<< endl << endl << "-----Methode variable la plus contrainte-----" <<endl;
                    values = d.AssignmentOptimizationMethod(element, values);
                    d.showFinalState(values);
                    break;
                    
                case 4:
                    cout<< endl << endl << "-----Coherence d arete-----" <<endl;
                    values = d.EdgeConsistencyMethod(element, values);
                    d.showFinalState(values);
                    break;
                    
                default:
                    cout<< endl << endl << "Choix inconnu" << endl;
                    break;
            }
            
            cout<< endl << endl << "Voulez-vous changer de methode de calcul ? (o/oui/n/non) : ";
            cin >> response;
            
        } while (response == "o" || response == "O" || response == "oui" || response == "OUI");
        
        
        cout<< endl << endl << "Voulez-vous charger un autre fichier ? (o/oui/n/non) : ";
        cin >> response;
        
    } while (response == "o" || response == "O" || response == "oui" || response == "OUI");
    
    return 0;
}
