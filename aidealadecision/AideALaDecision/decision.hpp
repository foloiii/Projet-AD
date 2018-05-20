#ifndef DECISION_H_INCLUDED
#define DECISION_H_INCLUDED

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>

using namespace std;

typedef struct Constraint {
    short type;
    int parameter;
    map< int, int > secondPart;
    vector< int > variables;
} Constraint;

typedef struct Node {
    int variable;
    int value;
} Node;

typedef struct Final {
    map< int, int > values;
    bool check;
} Final;




class Decision {
public:
    Decision();
    Decision(string filepath);

    void load(string const filepath);
    void addDomain(int variable, int value);
    void addConstraint(int constraint, short type, int variableDomain, int parameter = -1);
    void showInitialState();
    void showFinalState(Final values);
    void InitStatistics();
    string getTypeConstraint(int type);
    Final naiveMethod(Node element, Final values);
    bool checkConstraint(Final values, bool limit = false);
    Final ReductionDomainMethod(Node element, Final values);
    Final AssignmentOptimizationMethod(Node element, Final values);
    Final EdgeConsistencyMethod(Node element, Final values, vector< vector< int > > domains = vector< vector< int > >());
    vector< vector< int > > RemoveInconsistentValues(int edge, vector< vector< int > > domains);

private:
    int nbVariable;
    vector< vector< int > > domains;
    vector< Constraint > constraints;
    int edgeCreated;
    int arcIgnored;
};

#endif // DECISION_H_INCLUDED
