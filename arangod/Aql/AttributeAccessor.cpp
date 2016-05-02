////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#include "AttributeAccessor.h"
#include "Aql/AqlItemBlock.h"
#include "Aql/Variable.h"
#include "Basics/VelocyPackHelper.h"
#include "Utils/AqlTransaction.h"

using namespace arangodb::aql;

/// @brief create the accessor
AttributeAccessor::AttributeAccessor(
    std::vector<std::string> const& attributeParts, Variable const* variable)
    : _attributeParts(attributeParts),
      _variable(variable) {

  TRI_ASSERT(_variable != nullptr);
}
  
/// @brief replace the variable in the accessor
void AttributeAccessor::replaceVariable(std::unordered_map<VariableId, Variable const*> const& replacements) {
  for (auto const& it : replacements) {
    if (it.first == _variable->id) {
      _variable = it.second;
      return;
    }
  }
}

/// @brief execute the accessor
AqlValue AttributeAccessor::get(arangodb::AqlTransaction* trx,
                                AqlItemBlock const* argv, size_t startPos,
                                std::vector<Variable const*> const& vars,
                                std::vector<RegisterId> const& regs,
                                bool& mustDestroy) {
  size_t i = 0;
  for (auto it = vars.begin(); it != vars.end(); ++it, ++i) {
    if ((*it)->id == _variable->id) {
      // get the AQL value
      if (_attributeParts.size() == 1) {
        // use optimized version for single attribute (e.g. variable.attr)
        return argv->getValueReference(startPos, regs[i]).get(trx, _attributeParts[0], mustDestroy, true);
      } else {
        // use general version for multiple attributes (e.g. variable.attr.subattr)
        return argv->getValueReference(startPos, regs[i]).get(trx, _attributeParts, mustDestroy, true);
      }
    }
    // fall-through intentional
  }

  mustDestroy = false;
  return AqlValue(arangodb::basics::VelocyPackHelper::NullValue());
}

