#include "gtest/gtest.h"
#include "test/mock/mock_catalog.h"

#include "src/binder/include/query_binder.h"
#include "src/parser/include/parser.h"

using namespace graphflow::parser;
using namespace graphflow::binder;

using ::testing::NiceMock;
using ::testing::Test;

class BinderErrorTest : public Test {

public:
    void SetUp() override { catalog.setUp(); }

    string getBindingError(const string& input) {
        try {
            auto parsedQuery = Parser::parseQuery(input);
            QueryBinder(catalog).bind(*parsedQuery);
        } catch (const invalid_argument& exception) { return exception.what(); }
        return string();
    }

private:
    NiceMock<TinySnbCatalog> catalog;
};

TEST_F(BinderErrorTest, DisconnectedGraph1) {
    string expectedException = "Disconnect query graph is not supported.";
    auto input = "MATCH (a:person), (b:person) RETURN COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, DisconnectedGraph2) {
    string expectedException = "Disconnect query graph is not supported.";
    auto input = "MATCH (a:person) WITH * MATCH (b:person) RETURN COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, NodeLabelNotExist) {
    string expectedException = "Node label PERSON does not exist.";
    auto input = "MATCH (a:PERSON) RETURN COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, NodeRelNotConnect) {
    string expectedException = "Node label person doesn't connect to rel label workAt.";
    auto input = "MATCH (a:person)-[e1:workAt]->(b:person) RETURN COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, RepeatedRelName) {
    string expectedException =
        "Bind relationship e1 to relationship with same name is not supported.";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person)<-[e1:knows]-(:person) RETURN COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, RepeatedReturnColumnName) {
    string expectedException = "Multiple result column with the same name e1 are not supported.";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) RETURN *, e1;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, WITHExpressionAliased) {
    string expectedException = "Expression in WITH must be aliased (use AS).";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) WITH a.age RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindToDifferentVariableType1) {
    string expectedException = "a defined with conflicting type REL (expect NODE).";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) WITH e1 AS a MATCH (a) RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindToDifferentVariableType2) {
    string expectedException = "a defined with conflicting type INT64 (expect NODE).";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) WITH a.age + 1 AS a MATCH (a) RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindEmptyStar) {
    string expectedException =
        "RETURN or WITH * is not allowed when there are no variables in scope.";
    auto input = "RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindVariableNotInScope1) {
    string expectedException = "Variable a not defined.";
    auto input = "WITH a MATCH (a:person)-[e1:knows]->(b:person) RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindVariableNotInScope2) {
    string expectedException = "Variable foo not defined.";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) WHERE a.age > foo RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindPropertyLookUpOnExpression) {
    string expectedException = "a.age + 2 has data type INT64. REL, NODE was expected.";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) RETURN (a.age + 2).age;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindPropertyNotExist) {
    string expectedException = "Node a does not have property foo.";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) RETURN a.foo;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindIDArithmetic) {
    string expectedException = "id(a) has data type NODE_ID. DOUBLE, INT64 was expected.";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) WHERE id(a) + 1 < id(b) RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindDateAddDate) {
    string expectedException =
        "date('2031-02-01') has data type DATE. INTERVAL, INT64 was expected.";
    auto input = "MATCH (a:person) RETURN a.birthdate + date('2031-02-01');";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindTimestampArithmetic) {
    string expectedException = "1 has data type INT64. INTERVAL was expected.";
    auto input = "MATCH (a:person) WHERE a.registerTime + 1 < 5 RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindTimestampAddTimestamp) {
    string expectedException =
        "timestamp('2031-02-11 01:02:03') has data type TIMESTAMP. INTERVAL was expected.";
    auto input = "MATCH (a:person) RETURN a.registerTime + timestamp('2031-02-11 01:02:03');";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindNonExistingFunction) {
    string expectedException = "DUMMY function does not exist.";
    auto input = "MATCH (a:person) WHERE dummy() < 2 RETURN COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindFunctionWithWrongNumParams) {
    string expectedException = "Expected 1 parameters for date function but get 0.";
    auto input = "MATCH (a:person) WHERE date() < 2 RETURN COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, BindFunctionWithWrongParamType) {
    string expectedException = "2012 has data type INT64. UNSTRUCTURED, STRING was expected.";
    auto input = "MATCH (a:person) WHERE date(2012) < 2 RETURN COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, OrderByVariableNotInScope) {
    string expectedException = "Variable a not defined.";
    auto input = "MATCH (a:person)-[e1:knows]->(b:person) RETURN SUM(a.age) ORDER BY a;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, NestedAggregation) {
    string expectedException = "Expression SUM(SUM(a.age)) contains nested aggregation.";
    auto input = "MATCH (a:person) RETURN SUM(SUM(a.age));";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, OptionalMatchAsFirstMatch) {
    string expectedException = "First match statement cannot be optional match.";
    auto input = "OPTIONAL MATCH (a:person) RETURN *;";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}

TEST_F(BinderErrorTest, AggregationWithGroupBy) {
    string expectedException = "Aggregations with group by is not supported.";
    auto input = "MATCH (a:person) RETURN a, COUNT(*);";
    ASSERT_STREQ(expectedException.c_str(), getBindingError(input).c_str());
}
