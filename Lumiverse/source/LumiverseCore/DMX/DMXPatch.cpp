#include "DMXPatch.h"

#ifdef USE_KINET
#ifdef _WIN32
// I don't really know why I need this, but apparently the windows socket
// includes get really weird.
#define WIN32_LEAN_AND_MEAN
#endif
#include "KiNetInterface.h"
#endif

#ifdef USE_ARTNET
#include "ArtNetInterface.h"
#endif

#ifdef USE_DMXPRO2
#include "DMXPro2Interface.h"
#endif

#ifdef USE_OLA
#include "OLAInterface.h"
#endif

namespace Lumiverse {

DMXPatch::DMXPatch() {
  // Empty for now
}

DMXPatch::DMXPatch(const JSONNode data) {
  loadJSON(data);
}

void DMXPatch::loadJSON(const JSONNode data) {
  string patchName = data.name();
  map<string, DMXInterface*> ifaceMap;

  auto i = data.begin();
  // This is a two pass process. First pass initializes the interfaces and Device mappings
  // Second pass actually patches devices and assigns the interfaces to universes.
  while (i != data.end()) {
    std::string nodeName = i->name();

    if (nodeName == "interfaces") {
      JSONNode interfaces = *i;

      auto iface = interfaces.begin();
      while (iface != interfaces.end()) {
        auto type = iface->find("type");

        if (type != iface->end()) {
          // Currently the only supported physical type is the DMX Pro Mk 2 Interface
          if (type->as_string() == "DMXPro2Interface") {
#ifdef USE_DMXPRO2
            auto proNumNode = iface->find("proNum");
            auto out1Node = iface->find("out1");
            auto out2Node = iface->find("out2");

            if (proNumNode != iface->end() && out1Node != iface->end() && out2Node != iface->end()) {
              DMXPro2Interface* intface = new DMXPro2Interface(iface->name(), proNumNode->as_int(), out1Node->as_int(), out2Node->as_int());
              ifaceMap[iface->name()] = (DMXInterface*)intface;
            }

            Logger::log(INFO, "Added DMX USB Pro Mk 2 Interface");
#else
            Logger::log(WARN, "LumverseCore built without DMX Pro Mk II support. Skipping interface...");
#endif
          }
          else if (type->as_string() == "KiNetInterface") {
#ifdef USE_KINET
            auto host = iface->find("host");
            auto port = iface->find("port");
            auto protocolType = iface->find("protocolType");

            if (host != iface->end() && port != iface->end() && protocolType != iface->end()) {
              KiNetInterface* intface = new KiNetInterface(iface->name(), host->as_string(), port->as_int(), (KinetProtocolType)protocolType->as_int());
              ifaceMap[iface->name()] = (DMXInterface*)intface;
            }

            stringstream ss;
            ss << "Added KiNet Interface \"" << iface->name() << "\" with host " << host->as_string();
            Logger::log(INFO, ss.str());
#else
            Logger::log(WARN, "LumverseCore built without KiNet support. Skipping interface...");
#endif
          }
          else if (type->as_string() == "ArtNetInterface") {
#ifdef USE_ARTNET
            auto ip = iface->find("ip");
            auto broad = iface->find("broadcast");
            auto verb = iface->find("verbose");
            
            if (ip != iface->end() && broad != iface->end() && verb != iface->end()) {
              ArtNetInterface* intface = new ArtNetInterface(iface->name(), ip->as_string(), broad->as_string(), verb->as_bool());
              ifaceMap[iface->name()] = (DMXInterface*)intface;
            }

            stringstream ss;
            ss << "Added ArtNet Interface \"" << iface->name() << "\" with ip " << ip->as_string();
            Logger::log(INFO, ss.str());
#else
            Logger::log(WARN, "LumiverseCore built without ArtNet support. Skipping interface...");
#endif
          }
          else if (type->as_string() == "OLAInterface") {
#ifdef USE_OLA
            OLAInterface* intface = new OLAInterface(iface->name());
            ifaceMap[iface->name()] = (DMXInterface*) intface;
            stringstream ss;
            ss << "Added OLA Interface \"" << iface->name();
            Logger::log(INFO, ss.str());
#else
            Logger::log(WARN, "LumiverseCore built without OLA support. Skipping interface...");
#endif
          }
          else {
            stringstream ss;
            ss << "Unsupported Interface Type " << type->name() << " in " << patchName;
            Logger::log(LOG_LEVEL::WARN, ss.str());
          }
        }

        ++iface;
      }
    }
    if (nodeName == "deviceMaps") {
      loadDeviceMaps(*i);
    }

    ++i;
  }

  // Assign universes to interfaces
  auto universes = data.find("universes");
  if (universes != data.end()) {
    auto universe = universes->begin();
    while (universe != universes->end()) {
      if (ifaceMap[universe->name()] == nullptr) {
        stringstream ss;
        ss << "Can't add universe " << universe->as_int() << " to interface " << universe->name() << " because interface does not exist.";
        Logger::log(ERR, ss.str());
      }
      else {
        assignInterface(ifaceMap[universe->name()], universe->as_int());
      }
      ++universe;
    }
  }
  else {
    Logger::log(LOG_LEVEL::WARN, "No interfaces assignments found in rig");
  }

  // Patch the devices
  auto devices = data.find("devicePatch");
  if (devices != data.end()) {
    auto device = devices->begin();
    while (device != devices->end()) {
      string mapKey = (*device)["mapType"].as_string();
      unsigned int addr = (*device)["addr"].as_int();
      unsigned int universe = (*device)["universe"].as_int();

      DMXDevicePatch* patch = new DMXDevicePatch(mapKey, addr, universe);
      patchDevice(device->name(), patch);

      stringstream ss;
      ss << "Patched " << device->name() << " to " << universe << "/" << addr << " using profile " << mapKey;
      Logger::log(LOG_LEVEL::INFO, ss.str());

      ++device;
    }
  }
  else {
    Logger::log(LOG_LEVEL::WARN, "No devices found in rig");
  }
}

void DMXPatch::loadDeviceMaps(const JSONNode data) {
  auto i = data.begin();

  while (i != data.end()) {
    string name = i->name();
    map<string, patchData> dmxMap;

    auto j = i->begin();
    while (j != i->end()) {
      string paramName = j->name();

      // This assumes the next piece of data is arranged in a [ int, string ] format
      unsigned int addr = (*j)[0].as_int();
      string conversion = (*j)[1].as_string();

      dmxMap[paramName] = patchData(addr, conversion);

      ++j;
    }

    addDeviceMap(name, dmxMap);

    stringstream ss;
    ss << "Added DMX Map for " << name;
    Logger::log(LOG_LEVEL::INFO, ss.str());

    ++i;
  }
}

DMXPatch::~DMXPatch() {
  // Deallocate all interfaces after closing them.
  for (auto& interfaces : m_interfaces) {
    interfaces.second->closeInt();
    delete interfaces.second;
  }

  // Deallocate all patch objects
  for (auto& patches : m_patch) {
    delete patches.second;
  }
}

void DMXPatch::update(set<Device *> devices) {
  for (Device* d : devices) {
    // Skip if there is no DMX patch for the device stored
    try {
      // For each device, find the device patch stored.
      DMXDevicePatch devPatch = *(m_patch).at(d->getId());

      unsigned int uni = devPatch.getUniverse();
      
      // Skip if universes aren't allocated because the interface doesn't exist.
      // TODO: check this, logic may be questionable
      if (uni >= m_universes.size())
        continue;
      
      devPatch.updateDMX(&m_universes[uni].front(), d, m_deviceMaps[devPatch.getDMXMapKey()]);
    }
    catch (exception e) {
      continue;
    }
  }

  // Send updated data to interfaces
  for (auto& i : m_ifacePatch) {
    m_interfaces[i.first]->sendDMX(&m_universes[i.second].front(), i.second);
  }
}

void DMXPatch::init() {
  for (auto& iface : m_interfaces) {
    try {
      iface.second->init();
    }
    catch (exception e) {
      Logger::log(LOG_LEVEL::ERR, e.what());
    }
  }
}

void DMXPatch::close() {
  for (auto& interfaces : m_interfaces) {
    interfaces.second->closeInt();
  }
}

JSONNode DMXPatch::toJSON() {
  JSONNode root;

  root.push_back(JSONNode("type", getType()));
  JSONNode interfaces;
  interfaces.set_name("interfaces");
  for (auto i : m_interfaces) {
    JSONNode iface = i.second->toJSON();
    interfaces.push_back(iface);
  }
  root.push_back(interfaces);

  JSONNode universes;
  universes.set_name("universes");
  for (auto u : m_ifacePatch) {
    universes.push_back(JSONNode(u.first, u.second));
  }
  root.push_back(universes);

  JSONNode deviceMaps;
  deviceMaps.set_name("deviceMaps");
  for (auto dm : m_deviceMaps) {
    deviceMaps.push_back(deviceMapToJSON(dm.first, dm.second));
  }
  root.push_back(deviceMaps);

  JSONNode devicePatch;
  devicePatch.set_name("devicePatch");
  for (auto p : m_patch) {
    JSONNode dPatch;
    dPatch.set_name(p.first);
    dPatch.push_back(JSONNode("mapType", p.second->getDMXMapKey()));
    dPatch.push_back(JSONNode("addr", p.second->getBaseAddress()));
    dPatch.push_back(JSONNode("universe", p.second->getUniverse()));
    devicePatch.push_back(dPatch);
  }
  root.push_back(devicePatch);

  return root;
}

JSONNode DMXPatch::deviceMapToJSON(string id, map<string, patchData> data) {
  JSONNode root;
  root.set_name(id);

  for (auto d : data) {
    JSONNode mapping;
    mapping.set_name(d.first);
    mapping.push_back(JSONNode("start", d.second.startAddress));
#ifdef USE_C11_MAPS
    mapping.push_back(JSONNode("ctype", convTypeToString[d.second.type]));
#else
    mapping.push_back(JSONNode("ctype", convTypeToString(d.second.type)));
#endif
    root.push_back(mapping.as_array());
  }

  return root;
}

void DMXPatch::deleteDevice(string id) {
  delete m_patch[id];
  m_patch.erase(id);
}

void DMXPatch::assignInterface(DMXInterface* iface, unsigned int universe) {
  string id = iface->getInterfaceId();

  // Add to the interface list if doesn't exist.
  if (m_interfaces.count(id) == 0) {
    m_interfaces[id] = iface;
  }

  // Add to the interface patch.
  // But first check to see if the interface is already mapped to the given universe.
  auto ret = m_ifacePatch.equal_range(id);
  for (auto it = ret.first; it != ret.second; ++it) {
    if (it->second == universe)
      return;
  }

  m_ifacePatch.insert(make_pair(id, universe));

  // Update universe vector size.
  if (universe + 1 > m_universes.size()) {
    m_universes.resize(universe + 1);
    for (auto& uni : m_universes)
      uni.resize(512);
  }
}

void DMXPatch::assignInterface(string id, unsigned int universe) {
  if (m_interfaces.count(id) == 0) {
    Logger::log(ERR, "No interface with id " + id + " found in DMXPatch object.");
    return;
  }

  assignInterface(m_interfaces[id], universe);
}

void DMXPatch::removeInterface(unsigned int universe, string id) {
  vector<multimap<string, unsigned int>::iterator> toRemove;
  for (auto it = m_ifacePatch.begin(); it != m_ifacePatch.end(); it++) {
    if (it->second == universe && (it->first == id || id == "")) {
      toRemove.push_back(it);
    }
  }

  for (const auto& val : toRemove) {
    m_ifacePatch.erase(val);
  }
}

bool DMXPatch::addInterface(DMXInterface* iface) {
  if (m_interfaces.count(iface->getInterfaceId()) > 0)
    return false;

  m_interfaces[iface->getInterfaceId()] = iface;
  return true;
}

void DMXPatch::deleteInterface(string id) {
  // Close and delete the interface
  m_interfaces[id]->closeInt();
  delete m_interfaces[id];

  // Remove from the patch maps
  m_interfaces.erase(id);
  m_ifacePatch.erase(id);
}

DMXInterface* DMXPatch::getInterface(string id) {
  if (m_interfaces.count(id) > 0)
    return m_interfaces[id];

  return nullptr;
}

void DMXPatch::moveInterface(string id, unsigned int universeFrom, unsigned int universeTo) {
  // Find and delete the from element.
  auto fromLoc = m_ifacePatch.equal_range(id);

  for (auto it = fromLoc.first; it != fromLoc.second; ++it) {
    if (it->second == universeFrom) {
      m_ifacePatch.erase(it);
      break;
    }
  }

  // Insert the to element.
  m_ifacePatch.insert(make_pair(id, universeTo));
}

void DMXPatch::patchDevice(Device* device, DMXDevicePatch* patch) {
  m_patch[device->getId()] = patch;
}

void DMXPatch::patchDevice(string id, DMXDevicePatch* patch) {
  m_patch[id] = patch;
}

DMXDevicePatch* DMXPatch::getDevicePatch(string id) {
  return (m_patch.count(id) == 0) ? nullptr : m_patch[id];
}

void DMXPatch::addDeviceMap(string id, map<string, patchData> deviceMap) {
  m_deviceMaps[id] = deviceMap; // Replaces existing maps.
}

void DMXPatch::addParameter(string mapId, string paramId, unsigned int address, conversionType type) {
  m_deviceMaps[mapId][paramId] = patchData(address, type);
}

void DMXPatch::dumpUniverses() {
  for (unsigned int i = 0; i < m_universes.size(); i++) {
    dumpUniverse(i);
  }
}

void DMXPatch::dumpUniverse(unsigned int universe) {
  vector<unsigned char> uni = m_universes[universe];

  cout << "Universe " << universe << "\n";
  for (unsigned int i = 0; i < uni.size(); i++) {
    cout << i << ":" << (int)uni[i] << "\n";
  }
  cout << "\n";
}

bool DMXPatch::setRawData(unsigned int universe, vector<unsigned char> univData) {
  if (univData.size() != 512) {
    Logger::log(LOG_LEVEL::ERR, "Set raw DMX data failure: buffer is not 512 bytes long.");
    return false;
  }

  m_universes[universe] = univData;

  // Send updated data to interfaces
  for (auto& i : m_ifacePatch) {
    m_interfaces[i.first]->sendDMX(&m_universes[i.second].front(), i.second);
  }

  return true;
}

size_t DMXPatch::sizeOfDeviceMap(string id) {
  if (m_deviceMaps.count(id) == 0)
    return -1;

  unsigned int size = 0;
  for (auto pd : m_deviceMaps[id]) {
    switch (pd.second.type) {
    case (FLOAT_TO_SINGLE) :
    case (ENUM) :
      size += 1;
      break;
    case (FLOAT_TO_FINE) :
    case (ORI_TO_FINE) :
      size += 2;
      break;
    case (COLOR_RGB) :
      size += 3;
      break;
    case (COLOR_RGBW) :
      size += 4;
      break;
    case (COLOR_LUSTRPLUS) :
      size += 7;
      break;
    case (RGB_REPEAT2) :
      size += 3 * 2;
      break;
    case (RGB_REPEAT3) :
      size += 3 * 3;
      break;
    case (RGB_REPEAT4) :
      size += 3 * 4;
      break;
    default:
      break;
    }
  }

  return size;
}

vector<string> DMXPatch::getInterfaceIDs() {
  vector<string> ids;
  for (const auto& kvp : m_interfaces) {
    ids.push_back(kvp.first);
  }
  return ids;
}

}
