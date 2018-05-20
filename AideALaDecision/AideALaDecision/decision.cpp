#include "decision.hpp"
#include <fstream>

PSC::PSC() {
    this->nbVariable = 0;
    this->vectDomaines = vector< vector< int > > ();
    this->vectContrainte = vector< Contrainte >();
}

PSC::PSC(string filepath) {
    this->nbVariable = 0;
    this->vectDomaines = vector< vector< int > > ();
    this->vectContrainte = vector< Contrainte >();

    this->lecture(filepath);
    this->affichageEtatIni();
}

string PSC::getTypeContrainte(int type) {
    switch (type) {
        case 1:
            return "=";
        case 2:
            return "!=";
        case 3:
            return "<=";
        case 4:
            return "Sum(Xi) =";
        case 5:
            return "<";
        case 6:
            return ">";
        case 7:
            return "Sum(Xi) <";
        case 8:
            return ">=";
        case 9:
            return "Sum(Xi) = Yi * Ai";
        default:
            return "";
    }
}

void PSC::lecture(string const filepath) {
    ifstream file(filepath.c_str());
    string ligneActu;

    if (!file) {
        cout<<"Impossible de lire le fichier : "<<filepath<<endl<<endl;
        return;
    }

    file >> this->nbVariable;
    this->vectDomaines = vector< vector< int > > (this->nbVariable);
    getline(file, ligneActu);

    for (int i = 0; i < this->nbVariable; i++) {

        getline(file, ligneActu);
        istringstream iss(ligneActu);
        int j = 0, var = 0, cpt;

        while (iss >> cpt) {
            if (j == 0) {
                var = cpt;
            } else if (j != 1) {
                this->addDomaine(var, cpt);
            }
            j++;
        }
    }

    int nbConstraint = 0;

    while (!file.eof()) {
        getline(file, ligneActu);
        istringstream iss(ligneActu);
        int j = 0, type = 0, parametre = -1;
        int cpt = 0, cpt2 = -1;
        bool deuxiemeParti = false;

        while (iss >> cpt) {
            if (j == 0) {
                type = cpt;
            } else if ((type == 4 || type == 7) && j == 1) {
                parametre = cpt;
            } else if (type == 9 && cpt == -1) {
                deuxiemeParti = true;
            } else if (type == 9 && !deuxiemeParti) {
                this->addContrainte(nbConstraint, type, cpt, -1);
            } else if (type == 9 && deuxiemeParti) {
                if (cpt2 < 0) {
                    cpt2 = cpt;
                } else {
                    this->addContrainte(nbConstraint, type, cpt2, cpt);
                    cpt2 = -1;
                }
            } else if (cpt != -1) {
                this->addContrainte(nbConstraint, type, cpt, parametre);
            }
            j++;
        }
        nbConstraint++;
    }
}

void PSC::affichageEtatIni() {
    cout<< "\n\n(1) Variables" << endl << endl;


    cout<< this->nbVariable << " variables" << endl;
    for (int i = 0; i < this->nbVariable; i++) {
        cout<< "x" << i << ", ";
    }

    cout<< "\n\n(2) Domaines" << endl << endl;
    for (int i = 0; i < this->nbVariable; i++) {
        cout<< "x" << i << " = { ";

        for (unsigned int j = 0; j < this->vectDomaines[i].size() - 1; j++) {
            cout<< this->vectDomaines[i][j] << ", ";
        }
        cout<< this->vectDomaines[i][this->vectDomaines[i].size() - 1] << " }" << endl;
    }



    cout<< endl << "(3) Contraintes :" << endl << endl;

    for (unsigned int i = 0; i < this->vectContrainte.size(); i++) {
        cout<< i << " - Contrainte : " << this->getTypeContrainte(this->vectContrainte[i].type) << " - variables : { ";

        for (unsigned int j = 0; j < this->vectContrainte[i].variables.size() - 1; j++) {
            cout<< "x" << this->vectContrainte[i].variables[j] << ", ";
        }
        cout<< "x" << this->vectContrainte[i].variables[this->vectContrainte[i].variables.size() - 1] << " }" << endl;
    }
}

