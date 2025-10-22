// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/Project.h"
#include "core/primitives/RenderPlan.h"
#include "core/primitives/AudioRoute.h"
#include "core/primitives/AudioUnit.h"
#include "modules/Track/Track.h"
#include "modules/Mixer/Mixer.h"
// #include "core/Mixer.h"

#include <gtest/gtest.h>
#include <iostream>
#include <cstring>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <unordered_map>

struct Dummy : public slr::AudioUnit {
    Dummy() : slr::AudioUnit(slr::AudioUnitType::Effect) {

    }
    ~Dummy() {}

    RT_FUNC slr::frame_t process(const slr::AudioContext &ctx, const slr::Dependencies * const inputs, const uint32_t inputsCount) { 
        return 0;
    }

    RT_FUNC void prepareToPlay() {}
    RT_FUNC void prepareToRecord() {}
    RT_FUNC void stopPlaying() {}
    RT_FUNC void stopRecording() {}

};

bool compareMaps(int8_t * map1, int8_t * map2) {
    bool equal = true;
    for(int i=0; i<32; ++i) {
        if(map1[i] != map2[i]) {
            equal = false;
            break;
        }
    }

    return equal;
}

bool checkDependencies(slr::RenderPlan::Node &created, slr::RenderPlan::Node &expected) {
    using namespace slr;
    bool ret = true;

    if(created.depsCount != expected.depsCount) return false;

    int foundCount = 0;
    for(uint32_t i=0; i<created.depsCount; ++i) {
        Dependencies & crdep = created.deps[i];
        bool correct = false;

        for(uint32_t y=0; y<expected.depsCount; ++y) {
            Dependencies &exdep = expected.deps[y];

            if(crdep.external == exdep.external &&
                crdep.extId == exdep.extId &&
                crdep.buffer == exdep.buffer &&
                compareMaps(&crdep.channelMap[0], &exdep.channelMap[0])) {
                    correct = true;
                    foundCount++;
                }
        }

        if(!correct) {
            ret = false;
        }
    }

    if(foundCount != static_cast<int>(expected.depsCount)) {
        ret = false;
    }


    return ret;
}

