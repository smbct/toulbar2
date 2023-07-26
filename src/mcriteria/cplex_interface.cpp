#include "multicfn.hpp"
#include <memory>

using namespace std;

#ifdef ILOGCPLEX

//--------------------------------------------------------------------------------------------
Double MultiCFN::computeCriteriaSol(IloCplex& cplex, size_t index, bool weighted, vector<IloNumVarArray>& domain_vars, vector<shared_ptr<IloNumVarArray>>& tuple_vars) {

  Double res = 0.;

  // objective function
  for(size_t func_ind: networks[index]) {
    auto& func = cost_function[func_ind];

    Double func_cost = 0.;

    vector<int> tuples_selected;

    for(size_t tuple_ind = 0; tuple_ind < func.tuples.size(); tuple_ind ++) {
      
      // consider only finite non null costs
      if(func.costs[tuple_ind] == std::numeric_limits<Double>::infinity()) {
        continue;
      }
      if(weighted && fabs(func.costs[tuple_ind]) <= MultiCFN::epsilon) {
        continue;
      }

      IloNum cost = func.costs[tuple_ind];
      if(weighted) {
        cost *= weights[index];
      }

      // cout << "cost: " << cost << endl;

      if(func.scope.size() >= 2) {

        // if(cplex.getValue((*tuple_vars[func_ind].get())[tuple_ind]) >= 0.5) {
        //   res += cost;
        //   func_cost += cost;
        //   tuples_selected.push_back(tuple_ind);
        // }

        res += cost*cplex.getValue((*tuple_vars[func_ind].get())[tuple_ind]);
        func_cost += cost*cplex.getValue((*tuple_vars[func_ind].get())[tuple_ind]);
        tuples_selected.push_back(tuple_ind);

      } else { // special case for unary cost function

        size_t var_ind = func.scope[0];
        size_t val_ind = func.tuples[tuple_ind][0];
        mcriteria::Var& variable = var[var_ind];

        if(variable.nbValues() > 2) {
          // if(cplex.getValue(domain_vars[var_ind][val_ind]) >= 0.5) {
          //   res += cost;
          //   func_cost += cost;
          //   tuples_selected.push_back(tuple_ind);
          // }
          res += cost*cplex.getValue(domain_vars[var_ind][val_ind]);
          func_cost += cost*cplex.getValue(domain_vars[var_ind][val_ind]);
          tuples_selected.push_back(tuple_ind);

        } else {
          if(val_ind == 1) {
            // if(cplex.getValue(domain_vars[var_ind][0]) >= 0.5) {
            //   res += cost;
            //   func_cost += cost;
            //   tuples_selected.push_back(1);
            // }
            res += cost*cplex.getValue(domain_vars[var_ind][0]);
            func_cost += cost*cplex.getValue(domain_vars[var_ind][0]);
            tuples_selected.push_back(1);
          } else {
            res += cost*(1.-cplex.getValue(domain_vars[var_ind][0]));
            func_cost += cost*(1.-cplex.getValue(domain_vars[var_ind][0]));
            tuples_selected.push_back(0);
          }
        }

      }

    }

    // additional tuple for default cost
    if(!func.all_tuples && func.arity() >= 3 && func.default_cost != std::numeric_limits<Double>::infinity()) {
      IloNum cost = func.default_cost;
      if(weighted) {
        cost *= weights[index];
      }
      if(cplex.getValue((*tuple_vars[func_ind].get())[func.tuples.size()]) >= 0.5) {
        res += cost;
        func_cost += cost;
      }
    }

    if(fabs(func_cost) > 0.1) {
      cout << "cost function " << func_ind  << "(" << func.name << "):  " << func_cost << endl;
    }

  }

  // constant term
  if(weighted) {
    res += IloNum(_doriginal_lbs[index]*weights[index]);
  } else {
    res += IloNum(_doriginal_lbs[index]);
  }

  return res;
}