void PSC::afficherEtatFin(Finale valFin) {
    if (!valFin.test) {
        cout<< endl << "! - Pas de resultat selon les contraintes."<< endl;
    } else {
        cout<< endl << "Resultat :" << endl;
        for (unsigned int i = 0; i < valFin.valFin.size(); i++) {
            cout<< i << " = { " << valFin.valFin[i] << " }" << endl;
        }
    }

    cout<< endl << "Donnees: " << endl << this->bordAdd << " noeuds crees" << endl << this->arcOsef << " branches ignorees" << endl;
}


void PSC::addDomaine(int variable, int val){
    this->vectDomaines[variable - 1].push_back(val);
}

void PSC::addContrainte(int contrainte, short type, int varDomaine, int parametre){
    //We add the constraint structure containor of the domain if the contraint containor is too small
    if (this->vectContrainte.size() < (unsigned int)contrainte + 1) {
        Contrainte a;
        a.type = 0;
        a.variables = vector< int >();
        this->vectContrainte.push_back(a);
    }

    this->vectContrainte[contrainte].type = type;

    if (type == 9 && parametre != -1) {
        this->vectContrainte[contrainte].deuxiemeParti[varDomaine - 1] = parametre;
    } else {
        this->vectContrainte[contrainte].parametre = parametre;
        this->vectContrainte[contrainte].variables.push_back(varDomaine - 1);
    }
}


void PSC::StatIni() {
    this->bordAdd = 0;
    this->arcOsef = 0;
}




Finale PSC::MethodeNaif(Noeu element, Finale valFin) {

    //If it's a leaf we test the constraint, We know it's a leaf because the last variable is at the bottom of the tree
    if (element.variable == this->nbVariable - 1) {
        valFin.test = this->testContrainte(valFin);
        return valFin;
    }

    //We get the nextVariable
    const int nextVariable = element.variable + 1;

    //For each val of the domain of the variable, we create a Noeu and we make a recursive call, if the return of the Finale valFin is positive we return this couple of valFin because it's a solution
    for (unsigned int i = 0; i < this->vectDomaines[nextVariable].size(); i++) {
        Noeu enfant;
        Finale fin;

        enfant.variable = nextVariable;
        enfant.val = this->vectDomaines[nextVariable][i];
        fin = valFin;
        fin.valFin[nextVariable] = enfant.val;

        fin = MethodeNaif(enfant, fin);
        this->bordAdd++;

        if (fin.test) {
            return fin;
        }
    }

    //If we don't have already exit it's because there is no solution on this Noeu
    valFin.test = false;
    return valFin;
}


