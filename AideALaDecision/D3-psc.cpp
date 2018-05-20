#include "D3-psc.hpp"
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
            return "Somme(Xi) <";
        case 8:
            return ">=";
        case 9:
            return "Somme(Xi) = Ai * Bi";
        default:
            return "";
    }
}

//fonction de lecture des fichier .txt
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

//affichage du PSC non résolu
void PSC::affichageEtatIni() {
    cout<< "\n\n------ (1) Variables" << endl << endl;


    cout<< this->nbVariable << " variables" << endl;
    for (int i = 0; i < this->nbVariable; i++) {
        cout<< "x" << i << ", ";
    }

    cout<< "\n\n------ (2) Domaines" << endl << endl;
    for (int i = 0; i < this->nbVariable; i++) {
        cout<< "x" << i << " = { ";

        for (unsigned int j = 0; j < this->vectDomaines[i].size() - 1; j++) {
            cout<< this->vectDomaines[i][j] << ", ";
        }
        cout<< this->vectDomaines[i][this->vectDomaines[i].size() - 1] << " }" << endl;
    }



    cout<< endl << "------ (3) Contraintes :" << endl << endl;

    for (unsigned int i = 0; i < this->vectContrainte.size(); i++) {
        cout<< i << " - Contrainte : " << this->getTypeContrainte(this->vectContrainte[i].type) << " - variables : { ";

        for (unsigned int j = 0; j < this->vectContrainte[i].variables.size() - 1; j++) {
            cout<< "x" << this->vectContrainte[i].variables[j] << ", ";
        }
        cout<< "x" << this->vectContrainte[i].variables[this->vectContrainte[i].variables.size() - 1] << " }" << endl;
    }
}

//affichage du PSC résolu ou non
void PSC::afficherEtatFin(Finale valFin) {
    if (!valFin.test) {
        cout<< endl << "! - Pas de resultat selon les contraintes."<< endl;
    } else {
        cout<< endl << "Resultat :" << endl;
        for (unsigned int i = 0; i < valFin.valFin.size(); i++) {
            cout<<"x" <<i << " = " << valFin.valFin[i] << endl;
        }
    }

    cout<< endl << "Donnees: " << endl << this->bordAdd << " noeuds crees" << endl << this->arcOsef << " branches ignorees" << endl;
}

//ajout de domaine
void PSC::addDomaine(int variable, int val){
    this->vectDomaines[variable - 1].push_back(val);
}

