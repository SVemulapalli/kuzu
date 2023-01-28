#include "binder/binder.h"
#include "binder/expression/existential_subquery_expression.h"
#include "binder/expression_binder.h"
#include "parser/expression/parsed_subquery_expression.h"

namespace kuzu {
namespace binder {

shared_ptr<Expression> ExpressionBinder::bindExistentialSubqueryExpression(
    const ParsedExpression& parsedExpression) {
    auto& subqueryExpression = (ParsedSubqueryExpression&)parsedExpression;
    auto prevVariablesInScope = binder->enterSubquery();
    auto [queryGraph, _] = binder->bindGraphPattern(subqueryExpression.getPatternElements());
    auto name = binder->getUniqueExpressionName(parsedExpression.getRawName());
    auto boundSubqueryExpression =
        make_shared<ExistentialSubqueryExpression>(std::move(queryGraph), std::move(name));
    if (subqueryExpression.hasWhereClause()) {
        boundSubqueryExpression->setWhereExpression(
            binder->bindWhereExpression(*subqueryExpression.getWhereClause()));
    }
    binder->exitSubquery(std::move(prevVariablesInScope));
    return boundSubqueryExpression;
}

} // namespace binder
} // namespace kuzu