bool PSC::testContrainte(Finale valFin, bool limit) {

    // We test for every constraint on the valFin
    for (unsigned int i = 0; i < this->vectContrainte.size(); i++) {
        Contrainte contrainte = this->vectContrainte[i];
        int tmp, somme;
        bool bool1, bool2;

        // Depending of the type of constraint
        switch (contrainte.type) {

            // If it's an equal constraint, we test if the valFin are equals
            case 1:
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (limit && valFin.valFin.find(contrainte.variables[j]) == valFin.valFin.end()) {
                        continue;
                    }

                    for (unsigned int k = 0; k < contrainte.variables.size(); k++) {
                        // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                        if (limit && valFin.valFin.find(contrainte.variables[k]) == valFin.valFin.end()) {
                            continue;
                        }

                        if (j == k) {
                            continue;
                        }

                        if (valFin.valFin[contrainte.variables[j]] != valFin.valFin[contrainte.variables[k]]) {
                            return false;
                        }
                    }
                }
                break;

            // If it's an not equal constraint, we test if the valFin are not equals
            case 2:
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (limit && valFin.valFin.find(contrainte.variables[j]) == valFin.valFin.end()) {
                        continue;
                    }

                    for (unsigned int k = 0; k < contrainte.variables.size(); k++) {
                        // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                        if (limit && valFin.valFin.find(contrainte.variables[k]) == valFin.valFin.end()) {
                            continue;
                        }

                        if (j == k) {
                            continue;
                        }

                        if (valFin.valFin[contrainte.variables[j]] == valFin.valFin[contrainte.variables[k]]) {
                            return false;
                        }
                    }
                }
                break;

            // If it's an inferior constraint, we test if the valFin are inferior
            case 3:
                // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                if (!limit || (valFin.valFin.find(contrainte.variables[0]) != valFin.valFin.end() && valFin.valFin.find(contrainte.variables[1]) != valFin.valFin.end())) {
                    if (valFin.valFin[contrainte.variables[0]] > valFin.valFin[contrainte.variables[1]]) {
                        return false;
                    }
                }
                break;

            // Sum of all variables, we test if the total is equal to the parameter
            case 4:
                tmp = 0;
                bool1 = false;
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (limit && valFin.valFin.find(contrainte.variables[j]) == valFin.valFin.end()) {
                        bool1 = true;
                        continue;
                    }
                    tmp += valFin.valFin[contrainte.variables[j]];
                }

                if (!bool1) {
                    if (tmp != contrainte.parametre) {
                        return false;
                    }
                } else {
                    if (tmp > contrainte.parametre) {
                        return false;
                    }
                }

                break;

            // If it's a strict inferior constraint, we test if the valFin are inferior
            case 5:
                // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                if (!limit || (valFin.valFin.find(contrainte.variables[0]) != valFin.valFin.end() && valFin.valFin.find(contrainte.variables[1]) != valFin.valFin.end())) {
                    if (valFin.valFin[contrainte.variables[0]] >= valFin.valFin[contrainte.variables[1]]) {
                        return false;
                    }
                }
                break;

            // If it's a strict superior constraint, we test if the valFin are superior
            case 6:
                // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                if (!limit || (valFin.valFin.find(contrainte.variables[0]) != valFin.valFin.end() && valFin.valFin.find(contrainte.variables[1]) != valFin.valFin.end())) {
                    if (valFin.valFin[contrainte.variables[0]] <= valFin.valFin[contrainte.variables[1]]) {
                        return false;
                    }
                }
                break;

            // Sum of all variables, we test if the total is inferior to the parametre
            case 7:
                tmp = 0;
                bool1 = false;
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (limit && valFin.valFin.find(contrainte.variables[j]) == valFin.valFin.end()) {
                        bool1 = true;
                        continue;
                    }
                    tmp += valFin.valFin[contrainte.variables[j]];
                }

                if (!bool1) {
                    if (tmp >= contrainte.parametre) {
                        return false;
                    }
                } else {
                    if (tmp >= contrainte.parametre) {
                        return false;
                    }
                }

                break;

            // If it's a superior constraint, we test if the valFin are inferior
            case 8:
                // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                if (!limit || (valFin.valFin.find(contrainte.variables[0]) != valFin.valFin.end() && valFin.valFin.find(contrainte.variables[1]) != valFin.valFin.end())) {
                    if (valFin.valFin[contrainte.variables[0]] < valFin.valFin[contrainte.variables[1]]) {
                        return false;
                    }
                }
                break;


            // Sum of all variables, we test if the total is equal to the sum of the deuxiemeParti with a ponderation
            case 9:
                tmp = 0;
                somme = 0;
                bool1 = false;
                bool2 = false;
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (limit && valFin.valFin.find(contrainte.variables[j]) == valFin.valFin.end()) {
                        bool1 = true;
                        continue;
                    }
                    tmp += valFin.valFin[contrainte.variables[j]];
                }

                for (const auto &element : contrainte.deuxiemeParti) {
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (limit && valFin.valFin.find(element.first) == valFin.valFin.end()) {
                        bool2 = true;
                        continue;
                    }
                    somme += valFin.valFin[element.first] * element.second;

                }

                if (!bool1 && ! bool2) {
                    if (tmp != somme) {
                        return false;
                    }
                } else if (!bool2) {
                    if (tmp > somme) {
                        return false;
                    }
                }

                break;


            default:
                return false;
                break;
        }

    }

    return true;
}