//--------------------------------------------------------------------------------------------
void MultiCFN::addCriterion(IloExpr& expr, size_t index, bool weighted, bool negShift, vector<IloNumVarArray>& domain_vars, vector<shared_ptr<IloNumVarArray>>& tuple_vars) {

  Double shift = 0.;

  // objective function
  for(size_t func_ind: networks[index]) {

    auto& func = cost_function[func_ind];

    // compute a minimum cost for this cost function
    Double min_cost;
    if(weighted && negShift) {
      for(size_t tuple_ind = 0; tuple_ind < func.tuples.size(); tuple_ind ++) {
        if(func.costs[tuple_ind] != std::numeric_limits<Double>::infinity()) {
          if(tuple_ind == 0 || min_cost > func.costs[tuple_ind]*weights[index]) {
            min_cost = func.costs[tuple_ind]*weights[index];
          }
        }
      }
      if(func.default_cost != std::numeric_limits<Double>::infinity()) {
        if(min_cost > func.default_cost*weights[index]) {
          min_cost = func.default_cost*weights[index];
        }
      }
      if(min_cost < 0.) {
        shift += min_cost;
      }
    }

    // add costs for the cost function "func"
    for(size_t tuple_ind = 0; tuple_ind < func.tuples.size(); tuple_ind ++) {
      
      // consider only finite positive costs
      if(func.costs[tuple_ind] == std::numeric_limits<Double>::infinity()) {
        continue;
      }
      // if(weighted && fabs(func.costs[tuple_ind]) <= MultiCFN::epsilon) {
      //   continue;
      // }

      IloNum cost = func.costs[tuple_ind];
      if(weighted) {
        cost *= weights[index];
      }
      if(weighted && negShift && min_cost < 0.) {
        cost -= min_cost;
      }

      // cout << "cost: " << cost << endl;

      if(func.scope.size() >= 2) {

        expr += (*tuple_vars[func_ind].get())[tuple_ind]*cost;
        
      } else { // special case for unary cost function

        size_t var_ind = func.scope[0];
        size_t val_ind = func.tuples[tuple_ind][0];
        mcriteria::Var& variable = var[var_ind];

        if(variable.nbValues() > 2) {
          expr += domain_vars[var_ind][val_ind]*cost;
        } else if(variable.nbValues() == 2) {
          if(val_ind == 1) {
            expr += domain_vars[var_ind][0]*cost;
          } else {
            expr += (1-domain_vars[var_ind][0])*cost;
          }
        } else {
          expr += cost;
        }
      }

    }

    // additional tuple for default cost
    if(!func.all_tuples && func.arity() >= 3 && func.default_cost != std::numeric_limits<Double>::infinity()) {
      IloNum cost = func.default_cost;
      if(weighted) {
        cost *= weights[index];
      }
      if(weighted && negShift && min_cost < 0.) {
        cost -= min_cost;
      }
      expr += (*tuple_vars[func_ind].get())[func.tuples.size()]*cost;
    }

  }

  // constant term
  if(weighted) {
    expr += IloNum(_doriginal_lbs[index]*weights[index]);
  } else {
    expr += IloNum(_doriginal_lbs[index]);
  }

  // shift ensuring only positive tuple costs
  if(weighted && negShift) {
    expr += IloNum(shift);
  }
}

