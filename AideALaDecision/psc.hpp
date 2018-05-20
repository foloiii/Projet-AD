#ifndef PSC_H_INCLUDED
#define PSC_H_INCLUDED

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>

using namespace std;

typedef struct Contrainte {
    short type;
    int parametre;
    vector< int > variables;
    map< int, int > deuxiemeParti;
} Contrainte;

typedef struct Noeu {
    int variable;
    int val;
} Noeu;

typedef struct Finale {
    map< int, int > valFin;
    bool test;
} Finale;


class PSC {
public:
    PSC();
    PSC(string filepath);

    void lecture(string const filepath);
    void addDomaine(int variable, int val);
    void addContrainte(int constraint, short type, int varDomaine, int parametre = -1);
    void affichageEtatIni();
    void afficherEtatFin(Finale valFin);
    void StatIni();
    string getTypeContrainte(int type);
    Finale MethodeNaif(Noeu element, Finale valFin);
    bool testContrainte(Finale valFin, bool limit = false);
    Finale MethodeReductionDomaine(Noeu element, Finale valFin);
    Finale MethodeOptimisation(Noeu element, Finale valFin);
    Finale MethodeCohe(Noeu element, Finale valFin, vector< vector< int > > vectDomaines = vector< vector< int > >());
    vector< vector< int > > RemoveInconsistentvalFin(int edge, vector< vector< int > > vectDomaines);

private:
    int nbVariable;
    vector< vector< int > > vectDomaines;
    vector< Contrainte > vectContrainte;
    int bordAdd;
    int arcOsef;
};

#endif