Finale PSC::MethodeReductionDomaine(Noeu element, Finale valFin) {
    //If it's a leaf we test the constraint, We know it's a leaf because the last variable is at the bottom of the tree. And it's a solution because we pass all the reduction of domain
    if (element.variable == this->nbVariable - 1) {
        valFin.test = true;
        return valFin;
    }

    //We get the nextVariable
    const int nextVariable = element.variable + 1;
    Noeu child;
    Finale temp;
    child.variable = nextVariable;
    temp = valFin;

    //For each val of the domain of the variable, we create a Noeu and we test the constraint we can already test, if all the vectContrainte are satisfy we can make a recursive call
    for (unsigned int i = 0; i < this->vectDomaines[nextVariable].size(); i++) {

        child.val = this->vectDomaines[nextVariable][i];
        temp.valFin[nextVariable] = child.val;

        if (nextVariable == 0 || this->testContrainte(temp, true)) {
            temp = MethodeReductionDomaine(child, temp);
            this->bordAdd++;

            if (temp.test) {
                return temp;
            }
        } else {
            this->arcOsef++;
        }


    }

    //If we don't have already exit it's because there is no solution on this Noeu
    valFin.test = false;
    return valFin;
}

Finale PSC::MethodeOptimisation(Noeu element, Finale valFin) {
    //If it's a leaf we test the constraint, We know it's a leaf because the last variable is at the bottom of the tree. And it's a solution because we pass all the reduction of domain
    if ((unsigned int)valFin.valFin.size() == (unsigned int)this->nbVariable) {
        valFin.test = true;
        return valFin;
    }

    int min = -1;
    int nextVariable = 0;

    //We get the nextVariable
    for (int i = 0; i < this->nbVariable; i++) {
        if (valFin.valFin.find(i) == valFin.valFin.end() ) {
            if (min == -1 || (unsigned int)min > this->vectDomaines[i].size()) {
                min = (int) this->vectDomaines[i].size();
                nextVariable = i;
            }
        }
    }

    Noeu child;
    Finale temp;
    child.variable = nextVariable;
    temp = valFin;

    //For each val of the domain of the variable, we create a Noeu and we test the constraint we can already test, if all the vectContrainte are satisfy we can make a recursive call
    for (unsigned int i = 0; i < this->vectDomaines[nextVariable].size(); i++) {

        child.val = this->vectDomaines[nextVariable][i];
        temp.valFin[nextVariable] = child.val;

        if (this->testContrainte(temp, true)) {
            temp = MethodeOptimisation(child, temp);
            this->bordAdd++;

            if (temp.test) {
                return temp;
            }
        } else {
            this->arcOsef++;
        }


    }

    //If we don't have already exit it's because there is no solution on this Noeu
    valFin.test = false;
    return valFin;
}

