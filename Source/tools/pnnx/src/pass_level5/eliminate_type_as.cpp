// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2023 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "eliminate_type_as.h"

#include <algorithm>
#include "pass_level2.h"

namespace pnnx {

void eliminate_type_as(Graph& graph)
{
    while (1)
    {
        bool matched = false;

        for (size_t i = 0; i < graph.ops.size(); i++)
        {
            Operator* op = graph.ops[i];

            if (op->type != "Tensor.type_as")
                continue;

            if (op->inputs[0]->type == 0 || op->outputs[0]->type == 0)
                continue;

            if (op->inputs[0]->type != op->outputs[0]->type)
                continue;

            // delete noop-like type_as
            matched = true;

            for (auto& x : op->inputs)
            {
                x->remove_consumer(op);
            }

            Operand* type_as_out = op->outputs[0];

            for (auto& x : type_as_out->consumers)
            {
                for (size_t j = 0; j < x->inputs.size(); j++)
                {
                    if (x->inputs[j] == type_as_out)
                        x->inputs[j] = op->inputs[0];
                }

                op->inputs[0]->consumers.push_back(x);
            }

            op->inputs[0]->name = type_as_out->name;

            type_as_out->producer = 0;
            type_as_out->consumers.clear();

            graph.operands.erase(std::find(graph.operands.begin(), graph.operands.end(), type_as_out));
            delete type_as_out;

            op->inputs.clear();
            op->outputs.clear();

            graph.ops.erase(graph.ops.begin() + i);
            delete op;

            break;
        }

        if (!matched)
            break;
    }
}

} // namespace pnnx
