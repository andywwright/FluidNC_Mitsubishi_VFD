// Copyright (c) 2021 -	Stefan de Bruijn
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include <vector>
#include "../Logging.h"

#include "HandlerBase.h"
#include <algorithm>

namespace Configuration {
    template <typename BaseType>
    class GenericFactory {
        static GenericFactory& instance() {
            static GenericFactory instance_;
            return instance_;
        }

        GenericFactory() = default;

        GenericFactory(const GenericFactory&)            = delete;
        GenericFactory& operator=(const GenericFactory&) = delete;

        class BuilderBase {
            const char* name_;

        public:
            BuilderBase(const char* name) : name_(name) {}

            BuilderBase(const BuilderBase& o)            = delete;
            BuilderBase& operator=(const BuilderBase& o) = delete;

            virtual BaseType* create() const = 0;
            const char*       name() const { return name_; }

            virtual ~BuilderBase() = default;
        };

        std::vector<BuilderBase*> builders_;

        inline static void registerBuilder(BuilderBase* builder) { instance().builders_.push_back(builder); }

    public:
        template <typename DerivedType>
        class InstanceBuilder : public BuilderBase {
        public:
            explicit InstanceBuilder(const char* name) : BuilderBase(name) { instance().registerBuilder(this); }

            BaseType* create() const override { return new DerivedType(); }
        };

        static void factory(Configuration::HandlerBase& handler, BaseType*& inst) {
            if (inst == nullptr) {
                auto& builders = instance().builders_;
                auto  it       = std::find_if(
                    builders.begin(), builders.end(), [&](auto& builder) { return handler.matchesUninitialized(builder->name()); });
                if (it != builders.end()) {
                    inst = (*it)->create();
                    handler.enterFactory((*it)->name(), *inst);
                }
            } else {
                handler.enterSection(inst->name(), inst);
            }
        }
        static void factory(Configuration::HandlerBase& handler, std::vector<BaseType*>& inst) {
            if (handler.handlerType() == HandlerType::Parser) {
                auto& builders = instance().builders_;
                auto  it       = std::find_if(
                    builders.begin(), builders.end(), [&](auto& builder) { return handler.matchesUninitialized(builder->name()); });
                if (it != builders.end()) {
                    auto product = (*it)->create();
                    inst.push_back(product);
                    handler.enterFactory((*it)->name(), *product);
                }
            } else {
                for (auto it : inst) {
                    handler.enterSection(it->name(), it);
                }
            }
        }
    };
}