//--------------------------------------------------------------------------------------------
void MultiCFN::makeIloModel(IloEnv& env, IloModel& model, ILP_encoding encoding, vector<size_t>& objectives, vector<pair<size_t, pair<Double, Double>>>& constraints, vector<IloNumVarArray>& domain_vars, vector<shared_ptr<IloNumVarArray>>& tuple_vars) {

  // variables definition

  // for binary variables, one boolean var = 1 iif the variable has value 1 (implies no bool var representing 0 value)

  // domain variables for each variable
  // vector<IloNumVarArray> domain_vars;

  for(size_t index = 0; index < var.size(); index ++) {
    mcriteria::Var& variable = var[index];
    if(variable.nbValues() > 2) {
      domain_vars.push_back(IloNumVarArray(env, variable.nbValues(), 0, 1, ILOINT));
    } else if(variable.nbValues() == 2) {
      domain_vars.push_back(IloNumVarArray(env, 1, 0, 1, ILOINT));
    } else {
      domain_vars.push_back(IloNumVarArray(env, 0, 0, 1, ILOINT));
      continue; // do not create a cplex variable if the domain contains 1 value
    }
    domain_vars.back().setNames(string("dom_" + variable.name).c_str());
  }

  // tuple variables for each pair of cost functions
  // vector<shared_ptr<IloNumVarArray>> tuple_vars(cost_function.size(), nullptr);

  for(size_t func_ind = 0; func_ind < cost_function.size(); func_ind ++) {
    
    auto func = cost_function[func_ind];

    if(func.scope.size() < 2) {
      continue;
    }

    // one artificial tuple variable is added if not all tuples are defined in the cost function (default_cost)
    if(func.all_tuples || func.default_cost == std::numeric_limits<Double>::infinity()) {
      tuple_vars[func_ind] = std::make_shared<IloNumVarArray>(env, func.tuples.size(), 0., 1., ILOFLOAT);
    } else {
      tuple_vars[func_ind] = std::make_shared<IloNumVarArray>(env, func.tuples.size()+1, 0., 1., ILOFLOAT);
    }
    tuple_vars[func_ind]->setNames(string("tuples_"+to_string(func_ind)).c_str());
    

  }
  

  // constraints definition

  // only one variable per domain
  for (size_t var_ind = 0; var_ind < domain_vars.size(); ++var_ind) {

    if(var[var_ind].nbValues() <= 2) {
      continue;
    }

    IloExpr expr(env);
    for (IloInt val_ind = 0; val_ind < domain_vars[var_ind].getSize(); ++val_ind) {
      expr += domain_vars[var_ind][val_ind];
    }
    model.add(expr == 1);
    expr.end();
  }

  // only one tuple is selected, necessary when default cost is used or for direct encoding when there is a constraint
  for(unsigned int func_ind = 0; func_ind < tuple_vars.size(); func_ind ++) {
    if(tuple_vars[func_ind] != nullptr) {

      if(!constraints.empty() && ((size_t)tuple_vars[func_ind].get()->getSize() > cost_function[func_ind].tuples.size() || encoding == ILP_Direct)) {

        IloExpr expr(env);
        for(unsigned int tuple_ind = 0; tuple_ind < tuple_vars[func_ind].get()->getSize(); tuple_ind ++) {
          expr += (*tuple_vars[func_ind].get())[tuple_ind];
        }
        model.add(expr == 1);
        expr.end();
        
      }
    }
  }


  // tuple corresponding to default cost must be used if no other tuple is 
  // for(size_t func_ind = 0; func_ind < cost_function.size(); func_ind ++) {
  //   auto func = cost_function[func_ind];
  //   if(func.all_tuples || func.arity() < 2 || func.default_cost == std::numeric_limits<Double>::infinity()) {
  //     continue;
  //   }
  //   IloExpr expr(env);
  //   for(size_t tuple_ind = 0; tuple_ind < func.tuples.size(); tuple_ind ++) {
  //     expr += (*tuple_vars[func_ind].get())[tuple_ind];
  //   }
  //   expr += (*tuple_vars[func_ind].get())[func.tuples.size()];
  //   model.add(expr >= 1.);
  //   expr.end();
  // }

  // direct encoding

  if(encoding == ILP_Direct) {

    // tuples imply their corresponding values are used
    for(size_t func_ind = 0; func_ind < cost_function.size(); func_ind ++) {
      auto& func = cost_function[func_ind];

      if(func.scope.size() < 2) {
        continue;
      }

      for(size_t tuple_ind = 0; tuple_ind < func.tuples.size(); tuple_ind ++) {

        auto& tuple = func.tuples[tuple_ind];

        IloExpr expr(env);
        for(size_t scope_ind = 0; scope_ind < func.scope.size(); ++ scope_ind)  {

          size_t var_ind = func.scope[scope_ind];
          mcriteria::Var& variable = var[var_ind];
          auto val_ind = tuple[scope_ind];

          if(variable.nbValues() > 2) {
            expr += (IloNum(1)-domain_vars[var_ind][val_ind]);
          } else if(variable.nbValues() == 2) { // special case for binary variables
            if(val_ind == 1) {
              expr += (IloNum(1)-domain_vars[var_ind][0]);
            } else {
              expr += domain_vars[var_ind][val_ind];
            }
          }

        }

        if(func.costs[tuple_ind] != std::numeric_limits<Double>::infinity()) {
          expr += (*tuple_vars[func_ind].get())[tuple_ind];
        }

        model.add(expr >= 1);
        expr.end();

      }
    }

  } else if(encoding == ILP_Tuple) {

    for(size_t func_ind = 0; func_ind < cost_function.size(); func_ind ++) {
      
      auto& func = cost_function[func_ind];

      // nothing to do for unary cost functions (only domain variables)
      if(func.scope.size() < 2) {
        continue;
      }

      for (size_t scope_ind = 0; scope_ind < func.scope.size(); scope_ind ++) {

        size_t var_ind = func.scope[scope_ind];
        mcriteria::Var& variable = var[var_ind];

        if(variable.nbValues() < 2) {
          continue;
        }

        for (size_t val_ind = 0; val_ind < variable.nbValues(); ++val_ind) {

          IloExpr expr(env);

          size_t n_tuples_value = 0;
          size_t n_tuples_cost = 0;
          for(size_t tuple_ind = 0; tuple_ind < func.tuples.size(); tuple_ind ++) {
            if(func.tuples[tuple_ind][scope_ind] == val_ind) {
              n_tuples_value ++;
              if(func.costs[tuple_ind] != std::numeric_limits<Double>::infinity()) {
                expr += (*tuple_vars[func_ind].get())[tuple_ind];
                n_tuples_cost ++;
              }
            }
          }

          if(n_tuples_cost == 0) {
            expr.end();
            continue;
          }

          // additional tuple for default cost if not all tuples for this value are specified
          if(func.default_cost != std::numeric_limits<Double>::infinity() && n_tuples_value*variable.nbValues() < func.n_total_tuples) {
            expr += (*tuple_vars[func_ind].get())[func.tuples.size()];
          }

          if(variable.nbValues() > 2) {

            if(func.default_cost != std::numeric_limits<Double>::infinity() && n_tuples_value*variable.nbValues() < func.n_total_tuples) {
              model.add(expr >= domain_vars[var_ind][val_ind]);
            } else {
              model.add(expr == domain_vars[var_ind][val_ind]);
            }
            

          } else if(variable.nbValues() == 2) { // special case for binary variables

            if(val_ind == 1) {

              if(func.default_cost != std::numeric_limits<Double>::infinity() && n_tuples_value*variable.nbValues() < func.n_total_tuples) {
                model.add(expr >= domain_vars[var_ind][0]);
              } else {
                model.add(expr == domain_vars[var_ind][0]);
              }
              
            } else {
              if(func.default_cost != std::numeric_limits<Double>::infinity() && n_tuples_value*variable.nbValues() < func.n_total_tuples) {
                model.add(expr >= (1-domain_vars[var_ind][0]));
              } else {
                model.add(expr == (1-domain_vars[var_ind][0]));
              }
            }

          }
          
          expr.end();

        }
      }
    }

    

  } else {
    cerr << "Error: undefined ILP encoding type" << endl;
    return;
  }
  
  // hard constraints
  for(size_t func_ind = 0; func_ind < cost_function.size(); func_ind ++) {

    auto& func = cost_function[func_ind];

    // * unary cost functions
    if(func.scope.size() == 1) {

      for(size_t tuple_ind = 0; tuple_ind < func.tuples.size(); tuple_ind ++) {

        // filter out infinite costs
        if(func.costs[tuple_ind] != std::numeric_limits<Double>::infinity()) {
          continue;
        }

        size_t var_ind = func.scope.front();
        mcriteria::Var& variable = var[var_ind];
        size_t val_ind = func.tuples[tuple_ind].front();

        IloExpr expr(env);
        if(variable.nbValues() > 2) {
          expr += domain_vars[var_ind][val_ind];
          model.add(expr == IloNum(0));
        } else if(variable.nbValues() == 2) {
          expr += domain_vars[var_ind][0];
          if(val_ind == 0) {
            model.add(expr == IloNum(1));
          } else {
            model.add(expr == IloNum(0));
          }
        }
        expr.end();

      }
      continue;
    }

    // * cost functions with arity >= 2
    for(size_t tuple_ind = 0; tuple_ind < func.tuples.size(); tuple_ind ++) {

      // select only tuples with infinite costs
      if(func.costs[tuple_ind] != std::numeric_limits<Double>::infinity()) {
        continue;
      }

      IloExpr expr(env);
      auto& tuple = func.tuples[tuple_ind];
      for(size_t scope_ind = 0; scope_ind < tuple.size(); ++ scope_ind) {

        size_t var_ind = func.scope[scope_ind];
        size_t val_ind = tuple[scope_ind];
        mcriteria::Var& variable = var[var_ind];

        if(variable.nbValues() > 2) {
          expr += (1-domain_vars[var_ind][val_ind]);
        } else if (variable.nbValues() == 2) { // binary variable
          if(val_ind == 1) {
            expr += (1-domain_vars[var_ind][0]);
          } else {
            expr += domain_vars[var_ind][0];
          }
        }
      
      }
      model.add(expr >= 1);
      expr.end();


    }
  }
  
  // global cfn constraints
  for(auto cstr: constraints) {

    size_t network_index = cstr.first;

    // weights on constraints ?
    // assert(abs(weights[network_index]) > epsilon);

    auto bounds = cstr.second;

    IloExpr cstr_expr(env);
    addCriterion(cstr_expr, network_index, false, false, domain_vars, tuple_vars);

    assert(bounds.first != std::numeric_limits<Double>::infinity() || bounds.second != std::numeric_limits<Double>::infinity());

    if(bounds.first == std::numeric_limits<Double>::infinity()) {
      model.add(cstr_expr <= IloNum(bounds.second));
    } else if(bounds.second == std::numeric_limits<Double>::infinity()) {
      model.add(IloNum(bounds.first) <= cstr_expr);
    } else {
      model.add(IloNum(bounds.first) <= cstr_expr <= IloNum(bounds.second));
    }

    cstr_expr.end();


  }

  // objective function
  IloExpr obj(env);
  for(size_t pb_index: objectives) {

    // do not add nulled cfn
    assert(abs(weights[pb_index]) > epsilon);
    
    if(constraints.empty() == 0) {
      addCriterion(obj, pb_index, true, true, domain_vars, tuple_vars);
    } else {
      addCriterion(obj, pb_index, true, false, domain_vars, tuple_vars);
    }
  }
  
  model.add(IloMinimize(env, obj));
  obj.end();

}


