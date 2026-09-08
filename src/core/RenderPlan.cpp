// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/RenderPlan.h"
#include "core/primitives/AudioUnit.h"
#include "core/primitives/AudioBuffer.h"
#include "core/primitives/MidiBuffer.h"
#include "core/primitives/MidiEvent.h"
#include "core/Project.h"
#include "core/ModulationEngine.h"
#include "core/StepSequencer.h"
#include "core/utility/ControlContext.h"
#include "common/logger.h"

#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <algorithm>


namespace slr {

void copyMap(const int8_t *src, int8_t *dst) {
    for(int i=0; i<32; ++i) {
        dst[i] = src[i];
    }
}

namespace PlanBuilder {

bool clearPlan(RenderPlan * plan) {
    plan->timeline = nullptr;
    plan->metro = nullptr;
    plan->nodes = nullptr;
    plan->nodesCount = 0;
    plan->modulationPatterns = nullptr;
    plan->modPatternCount = 0;
    plan->sequences = nullptr;
    plan->sequenceCount = 0;
    return true;
}

bool clearHolder(PlanHolder *holder) {
    clearPlan(holder->plan.get());
    holder->nodes.clear();
    for(auto &h : holder->nodeDepHolder) {
        h.audioDep.clear();
        h.midiDep.clear();
    }
    holder->nodeDepHolder.clear();
    holder->outDeps.audioDep.clear();
    holder->outDeps.midiDep.clear();
    holder->modPatterns.clear();
    holder->sequences.clear();
    return true;
}

bool buildUnits(ControlContext &ctx, PlanHolder *holder) {
    //1. find all targets to render
    std::vector<ID> targets;
    const std::vector<AudioRoute> &audioRoutes = ctx.project->routes();
    const std::vector<MidiRoute> &midiRoutess = ctx.project->midiRoutes();
    {
        for(std::size_t i=0; i<audioRoutes.size(); ++i) {
            const AudioRoute &r = audioRoutes.at(i);
            if(r._sourceType == AudioRoute::Type::INT)
                targets.push_back(r._sourceId);
            if(r._targetType == AudioRoute::Type::INT)
                targets.push_back(r._targetId);
        }

        for(std::size_t i=0; i<midiRoutess.size(); ++i) {
            const MidiRoute &r = midiRoutess.at(i);
            if(r._sourceType == MidiRoute::Type::INT)
                targets.push_back(r._sourceId);
            if(r._targetType == MidiRoute::Type::INT)
                targets.push_back(r._targetId);
        }

        //filter 'em up to get rid of duplicates
        std::set<ID> tmp;
        for(std::size_t i=0; i<targets.size(); ++i) tmp.insert(targets.at(i));
        targets.clear();
        targets.assign(tmp.begin(), tmp.end());
    }

    struct NodeInt {
        ID id;

        struct AudioDeps {
            AudioRoute::Type type;

            ID id;
            int8_t channelMap[32];
        };

        struct MidiDeps {
            MidiRoute::Type type;

            ID id;
            uint8_t channelFrom;
            uint8_t channelTo;
        };

        std::vector<AudioDeps> audioDeps;
        std::vector<MidiDeps> midiDeps;

        bool audioGoesToOutput;
        int8_t outputChannelMap[32];

        bool midiGoesToOutput;
    };

    //init
    std::vector<NodeInt> nodes;
    for(std::size_t i=0; i<targets.size(); ++i) {
        NodeInt n;
        n.id = targets.at(i);
        n.audioGoesToOutput = false;
        n.midiGoesToOutput = false;
        nodes.push_back(n);
    }

    //build nodes
    for(NodeInt &n : nodes) {
        //audio dependencies
        /* 
        cases:
        1. EXT to node
        2. INT to node
        3. node to EXT
        */
        for(const AudioRoute &ar : audioRoutes) {
            if(ar._sourceId == n.id && ar._sourceType == AudioRoute::Type::INT) {
                //node sending smth out, we are interested only in send to ext
                if(ar._targetType == AudioRoute::Type::EXT) {
                    n.audioGoesToOutput = true;
                    copyMap(&ar._channelMap[0], &n.outputChannelMap[0]);
                }
            }
            if(ar._targetId == n.id && ar._targetType == AudioRoute::Type::INT) {
                //node receiving from somewhere...
                NodeInt::AudioDeps adep;
                adep.type = ar._sourceType;
                adep.id = ar._sourceId;
                copyMap(&ar._channelMap[0], &adep.channelMap[0]);

                n.audioDeps.push_back(adep);
            }
        }

        //midi dependencies
        for(const MidiRoute &mr : midiRoutess) {
            if(mr._sourceId == n.id && mr._sourceType == MidiRoute::Type::INT) {
                if(mr._targetType == MidiRoute::Type::EXT) {
                    n.midiGoesToOutput = true;
                }
            }
            if(mr._targetId == n.id && mr._targetType == MidiRoute::Type::INT) {
                NodeInt::MidiDeps mdep;
                mdep.type = mr._sourceType;
                mdep.id = mr._sourceId;
                mdep.channelFrom = mr._sourceChannel;
                mdep.channelTo = mr._targetChannel;
                n.midiDeps.push_back(mdep);
            }
        }
    }

    //build nodes order
    std::unordered_map<ID, std::vector<ID>> edges;
    std::unordered_map<ID, int> indegree;

    for(NodeInt &n : nodes) {
        for(auto &ad : n.audioDeps) {
            if(ad.type == AudioRoute::Type::EXT) continue;

            edges[ad.id].push_back(n.id);
            indegree[n.id]++;
        }
        for(auto &md : n.midiDeps) {
            if(md.type == MidiRoute::Type::EXT) continue;

            edges[md.id].push_back(n.id);
            indegree[n.id]++;
        }

        indegree.try_emplace(n.id, 0);
    }

    auto topoSort = [](
        const std::unordered_map<ID, std::vector<ID>>& edges,
        std::unordered_map<ID, int> indegree) -> std::vector<ID>
    {
        std::queue<ID> q;
        for (auto& [n, deg] : indegree)
            if (deg == 0)
                q.push(n);

        std::vector<ID> out;
        out.reserve(indegree.size());

        while (!q.empty()) {
            ID n = q.front();
            q.pop();
            out.push_back(n);

            auto it = edges.find(n);
            if (it == edges.end()) continue;

            for (int to : it->second) {
                if (--indegree[to] == 0)
                    q.push(to);
            }
        }

        if (out.size() != indegree.size()) {
            // throw std::runtime_error("cycle detected");
            LOG_ERROR("Cyclic dependencie, abort building plan");
            return std::vector<ID>();
        }

        return out;
    };

    std::vector<ID> order = topoSort(edges, indegree);
    if(order.size() == 0) {
        //cycle detected or there is no ID's
        LOG_WARN("No items to render, returning dummy Render Plan");
        return false; 
    }

    /*
    LOG_WARN("Render Order: ");
    for(ID & id : order) {
        std::cout << id << " ";
    }
    std::cout << std::endl;
    LOG_WARN("End Render order");
    */

    //Build plan

    //Build Nodes
    if(holder->nodes.capacity() < order.size()) {
        holder->nodes.reserve(order.size());
    }

    if(holder->nodeDepHolder.capacity() < order.size()) {
        holder->nodeDepHolder.reserve(order.size());
    }

    for(std::size_t i=0; i<order.size(); ++i) {
        const ID &id = order.at(i);
        RenderPlan::Node rnode;// = holder->nodes[i];

        const AudioUnit *unit = ctx.project->getUnitById(id);
        if(!unit) {
            LOG_ERROR("Failed to find unit &d during building plan", id);
            return false;
        }

        rnode.target = unit;
        auto found = std::find_if(
            nodes.begin(),
            nodes.end(),
            [id](const NodeInt& n) {
                return n.id == id;
            }
        );

        if(found == nodes.end()) {
            LOG_ERROR("Failed to find node for id %d");
            return false;
        }

        DepHolder nodeDeps;
        //audio dependencies for node
        if(found->audioDeps.size()) {
            std::vector<AudioDependencie> dvec;// = holder->nodeDepHolder[i].audioDep;
            if(dvec.capacity() < found->audioDeps.size()) {
                dvec.reserve(found->audioDeps.size());
            }

            for(std::size_t ad=0; ad<found->audioDeps.size(); ++ad) {
                AudioDependencie dep;// = holder->audioDeps[ad];
                const NodeInt::AudioDeps &adep = found->audioDeps.at(ad);

                if(adep.type == AudioRoute::Type::EXT) {
                    dep.external = true;
                    dep.extId = adep.id;
                    dep.buffer = nullptr;
                } else {
                    dep.external = false;
                    dep.extId = 0;
                    
                    AudioUnit *u = ctx.project->getUnitById(adep.id);
                    if(!u) {
                        LOG_FATAL("Failed to find unit id %d during Render Plan building", adep.id);
                    }
                    dep.buffer = u->outputs();
                }
                copyMap(&adep.channelMap[0], &dep.channelMap[0]);

                dvec.push_back(dep);
            }

            // holder->nodeDepHolder[i].audioDep = dvec;
            nodeDeps.audioDep = dvec;
            // rnode.deps.audio = holder->nodeDepHolder[i].audioDep.data();
            // rnode.deps.audioDepsCnt = static_cast<uint32_t>(dvec.size());
        }

        //midi dependencies
        if(found->midiDeps.size()) {
            std::vector<MidiDependencie> mvec;// = holder->nodeDepHolder[i].midiDep;
            if(mvec.capacity() < found->midiDeps.size()) {
                mvec.reserve(found->midiDeps.size());
            }

            for(std::size_t md=0; md<found->midiDeps.size(); ++md) {
                MidiDependencie dep;
                const NodeInt::MidiDeps &mdep = found->midiDeps.at(md);
                
                
                if(mdep.type == MidiRoute::Type::EXT) {
                    dep.external = true;
                    dep.extId = mdep.id;
                    dep.buf = nullptr;
                } else {
                    dep.external = false;
                    dep.extId = 0;

                    AudioUnit *u = ctx.project->getUnitById(mdep.id);
                    if(!u) {
                        LOG_FATAL("Failed to find unit id %d during Render Plan building", mdep.id);
                    }
                    dep.buf = u->midiOutputs();
                }
                dep.channelFrom = mdep.channelFrom;
                dep.channelTo = mdep.channelTo;

                mvec.push_back(dep);
            }

            // holder->nodeDepHolder[i].midiDep = mvec;
            // rnode.deps.midi = holder->nodeDepHolder[i].midiDep.data();
            // rnode.deps.midiDepsCnt = static_cast<uint32_t>(mvec.size());
            nodeDeps.midiDep = mvec;
        }

        holder->nodeDepHolder.push_back(nodeDeps);
        rnode.deps.audio = holder->nodeDepHolder[i].audioDep.data();
        rnode.deps.audioDepsCnt = holder->nodeDepHolder[i].audioDep.size();
        rnode.deps.midi = holder->nodeDepHolder[i].midiDep.data();
        rnode.deps.midiDepsCnt = holder->nodeDepHolder[i].midiDep.size();

        holder->nodes.push_back(rnode);
    }


    holder->plan->nodes = holder->nodes.data();
    holder->plan->nodesCount = holder->nodes.size();
    
    //output dependencies
    // Dependencies &outDeps = plan->outputDeps;
    uint32_t outputCnt = 0;

    for(NodeInt &n : nodes) {
        if(n.audioGoesToOutput) {
            outputCnt++;
        }
    }

    int processed = 0;

    if(holder->outDeps.audioDep.capacity() < outputCnt) {
        holder->outDeps.audioDep.reserve(outputCnt);
    }

    for(NodeInt &n : nodes) {
        if(!n.audioGoesToOutput) continue;
        AudioDependencie dep;// = outDeps.audio[processed];

        dep.external = false;
        dep.extId = 0;

        AudioUnit *u = ctx.project->getUnitById(n.id);
        if(!u) {
            LOG_FATAL("Failed to find unit id %d duting Render Plan building", n.id);
        }

        dep.buffer = u->outputs();
        copyMap(&n.outputChannelMap[0], &dep.channelMap[0]);
        processed++;
        holder->outDeps.audioDep.push_back(dep);
    }
    // }

    if(processed != outputCnt) {
        LOG_FATAL("Failed to build output dependencies, %d resolved instead of %d", processed, outputCnt);
        return false;
    }

    //no midi for now
    // outDeps.midi = nullptr;
    // outDeps.midiDepsCnt = 0;
    holder->plan->outputDeps.audio = holder->outDeps.audioDep.data();
    holder->plan->outputDeps.audioDepsCnt = outputCnt;
    holder->plan->outputDeps.midi = nullptr;
    holder->plan->outputDeps.midiDepsCnt = 0;

    return true;
}
bool copyUnits(PlanHolder *dst, const PlanHolder *src) {
    
    return true;
}
bool buildModulations(ControlContext &ctx, PlanHolder *holder) {
    holder->modPatterns = ctx.project->modulationEngine()->allPatterns();
    holder->plan->modulationPatterns = holder->modPatterns.data()[0];
    holder->plan->modPatternCount = holder->modPatterns.size();
    return true;
}
bool copyModulations(PlanHolder *dst, const PlanHolder *src) {
    return true;
}
bool buildSequences(ControlContext &ctx, PlanHolder *holder) {
    holder->sequences = ctx.project->stepSequencer()->allSequences();
    holder->plan->sequences = holder->sequences.data()[0];
    holder->plan->sequenceCount = holder->sequences.size();
    return true;
}
bool copySequences(PlanHolder *dst, const PlanHolder *src) {
    return true;
}

}

}