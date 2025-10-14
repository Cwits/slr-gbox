// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Project.h"
#include "core/primitives/RenderPlan.h"
#include "core/primitives/AudioRoute.h"
#include "modules/Track/Track.h"
#include "core/Mixer.h"

#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <condition_variable>
#include <mutex>

TEST(RenderPlan, buildingPlan) {
    using namespace slr;

    Project * prj = new Project;

    //TODO: valgrind reports definitely lost: 1,024 bytes in 32 blocks
    // in track creating
    std::unique_ptr<Track> track1 = std::make_unique<Track>();
    AudioUnit * track1Unit = static_cast<AudioUnit*>(track1.get());
    std::unique_ptr<Track> track2 = std::make_unique<Track>();
    AudioUnit * track2Unit = static_cast<AudioUnit*>(track2.get());

    prj->addUnit(std::move(track1));
    prj->addUnit(std::move(track2));

    {
        AudioRoute routeToMix;
        //ext to track1
        routeToMix._sourceType = AudioRoute::Type::EXT;
        routeToMix._sourceId = 0;
        routeToMix._targetType = AudioRoute::Type::INT;
        routeToMix._targetId = track1Unit->id();
        routeToMix._channelMap[0] = 0;
        routeToMix._channelMap[1] = 1;
        prj->addRoute(routeToMix);
        //track1 to mixer
        routeToMix._sourceType = AudioRoute::Type::INT;
        routeToMix._sourceId = track1Unit->id();
        routeToMix._targetType = AudioRoute::Type::INT;
        routeToMix._targetId = prj->mixer()->id();
        routeToMix._channelMap[0] = 0;
        routeToMix._channelMap[1] = 1;
        prj->addRoute(routeToMix);
        //track2 to mixer
        routeToMix._sourceType = AudioRoute::Type::INT;
        routeToMix._sourceId = track2Unit->id();
        routeToMix._targetType = AudioRoute::Type::INT;
        routeToMix._targetId = prj->mixer()->id();
        routeToMix._channelMap[0] = 0;
        routeToMix._channelMap[1] = 1;
        prj->addRoute(routeToMix);
        //mixer to ext added in project constructor
    }

    RenderPlan * expected = new RenderPlan;
    {
        Dependencies * deps = new Dependencies[1];
        deps->external = true;
        deps->extId = 0;
        deps->buffer = nullptr;
        deps->channelMap[0] = 0;
        deps->channelMap[1] = 1;

        RenderPlan::Node * nodes = new RenderPlan::Node[2];
        nodes[0].target = track1Unit;
        nodes[0].deps = deps;
        nodes[0].depsCount = 1;

        nodes[1].target = track2Unit;
        nodes[1].deps = nullptr;
        nodes[1].depsCount = 0;

        expected->nodes = nodes;
        expected->nodesCount = 2;

        Dependencies * mixDeps = new Dependencies[2];
        mixDeps[0].external = false;
        mixDeps[0].extId = -1;
        mixDeps[0].buffer = track1Unit->outputs();
        mixDeps[0].channelMap[0] = 0;
        mixDeps[0].channelMap[1] = 1;
        mixDeps[1].external = false;
        mixDeps[1].extId = -1;
        mixDeps[1].buffer = track2Unit->outputs();
        mixDeps[1].channelMap[0] = 0;
        mixDeps[1].channelMap[1] = 1;

        expected->mixerDeps = mixDeps;
        expected->mixerDepsCount = 2;
    }

    RenderPlan * created = buildPlan(prj, prj->routes());

    //check regular
    ASSERT_EQ(created->nodesCount, expected->nodesCount);
    uint32_t count = created->nodesCount;
    for(uint32_t i=0; i<count; ++i) {
        RenderPlan::Node * nodes = created->nodes;
        RenderPlan::Node * expnodes = expected->nodes;
        ASSERT_EQ(nodes[i].target, expnodes[i].target);

        if(nodes[i].depsCount != 0) {
            ASSERT_EQ(nodes[i].depsCount, expnodes[i].depsCount);
            for(uint32_t d=0; d<nodes[i].depsCount; ++d) {
                Dependencies *dps = nodes[i].deps;
                Dependencies *expdps = expnodes[i].deps;

                ASSERT_EQ(dps[d].external, expdps[d].external);
                ASSERT_EQ(dps[d].extId, expdps[d].extId);
                ASSERT_EQ(dps[d].buffer, expdps[d].buffer);
                for(int z=0; z<32; ++z) {
                    ASSERT_EQ(dps->channelMap[z], expdps->channelMap[z]);
                }
            }
        } else {
            ASSERT_EQ(nodes[i].deps, nullptr);
        }
    }

    //check mixer
    ASSERT_EQ(created->mixerDepsCount, expected->mixerDepsCount);
    Dependencies * dep = created->mixerDeps;
    Dependencies * exp = expected->mixerDeps;
    for(uint32_t i=0; i<created->mixerDepsCount; ++i) {
        ASSERT_EQ(dep[i].external, exp[i].external);
        ASSERT_EQ(dep[i].extId, exp[i].extId);
        ASSERT_EQ(dep[i].buffer, exp[i].buffer);
        for(int z=0; z<32; ++z) {
            ASSERT_EQ(dep[i].channelMap[z], exp[i].channelMap[z]);
        }
    }

    destroyPlan(created);

    {
        delete [] expected->mixerDeps;
        for(uint32_t i=0; i<expected->nodesCount; ++i) {
            delete [] expected->nodes[i].deps;
        }
        delete [] expected->nodes;
    }

    delete expected;
    delete prj;
}



int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}