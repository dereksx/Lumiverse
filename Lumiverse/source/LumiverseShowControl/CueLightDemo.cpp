#include <string>
#include "LumiverseCore.h"
#include "Cue.h"
#include "CueList.h"
#include "Layer.h"
#include "Playback.h"

#ifdef USE_ARNOLD
#include "Simulation/ArnoldAnimationPatch.h"
#endif


using namespace std;
using namespace Lumiverse;
using namespace Lumiverse::ShowControl;

void simulation() {
    Rig rig("J:/Lumiverse/Lumiverse/data/movingLights_box2.rig.json");

    rig.init();

#ifdef USE_ARNOLD
    ArnoldAnimationPatch *aap = (ArnoldAnimationPatch*)rig.getSimulationPatch("ArnoldAnimationPatch");
#endif

    rig.run();

    shared_ptr<CueList> list1(new CueList("list1"));
    shared_ptr<Layer> layer1(new Layer(&rig, "layer1", 1));

    layer1->setMode(Layer::BLEND_OPAQUE);
    layer1->activate();

    Playback pb(&rig);
    pb.attachToRig();

    rig.init();

    DeviceSet vizi = rig.query("vizi");
    vizi.setParam("intensity", 0.02f);

    DeviceSet inno1 = rig.query("inno1");

    LumiverseColor* color = (LumiverseColor*)rig.getDevice("inno1")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Red", 0.2);
    color->setColorChannel("White", 0.02);

    inno1.setParam("intensity", 1.f);
    inno1.setParam("shutter", "OPEN");
    inno1.setParam("tilt", 0.f);
    inno1.setParam("pan", 0.f);

    //
    DeviceSet inno2 = rig.query("inno2");

    color = (LumiverseColor*)rig.getDevice("inno2")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Green", 0.2);
    color->setColorChannel("White", 0.02);

    inno2.setParam("intensity", 1.f);
    inno2.setParam("shutter", "OPEN");
    inno2.setParam("tilt", 0.f);
    inno2.setParam("pan", 0.f);

    //
    DeviceSet inno3 = rig.query("inno3");

    color = (LumiverseColor*)rig.getDevice("inno3")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Red", 0.2);
    color->setColorChannel("White", 0.02);

    inno3.setParam("intensity", 1.f);
    inno3.setParam("shutter", "OPEN");
    inno3.setParam("tilt", 0.f);
    inno3.setParam("pan", 0.f);

    Cue cue1(&rig, 1.0f, 1.0f, 1.0f);
    list1->storeCue(1, cue1);

    ////////////////////

    color = (LumiverseColor*)rig.getDevice("inno1")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Green", 0.2);
    color->setColorChannel("White", 0.02);

    inno1.setParam("intensity", 1.f);
    inno1.setParam("shutter", "OPEN");
    inno1.setParam("pan", 270.f);
    inno1.setParam("tilt", 20.f);

    //

    color = (LumiverseColor*)rig.getDevice("inno2")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Green", 0);
    color->setColorChannel("Red", 0.2);
    color->setColorChannel("White", 0.02);

    inno2.setParam("intensity", 1.f);
    inno2.setParam("shutter", "OPEN");
    inno2.setParam("tilt", 0.f);
    inno2.setParam("pan", 0.f);

    //

    color = (LumiverseColor*)rig.getDevice("inno3")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Green", 0.2);
    color->setColorChannel("White", 0.02);

    inno3.setParam("intensity", 1.f);
    inno3.setParam("shutter", "OPEN");
    inno3.setParam("pan", 90.f);
    inno3.setParam("tilt", 20.f);

    Cue cue2(&rig);
    list1->storeCue(2, cue2);

    ////////////////////

    color = (LumiverseColor*)rig.getDevice("inno1")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Green", 0.2);
    color->setColorChannel("Red", 0.4);
    color->setColorChannel("White", 0.1);

    inno1.setParam("pan", 0.f);
    inno1.setParam("tilt", 45.f);

    //

    color = (LumiverseColor*)rig.getDevice("inno2")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Green", 0);
    color->setColorChannel("Red", 0.2);
    color->setColorChannel("White", 0.2);

    inno2.setParam("tilt", 45.f);
    inno2.setParam("pan", 0.f);

    //

    color = (LumiverseColor*)rig.getDevice("inno3")->getParam("color");
    color->setColorChannel("Blue", 0.8);
    color->setColorChannel("Green", 0.2);
    color->setColorChannel("Red", 0.4);
    color->setColorChannel("White", 0.2);

    inno3.setParam("pan", 0.f);
    inno3.setParam("tilt", 45.f);

    Cue cue3(&rig);
    list1->storeCue(3, cue3);

    pb.addCueList(list1);
    pb.addLayer(layer1);
    pb.addCueListToLayer("list1", "layer1");

    //par.reset();
    //list1->getCue(1)->insertKeyframe(5, par);

    //chan1.setParam("intensity", 1.0f);
    //list1.getCue(1)->insertKeyframe(4.5f, chan1);

    // Test keyframe overwrite
    //chan1.setParam("intensity", 0.0f);
    //list1.getCue(1)->insertKeyframe(4.5f, chan1);
    pb.save("J:/Lumiverse/Lumiverse/data/movingLights_box2.pb.json", true);
    pb.start();
    rig.run();

    layer1->go();
    layer1->go();

    return;

    time_t t = 0;
    while (1) {
        /*
        float val;
        rig["par1"]->getParam("intensity", val);
        cout << "par1 Intensity: " << val << "/t" << t << "/n";
        this_thread::sleep_for(chrono::milliseconds(500));
        t += 500;
        */
    }
}

