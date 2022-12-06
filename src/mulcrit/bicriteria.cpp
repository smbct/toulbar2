
#include "toulbar2lib.hpp"

#include "bicriteria.hpp"

#include <algorithm>

using namespace std;

using namespace mulcrit;

//--------------------------------------------------------------------------------------------
vector<Bicriteria::Weights> Bicriteria::_weights = vector<Weights>();
vector<Bicriteria::Point> Bicriteria::_points = vector<Point>();
vector<mulcrit::Solution> Bicriteria::_solutions = vector<mulcrit::Solution>();

//--------------------------------------------------------------------------------------------
void Bicriteria::sortSolutions(pair<OptimDir, OptimDir> optim_dir) {
    
  // the points are sorted from left to right
  vector<unsigned int> sol_indexes(_points.size());
  for(unsigned int ind = 0; ind < sol_indexes.size(); ind ++) {
    sol_indexes[ind] = ind;
  }

  if(optim_dir.first == optim_dir.second) {
    sort(sol_indexes.begin(), sol_indexes.end(), [](unsigned int& ind1, unsigned int& ind2) { return (_points[ind1].first < _points[ind2].first) || (_points[ind1].first == _points[ind2].first && _points[ind1].second > _points[ind2].second); } );
  } else {
    sort(sol_indexes.begin(), sol_indexes.end(), [](unsigned int& ind1, unsigned int& ind2) { return (_points[ind1].first < _points[ind2].first) || (_points[ind1].first == _points[ind2].first && _points[ind1].second < _points[ind2].second); } );
  }

  vector<Weights> temp_weights = _weights;
  for(unsigned int ind = 0; ind < _weights.size(); ind ++) {
    _weights[ind] = temp_weights[sol_indexes[ind]];
  }

  vector<Point> temp_points = _points;
  for(unsigned int ind = 0; ind < _points.size(); ind ++) {
    _points[ind] = temp_points[sol_indexes[ind]];
  }

  vector<mulcrit::Solution> temp_sol = _solutions;
  for(unsigned int ind = 0; ind < _solutions.size(); ind ++) {
    _solutions[ind] = temp_sol[sol_indexes[ind]];
  }

}

//--------------------------------------------------------------------------------------------
bool Bicriteria::notEqual(Bicriteria::Point p1, Bicriteria::Point p2) {
  return fabs(p1.first-p2.first) >= MultiWCSP::epsilon || fabs(p1.second-p2.second) >= MultiWCSP::epsilon;
}

//--------------------------------------------------------------------------------------------
bool Bicriteria::equal(Point p1, Point p2) {
  return fabs(p1.first-p2.first) <= MultiWCSP::epsilon && fabs(p1.second-p2.second) <= MultiWCSP::epsilon;
}

//--------------------------------------------------------------------------------------------
bool Bicriteria::dominates(Point p1, Point p2, pair<OptimDir, OptimDir> optim_dir) {

  unsigned int obj_1 = 0;
  if( (optim_dir.first == Optim_Min && p1.first < p2.first) || (optim_dir.first == Optim_Max && p1.first > p2.first)) {
    obj_1 = 2;
  } else if(fabs(p1.first-p2.first) <= MultiWCSP::epsilon) {
    obj_1 = 1;
  }

  unsigned int obj_2 = 0;
  if( (optim_dir.second == Optim_Min && p1.second < p2.second) || (optim_dir.second == Optim_Max && p1.second > p2.second)) {
    obj_2 = 2;
  } else if(fabs(p1.second-p2.second) <= MultiWCSP::epsilon) {
    obj_2 = 1;
  }

  return obj_1 + obj_2 >= 4;
}