//ajout de contrainte
void PSC::addContrainte(int contrainte, short type, int varDomaine, int parametre){
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

//initialise les données
void PSC::StatIni() {
    this->bordAdd = 0;
    this->arcOsef = 0;
}

//méthode naive
Finale PSC::MethodeNaif(Noeu element, Finale valFin) {

    if (element.variable == this->nbVariable - 1) {
        valFin.test = this->testContrainte(valFin);
        return valFin;
    }

    //on va a la prochaine variable
    const int nextVariable = element.variable + 1;

    //pour chaque valeur du domaine on crée un noeud et on fait un appel recursif, si le return du dernier Valfin est positif on a une solution que l'on renvoie
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
    //Si ca n'a pas quitté avant alors il n'y a pas de solution sur ce noeud
    valFin.test = false;
    return valFin;
}


bool PSC::testContrainte(Finale valFin, bool limit) {

    for (unsigned int i = 0; i < this->vectContrainte.size(); i++) {
        Contrainte contrainte = this->vectContrainte[i];
        int tmp, somme;
        bool bool1, bool2;

        switch (contrainte.type) {

            case 1:
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                    if (limit && valFin.valFin.find(contrainte.variables[j]) == valFin.valFin.end()) {
                        continue;
                    }

                    for (unsigned int k = 0; k < contrainte.variables.size(); k++) {
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

            case 2:
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                    if (limit && valFin.valFin.find(contrainte.variables[j]) == valFin.valFin.end()) {
                        continue;
                    }

                    for (unsigned int k = 0; k < contrainte.variables.size(); k++) {
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

            case 3:
                if (!limit || (valFin.valFin.find(contrainte.variables[0]) != valFin.valFin.end() && valFin.valFin.find(contrainte.variables[1]) != valFin.valFin.end())) {
                    if (valFin.valFin[contrainte.variables[0]] > valFin.valFin[contrainte.variables[1]]) {
                        return false;
                    }
                }
                break;

            case 4:
                tmp = 0;
                bool1 = false;
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
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

            case 5:
                if (!limit || (valFin.valFin.find(contrainte.variables[0]) != valFin.valFin.end() && valFin.valFin.find(contrainte.variables[1]) != valFin.valFin.end())) {
                    if (valFin.valFin[contrainte.variables[0]] >= valFin.valFin[contrainte.variables[1]]) {
                        return false;
                    }
                }
                break;

            case 6:
                if (!limit || (valFin.valFin.find(contrainte.variables[0]) != valFin.valFin.end() && valFin.valFin.find(contrainte.variables[1]) != valFin.valFin.end())) {
                    if (valFin.valFin[contrainte.variables[0]] <= valFin.valFin[contrainte.variables[1]]) {
                        return false;
                    }
                }
                break;

            case 7:
                tmp = 0;
                bool1 = false;
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
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

            case 8:
                if (!limit || (valFin.valFin.find(contrainte.variables[0]) != valFin.valFin.end() && valFin.valFin.find(contrainte.variables[1]) != valFin.valFin.end())) {
                    if (valFin.valFin[contrainte.variables[0]] < valFin.valFin[contrainte.variables[1]]) {
                        return false;
                    }
                }
                break;


            case 9:
                tmp = 0;
                somme = 0;
                bool1 = false;
                bool2 = false;
                for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                    if (limit && valFin.valFin.find(contrainte.variables[j]) == valFin.valFin.end()) {
                        bool1 = true;
                        continue;
                    }
                    tmp += valFin.valFin[contrainte.variables[j]];
                }

                for (const auto &element : contrainte.deuxiemeParti) {
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

//methode de reduction de domaine
Finale PSC::MethodeReductionDomaine(Noeu element, Finale valFin) {
    if (element.variable == this->nbVariable - 1) {
        valFin.test = true;
        return valFin;
    }

    const int nextVariable = element.variable + 1;
    Noeu child;
    Finale temp;
    child.variable = nextVariable;
    temp = valFin;

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

    valFin.test = false;
    return valFin;
}

//methode de variable la plus contrante
Finale PSC::MethodeOptimisation(Noeu element, Finale valFin) {
    if ((unsigned int)valFin.valFin.size() == (unsigned int)this->nbVariable) {
        valFin.test = true;
        return valFin;
    }

    int min = -1;
    int nextVariable = 0;

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

    valFin.test = false;
    return valFin;
}

//Coherence d'arete
Finale PSC::MethodeCohe(Noeu element, Finale valFin, vector< vector< int > > vectDomaines) {

    if (valFin.valFin.size() == (unsigned int)this->nbVariable) {
        valFin.test = true;
        return valFin;
    }

    if (vectDomaines.size() == 0) {
        vectDomaines = this->vectDomaines;
    }

    int min = -1;
    int nextVariable = 0;

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

    vector< vector< int > > old = vectDomaines;

    for (unsigned int i = 0; i < old[nextVariable].size(); i++) {

        child.val = old[nextVariable][i];
        temp.valFin[nextVariable] = child.val;

        vectDomaines[nextVariable] = vector< int >();
        vectDomaines[nextVariable].push_back(old[nextVariable][i]);

        vectDomaines = this->SuppInc(nextVariable, vectDomaines);

        if (this->testContrainte(temp, true)) {
            temp = MethodeCohe(child, temp, vectDomaines);
            this->bordAdd++;

            if (temp.test) {
                return temp;
            }
        } else {
            this->arcOsef++;
        }

        vectDomaines = old;


    }

    valFin.test = false;
    return valFin;
}

vector< vector< int > > PSC::SuppInc(int edge, vector< vector< int > > vectDomaines) {

    vector< int > fifo;
    fifo.push_back(edge);

    while (!fifo.empty()) {


        for (unsigned int i = 0; i < this->vectContrainte.size(); i++) {
            Contrainte contrainte = this->vectContrainte[i];
            int current = fifo.front();
            int tmp, min, minCurrent;

            if (find(contrainte.variables.begin(), contrainte.variables.end(), current) == contrainte.variables.end()) {
                continue;
            }

            switch (contrainte.type) {

                case 1:
                    for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                        for (unsigned int k = 0; k < contrainte.variables.size(); k++) {

                            if (j == k || (contrainte.variables[j] != current && contrainte.variables[k] != current)) {
                                continue;
                            }

                            for (unsigned int l = 0; l < vectDomaines[contrainte.variables[j]].size(); l++) {
                                for (unsigned int m = 0; m < vectDomaines[contrainte.variables[k]].size(); m++) {

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


                case 2:
                    for (unsigned int j = 0; j < contrainte.variables.size(); j++) {
                        for (unsigned int k = 0; k < contrainte.variables.size(); k++) {

                            if (j == k || (contrainte.variables[j] != current && contrainte.variables[k] != current)) {
                                continue;
                            }

                            for (unsigned int l = 0; l < vectDomaines[contrainte.variables[j]].size(); l++) {
                                for (unsigned int m = 0; m < vectDomaines[contrainte.variables[k]].size(); m++) {

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

                case 3:
                    if (contrainte.variables[0] == current || contrainte.variables[1] == current) {

                        for (unsigned int l = 0; l < vectDomaines[contrainte.variables[0]].size(); l++) {
                            for (unsigned int m = 0; m < vectDomaines[contrainte.variables[1]].size(); m++) {

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

                case 5:
                    if (contrainte.variables[0] == current || contrainte.variables[1] == current) {

                        for (unsigned int l = 0; l < vectDomaines[contrainte.variables[0]].size(); l++) {
                            for (unsigned int m = 0; m < vectDomaines[contrainte.variables[1]].size(); m++) {

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

                case 6:
                    if (contrainte.variables[0] == current || contrainte.variables[1] == current) {

                        for (unsigned int l = 0; l < vectDomaines[contrainte.variables[0]].size(); l++) {
                            for (unsigned int m = 0; m < vectDomaines[contrainte.variables[1]].size(); m++) {

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


                case 8:
                    if (contrainte.variables[0] == current || contrainte.variables[1] == current) {

                        for (unsigned int l = 0; l < vectDomaines[contrainte.variables[0]].size(); l++) {
                            for (unsigned int m = 0; m < vectDomaines[contrainte.variables[1]].size(); m++) {

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