Finale PSC::MethodeCohe(Noeu element, Finale valFin, vector< vector< int > > vectDomaines) {

    //If it's a leaf we test the constraint, We know it's a leaf because the last variable is at the bottom of the tree. And it's a solution because we pass all the reduction of domain
    if (valFin.valFin.size() == (unsigned int)this->nbVariable) {
        valFin.test = true;
        return valFin;
    }

    // If we don't have a vectDomaines it means that it's the first time the function is call
    if (vectDomaines.size() == 0) {
        vectDomaines = this->vectDomaines;
    }

    int min = -1;
    int nextVariable = 0;

    //We get the nextVariable
    for (int i = 0; i < this->nbVariable; i++) {

        if (vectDomaines[i].size() == 0) {
            valFin.test = false;
            return valFin;
        }

        if (valFin.valFin.find(i) == valFin.valFin.end() ) {
            if (min == -1 || (unsigned int)min > vectDomaines[i].size()) {
                min = (int) vectDomaines[i].size();
                nextVariable = i;
            }
        }
    }

    Noeu child;
    Finale temp;
    child.variable = nextVariable;
    temp = valFin;

    // We make a copy of the domain in the case we don't find the good valFin at the first shoot (to do a backtracking)
    vector< vector< int > > old = vectDomaines;

    //For each val of the domain of the variable, we create a Noeu and we test the constraint we can already test, if all the vectContrainte are satisfy we can make a recursive call
    for (unsigned int i = 0; i < old[nextVariable].size(); i++) {

        child.val = old[nextVariable][i];
        temp.valFin[nextVariable] = child.val;

        // We reduce the domain of the variable at only the val we're testing
        vectDomaines[nextVariable] = vector< int >();
        vectDomaines[nextVariable].push_back(old[nextVariable][i]);

        // We reduce the domain of all the neighboorhood of the variable, depending on the vectContrainte
        vectDomaines = this->RemoveInconsistentvalFin(nextVariable, vectDomaines);

        // We test all the vectContrainte (especially sum that are not tested in the removeinconsistenvalFin)
        if (this->testContrainte(temp, true)) {
            temp = MethodeCohe(child, temp, vectDomaines);
            this->bordAdd++;

            if (temp.test) {
                return temp;
            }
        } else {
            this->arcOsef++;
        }

        // We do backtracking in case the vectContrainte in the sub-tree was not fullfill
        vectDomaines = old;


    }

    //If we don't have already exit it's because there is no solution on this Noeu
    valFin.test = false;
    return valFin;
}