//--------------------------------------------------------------------------------------------
bool Bicriteria::solveScalarization(MultiWCSP* multiwcsp, pair<Double,Double> weights, Solution* solution, Bicriteria::Point* point) {

  cout << "current weights: " << weights.first << ", " << weights.second << endl;

  multiwcsp->setWeight(0, weights.first);
  multiwcsp->setWeight(1, weights.second);

  tb2init();

  // WeightedCSPSolver* solver = WeightedCSPSolver::makeWeightedCSPSolver(MAX_COST);

  // WCSP* pb = dynamic_cast<WCSP*>(solver->getWCSP());
  // combiner.exportToWCSP(pb);

  /* debug */
  // ofstream file("latin_combined.cfn");
  // pb->dump_CFN(file);
  // file.close();

  WeightedCSP* wcsp = multiwcsp->makeWeightedCSP();

  cout << "n variables in the final wcsp: " << wcsp->numberOfVariables() << ", " << wcsp->numberOfConstraints() << endl;

  WeightedCSPSolver* solver = WeightedCSPSolver::makeWeightedCSPSolver(MAX_COST, wcsp);
  solver->getWCSP()->sortConstraints();

  vector<Double> sol_values;

  bool result = solver->solve();

  if(result) {
    // cout << "solution found" << endl;
    // combiner.getSolution(solver, &sol_values, solution);
    sol_values = multiwcsp->getSolutionValues();

    if(solution != nullptr) {
      *solution = multiwcsp->getSolution();
    }

    if(point != nullptr) {
      *point = make_pair(sol_values[0], sol_values[1]);
    }

    // solver->getMultiWCSPSolution(sol_values);
  } else {
    // cout << "no solution !" << endl;
  }

  delete solver;
  delete wcsp; 

  return result;

}

//--------------------------------------------------------------------------------------------
void Bicriteria::computeAdditionalSolutions(mulcrit::MultiWCSP* multiwcsp, pair<Bicriteria::OptimDir, Bicriteria::OptimDir> optim_dir, unsigned int solIndex, unsigned int nbLimit) {

  tb2init();
  ToulBar2::allSolutions = nbLimit;

  Weights weights = make_pair(fabs(_points[solIndex+1].second-_points[solIndex].second), fabs(_points[solIndex+1].first-_points[solIndex].first));

  if(optim_dir.first == Optim_Max && weights.first > 0) {
    weights.first = -weights.first;
  }

  if(optim_dir.second == Optim_Max && weights.second > 0) {
    weights.second = -weights.second;
  }

  cout << "selected weights: " << weights.first << ", " << weights.second << endl;

  // compute the "corner" to determine a new ub
  Point corner;
  if(optim_dir.first == Optim_Min) {
    corner.first = max(_points[solIndex].first, _points[solIndex+1].first);
  } else {
    corner.first = min(_points[solIndex].first, _points[solIndex+1].first);
  }
  if(optim_dir.second == Optim_Min) {
    corner.second = max(_points[solIndex].second, _points[solIndex+1].second);
  } else {
    corner.second = min(_points[solIndex].second, _points[solIndex+1].second);
  }

  Double new_ub = corner.first*weights.first + corner.second*weights.second;

  multiwcsp->setWeight(0, weights.first);
  multiwcsp->setWeight(1, weights.second);

  WeightedCSP* wcsp = multiwcsp->makeWeightedCSP();
  wcsp->setUb(wcsp->DoubletoCost(new_ub));

  WeightedCSPSolver* solver = WeightedCSPSolver::makeWeightedCSPSolver(MAX_COST, wcsp);
  solver->getWCSP()->sortConstraints();

  vector<Double> sol_values;

  bool result = solver->solve();

  if(result) {

    vector<pair<Double, vector<Value>>> tb2_sol = solver->getSolutions();

    cout << "number of solutions: " << tb2_sol.size() << endl;

    /* compute the solutions and points */
    vector<mulcrit::Solution> sol;
    for(unsigned int ind = 0; ind < tb2_sol.size(); ind ++) {
      sol.push_back(multiwcsp->convertToSolution(tb2_sol[ind].second));
    }

    vector<Point> points;
    for(unsigned int ind = 0; ind < sol.size(); ind ++) {
      vector<Double> values = multiwcsp->computeSolutionValues(sol[ind]);
      points.push_back(make_pair(values[0], values[1]));
    }


    // build a list of indexes on the solutions
    vector<unsigned int> sol_indexes_temp(sol.size());
    for(unsigned int ind = 0; ind < sol.size(); ind ++) {
      sol_indexes_temp[ind] = ind;
    }

    // filter the points by indexes
    vector<unsigned int> sol_indexes;
    for(unsigned int ind: sol_indexes_temp) {
      
      bool add = true;

      // check if the point correspond to one the the supported points
      if(Bicriteria::equal(points[ind], _points[solIndex])) {
        add = false;
      } else if(Bicriteria::equal(points[ind], _points[solIndex+1])) {
        add = false;
      }
      
      // check if the point is dominated by one of the point given as index
      if(add && Bicriteria::dominates(points[solIndex], points[ind], optim_dir)) {
        add = false;
      } else if(Bicriteria::dominates(points[solIndex+1], points[ind], optim_dir)) {
        add = false;
      }

      // make sure the point dominates the corner
      if(add && !Bicriteria::dominates(points[ind], corner, optim_dir)) {
        add = false;
      }

      // look for dominating points
      if(add && find_if(sol_indexes_temp.begin(), sol_indexes_temp.end(), [ind, points, optim_dir](unsigned int ind2) 
      { return ind != ind2 && dominates(points[ind2], points[ind], optim_dir); }) != sol_indexes_temp.end()) {
        add = false;
      }

      if(add) {
        sol_indexes.push_back(ind);
      }

    }

    cout << "sol indexes temp size: " << sol_indexes_temp.size() << endl;

    cout << "sol indexes size: " << sol_indexes.size() << endl;

    vector<mulcrit::Solution> temp_sol;
    vector<Point> temp_points;

    for(unsigned int index: sol_indexes) {
      temp_sol.push_back(sol[index]);
      temp_points.push_back(points[index]);
    }

    sol = temp_sol;
    points = temp_points;

    cout << "n solutions: " << sol.size() << endl;

    for(unsigned int ind = 0; ind < points.size(); ind ++) {
      cout << "new additional point: " << points[ind].first << ", " << points[ind].second << endl;
    }

    /* solutions are added to the list, then sorted */
    for(unsigned int ind = 0; ind < sol.size(); ind ++) {
      _solutions.push_back(sol[ind]);
      _points.push_back(points[ind]);
      _weights.push_back(make_pair(0., 0.));
    }

    sortSolutions(optim_dir);

  } else {
    
  }

  delete solver;
  delete wcsp; 

}