TEST(RenderPlan, buildingPlan2) {
    using namespace slr;

    //creating...
    Project * prj = new Project;
    std::unique_ptr<Mixer> mixer = std::make_unique<Mixer>();
    std::unique_ptr<Track> track1 = std::make_unique<Track>();
    std::unique_ptr<Track> track2 = std::make_unique<Track>();
    AudioUnit * track1Unit = static_cast<AudioUnit*>(track1.get());
    AudioUnit * track2Unit = static_cast<AudioUnit*>(track2.get());
    AudioUnit * mixerUnit = static_cast<AudioUnit*>(mixer.get());

    prj->addUnit(std::move(mixer));
    prj->addUnit(std::move(track1));
    prj->addUnit(std::move(track2));

    //make routes
    if(1) {
        AudioRoute route;
        AudioRoute::clearChannels(route);

        //mixer to driver out
        route._sourceId = mixerUnit->id();
        route._sourceType = AudioRoute::Type::INT;
        route._targetId = 0;
        route._targetType = AudioRoute::Type::EXT;
        route._channelMap[0] = 0;
        route._channelMap[1] = 1;
        prj->addRoute(route);

        //track1 to mixer
        route._sourceId = track1Unit->id();
        route._sourceType = AudioRoute::Type::INT;
        route._targetId = mixerUnit->id();
        route._targetType = AudioRoute::Type::INT;
        route._channelMap[0] = 0;
        route._channelMap[1] = 1;
        prj->addRoute(route);

        //track2 to mixer
        route._sourceId = track2Unit->id();
        prj->addRoute(route);

        //driver input stereo to track1
        route._sourceId = 0;
        route._sourceType = AudioRoute::Type::EXT;
        route._targetId = track1Unit->id();
        route._targetType = AudioRoute::Type::INT;
        route._channelMap[0] = 0;
        route._channelMap[1] = 1;
        prj->addRoute(route);

        //driver input mono to mixer
        route._targetId = mixerUnit->id();
        route._channelMap[0] = 0;
        route._channelMap[1] = 0;
        prj->addRoute(route);
    }

    /*
    Driver -> track1 -> mixer -> out
    Driver ->           mixer -> out
              track2 -> mixer -> out
    mixer id - 2
    track1 id - 3
    track2 id - 4
    */

    //expected plan
    RenderPlan * expected = new RenderPlan;
    if(1) {
        Dependencies * deps = nullptr;
        //total of 3 nodes - track1 track2 mixer
        RenderPlan::Node * nodes = new RenderPlan::Node[3];

        //track1
        nodes[0].target = track1Unit;
        nodes[0].depsCount = 1;
        
        deps = new Dependencies[1];
        deps[0].buffer = nullptr;
        deps[0].external = true;
        deps[0].extId = 0;
        clearChannelMap(&deps[0].channelMap[0]);
        deps[0].channelMap[0] = 0;
        deps[0].channelMap[1] = 1;

        nodes[0].deps = deps;

        //track2
        nodes[1].target = track2Unit;
        nodes[1].depsCount = 0;
        nodes[1].deps = nullptr;

        //mixer
        nodes[2].target = mixerUnit;
        nodes[2].depsCount = 3;

        deps = new Dependencies[3];
        for(int i=0; i<3; ++i) {
            clearChannelMap(&deps[i].channelMap[0]);
        }
        deps[0].buffer = nullptr;
        deps[0].external = true;
        deps[0].extId = 0;
        deps[0].channelMap[0] = 0;
        deps[0].channelMap[1] = 0;

        deps[1].buffer = track1Unit->outputs();
        deps[1].external = false;
        deps[1].extId = 0;
        deps[1].channelMap[0] = 0;
        deps[1].channelMap[1] = 1;
        
        deps[2].buffer = track2Unit->outputs();
        deps[2].external = false;
        deps[2].extId = 0;
        deps[2].channelMap[0] = 0;
        deps[2].channelMap[1] = 1;
        
        nodes[2].deps = deps;

        //mixer to output
        Dependencies * outdeps = new Dependencies[1];
        clearChannelMap(&outdeps[0].channelMap[0]);
        outdeps[0].buffer = mixerUnit->outputs();
        outdeps[0].external = false;
        outdeps[0].extId = 0;
        outdeps[0].channelMap[0] = 0;
        outdeps[0].channelMap[1] = 1;
        
        expected->outputDeps = outdeps;
        expected->outputDepsCount = 1;
        expected->nodes = nodes;
        expected->nodesCount = 3;
    }

    RenderPlan * created = buildPlan(prj, prj->routes());

    //compare
    if(1) {
        ASSERT_EQ(created->nodesCount, expected->nodesCount);
        ASSERT_EQ(created->outputDepsCount, expected->outputDepsCount);
        
        //outputDeps
        for(uint32_t i=0; i<created->outputDepsCount; ++i) {
            Dependencies & dep = created->outputDeps[i];
            Dependencies & cmp = expected->outputDeps[i];

            ASSERT_EQ(dep.external, cmp.external);
            ASSERT_EQ(dep.extId, cmp.extId);
            ASSERT_EQ(dep.buffer, cmp.buffer);
            ASSERT_TRUE(compareMaps(&dep.channelMap[0], &cmp.channelMap[0]));
        }

        //nodes
        for(uint32_t i=0; i<created->nodesCount; ++i) {
            RenderPlan::Node & crnode = created->nodes[i];
            RenderPlan::Node & exnode = expected->nodes[i];

            ASSERT_EQ(crnode.target, exnode.target);
            ASSERT_TRUE(checkDependencies(crnode, exnode));
        }
    }

    destroyPlan(created);
    destroyPlan(expected);
    delete prj;
}