vector< vector< int > > PSC::RemoveInconsistentvalFin(int edge, vector< vector< int > > vectDomaines) {

    vector< int > fifo;
    fifo.push_back(edge);

    while (!fifo.empty()) {


        // We test for every constraint on the valFin
        for (unsigned int i = 0; i < this->vectContrainte.size(); i++) {
            Contrainte contrainte = this->vectContrainte[i];
            int current = fifo.front();
            int tmp, min, minCurrent;

            // If the constraint doesn't include the edge we're testing we pass this constraint
            if (find(contrainte.variables.begin(), contrainte.variables.end(), current) == contrainte.variables.end()) {
                continue;
            }

            // Depending of the type of constraint
            switch (contrainte.type) {

                // If it's an equal constraint
                case 1:
                    // We're testin for all the vectContrainte of variables
                    for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                        for (unsigned int k = 0; k < contrainte.variables.size(); k++) {

                            // We test that one variable is the current variable
                            if (j == k || (contrainte.variables[j] != current && contrainte.variables[k] != current)) {
                                continue;
                            }

                            // We test for the entire domain of each
                            for (unsigned int l = 0; l < vectDomaines[contrainte.variables[j]].size(); l++) {
                                for (unsigned int m = 0; m < vectDomaines[contrainte.variables[k]].size(); m++) {

                                    // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                    if (vectDomaines[contrainte.variables[j]][l] != vectDomaines[contrainte.variables[k]][m]) {

                                        if (contrainte.variables[j] == current) {
                                            vectDomaines[contrainte.variables[k]].erase(vectDomaines[contrainte.variables[k]].begin() + m);
                                            m--;

                                            if (find(fifo.begin(), fifo.end(), contrainte.variables[k]) == fifo.end()) {
                                                fifo.push_back(contrainte.variables[k]);
                                            }
                                        } else {
                                            vectDomaines[contrainte.variables[j]].erase(vectDomaines[contrainte.variables[j]].begin() + l);
                                            l--;

                                            if (find(fifo.begin(), fifo.end(), contrainte.variables[j]) == fifo.end()) {
                                                fifo.push_back(contrainte.variables[j]);
                                            }
                                        }

                                    }

                                }
                            }
                        }
                    }
                    break;


                // If it's a not equal constraint
                case 2:
                    // We're testin for all the vectContrainte of variables
                    for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                        for (unsigned int k = 0; k < contrainte.variables.size(); k++) {

                            // We test that one variable is the current variable
                            if (j == k || (contrainte.variables[j] != current && contrainte.variables[k] != current)) {
                                continue;
                            }

                            // We test for the entire domain of each
                            for (unsigned int l = 0; l < vectDomaines[contrainte.variables[j]].size(); l++) {
                                for (unsigned int m = 0; m < vectDomaines[contrainte.variables[k]].size(); m++) {

                                    // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                    if (vectDomaines[contrainte.variables[j]][l] == vectDomaines[contrainte.variables[k]][m]) {

                                        if (contrainte.variables[j] == current) {
                                            vectDomaines[contrainte.variables[k]].erase(vectDomaines[contrainte.variables[k]].begin() + m);
                                            m--;

                                            if (find(fifo.begin(), fifo.end(), contrainte.variables[k]) == fifo.end()) {
                                                fifo.push_back(contrainte.variables[k]);
                                            }
                                        } else {
                                            vectDomaines[contrainte.variables[j]].erase(vectDomaines[contrainte.variables[j]].begin() + l);
                                            l--;

                                            if (find(fifo.begin(), fifo.end(), contrainte.variables[j]) == fifo.end()) {
                                                fifo.push_back(contrainte.variables[j]);
                                            }
                                        }

                                    }

                                }
                            }
                        }
                    }
                    break;

                // If it's an inferior constraint
                case 3:
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (contrainte.variables[0] == current || contrainte.variables[1] == current) {

                        // We test for the entire domain of each variable
                        for (unsigned int l = 0; l < vectDomaines[contrainte.variables[0]].size(); l++) {
                            for (unsigned int m = 0; m < vectDomaines[contrainte.variables[1]].size(); m++) {

                                // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                if (vectDomaines[contrainte.variables[0]][l] > vectDomaines[contrainte.variables[1]][m]) {

                                    if (contrainte.variables[0] == current) {
                                        vectDomaines[contrainte.variables[1]].erase(vectDomaines[contrainte.variables[1]].begin() + m);
                                        m--;

                                        if (find(fifo.begin(), fifo.end(), contrainte.variables[1]) == fifo.end()) {
                                            fifo.push_back(contrainte.variables[1]);
                                        }
                                    } else {
                                        vectDomaines[contrainte.variables[0]].erase(vectDomaines[contrainte.variables[0]].begin() + l);
                                        l--;

                                        if (find(fifo.begin(), fifo.end(), contrainte.variables[0]) == fifo.end()) {
                                            fifo.push_back(contrainte.variables[0]);
                                        }
                                    }

                                }

                            }
                        }

                    }
                    break;

                // If it's a strict inferior constraint
                case 5:
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (contrainte.variables[0] == current || contrainte.variables[1] == current) {

                        // We test for the entire domain of each variable
                        for (unsigned int l = 0; l < vectDomaines[contrainte.variables[0]].size(); l++) {
                            for (unsigned int m = 0; m < vectDomaines[contrainte.variables[1]].size(); m++) {

                                // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                if (vectDomaines[contrainte.variables[0]][l] >= vectDomaines[contrainte.variables[1]][m]) {

                                    if (contrainte.variables[0] == current) {
                                        vectDomaines[contrainte.variables[1]].erase(vectDomaines[contrainte.variables[1]].begin() + m);
                                        m--;

                                        if (find(fifo.begin(), fifo.end(), contrainte.variables[1]) == fifo.end()) {
                                            fifo.push_back(contrainte.variables[1]);
                                        }
                                    } else {
                                        vectDomaines[contrainte.variables[0]].erase(vectDomaines[contrainte.variables[0]].begin() + l);
                                        l--;

                                        if (find(fifo.begin(), fifo.end(), contrainte.variables[0]) == fifo.end()) {
                                            fifo.push_back(contrainte.variables[0]);
                                        }
                                    }

                                }

                            }
                        }

                    }
                    break;

                // If it's a strict superior constraint
                case 6:
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (contrainte.variables[0] == current || contrainte.variables[1] == current) {

                        // We test for the entire domain of each variable
                        for (unsigned int l = 0; l < vectDomaines[contrainte.variables[0]].size(); l++) {
                            for (unsigned int m = 0; m < vectDomaines[contrainte.variables[1]].size(); m++) {

                                // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                if (vectDomaines[contrainte.variables[0]][l] <= vectDomaines[contrainte.variables[1]][m]) {

                                    if (contrainte.variables[0] == current) {
                                        vectDomaines[contrainte.variables[1]].erase(vectDomaines[contrainte.variables[1]].begin() + m);
                                        m--;

                                        if (find(fifo.begin(), fifo.end(), contrainte.variables[1]) == fifo.end()) {
                                            fifo.push_back(contrainte.variables[1]);
                                        }
                                    } else {
                                        vectDomaines[contrainte.variables[0]].erase(vectDomaines[contrainte.variables[0]].begin() + l);
                                        l--;

                                        if (find(fifo.begin(), fifo.end(), contrainte.variables[0]) == fifo.end()) {
                                            fifo.push_back(contrainte.variables[0]);
                                        }
                                    }

                                }

                            }
                        }

                    }
                    break;


                // If it's a inferior sum constraint
                case 7:
                    tmp = 0;
                    minCurrent = -1;

                    for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                        min = -1;
                        for (unsigned int k = 0; k < vectDomaines[contrainte.variables[j]].size(); k++) {
                            if (vectDomaines[contrainte.variables[j]][k] > min) {
                                min = vectDomaines[contrainte.variables[j]][k];
                                if (contrainte.variables[j] == current) {
                                    minCurrent = k;
                                }
                            }
                        }
                        tmp += min;
                    }

                    if (minCurrent != -1 && tmp > contrainte.parametre) {
                         vectDomaines[current].erase(vectDomaines[current].begin() + minCurrent);
                    }

                    break;




                // If it's a superior constraint
                case 8:
                    // In the case we do a reduction of domain, not all valFin are initialized so we need to skip those which are not initialize
                    if (contrainte.variables[0] == current || contrainte.variables[1] == current) {

                        // We test for the entire domain of each variable
                        for (unsigned int l = 0; l < vectDomaines[contrainte.variables[0]].size(); l++) {
                            for (unsigned int m = 0; m < vectDomaines[contrainte.variables[1]].size(); m++) {

                                // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                if (vectDomaines[contrainte.variables[0]][l] < vectDomaines[contrainte.variables[1]][m]) {

                                    if (contrainte.variables[0] == current) {
                                        vectDomaines[contrainte.variables[1]].erase(vectDomaines[contrainte.variables[1]].begin() + m);
                                        m--;

                                        if (find(fifo.begin(), fifo.end(), contrainte.variables[1]) == fifo.end()) {
                                            fifo.push_back(contrainte.variables[1]);
                                        }
                                    } else {
                                        vectDomaines[contrainte.variables[0]].erase(vectDomaines[contrainte.variables[0]].begin() + l);
                                        l--;

                                        if (find(fifo.begin(), fifo.end(), contrainte.variables[0]) == fifo.end()) {
                                            fifo.push_back(contrainte.variables[0]);
                                        }
                                    }

                                }

                            }
                        }

                    }
                    break;



                default:
                    break;
            }

        }

        fifo.pop_back();
    }

    return vectDomaines;
}