//--------------------------------------------------------------------------------------------
void Bicriteria::computeSupportedPoints(mulcrit::MultiWCSP* multiwcsp, pair<Bicriteria::OptimDir, Bicriteria::OptimDir> optim_dir, Double delta) {

  _solutions.clear();
  _points.clear();
  _weights.clear();

  tb2init();

  ToulBar2::verbose = -1;

  ToulBar2::cfn = true;

  Point point1, point2;

  Double lambda1 = 1., lambda2 = 1.;
  if(optim_dir.first == Optim_Max) {
    lambda1 = -1.;
  }
  if(optim_dir.second == Optim_Max) {
    lambda2 = -1.;
  }

  Solution sol1, sol2;

  // cout << endl << endl;
  // cout << "optimizing 1 separately: " << endl;
  // // point1 = solve_scalarization(multiwcsp, make_pair(lambda1,-0.01), &sol1);
  // point1 = solve_scalarization(multiwcsp, make_pair(lambda1,0), &sol1);
  // cout << "Optimal point for 1: " << point1.first << ";" << point1.second << endl;

  // cout << endl << endl;
  // cout << "optimizing 2 separately: " << endl;
  // // point2 = solve_scalarization(multiwcsp, make_pair(-0.01,lambda2), &sol2);
  // point2 = solve_scalarization(multiwcsp, make_pair(0,lambda2), &sol2);
  // cout << "Optimal point for 2: " << point2.first << ";" << point2.second << endl;
  // cout << endl << endl;

  // delta is compared to an internal decimalPoint because ToulBar2::decimalPoint is only initialized when created the final wcsp (makeMultiWCSP)
  if(log10(delta) > multiwcsp->getDecimalPoint()) {
    cerr << "Error: delta constant (" << delta << ") is incompatible with decimalPoint (" << ToulBar2::decimalPoint << ")" << endl; 
  }

  bool result1, result2;
  Weights weights1, weights2;

  // cout << "optimizing 1 separately: " << endl;
  if(optim_dir.second == Optim_Min) {
    weights1 = make_pair(lambda1,-delta);
  } else {
    weights1 = make_pair(lambda1,delta);
  }
  result1 = solveScalarization(multiwcsp, weights1, &sol1, &point1);

  // cout << "Optimal point for 1: " << point1.first << ";" << point1.second << endl;

  // cout << "optimizing 2 separately: " << endl;
  if(optim_dir.first == Optim_Min) {
    weights2 = make_pair(-delta,lambda2);
  } else {
    weights2 = make_pair(delta,lambda2);
  }
  result2 = solveScalarization(multiwcsp, weights2, &sol2, &point2);    

  // cout << "Optimal point for 2: " << point2.first << ";" << point2.second << endl;
  // cout << endl << endl;

  stack<pair<Point, Point>> pending;

  if(result1) { // make sure there is a solution
    _weights.push_back(weights1);
    _points.push_back(point1);
    _solutions.push_back(sol1);
  }
  
  if(result2 && notEqual(point1, point2)) {
    pending.push(make_pair(point1, point2));
    _weights.push_back(weights2);
    _points.push_back(point2);
    _solutions.push_back(sol2);
  }

  unsigned int iter = 0;

  mulcrit::Solution new_sol;

  while(!pending.empty()) {

    iter ++;

    pair<Point,Point> top = pending.top();
    pending.pop();

    // compute the new weights

    if(optim_dir.first == optim_dir.second) {
      lambda1 = -top.second.second + top.first.second;
      lambda2 = -top.first.first + top.second.first;
    } else {
      lambda1 = top.second.second - top.first.second;
      lambda2 = top.first.first - top.second.first;
    }


    // cout << "weights : " << lambda1 << ", " << lambda2 << endl;
    Point new_point;

    Weights new_weights = make_pair(lambda1,lambda2);
    bool result = solveScalarization(multiwcsp, new_weights, &new_sol, &new_point);

    // jump to the next weights if there is no solution
    if(!result) {
      continue;
    }

    // cout << "new point: " << new_point.first << ", " << new_point.second << endl;
    // cout << "from " << top.first.first << "," << top.first.second << " and " << top.second.first << ", " << top.second.second << endl;  

    // cut the search if the point was alrady encountered
    auto it = std::find_if(_points.begin(), _points.end(), [new_point](Point& point) { return equal(point, new_point); });
    if(it != _points.end()) {
      continue;
    }

    if(notEqual(new_point, top.first) && notEqual(new_point, top.second)) {

      _weights.push_back(new_weights);
      _points.push_back(new_point);
      _solutions.push_back(new_sol);

      // add two new scalarizations
      if(fabs(top.first.first - new_point.first) >= MultiWCSP::epsilon && fabs(top.first.second - new_point.second) >= MultiWCSP::epsilon) {
        pending.push(make_pair(top.first, new_point));
      }
      if(fabs(top.second.first - new_point.first) >= MultiWCSP::epsilon && fabs(top.second.second - new_point.second) >= MultiWCSP::epsilon) {
        pending.push(make_pair(new_point, top.second));
      }

    }

    // cout << endl << endl << "debug" << endl << endl << endl;

    // cout << "points found so far: ";
    // for(auto& point: sol_points) {
    //   cout << "(" << point.first << "," << point.second << "), ";
    // }
    // cout << endl;

  }

  // cout << "Supported points: " << endl;
  // for(unsigned int point_ind = 0; point_ind < sol_points.size(); point_ind ++) {
  //   cout << sol_points[point_ind].first << ", " << sol_points[point_ind].second << endl;
  // }

  // the points are sorted from left to right
  sortSolutions(optim_dir);



}

//--------------------------------------------------------------------------------------------
std::vector<mulcrit::Solution> Bicriteria::getSolutions() {
  return _solutions;
}

//--------------------------------------------------------------------------------------------
std::vector<Bicriteria::Point> Bicriteria::getPoints() {
  return _points;
}

//--------------------------------------------------------------------------------------------
std::vector<Bicriteria::Weights> Bicriteria::getWeights() {
  return _weights;
}