//--------------------------------------------------------------------------------------------
void MultiCFN::getCplexSolution(IloCplex& cplex, std::vector<IloNumVarArray>& domain_vars, MultiCFN::Solution& solution) {

  IloNum const tolerance = cplex.getParam(IloCplex::Param::MIP::Tolerances::Integrality);

  // cout << std::setprecision(15);

  for(size_t var_ind = 0; var_ind < var.size(); var_ind ++) {
    
    mcriteria::Var& variable = var[var_ind];
    size_t val_ind = 0;

    // cout << "var " << var_ind << "(" << variable.name <<  "): ";

    // look for the binary variable corresponding to the value selected
    if(variable.nbValues() > 2) {
      int cpt = 0;
      for(size_t ind = 0; ind < static_cast<size_t>(domain_vars[var_ind].getSize()); ind ++) {
        // cout << cplex.getValue(domain_vars[var_ind][ind]) << ", ";
        if(cplex.isExtracted(domain_vars[var_ind][ind])) {
          if(fabs(cplex.getValue(domain_vars[var_ind][ind])-1.) <= 1e-4) {
            val_ind = ind;
            cpt ++;
          }
        }
      }
      assert(cpt == 1);
    } else if(variable.nbValues() == 2) {
      // cout << cplex.getValue(domain_vars[var_ind][0]) << ", ";
      if(cplex.isExtracted(domain_vars[var_ind][0])) {
        if(fabs(cplex.getValue(domain_vars[var_ind][0])-1.) <= 1e-4) {
          val_ind = 1;
        }
      }
    }

    solution.insert(make_pair(variable.name, variable.domain_str[val_ind]));

    // cout << endl;

  }

}


#endif