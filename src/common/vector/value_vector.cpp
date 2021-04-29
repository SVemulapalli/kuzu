#include "src/common/include/vector/value_vector.h"

#include "src/common/include/operations/comparison_operations.h"
#include "src/common/include/vector/operations/vector_arithmetic_operations.h"
#include "src/common/include/vector/operations/vector_boolean_operations.h"
#include "src/common/include/vector/operations/vector_cast_operations.h"
#include "src/common/include/vector/operations/vector_comparison_operations.h"
#include "src/common/include/vector/operations/vector_node_id_operations.h"

using namespace graphflow::common::operation;

namespace graphflow {
namespace common {

std::function<void(ValueVector&, ValueVector&)> ValueVector::getUnaryOperation(
    ExpressionType type) {
    switch (type) {
    case NOT:
        return VectorBooleanOperations::Not;
    case NEGATE:
        return VectorArithmeticOperations::Negate;
    case IS_NULL:
        return VectorComparisonOperations::IsNull;
    case IS_NOT_NULL:
        return VectorComparisonOperations::IsNotNull;
    case HASH_NODE_ID:
        return VectorNodeIDOperations::Hash;
    case DECOMPRESS_NODE_ID:
        return VectorNodeIDOperations::Decompress;
    case CAST_TO_STRING:
        return VectorCastOperations::castStructuredToStringValue;
    case CAST_TO_UNSTRUCTURED_VECTOR:
        return VectorCastOperations::castStructuredToUnstructuredValue;
    case CAST_UNSTRUCTURED_VECTOR_TO_BOOL_VECTOR:
        return VectorCastOperations::castUnstructuredToBoolValue;
    default:
        assert(false);
    }
}

std::function<void(ValueVector&, ValueVector&, ValueVector&)> ValueVector::getBinaryOperation(
    ExpressionType type) {
    switch (type) {
    case AND:
        return VectorBooleanOperations::And;
    case OR:
        return VectorBooleanOperations::Or;
    case XOR:
        return VectorBooleanOperations::Xor;
    case EQUALS:
        return VectorComparisonOperations::Equals;
    case NOT_EQUALS:
        return VectorComparisonOperations::NotEquals;
    case GREATER_THAN:
        return VectorComparisonOperations::GreaterThan;
    case GREATER_THAN_EQUALS:
        return VectorComparisonOperations::GreaterThanEquals;
    case LESS_THAN:
        return VectorComparisonOperations::LessThan;
    case LESS_THAN_EQUALS:
        return VectorComparisonOperations::LessThanEquals;
    case EQUALS_NODE_ID:
        return VectorNodeIDCompareOperations::Equals;
    case NOT_EQUALS_NODE_ID:
        return VectorNodeIDCompareOperations::NotEquals;
    case GREATER_THAN_NODE_ID:
        return VectorNodeIDCompareOperations::GreaterThan;
    case GREATER_THAN_EQUALS_NODE_ID:
        return VectorNodeIDCompareOperations::GreaterThanEquals;
    case LESS_THAN_NODE_ID:
        return VectorNodeIDCompareOperations::LessThan;
    case LESS_THAN_EQUALS_NODE_ID:
        return VectorNodeIDCompareOperations::LessThanEquals;
    case ADD:
        return VectorArithmeticOperations::Add;
    case SUBTRACT:
        return VectorArithmeticOperations::Subtract;
    case MULTIPLY:
        return VectorArithmeticOperations::Multiply;
    case DIVIDE:
        return VectorArithmeticOperations::Divide;
    case MODULO:
        return VectorArithmeticOperations::Modulo;
    case POWER:
        return VectorArithmeticOperations::Power;
    default:
        assert(false);
    }
}

template<class T, class FUNC = std::function<uint8_t(T)>>
static void fillOperandNullMask(ValueVector& operand) {
    auto values = (T*)operand.values;
    if (operand.state->isFlat()) {
        operand.nullMask[operand.state->getCurrSelectedValuesPos()] =
            IsNull::operation(values[operand.state->getCurrSelectedValuesPos()]);
    } else {
        auto size = operand.state->numSelectedValues;
        for (uint64_t i = 0; i < size; i++) {
            operand.nullMask[i] = IsNull::operation(values[operand.state->selectedValuesPos[i]]);
        }
    }
}

void ValueVector::fillNullMask() {
    switch (dataType) {
    case BOOL:
        fillOperandNullMask<uint8_t>(*this);
        break;
    case INT32:
        fillOperandNullMask<int32_t>(*this);
        break;
    case DOUBLE:
        fillOperandNullMask<double_t>(*this);
        break;
    case STRING:
        // TODO: fillOperandNullMask<gf_string_t>(*this);
        //  Currently we do not distinguish empty and NULL gf_string_t.
        break;
    default:
        throw std::invalid_argument("Invalid or unsupported type for comparison.");
    }
}

} // namespace common
} // namespace graphflow
