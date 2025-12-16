// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/RenderPlan.h"
#include "core/primitives/AudioUnit.h"
#include "core/Project.h"
#include "logger.h"

#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <iostream>


namespace slr {

void copyMap(const int8_t *src, int8_t *dst) {
    for(int i=0; i<32; ++i) {
        dst[i] = src[i];
    }
}

void destroyPlan(const RenderPlan * plan) {
    for(uint32_t i=0; i<plan->nodesCount; ++i) {
        RenderPlan::Node & n = plan->nodes[i];
        delete [] n.deps.audio;
        delete [] n.deps.midi;
    }
    delete [] plan->nodes;
    delete plan;
}

RenderPlan * buildPlan(Project *prj) {
    //1. find all targets to render
    std::vector<ID> targets;
    const std::vector<AudioRoute> &audioRoutes = prj->routes();
    const std::vector<MidiRoute> &midiRoutes = prj->midiRoutes();
    {
        for(std::size_t i=0; i<audioRoutes.size(); ++i) {
            const AudioRoute &r = audioRoutes.at(i);
            if(r._sourceType == AudioRoute::Type::INT)
                targets.push_back(r._sourceId);
            if(r._targetType == AudioRoute::Type::INT)
                targets.push_back(r._targetId);
        }

        for(std::size_t i=0; i<midiRoutes.size(); ++i) {
            const MidiRoute &r = midiRoutes.at(i);
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
        for(const MidiRoute &mr : midiRoutes) {
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

        if (out.size() != indegree.size())
            throw std::runtime_error("cycle detected");

        return out;
    };

    std::vector<ID> order = topoSort(edges, indegree);
    LOG_WARN("Render Order: ");
    for(ID & id : order) {
        std::cout << id << " ";
    }
    std::cout << std::endl;
    LOG_WARN("End Render order");

    //build plan
    RenderPlan * plan = new RenderPlan;
    if(audioRoutes.size() + midiRoutes.size() == 0) {
        plan->nodes = nullptr;
        plan->nodesCount = 0;
        plan->outputDeps.audio = nullptr;
        plan->outputDeps.audioDepsCnt = 0;
        plan->outputDeps.midi = nullptr;
        plan->outputDeps.midiDepsCnt = 0;
        return plan;
    }

    //build nodes
    RenderPlan::Node * renderNodes = new RenderPlan::Node[order.size()];
    plan->nodes = renderNodes;
    plan->nodesCount = static_cast<uint32_t>(order.size());

    for(std::size_t i=0; i<order.size(); ++i) {
        const ID &id = order.at(i);
        RenderPlan::Node &rnode = renderNodes[i];
        
        AudioUnit *unit = prj->getUnitById(id);
        if(!unit) {
            LOG_FATAL("Failed to find unit id %d during Render Plan building", id);
        }

        rnode.target = unit;

        NodeInt * node = nullptr;
        for(NodeInt &no : nodes) {
            if(no.id == id) {
                node = &no;
                break;
            }
        }

        if(!node) {
            LOG_FATAL("Failed to find node for id %d during Render Plan building", id);
        }

        //audio dependencies
        std::size_t aDepsSize = node->audioDeps.size();
        rnode.deps.audio = aDepsSize ?
            new AudioDependencie[aDepsSize] :
            nullptr;
        rnode.deps.audioDepsCnt = static_cast<uint32_t>(aDepsSize);

        for(std::size_t j=0; j<aDepsSize; ++j) {
            AudioDependencie &dep = rnode.deps.audio[j];
            const NodeInt::AudioDeps &adep = node->audioDeps.at(j);

            if(adep.type == AudioRoute::Type::EXT) {
                dep.external = true;
                dep.extId = adep.id;
                dep.buffer = nullptr;
            } else {
                dep.external = false;
                dep.extId = 0;
                
                AudioUnit *u = prj->getUnitById(adep.id);
                if(!u) {
                    LOG_FATAL("Failed to find unit id %d during Render Plan building", adep.id);
                }
                dep.buffer = u->outputs();
            }
            copyMap(&adep.channelMap[0], &dep.channelMap[0]);
        }

        //midi dependencies
        std::size_t mDepsSize = node->midiDeps.size();
        rnode.deps.midi = mDepsSize ? 
            new MidiDependencie[mDepsSize] :
            nullptr;
        rnode.deps.midiDepsCnt = static_cast<uint32_t>(mDepsSize);

        for(std::size_t j=0; j<mDepsSize; ++j) {
            MidiDependencie &dep = rnode.deps.midi[j];
            const NodeInt::MidiDeps &mdep = node->midiDeps.at(j);

            if(mdep.type == MidiRoute::Type::EXT) {
                dep.external = true;
                dep.extId = mdep.id;
                dep.buf = nullptr;
            } else {
                dep.external = false;
                dep.extId = 0;

                AudioUnit *u = prj->getUnitById(mdep.id);
                if(!u) {
                    LOG_FATAL("Failed to find unit id %d during Render Plan building", mdep.id);
                }
                dep.buf = u->midiOutputs();
            }
            dep.channelFrom = mdep.channelFrom;
            dep.channelTo = mdep.channelTo;
        }
    }

    //build output
    Dependencies &outDeps = plan->outputDeps;
    uint32_t outputCnt = 0;

    for(NodeInt &n : nodes) {
        if(n.audioGoesToOutput) {
            outputCnt++;
        }
    }

    outDeps.audioDepsCnt = outputCnt;
    outDeps.audio = outputCnt ? 
            new AudioDependencie[outputCnt] : 
            nullptr;

    int processed = 0;

    // for(uint32_t i=0; i<outputCnt; ++i) {
    //     AudioDependencie &dep = outDeps.audio[i];
    for(NodeInt &n : nodes) {
        if(!n.audioGoesToOutput) continue;
        AudioDependencie &dep = outDeps.audio[processed];

        dep.external = false;
        dep.extId = 0;

        AudioUnit *u = prj->getUnitById(n.id);
        if(!u) {
            LOG_FATAL("Failed to find unit id %d duting Render Plan building", n.id);
        }

        dep.buffer = u->outputs();
        copyMap(&n.outputChannelMap[0], &dep.channelMap[0]);
        processed++;
    }
    // }

    if(processed != outputCnt) {
        LOG_FATAL("Failed to build output dependencies, %d resolved instead of %d", processed, outputCnt);
    }

    //no midi for now
    outDeps.midi = nullptr;
    outDeps.midiDepsCnt = 0;

    return plan;
}

}