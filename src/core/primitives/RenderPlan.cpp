// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/primitives/RenderPlan.h"
#include "core/primitives/AudioUnit.h"
#include "core/Mixer.h"
#include "core/Project.h"


namespace slr {


RenderPlan * buildPlan(Project * prj, const std::vector<AudioRoute> & routes) {
    //find everything except mixer and add to plan.
    //if there is some connection from ext - add as deps.

    //ext -> track1 -> mixer -> ext
    //       track2 -> mixer -> ext
    /* 
        1. mixer to ext route
        2. track1 to mixer route
        3. track2 to mixer route
        4. ext to track1 route
   

    so what i want:
    1. find all unique render targets
    2. resolve dependencies for targets
    3. resolve dependencies for mixer
    4. build plan
    */
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

void destroyPlan(const RenderPlan * plan) {
    for(uint32_t i=0; i<plan->nodesCount; ++i) {
        delete [] plan->nodes[i].deps;
    }

    delete [] plan->mixerDeps;
    delete [] plan->nodes;
    // delete [] plan->mixerDeps;
    delete plan;
}

}