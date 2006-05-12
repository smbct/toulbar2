/** \file tb2wcsp.hpp
 *  \brief Global soft constraint representing a weighted CSP
 * 
 */
 
#ifndef TB2WCSP_HPP_
#define TB2WCSP_HPP_

#include "toulbar2.hpp"
#include "tb2variable.hpp"
#include "tb2constraint.hpp"

class WCSP : public WeightedCSP {
    static int wcspCounter; // count the number of instantiations of WCSP
    int instance; // instantiation occurence number
    Store *storeData;
    StoreCost lb;
    Cost ub;
    vector<Variable *> vars;
    vector<Constraint *> constrs;
    int NCBucketSize;
    vector< VariableList > NCBuckets;         // vector of backtrackable lists
    Queue NC;                                 // non backtrackable list
    Queue IncDec;                             // non backtrackable list
    Queue AC;                                 // non backtrackable list
    Queue DAC;                                // non backtrackable list
    bool objectiveChanged;
    long long nbNodes;                        // used as a time-stamp by Queue methods

    // make it private because we don't want copy nor assignment
    WCSP(const WCSP &wcsp);
    WCSP& operator=(const WCSP &wcsp);
    
public:
    WCSP(Store *s, Cost upperBound);
    
    virtual ~WCSP();
    
    // General API for weighted CSP global constraint
    
    int getIndex() const {return instance;} // instantiation occurence number of current WCSP object
    
    Cost getLb() const {return lb;}
    Cost getUb() const {return ub;}

    // avoid cost overflow
    Cost add(const Cost a, const Cost b) const {return (a + b >= getUb())?getUb():(a+b);}
    // avoid weakning hard costs
    Cost sub(const Cost a, const Cost b) const {assert(b <= a); return (a >= getUb())?a:(a-b);}

    void updateUb(Cost newUb) {
        ub = min(ub,newUb);
    }
    void enforceUb() {
        if (lb >= ub) THROWCONTRADICTION;
        objectiveChanged=true;
    }
    void increaseLb(Cost newLb) {
        if (newLb > lb) {
            if (newLb >= ub) THROWCONTRADICTION;
            lb = newLb;
            objectiveChanged=true;
            if (ToulBar2::setminobj) (*ToulBar2::setminobj)(getIndex(), -1, newLb);
        }
    }
    void decreaseUb(Cost newUb) {
        if (newUb < ub) {
            if (newUb <= lb) THROWCONTRADICTION;
            ub = newUb;
            objectiveChanged=true;
        }
    }

    bool enumerated(int varIndex) const {return vars[varIndex]->enumerated();}
    
    string getName(int varIndex) const {return vars[varIndex]->getName();}
    Value getInf(int varIndex) const {return vars[varIndex]->getInf();}
    Value getSup(int varIndex) const {return vars[varIndex]->getSup();}
    Value getValue(int varIndex) const {return vars[varIndex]->getValue();}
    unsigned int getDomainSize(int varIndex) const {return vars[varIndex]->getDomainSize();}
    bool getEnumDomain(int varIndex, Value *array);
    bool getEnumDomainAndCost(int varIndex, ValueCost *array);

    bool assigned(int varIndex) const {return vars[varIndex]->assigned();}
    bool unassigned(int varIndex) const {return vars[varIndex]->unassigned();}
    bool canbe(int varIndex, Value v) const {return vars[varIndex]->canbe(v);}
    bool cannotbe(int varIndex, Value v) const {return vars[varIndex]->cannotbe(v);}
       
    void increase(int varIndex, Value newInf) {vars[varIndex]->increase(newInf);}
    void decrease(int varIndex, Value newSup) {vars[varIndex]->decrease(newSup);}
    void assign(int varIndex, Value newValue) {vars[varIndex]->assign(newValue);}
    void remove(int varIndex, Value remValue) {vars[varIndex]->remove(remValue);}
        
    Cost getUnaryCost(int varIndex, Value v) const {return vars[varIndex]->getCost(v);}
    Value getSupport(int varIndex) const {return vars[varIndex]->getSupport();}
    
    int getDegree(int varIndex) const {return vars[varIndex]->getDegree();}

    void whenContradiction();       // after a contradiction, reset propagation queues and increase nbNodes
    void propagate();               // propagate until a fix point and increase nbNodes
    bool verify();

    unsigned int numberOfVariables() const {return vars.size();};
    unsigned int numberOfConstraints() const {return constrs.size();};
    Value getDomainSizeSum();       // total current number of values

    int makeEnumeratedVariable(string n, Value iinf, Value isup);
    int makeEnumeratedVariable(string n, Value *d, int dsize);
    int makeIntervalVariable(string n, Value iinf, Value isup);
    
    void postBinaryConstraint(int xIndex, int yIndex, vector<Cost> &costs);
    void postSupxyc(int xIndex, int yIndex, Value cste);
    
    void read_wcsp(const char *fileName);

    // Specific API for Variable and Constraint classes

    Store *getStore() {return storeData;}
    
    void link(Variable *x) {vars.push_back(x);}
    void link(Constraint *c) {constrs.push_back(c);}

    int getNCBucketSize() const {return NCBucketSize;}
    void changeNCBucket(int oldBucket, int newBucket, DLink<Variable *> *elt) {
        if (oldBucket >= 0) NCBuckets[oldBucket].erase(elt, true);
        if (newBucket >= 0) NCBuckets[newBucket].push_back(elt, true);
    }
    void printNCBuckets();

    void queueNC(DLink<VariableWithTimeStamp> *link) {NC.push(link, nbNodes);}
    void queueInc(DLink<VariableWithTimeStamp> *link) {IncDec.push(link, INCREASE_EVENT, nbNodes);}
    void queueDec(DLink<VariableWithTimeStamp> *link) {IncDec.push(link, DECREASE_EVENT, nbNodes);}
    void queueAC(DLink<VariableWithTimeStamp> *link) {AC.push(link, nbNodes);}
    void queueDAC(DLink<VariableWithTimeStamp> *link) {DAC.push(link, nbNodes);}

    void propagateNC();
    void propagateIncDec();
    void propagateAC();
    void propagateDAC();

    void sortConstraints();

    void print(ostream& os);
    friend ostream& operator<<(ostream& os, WCSP &wcsp);
};

#endif /*TB2WCSP_HPP_*/
