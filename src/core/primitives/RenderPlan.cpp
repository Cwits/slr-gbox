// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/RenderPlan.h"
#include "core/primitives/AudioUnit.h"
#include "core/Mixer.h"
#include "core/Project.h"
#include "logger.h"

#include <vector>
#include <set>
#include <unordered_set>


namespace slr {

void copyMap(const int8_t *src, int8_t *dst) {
    for(int i=0; i<32; ++i) {
        dst[i] = src[i];
    }
}

/*
RenderPlan * buildPlan(Project * prj, const std::vector<AudioRoute> & routes) {
    //find everything except mixer and add to plan.
    //if there is some connection from ext - add as deps.

    //ext -> track1 -> mixer -> ext
    //       track2 -> mixer -> ext
    
    //     1. mixer to ext route
    //     2. track1 to mixer route
    //     3. track2 to mixer route
    //     4. ext to track1 route
   

    // so what i want:
    // 1. find all unique render targets
    // 2. resolve dependencies for targets
    // 3. resolve dependencies for mixer
    // 4. build plan

    std::size_t count = routes.size();
    ID mixerId = prj->mixer()->id();
    //1.
    std::vector<RenderPlan::Node> targets;
    for(std::size_t i=0; i<count; ++i) {
        const AudioRoute &r = routes.at(i);

        bool isInternal = false;
        bool srctgt = false; //false -> source, true -> target
        if(r._sourceType == AudioRoute::Type::INT) {
            isInternal = true;
            srctgt = false;
        } else if(r._targetType == AudioRoute::Type::INT) {
            isInternal = true;
            srctgt = true;
        }

        if(isInternal) {
            bool found = false;
            for(auto &target : targets) {
                ID id = (srctgt ? r._targetId : r._sourceId);
                if(target.target->id() == id) {
                    found = true;
                }
            }

            if(!found) {
                ID id = (srctgt ? r._targetId : r._sourceId);
                if(id == mixerId) continue;

                RenderPlan::Node n;
                n.target = prj->getUnitById(id);
                n.deps = nullptr;
                n.depsCount = 0;
                targets.push_back(n);
            }
        }

    }

    //2. resolve node dependencies
    for(RenderPlan::Node &node : targets) {
        ID targetId = node.target->id();
        std::vector<Dependencies> nodeDeps;

        for(const AudioRoute &r : routes) {
           if(r._targetId == targetId && r._targetType == AudioRoute::Type::INT) {
                Dependencies d;
                if(r._sourceType == AudioRoute::Type::EXT) {
                    d.external = true;
                    d.extId = 0;
                    d.buffer = nullptr;
                } else if(r._sourceType == AudioRoute::Type::INT) {
                    d.external = false;
                    d.extId = -1;
                    d.buffer = prj->getUnitById(r._sourceId)->outputs();
                }
                for(int i=0; i<32; ++i) {
                    d.channelMap[i] = r._channelMap[i];
                }
                // d.channelMap[0] = 0;
                // d.channelMap[1] = 1;
                nodeDeps.push_back(d);
            }
        }

        std::size_t depssize = nodeDeps.size();
        if(depssize != 0) {
            Dependencies * dps = new Dependencies[depssize];
            for(std::size_t i=0; i<depssize; ++i) {
                Dependencies & depp = nodeDeps.at(i);
                dps[i].external = depp.external;
                dps[i].extId = depp.extId;
                dps[i].buffer = depp.buffer;
                for(int z=0; z<32; ++z) {
                    dps[i].channelMap[z] = depp.channelMap[z];
                }
            }
            node.deps = dps;
            node.depsCount = static_cast<uint32_t>(depssize);
        } else {
            node.deps = nullptr;
            node.depsCount = 0;
        }
    }

    //3 resolve mixer dependencies
    std::vector<Dependencies> mixerDeps;
    for(const AudioRoute &r : routes) {
        if(r._targetId == mixerId && r._targetType == AudioRoute::Type::INT) {
            Dependencies md;
            if(r._sourceType == AudioRoute::Type::EXT) {
                md.external = true;
                md.extId = 0;
                md.buffer = nullptr;
            } else if(r._sourceType == AudioRoute::Type::INT) {
                md.external = false;
                md.extId = -1;
                md.buffer = prj->getUnitById(r._sourceId)->outputs();
            }
            for(int i=0; i<32; ++i) {
                md.channelMap[i] = r._channelMap[i];
            }
            // md.channelMap[0] = 0;
            // md.channelMap[1] = 1;
            mixerDeps.push_back(md);
        }
    }


    //4. build plan
    RenderPlan * ret = new RenderPlan;

    std::size_t nodecnt = targets.size();
    RenderPlan::Node * nodes = new RenderPlan::Node[nodecnt];
    for(std::size_t i=0; i<nodecnt; ++i) {
        RenderPlan::Node & nd = targets.at(i);
        nodes[i].target = nd.target;
        nodes[i].deps = nd.deps;
        nodes[i].depsCount = nd.depsCount;
    }

    ret->nodes = nodes;
    ret->nodesCount = static_cast<uint32_t>(nodecnt);

    std::size_t mixDepsSize = mixerDeps.size();
    Dependencies * mixDeps = new Dependencies[mixDepsSize];
    for(std::size_t i=0; i<mixDepsSize; ++i) {
        Dependencies & md = mixerDeps.at(i);
        mixDeps[i].external = md.external;
        mixDeps[i].extId = md.extId;
        mixDeps[i].buffer = md.buffer;
        for(int z=0; z<32; ++z) {
            mixDeps[i].channelMap[z] = md.channelMap[z];
        }
    }

    ret->mixerDeps = mixDeps;
    ret->mixerDepsCount = static_cast<uint32_t>(mixDepsSize);

    return ret;
}
*/
RenderPlan * buildPlan(Project * prj, const std::vector<AudioRoute> & routes) {
    //1. find all unique targets
    std::size_t routesCount = routes.size();
    std::vector<ID> targets;

    //find all targets
    for(std::size_t i=0; i<routesCount; ++i) {
        const AudioRoute &r = routes.at(i);

        if(r._sourceType == AudioRoute::Type::INT) {
            targets.push_back(r._sourceId);
        }
        if(r._targetType == AudioRoute::Type::INT) {
            targets.push_back(r._targetId);
        }
    }

    //get rid of duplicate id
    {
        std::set<ID> targetstmp;
        std::size_t tmpsize = targets.size();
        for(std::size_t i=0; i<tmpsize; ++i) targetstmp.insert(targets.at(i));
        targets.clear();
        targets.assign(targetstmp.begin(), targetstmp.end());
    }

    struct Node {
        struct NodeDep {
            AudioRoute::Type type;
            ID id;
            int8_t channelMap[32] = {-1};
        };

        bool goesToOutput;
        int8_t outputMap[32] = {-1};
        // std::vector<ID> outputsId;

        ID id;
        std::vector<NodeDep> dependencies;
    };

    std::vector<Node> nodes;
    //init
    for(std::size_t i=0; i<targets.size(); ++i) {
        Node n;
        n.id = targets.at(i);
        n.goesToOutput = false;
        nodes.push_back(n);
    }

    //build nodes
    for(Node & n : nodes) {
        for(std::size_t i=0; i<routesCount; ++i) {
            const AudioRoute &r = routes.at(i);

            if(r._sourceId == n.id && r._sourceType == AudioRoute::Type::INT) {
                if(r._targetType == AudioRoute::Type::EXT) {
                    n.goesToOutput = true;
                    // Node::NodeDep d;
                    // d.type = r._targetType;
                    // d.id = r._targetId;
                    copyMap(&r._channelMap[0], &n.outputMap[0]);
                    // n.dependencies.push_back(d);
                    // n.outputsId.push_back(r._targetId);
                }
            }
            if(r._targetId == n.id && r._targetType == AudioRoute::Type::INT) {
                Node::NodeDep d;
                d.type = r._sourceType;
                d.id = r._sourceId;
                copyMap(&r._channelMap[0], &d.channelMap[0]);
                n.dependencies.push_back(d);
            }
        }
    }

    //build order
    std::vector<ID> processingOrder;
    std::unordered_set<ID> resolved;
    while (processingOrder.size() < nodes.size()) {
        bool progress = false;

        for (auto& n : nodes) {
            if (resolved.count(n.id))
                continue;

            bool depsReady = true;
            for (auto& dep : n.dependencies) {
                if (dep.type == AudioRoute::Type::INT && !resolved.count(dep.id)) {
                    depsReady = false;
                    break;
                }
            }

            if (depsReady) {
                processingOrder.push_back(n.id);
                resolved.insert(n.id);
                progress = true;
            }
        }

        if (!progress) {
            // loop or error
            LOG_WARN("Building render plan error!");
            break;
        }
    }

    //build plan
    RenderPlan * plan = new RenderPlan;
    if(routesCount == 0) {
        plan->nodes = nullptr;
        plan->nodesCount = 0;
        plan->outputDeps = nullptr;
        plan->outputDepsCount = 0;
        return plan;
    }
    
    RenderPlan::Node * rendernodes = new RenderPlan::Node[processingOrder.size()];

    plan->nodes = rendernodes;
    plan->nodesCount = static_cast<uint32_t>(processingOrder.size());

    for(std::size_t i=0; i<processingOrder.size(); ++i) {
        const ID & id = processingOrder.at(i);
        RenderPlan::Node * n = &rendernodes[i];

        AudioUnit * unit = prj->getUnitById(id);
        if(!unit) {
            LOG_FATAL("Failed to find unit");
        }
        n->target = unit;
        
        Node * node = nullptr;
        for(Node & no : nodes) {
            if(no.id == id) {
                node = &no;
                break;
            }
        }

        if(!node) {
            LOG_FATAL("Failed2");
        }

        n->depsCount = static_cast<uint32_t>(node->dependencies.size());

        if(n->depsCount == 0) {
            n->deps = nullptr;
            continue;
        }

        Dependencies * deps = new Dependencies[node->dependencies.size()];
        for(std::size_t ii=0; ii<node->dependencies.size(); ++ii) {
            Node::NodeDep & dep = node->dependencies.at(ii);
            if(dep.type == AudioRoute::Type::EXT) {
                deps[ii].external = true;
                deps[ii].extId = static_cast<uint16_t>(dep.id);
                deps[ii].buffer = nullptr;
            } else if(dep.type == AudioRoute::Type::INT) {
                deps[ii].external = false;
                AudioUnit * tmpunit = prj->getUnitById(dep.id);
                if(!tmpunit) {
                    LOG_FATAL("Failed3");
                }
                deps[ii].buffer = tmpunit->outputs();
                deps[ii].extId = 0;
            }
            copyMap(&dep.channelMap[0], &deps[ii].channelMap[0]);
        }

        n->deps = deps;
    }

    int outputCount = 0;
    for(Node & n : nodes) {
        if(n.goesToOutput) 
            outputCount++;
    }

    plan->outputDepsCount = static_cast<uint32_t>(outputCount);
    if(outputCount == 0) {
        plan->outputDeps = nullptr;
    } else {
        Dependencies * outDeps = new Dependencies[outputCount];
        int processed = 0;
        for(int i=0; i<outputCount; ++i) {
            for(Node & n : nodes) {
                if(!n.goesToOutput) continue;
                outDeps[processed].external = false;
                outDeps[processed].extId = 0;
                AudioUnit * unit = prj->getUnitById(n.id);
                if(!unit) LOG_FATAL("Failed5");
                outDeps[processed].buffer = unit->outputs();

                copyMap(&n.outputMap[0], &outDeps[processed].channelMap[0]);

            }
            processed++;
        }

        if(processed != outputCount) {
            LOG_FATAL("Failed4");
        }

        plan->outputDeps = outDeps;
    }

    int i=15;
    i+=15;


    return plan;
}

void destroyPlan(const RenderPlan * plan) {
    for(uint32_t i=0; i<plan->nodesCount; ++i) {
        delete [] plan->nodes[i].deps;
    }

    delete [] plan->outputDeps;
    delete [] plan->nodes;
    // delete [] plan->mixerDeps;
    delete plan;
}

}