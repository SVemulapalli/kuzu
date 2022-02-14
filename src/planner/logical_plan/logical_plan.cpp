#include "src/planner/include/logical_plan/logical_plan.h"

#include <utility>

namespace graphflow {
namespace planner {

void LogicalPlan::appendOperator(shared_ptr<LogicalOperator> op) {
    lastOperator = move(op);
}

unique_ptr<LogicalPlan> LogicalPlan::copy() const {
    auto plan = make_unique<LogicalPlan>(schema->copy());
    plan->lastOperator = lastOperator;
    plan->cost = cost;
    plan->expressionsToCollect = expressionsToCollect;
    return plan;
}

unique_ptr<LogicalPlan> LogicalPlan::deepCopy() const {
    auto plan = this->copy();
    plan->lastOperator = plan->lastOperator ? plan->lastOperator->copy() : plan->lastOperator;
    return plan;
}

vector<DataType> LogicalPlan::getExpressionsToCollectDataTypes() const {
    vector<DataType> dataTypes;
    for (auto& expression : expressionsToCollect) {
        dataTypes.push_back(expression->getDataType());
    }
    return dataTypes;
}

} // namespace planner
} // namespace graphflow