TEST(RenderPlan, Ultimate) {
    using namespace slr;

    /*  16 routes in total
        EXT IN(mono L) ->           FX1 -> FX2          -> EXT OUT +
        EXT IN(mono R) -> Track1 -> FX3        -> Mixer -> EXT OUT +
        EXT IN(stereo) -> Track2(record only but still process)    +
                          Track3 -> FX4        -> Mixer -> EXT OUT +
                          Track4 -> FX5 -> FX6 -> Mixer -> EXT OUT +
                          Track5 -> FX7                 -> EXT OUT
                          Track6 -> FX8(both not in plan\not processing)
        ID's:
        Mixer  - 5
        Track1 - 8
        Track2 - 10
        Track3 - 11
        Track4 - 13
        Track5 - 16
        Track6 - 18
        FX1    - 6
        FX2    - 7
        FX3    - 9
        FX4    - 12
        FX5    - 14
        FX6    - 15
        FX7    - 17
        FX8    - 19

        expected output(id's) variants:
        [ whatever happens here, actually order doesn't matters?, (3, 13, 1)]...
        than what to check?
        1. that all units, (excluding Track6 and FX8) presented in render plan.
        2. ... 
        3. that dependencies for all used units are correct?
        4. that Track2 unit is presented as well?
        5. partially order - (e.g. FX1 processed before FX2; Track1 before FX3, and FX3 before Mixer; and etc...) ?
    */

    //create everything
    Project * prj = new Project;

    AudioUnit * mixerUnit = nullptr;
    AudioUnit * track1Unit = nullptr;
    AudioUnit * track2Unit = nullptr;
    AudioUnit * track3Unit = nullptr;
    AudioUnit * track4Unit = nullptr;
    AudioUnit * track5Unit = nullptr;
    AudioUnit * track6Unit = nullptr;
    AudioUnit * fx1Unit = nullptr;
    AudioUnit * fx2Unit = nullptr;
    AudioUnit * fx3Unit = nullptr;
    AudioUnit * fx4Unit = nullptr;
    AudioUnit * fx5Unit = nullptr;
    AudioUnit * fx6Unit = nullptr;
    AudioUnit * fx7Unit = nullptr;
    AudioUnit * fx8Unit = nullptr;

    //create units
    if(1) {
        std::unique_ptr<Mixer> mixer = std::make_unique<Mixer>();
        std::unique_ptr<Dummy> fx1 = std::make_unique<Dummy>();
        std::unique_ptr<Dummy> fx2 = std::make_unique<Dummy>();
        std::unique_ptr<Track> track1 = std::make_unique<Track>();
        std::unique_ptr<Dummy> fx3 = std::make_unique<Dummy>();
        std::unique_ptr<Track> track2 = std::make_unique<Track>();
        std::unique_ptr<Track> track3 = std::make_unique<Track>();
        std::unique_ptr<Dummy> fx4 = std::make_unique<Dummy>();
        std::unique_ptr<Track> track4 = std::make_unique<Track>();
        std::unique_ptr<Dummy> fx5 = std::make_unique<Dummy>();
        std::unique_ptr<Dummy> fx6 = std::make_unique<Dummy>();
        std::unique_ptr<Track> track5 = std::make_unique<Track>();
        std::unique_ptr<Dummy> fx7 = std::make_unique<Dummy>();
        std::unique_ptr<Track> track6 = std::make_unique<Track>();
        std::unique_ptr<Dummy> fx8 = std::make_unique<Dummy>();

        mixerUnit = mixer.get();
        track1Unit = track1.get();
        track2Unit = track2.get();
        track3Unit = track3.get();
        track4Unit = track4.get();
        track5Unit = track5.get();
        track6Unit = track6.get();
        fx1Unit = fx1.get();
        fx2Unit = fx2.get();
        fx3Unit = fx3.get();
        fx4Unit = fx4.get();
        fx5Unit = fx5.get();
        fx6Unit = fx6.get();
        fx7Unit = fx7.get();
        fx8Unit = fx8.get();

        prj->addUnit(std::move(mixer));
        prj->addUnit(std::move(track1));
        prj->addUnit(std::move(track2));
        prj->addUnit(std::move(track3));
        prj->addUnit(std::move(track4));
        prj->addUnit(std::move(track5));
        prj->addUnit(std::move(track6));
        prj->addUnit(std::move(fx1));
        prj->addUnit(std::move(fx2));
        prj->addUnit(std::move(fx3));
        prj->addUnit(std::move(fx4));
        prj->addUnit(std::move(fx5));
        prj->addUnit(std::move(fx6));
        prj->addUnit(std::move(fx7));
        prj->addUnit(std::move(fx8));
    }

    //make connections
    if(1) {
        AudioRoute r;
        AudioRoute::clearChannels(r);

        //Mixer -> EXT OUT
        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = mixerUnit->id();
        r._targetType = AudioRoute::Type::EXT;
        r._targetId = 0;
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);


        //EXT IN(mono L) ->           FX1 -> FX2          -> EXT OUT
        r._sourceType = AudioRoute::Type::EXT;
        r._sourceId = 0;
        r._targetType = AudioRoute::Type::INT;
        r._targetId = fx1Unit->id();
        r._channelMap[0] = 0;
        r._channelMap[1] = 0;
        prj->addRoute(r);

        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = fx1Unit->id();
        r._targetType = AudioRoute::Type::INT;
        r._targetId = fx2Unit->id();
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);

        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = fx2Unit->id();
        r._targetType = AudioRoute::Type::EXT;
        r._targetId = 0;
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);


        //EXT IN(mono R) -> Track1 -> FX3        -> Mixer
        r._sourceType = AudioRoute::Type::EXT;
        r._sourceId = 0;
        r._targetType = AudioRoute::Type::INT;
        r._targetId = track1Unit->id();
        r._channelMap[0] = 1;
        r._channelMap[1] = 1;
        prj->addRoute(r);

        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = track1Unit->id();
        r._targetType = AudioRoute::Type::INT;
        r._targetId = fx3Unit->id();
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);

        r._sourceId = fx3Unit->id();
        r._targetId = mixerUnit->id();
        prj->addRoute(r);


        //EXT IN(stereo) -> Track2(record only but still process)
        r._sourceType = AudioRoute::Type::EXT;
        r._sourceId = 0;
        r._targetType = AudioRoute::Type::INT;
        r._targetId = track2Unit->id();
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);

        //Track3 -> FX4        -> Mixer
        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = track3Unit->id();
        r._targetType = AudioRoute::Type::INT;
        r._targetId = fx4Unit->id();
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);

        r._sourceId = fx4Unit->id();
        r._targetId = mixerUnit->id();
        prj->addRoute(r);


        //Track4 -> FX5 -> FX6 -> Mixer
        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = track4Unit->id();
        r._targetType = AudioRoute::Type::INT;
        r._targetId = fx5Unit->id();
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);

        r._sourceId = fx5Unit->id();
        r._targetId = fx6Unit->id();
        prj->addRoute(r);

        r._sourceId = fx6Unit->id();
        r._targetId = mixerUnit->id();
        prj->addRoute(r);

        //Track5 -> FX7                 -> EXT OUT
        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = track5Unit->id();
        r._targetType = AudioRoute::Type::INT;
        r._targetId = fx7Unit->id();
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);

        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = fx7Unit->id();
        r._targetType = AudioRoute::Type::EXT;
        r._targetId = 0;
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);

        //Track6 -> FX8
        r._sourceType = AudioRoute::Type::INT;
        r._sourceId = track6Unit->id();
        r._targetType = AudioRoute::Type::INT;
        r._targetId = fx8Unit->id();
        r._channelMap[0] = 0;
        r._channelMap[1] = 1;
        prj->addRoute(r);
    }

    RenderPlan * created = buildPlan(prj, prj->routes());

    //id's in plan
    for(uint32_t i=0; i<created->nodesCount; ++i) {
        std::cout << created->nodes[i].target->id() << " ";
    }
    std::cout << std::endl;

    //
    struct presented {
        ID id;
        bool presented;
    };

    std::vector<presented> presentedTest; 
    //fill
    if(1) {
        presentedTest.push_back({mixerUnit->id(), false});
        presentedTest.push_back({track1Unit->id(), false});
        presentedTest.push_back({track2Unit->id(), false});
        presentedTest.push_back({track3Unit->id(), false});
        presentedTest.push_back({track4Unit->id(), false});
        presentedTest.push_back({track5Unit->id(), false});
        presentedTest.push_back({track6Unit->id(), false});
        presentedTest.push_back({fx1Unit->id(), false});
        presentedTest.push_back({fx2Unit->id(), false});
        presentedTest.push_back({fx3Unit->id(), false});
        presentedTest.push_back({fx4Unit->id(), false});
        presentedTest.push_back({fx5Unit->id(), false});
        presentedTest.push_back({fx6Unit->id(), false});
        presentedTest.push_back({fx7Unit->id(), false});
        presentedTest.push_back({fx8Unit->id(), false});
    }

    //check
    for(uint32_t i=0; i<created->nodesCount; ++i) {
        ID idInPlan = created->nodes[i].target->id();
        for(std::size_t t=0; t<presentedTest.size(); ++t) {
            presented & p = presentedTest.at(t);
            if(p.id == idInPlan) {
                if(p.presented == true) {
                    std::cout << "ID already presented!" << std::endl;
                    ASSERT_TRUE(false);
                }
                p.presented = true;
            }
        }
    }

    for(std::size_t i=0; i<presentedTest.size(); ++i) {
        if(presentedTest.at(i).presented == false) {
            std::cout << "ID not presented" << std::endl;
            ASSERT_TRUE(false);
        }
    }


    destroyPlan(created);
    delete prj;
}




int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}