void testArnoldAnimation() {
    Rig rig("/afs/andrew.cmu.edu/usr1/chenxil/Documents/Lumiverse/Lumiverse/data/arnold_photometric_cue.json");
    DeviceSet par1 = rig.query("par1");

    rig.init();
    rig.run();

    this_thread::sleep_for(chrono::seconds(2));

    par1.setParam("intensity", 0.5);

    this_thread::sleep_for(chrono::seconds(6));

    par1.setParam("intensity", 1.8);

    this_thread::sleep_for(chrono::seconds(1));
    rig.stop();
//  ArnoldAnimationPatch *ap = (ArnoldAnimationPatch*)rig.getSimulationPatch("ArnoldAnimationPatch");
//  ap->close();

    while (1) {

    }

}

int main(int argc, char**argv) {
  simulation();
    return 0;

  Rig rig("E:/Users/falindrith/Documents/Programming/Lumiverse/Core/Lumiverse/data/movingLights_noArnold.rig.json");
  shared_ptr<CueList> list1(new CueList("list1"));
  shared_ptr<Layer> layer1(new Layer(&rig, "layer1", 1));
  shared_ptr<CueList> list2(new CueList("list2"));
  shared_ptr<Layer> layer2(new Layer(&rig, "layer2", 2));

  layer1->setMode(Layer::BLEND_OPAQUE);
  layer1->activate();
  layer2->activate();
  layer2->setOpacity(1);

  Playback pb(&rig);
  pb.attachToRig();

  rig.init();

  DeviceSet inno = rig.query("inno");

  pb.getProgrammer()->setParam("inno", "pan", 0.2f);

  LumiverseColor* color = (LumiverseColor*)rig.getDevice("inno")->getParam("color");
  color->setxy(0.4, 0.4);

  inno.setParam("intensity", 0.0f);
  inno.setParam("shutter", "OPEN");
  inno.setParam("tilt", 0.5f);
  inno.setParam("pan", 0.75f);

  Cue cue1(&rig, 5);
  list1->storeCue(1, cue1);

  color->setxy(0.2, 0.3);
  inno.setParam("intensity", 1.0f);

  Cue cue2(&rig, 3);
  list1->storeCue(2, cue2);

  inno.reset();
  inno.setParam("pan", 0.5f);

  Cue cue3(&rig);
  list2->storeCue(1, cue3);

  // Add cue list to playback
  pb.addCueList(list1);
  pb.addCueList(list2);

  // Add layer to playback
  pb.addLayer(layer1);
  pb.addLayer(layer2);

  // Add cue list to layer
  pb.addCueListToLayer("list1", "layer1");
  pb.addCueListToLayer("list2", "layer2");

  layer1->goToCueAtTime(2, 3);

  // Prepare playback
  pb.start();
  rig.run();

  // Test keyframe insertion
  //DeviceSet chan1 = rig.query("#1");
  //chan1.setParam("intensity", 0.0f);
  //list1.getCue(1)->insertKeyframe(4, chan1);
  //list1.getCue(1)->insertKeyframe(5, chan1);

  //chan1.setParam("intensity", 1.0f);
  //list1.getCue(1)->insertKeyframe(4.5f, chan1);

  // Test keyframe overwrite
  //chan1.setParam("intensity", 0.0f);
  //list1.getCue(1)->insertKeyframe(4.5f, chan1);

  getchar();
  pb.getProgrammer()->clearAndReset();
  layer1->goToCue(1);
  layer2->goToCue(1);

  this_thread::sleep_for(chrono::seconds(5));

  cout << "Layers ready.";
  getchar();
  layer1->go();

  //this_thread::sleep_for(chrono::seconds(5));

  while (1) {
    float val;
    rig["inno"]->getParam("intensity", val);
    cout << "inno Intensity: " << val << "\n";
    this_thread::sleep_for(chrono::milliseconds(10));
  }
}