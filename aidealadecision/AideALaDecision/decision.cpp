#include "decision.hpp"
#include <fstream>

Decision::Decision() {
    this->nbVariable = 0;
    this->domains = vector< vector< int > > ();
    this->constraints = vector< Constraint >();
}

Decision::Decision(string filepath) {
    this->nbVariable = 0;
    this->domains = vector< vector< int > > ();
    this->constraints = vector< Constraint >();

    this->load(filepath);
    this->showInitialState();
}

void Decision::load(string const filepath) {
    ifstream file(filepath.c_str());
    string line;

    if (!file) {
        cout<<"Impossible de lire le fichier : "<<filepath<<endl<<endl;
        return;
    }

    file >> this->nbVariable;
    this->domains = vector< vector< int > > (this->nbVariable);
    getline(file, line);

    // Saving of variables and their domain
    for (int i = 0; i < this->nbVariable; i++) {

        getline(file, line);
        istringstream iss(line);
        int j = 0, variable = 0, tmp;

        while (iss >> tmp) {
            if (j == 0) {
                variable = tmp;
            } else if (j != 1) {
                this->addDomain(variable, tmp);
            }
            j++;
        }
    }

    // We save each constraint
    int nbConstraint = 0;

    while (!file.eof()) {
        getline(file, line);
        istringstream iss(line);
        int j = 0, type = 0, parameter = -1, tmp, tmp2 = -1;
        bool secondPart = false;

        while (iss >> tmp) {
            if (j == 0) {
                type = tmp;
            } else if ((type == 4 || type == 7) && j == 1) {
                parameter = tmp;
            } else if (type == 9 && tmp == -1) {
                secondPart = true;
            } else if (type == 9 && !secondPart) {
                this->addConstraint(nbConstraint, type, tmp, -1);
            } else if (type == 9 && secondPart) {
                if (tmp2 < 0) {
                    tmp2 = tmp;
                } else {
                    this->addConstraint(nbConstraint, type, tmp2, tmp);
                    tmp2 = -1;
                }
            } else if (tmp != -1) {
                this->addConstraint(nbConstraint, type, tmp, parameter);
            }
            j++;
        }
        nbConstraint++;
    }
}

void Decision::addDomain(int variable, int value){
    this->domains[variable - 1].push_back(value);
}

void Decision::addConstraint(int constraint, short type, int variableDomain, int parameter){
    //We add the constraint structure containor of the domain if the contraint containor is too small
    if (this->constraints.size() < constraint + 1) {
        Constraint a;
        a.type = 0;
        a.variables = vector< int >();
        this->constraints.push_back(a);
    }

    this->constraints[constraint].type = type;
    
    if (type == 9 && parameter != -1) {
        this->constraints[constraint].secondPart[variableDomain - 1] = parameter;
    } else {
        this->constraints[constraint].parameter = parameter;
        this->constraints[constraint].variables.push_back(variableDomain - 1);
    }
}

void Decision::showInitialState() {
    cout<< "Variables / Valeurs possibles :" << endl << endl;

    for (int i = 0; i < this->nbVariable; i++) {
        cout<< i << " = { ";

        for (int j = 0; j < this->domains[i].size() - 1; j++) {
            cout<< this->domains[i][j] << ", ";
        }
        cout<< this->domains[i][this->domains[i].size() - 1] << " }" << endl;
    }

    cout<< endl << "Contraintes :" << endl << endl;

    for (int i = 0; i < this->constraints.size(); i++) {
        cout<< i << " type : " << this->getTypeConstraint(this->constraints[i].type) << " variables : { ";

        for (int j = 0; j < this->constraints[i].variables.size() - 1; j++) {
            cout<< "x" << this->constraints[i].variables[j] << ", ";
        }
        cout<< "x" << this->constraints[i].variables[this->constraints[i].variables.size() - 1] << " }" << endl;
    }
}

void Decision::showFinalState(Final values) {
    if (!values.check) {
        cout<< endl << "Aucune solution aux contraintes posees !"<< endl;
    } else {
        cout<< endl << "Valeurs finales qui correspondent aux contraintes :" << endl;
        for (int i = 0; i < values.values.size(); i++) {
            cout<< i << " = { " << values.values[i] << " }" << endl;
        }
    }

    cout<< endl << "Statistiques : " << endl << this->edgeCreated << " noeuds crees" << endl << this->arcIgnored << " branches ignorees" << endl;
}

void Decision::InitStatistics() {
    this->edgeCreated = 0;
    this->arcIgnored = 0;
}

string Decision::getTypeConstraint(int type) {
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


Final Decision::naiveMethod(Node element, Final values) {

    //If it's a leaf we check the constraint, We know it's a leaf because the last variable is at the bottom of the tree
    if (element.variable == this->nbVariable - 1) {
        values.check = this->checkConstraint(values);
        return values;
    }

    //We get the nextVariable
    const int nextVariable = element.variable + 1;

    //For each value of the domain of the variable, we create a node and we make a recursive call, if the return of the Final values is positive we return this couple of values because it's a solution
    for (int i = 0; i < this->domains[nextVariable].size(); i++) {
        Node child;
        Final temp;

        child.variable = nextVariable;
        child.value = this->domains[nextVariable][i];
        temp = values;
        temp.values[nextVariable] = child.value;

        temp = naiveMethod(child, temp);
        this->edgeCreated++;

        if (temp.check) {
            return temp;
        }
    }

    //If we don't have already exit it's because there is no solution on this node
    values.check = false;
    return values;
}


bool Decision::checkConstraint(Final values, bool limit) {

    // We check for every constraint on the values
    for (int i = 0; i < this->constraints.size(); i++) {
        Constraint constraint = this->constraints[i];
        int tmp, sum;
        bool skip, skip2;

        // Depending of the type of constraint
        switch (constraint.type) {

            // If it's an equal constraint, we check if the values are equals
            case 1:
                for (int j = 0; j < constraint.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (limit && values.values.find(constraint.variables[j]) == values.values.end()) {
                        continue;
                    }

                    for (int k = 0; k < constraint.variables.size(); k++) {
                        // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                        if (limit && values.values.find(constraint.variables[k]) == values.values.end()) {
                            continue;
                        }

                        if (j == k) {
                            continue;
                        }

                        if (values.values[constraint.variables[j]] != values.values[constraint.variables[k]]) {
                            return false;
                        }
                    }
                }
                break;

            // If it's an not equal constraint, we check if the values are not equals
            case 2:
                for (int j = 0; j < constraint.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (limit && values.values.find(constraint.variables[j]) == values.values.end()) {
                        continue;
                    }

                    for (int k = 0; k < constraint.variables.size(); k++) {
                        // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                        if (limit && values.values.find(constraint.variables[k]) == values.values.end()) {
                            continue;
                        }

                        if (j == k) {
                            continue;
                        }

                        if (values.values[constraint.variables[j]] == values.values[constraint.variables[k]]) {
                            return false;
                        }
                    }
                }
                break;

            // If it's an inferior constraint, we check if the values are inferior
            case 3:
                // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                if (!limit || (values.values.find(constraint.variables[0]) != values.values.end() && values.values.find(constraint.variables[1]) != values.values.end())) {
                    if (values.values[constraint.variables[0]] > values.values[constraint.variables[1]]) {
                        return false;
                    }
                }
                break;

            // Sum of all variables, we check if the total is equal to the parameter
            case 4:
                tmp = 0;
                skip = false;
                for (int j = 0; j < constraint.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (limit && values.values.find(constraint.variables[j]) == values.values.end()) {
                        skip = true;
                        continue;
                    }
                    tmp += values.values[constraint.variables[j]];
                }

                if (!skip) {
                    if (tmp != constraint.parameter) {
                        return false;
                    }
                } else {
                    if (tmp > constraint.parameter) {
                        return false;
                    }
                }

                break;

            // If it's a strict inferior constraint, we check if the values are inferior
            case 5:
                // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                if (!limit || (values.values.find(constraint.variables[0]) != values.values.end() && values.values.find(constraint.variables[1]) != values.values.end())) {
                    if (values.values[constraint.variables[0]] >= values.values[constraint.variables[1]]) {
                        return false;
                    }
                }
                break;

            // If it's a strict superior constraint, we check if the values are superior
            case 6:
                // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                if (!limit || (values.values.find(constraint.variables[0]) != values.values.end() && values.values.find(constraint.variables[1]) != values.values.end())) {
                    if (values.values[constraint.variables[0]] <= values.values[constraint.variables[1]]) {
                        return false;
                    }
                }
                break;

            // Sum of all variables, we check if the total is inferior to the parameter
            case 7:
                tmp = 0;
                skip = false;
                for (int j = 0; j < constraint.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (limit && values.values.find(constraint.variables[j]) == values.values.end()) {
                        skip = true;
                        continue;
                    }
                    tmp += values.values[constraint.variables[j]];
                }

                if (!skip) {
                    if (tmp >= constraint.parameter) {
                        return false;
                    }
                } else {
                    if (tmp >= constraint.parameter) {
                        return false;
                    }
                }

                break;
                
            // If it's a superior constraint, we check if the values are inferior
            case 8:
                // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                if (!limit || (values.values.find(constraint.variables[0]) != values.values.end() && values.values.find(constraint.variables[1]) != values.values.end())) {
                    if (values.values[constraint.variables[0]] < values.values[constraint.variables[1]]) {
                        return false;
                    }
                }
                break;
                
                
            // Sum of all variables, we check if the total is equal to the sum of the secondPart with a ponderation
            case 9:
                tmp = 0;
                sum = 0;
                skip = false;
                skip2 = false;
                for (int j = 0; j < constraint.variables.size(); j++) {
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (limit && values.values.find(constraint.variables[j]) == values.values.end()) {
                        skip = true;
                        continue;
                    }
                    tmp += values.values[constraint.variables[j]];
                }
                
                for (const auto &element : constraint.secondPart) {
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (limit && values.values.find(element.first) == values.values.end()) {
                        skip2 = true;
                        continue;
                    }
                    sum += values.values[element.first] * element.second;

                }
                
                if (!skip && ! skip2) {
                    if (tmp != sum) {
                        return false;
                    }
                } else if (!skip2) {
                    if (tmp > sum) {
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

Final Decision::ReductionDomainMethod(Node element, Final values) {
    //If it's a leaf we check the constraint, We know it's a leaf because the last variable is at the bottom of the tree. And it's a solution because we pass all the reduction of domain
    if (element.variable == this->nbVariable - 1) {
        values.check = true;
        return values;
    }

    //We get the nextVariable
    const int nextVariable = element.variable + 1;
    Node child;
    Final temp;
    child.variable = nextVariable;
    temp = values;

    //For each value of the domain of the variable, we create a node and we check the constraint we can already test, if all the constraints are satisfy we can make a recursive call
    for (int i = 0; i < this->domains[nextVariable].size(); i++) {

        child.value = this->domains[nextVariable][i];
        temp.values[nextVariable] = child.value;

        if (nextVariable == 0 || this->checkConstraint(temp, true)) {
            temp = ReductionDomainMethod(child, temp);
            this->edgeCreated++;

            if (temp.check) {
                return temp;
            }
        } else {
            this->arcIgnored++;
        }


    }

    //If we don't have already exit it's because there is no solution on this node
    values.check = false;
    return values;
}

Final Decision::AssignmentOptimizationMethod(Node element, Final values) {
    //If it's a leaf we check the constraint, We know it's a leaf because the last variable is at the bottom of the tree. And it's a solution because we pass all the reduction of domain
    if (values.values.size() == this->nbVariable) {
        values.check = true;
        return values;
    }

    int min = -1;
    int nextVariable = 0;

    //We get the nextVariable
    for (int i = 0; i < this->nbVariable; i++) {
        if (values.values.find(i) == values.values.end() ) {
            if (min == -1 || min > this->domains[i].size()) {
                min = (int) this->domains[i].size();
                nextVariable = i;
            }
        }
    }

    Node child;
    Final temp;
    child.variable = nextVariable;
    temp = values;

    //For each value of the domain of the variable, we create a node and we check the constraint we can already test, if all the constraints are satisfy we can make a recursive call
    for (int i = 0; i < this->domains[nextVariable].size(); i++) {

        child.value = this->domains[nextVariable][i];
        temp.values[nextVariable] = child.value;

        if (this->checkConstraint(temp, true)) {
            temp = AssignmentOptimizationMethod(child, temp);
            this->edgeCreated++;

            if (temp.check) {
                return temp;
            }
        } else {
            this->arcIgnored++;
        }


    }

    //If we don't have already exit it's because there is no solution on this node
    values.check = false;
    return values;
}

Final Decision::EdgeConsistencyMethod(Node element, Final values, vector< vector< int > > domains) {

    //If it's a leaf we check the constraint, We know it's a leaf because the last variable is at the bottom of the tree. And it's a solution because we pass all the reduction of domain
    if (values.values.size() == this->nbVariable) {
        values.check = true;
        return values;
    }

    // If we don't have a domains it means that it's the first time the function is call
    if (domains.size() == 0) {
        domains = this->domains;
    }

    int min = -1;
    int nextVariable = 0;

    //We get the nextVariable
    for (int i = 0; i < this->nbVariable; i++) {

        if (domains[i].size() == 0) {
            values.check = false;
            return values;
        }

        if (values.values.find(i) == values.values.end() ) {
            if (min == -1 || min > domains[i].size()) {
                min = (int) domains[i].size();
                nextVariable = i;
            }
        }
    }

    Node child;
    Final temp;
    child.variable = nextVariable;
    temp = values;

    // We make a copy of the domain in the case we don't find the good values at the first shoot (to do a backtracking)
    vector< vector< int > > old = domains;

    //For each value of the domain of the variable, we create a node and we check the constraint we can already test, if all the constraints are satisfy we can make a recursive call
    for (int i = 0; i < old[nextVariable].size(); i++) {

        child.value = old[nextVariable][i];
        temp.values[nextVariable] = child.value;

        // We reduce the domain of the variable at only the value we're testing
        domains[nextVariable] = vector< int >();
        domains[nextVariable].push_back(old[nextVariable][i]);

        // We reduce the domain of all the neighboorhood of the variable, depending on the constraints
        domains = this->RemoveInconsistentValues(nextVariable, domains);

        // We check all the constraints (especially sum that are not tested in the removeinconsistenvalues)
        if (this->checkConstraint(temp, true)) {
            temp = EdgeConsistencyMethod(child, temp, domains);
            this->edgeCreated++;

            if (temp.check) {
                return temp;
            }
        } else {
            this->arcIgnored++;
        }

        // We do backtracking in case the constraints in the sub-tree was not fullfill
        domains = old;


    }

    //If we don't have already exit it's because there is no solution on this node
    values.check = false;
    return values;
}

vector< vector< int > > Decision::RemoveInconsistentValues(int edge, vector< vector< int > > domains) {

    vector< int > fifo;
    fifo.push_back(edge);

    while (!fifo.empty()) {


        // We check for every constraint on the values
        for (int i = 0; i < this->constraints.size(); i++) {
            Constraint constraint = this->constraints[i];
            int current = fifo.front();
            int tmp, min, minCurrent;

            // If the constraint doesn't include the edge we're checking we pass this constraint
            if (find(constraint.variables.begin(), constraint.variables.end(), current) == constraint.variables.end()) {
                continue;
            }

            // Depending of the type of constraint
            switch (constraint.type) {

                // If it's an equal constraint
                case 1:
                    // We're checkin for all the constraints of variables
                    for (int j = 0; j < constraint.variables.size(); j++) {
                        for (int k = 0; k < constraint.variables.size(); k++) {

                            // We check that one variable is the current variable
                            if (j == k || (constraint.variables[j] != current && constraint.variables[k] != current)) {
                                continue;
                            }

                            // We check for the entire domain of each
                            for (int l = 0; l < domains[constraint.variables[j]].size(); l++) {
                                for (int m = 0; m < domains[constraint.variables[k]].size(); m++) {

                                    // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                    if (domains[constraint.variables[j]][l] != domains[constraint.variables[k]][m]) {

                                        if (constraint.variables[j] == current) {
                                            domains[constraint.variables[k]].erase(domains[constraint.variables[k]].begin() + m);
                                            m--;

                                            if (find(fifo.begin(), fifo.end(), constraint.variables[k]) == fifo.end()) {
                                                fifo.push_back(constraint.variables[k]);
                                            }
                                        } else {
                                            domains[constraint.variables[j]].erase(domains[constraint.variables[j]].begin() + l);
                                            l--;

                                            if (find(fifo.begin(), fifo.end(), constraint.variables[j]) == fifo.end()) {
                                                fifo.push_back(constraint.variables[j]);
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
                    // We're checkin for all the constraints of variables
                    for (int j = 0; j < constraint.variables.size(); j++) {
                        for (int k = 0; k < constraint.variables.size(); k++) {

                            // We check that one variable is the current variable
                            if (j == k || (constraint.variables[j] != current && constraint.variables[k] != current)) {
                                continue;
                            }

                            // We check for the entire domain of each
                            for (int l = 0; l < domains[constraint.variables[j]].size(); l++) {
                                for (int m = 0; m < domains[constraint.variables[k]].size(); m++) {

                                    // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                    if (domains[constraint.variables[j]][l] == domains[constraint.variables[k]][m]) {

                                        if (constraint.variables[j] == current) {
                                            domains[constraint.variables[k]].erase(domains[constraint.variables[k]].begin() + m);
                                            m--;

                                            if (find(fifo.begin(), fifo.end(), constraint.variables[k]) == fifo.end()) {
                                                fifo.push_back(constraint.variables[k]);
                                            }
                                        } else {
                                            domains[constraint.variables[j]].erase(domains[constraint.variables[j]].begin() + l);
                                            l--;

                                            if (find(fifo.begin(), fifo.end(), constraint.variables[j]) == fifo.end()) {
                                                fifo.push_back(constraint.variables[j]);
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
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (constraint.variables[0] == current || constraint.variables[1] == current) {

                        // We check for the entire domain of each variable
                        for (int l = 0; l < domains[constraint.variables[0]].size(); l++) {
                            for (int m = 0; m < domains[constraint.variables[1]].size(); m++) {

                                // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                if (domains[constraint.variables[0]][l] > domains[constraint.variables[1]][m]) {

                                    if (constraint.variables[0] == current) {
                                        domains[constraint.variables[1]].erase(domains[constraint.variables[1]].begin() + m);
                                        m--;

                                        if (find(fifo.begin(), fifo.end(), constraint.variables[1]) == fifo.end()) {
                                            fifo.push_back(constraint.variables[1]);
                                        }
                                    } else {
                                        domains[constraint.variables[0]].erase(domains[constraint.variables[0]].begin() + l);
                                        l--;

                                        if (find(fifo.begin(), fifo.end(), constraint.variables[0]) == fifo.end()) {
                                            fifo.push_back(constraint.variables[0]);
                                        }
                                    }

                                }

                            }
                        }

                    }
                    break;

                // If it's a strict inferior constraint
                case 5:
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (constraint.variables[0] == current || constraint.variables[1] == current) {

                        // We check for the entire domain of each variable
                        for (int l = 0; l < domains[constraint.variables[0]].size(); l++) {
                            for (int m = 0; m < domains[constraint.variables[1]].size(); m++) {

                                // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                if (domains[constraint.variables[0]][l] >= domains[constraint.variables[1]][m]) {

                                    if (constraint.variables[0] == current) {
                                        domains[constraint.variables[1]].erase(domains[constraint.variables[1]].begin() + m);
                                        m--;

                                        if (find(fifo.begin(), fifo.end(), constraint.variables[1]) == fifo.end()) {
                                            fifo.push_back(constraint.variables[1]);
                                        }
                                    } else {
                                        domains[constraint.variables[0]].erase(domains[constraint.variables[0]].begin() + l);
                                        l--;

                                        if (find(fifo.begin(), fifo.end(), constraint.variables[0]) == fifo.end()) {
                                            fifo.push_back(constraint.variables[0]);
                                        }
                                    }

                                }

                            }
                        }

                    }
                    break;

                // If it's a strict superior constraint
                case 6:
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (constraint.variables[0] == current || constraint.variables[1] == current) {

                        // We check for the entire domain of each variable
                        for (int l = 0; l < domains[constraint.variables[0]].size(); l++) {
                            for (int m = 0; m < domains[constraint.variables[1]].size(); m++) {

                                // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                if (domains[constraint.variables[0]][l] <= domains[constraint.variables[1]][m]) {

                                    if (constraint.variables[0] == current) {
                                        domains[constraint.variables[1]].erase(domains[constraint.variables[1]].begin() + m);
                                        m--;

                                        if (find(fifo.begin(), fifo.end(), constraint.variables[1]) == fifo.end()) {
                                            fifo.push_back(constraint.variables[1]);
                                        }
                                    } else {
                                        domains[constraint.variables[0]].erase(domains[constraint.variables[0]].begin() + l);
                                        l--;

                                        if (find(fifo.begin(), fifo.end(), constraint.variables[0]) == fifo.end()) {
                                            fifo.push_back(constraint.variables[0]);
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
                    
                    for (int j = 0; j < constraint.variables.size(); j++) {
                        min = -1;
                        for (int k = 0; k < domains[constraint.variables[j]].size(); k++) {
                            if (domains[constraint.variables[j]][k] > min) {
                                min = domains[constraint.variables[j]][k];
                                if (constraint.variables[j] == current) {
                                    minCurrent = k;
                                }
                            }
                        }
                        tmp += min;
                    }
                    
                    if (minCurrent != -1 && tmp > constraint.parameter) {
                         domains[current].erase(domains[current].begin() + minCurrent);
                    }
                    
                    break;

                    
                    
                    
                // If it's a superior constraint
                case 8:
                    // In the case we do a reduction of domain, not all values are initialized so we need to skip those which are not initialize
                    if (constraint.variables[0] == current || constraint.variables[1] == current) {
                        
                        // We check for the entire domain of each variable
                        for (int l = 0; l < domains[constraint.variables[0]].size(); l++) {
                            for (int m = 0; m < domains[constraint.variables[1]].size(); m++) {
                                
                                // Depending of which is the current variable, we reduce the domain of the other and add the other variable to the queue, only if the variable doesn't respect the contraint
                                if (domains[constraint.variables[0]][l] < domains[constraint.variables[1]][m]) {
                                    
                                    if (constraint.variables[0] == current) {
                                        domains[constraint.variables[1]].erase(domains[constraint.variables[1]].begin() + m);
                                        m--;
                                        
                                        if (find(fifo.begin(), fifo.end(), constraint.variables[1]) == fifo.end()) {
                                            fifo.push_back(constraint.variables[1]);
                                        }
                                    } else {
                                        domains[constraint.variables[0]].erase(domains[constraint.variables[0]].begin() + l);
                                        l--;
                                        
                                        if (find(fifo.begin(), fifo.end(), constraint.variables[0]) == fifo.end()) {
                                            fifo.push_back(constraint.variables[0]);
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

    return domains;
}


