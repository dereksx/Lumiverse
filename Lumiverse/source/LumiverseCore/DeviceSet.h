/*! \file DeviceSet.h
* \brief A Set of Devices.
*/
#ifndef _DEVICESET_H_
#define _DEVICESET_H_

#pragma once

#include <sstream>
#include <set>
#include <regex>
#include <functional>

#include "Logger.h"
#include "Device.h"
#include "Rig.h"

namespace Lumiverse {
  class Rig;
  class LumiverseType;

  /*!
  * \brief A DeviceSet is a set of devices.
  *
  * More specifically, a DeviceSet is the set resulting
  * from a particular query or series of filtering operations.
  * These devices can be manupulated by setting properties as a group,
  * further filtering them, adding devices, etc.
  * DeviceSets are returned by the Rig when asking for more than one device.
  * Device sets can be filtered in chains, as each filtering operation
  * will return a new DeviceSet:
  * `rig.getDevices("angle", "back", true).remove("area", "3", true) ...` etc.
  * Returning a new set after each operation may use more memory, but
  * it does allow for the construction of a query history and saving of
  * that query during any point of its construction. This history is currently
  * not saved, but may in the future be part of this class.
  * Alternately, DeviceSets can be constructed from concise queries: 
  * https://github.com/ebshimizu/Lumiverse/wiki/Query-Syntax-Notes
  * \sa Device
  */
  class DeviceSet
  {
  public:
    /*!
    * \brief Constructs a DeviceSet unassociated with a Rig
    *
    * It should be noted that this constructor actually isn't particularly useful.
    * Without a Rig*, the DeviceSet can't realyl do any sort of queries, though
    * it can store an arbitrary list of deivces.
    * \sa DeviceSet(Rig*), DeviceSet(const DeviceSet&)
    */
    DeviceSet() { };

    /*!
    * \brief Constructs an empty set
    *
    * \param rig Pointer to a Rig to get devices from.
    * \sa DeviceSet(Rig*, set<Device *>), select(string), Rig
    */
    DeviceSet(Rig* rig);

    /*!
    * \brief Constructs a set with the given devices.
    *
    * Similar to a copy constructor.
    * \param rig Pointer to a Rig to get devices from.
    * \param devices List of Device pointers to initialize the set with.
    * \sa DeviceSet(Rig*), Device, Rig, select(string)
    */
    DeviceSet(Rig* rig, set<Device *> devices);

    /*!
    * \brief Copy a DeviceSet
    * 
    * \param dc DeviceSet to copy data from
    */
    DeviceSet(const DeviceSet& dc);

    /*!
    * \brief Destructor for the DeviceSet
    */
    ~DeviceSet();

    /*!
    * \brief Override for steam output
    *
    * Converts DeviceSet to a string representation and outputs it to a stream.
    * \return ostream object containing the DeviceSet string appended to it.
    * \sa info()
    */
    std::ostream & operator<< (std::ostream &str) {
      str << info();
      return str;
    };

    /*!
    * \brief Get devices matching a query from the Rig
    * 
    * This is the primary function to select Devices from the Rig.
    * Syntax details can be found here: https://github.com/ebshimizu/Lumiverse/wiki/Query-Syntax-Notes
    * \param selector Query string.
    * \return DeviceSet containing all Device objects matching the selector
    */
    DeviceSet select(string selector);

  private:
    // Grouped here for convenience
    
    /*!
    * \brief Parses a single selector.
    *
    * Redirects to appropriate functions. See implementation for details.
    * Boolean flag determines if selected Devices are added or subtracted from
    * the current DeviceSet.
    * \param selector Query string
    * \param filter If true, indicates that devices will be filtered out of the current device set
    * based on the result of the selector. If false, devices will be added based on the selector result.
    * \return DeviceSet containing the result of applying the selector to the current set.
    * \sa parseMetadataSelector(string, bool), parseChannelSelector(string, bool), parseParameterSelector(string, bool),
    * parseFloatParameter(string, string, float, bool, bool)
    */
    DeviceSet parseSelector(string selector, bool filter);

    /*!
    * \brief Processes a metadata selector
    * 
    * Helper for parseSelector.
    * \param selector Query string
    * \param filter If true, indicates that devices will be filtered out of the current device set
    * based on the result of the selector. If false, devices will be added based on the selector result.
    * \return DeviceSet containing the result of applying the metadata selector to the current set.
    * \sa parseSelector(string, bool)
    */
    DeviceSet parseMetadataSelector(string selector, bool filter);

    /*!
    * \brief Processes a channel selector
    *
    * Helper for parseSelector.
    * \param selector Query string
    * \param filter If true, indicates that devices will be filtered out of the current device set
    * based on the result of the selector. If false, devices will be added based on the selector result.
    * \return DeviceSet containing the result of applying the channel selector to the current set.
    * \sa parseSelector(string, bool)
    */
    DeviceSet parseChannelSelector(string selector, bool filter);

    /*!
    * \brief Processes a parameter selector
    *
    * Helper for parseSelector. This part of the query is a bit difficult to parse
    * due to LumiverseType objects actually being different types (enum, float, etc.).
    * \param selector Query string
    * \param filter If true, indicates that devices will be filtered out of the current device set
    * based on the result of the selector. If false, devices will be added based on the selector result.
    * \return DeviceSet containing the result of applying the parameter selector to the current set.
    * \sa parseSelector(string, bool), parseFloatParameter(string, string, float, bool, bool)
    */
    DeviceSet parseParameterSelector(string selector, bool filter);

    /*!
    * \brief Processes a float parameter selector
    *
    * Helper for parseParameterSelector.
    * \param param Parameter name
    * \param op Equality operation. One of "<", "<=", ">", ">=, "!=". Defaults to "==" if nothing specified
    * or if invalid option is specified.
    * \param val The value we're testing against
    * \param filter If true, indicates that devices will be filtered out of the current device set
    * based on the result of the selector. If false, devices will be added based on the selector result.
    * \param eq If set to false, this function will add everything that does not satisfy the selector.
    * It essentially inverts the query. For example, if you have `@intensity < .50` as the query with `eq = false`,
    * the returned set will contain `@intensity >= .5`. This parameter is intended for internal use only.
    * \return DeviceSet containing the result of applying the channel selector to the current set.
    * \sa parseParameterSelector(string, bool)
    */
    DeviceSet parseFloatParameter(string param, string op, float val, bool filter, bool eq);

  public:
    /*!
    * \brief Adds a Device to the set.
    * \param device Pointer to the Device to add.
    * \return DeviceSet containing its current contents plus the added Device.
    */
    DeviceSet add(Device* device);

    /*!
    * \brief Adds Devices in the specified channel to the set.
    * \param channel The channel to add to the DeviceSet
    * \return DeviceSet containing its current contents plus the added Devices.
    */
    DeviceSet add(unsigned int channel);

    /*!
    * \brief Adds Devices in the specified inclusive channel range to the set.
    * \param lower First channel in the range (inclusive)
    * \param upper Last channel in the range (inclusive)
    * \return DeviceSet containing its current contents plus the added Devices.
    */
    DeviceSet add(unsigned int lower, unsigned int upper);


    /*!
    * \brief Adds Devices matching the specified metadata value to the set.
    *
    * If a Device does not have a value for the key specified, it will not
    * be included in the set even if isEqual is set to false.
    * \param key Metadata field to look at
    * \param val Target value of the metadata field
    * \param isEqual If true, the Devices matching the metadata value will be added. If false,
    * devices that don't match the metadata value will be added.
    * \return DeviceSet containing its current contents plus the added Devices.
    * \sa add(string, regex, bool)
    */
    DeviceSet add(string key, string val, bool isEqual);

    /*!
    * \brief Adds Devices matching the specified metadata value to the set.
    *
    * Regex version of the normal metadata add.
    * If a Device does not have a value for the key specified, it will not
    * be included in the set even if isEqual is set to false.
    * \param key Metadata field to look at
    * \param val Target value of the metadata field. Since this is a `std::regex`, it is possible
    * to search for metadata using all of the nice regex features you're used to.
    * \param isEqual If true, the Devices matching the metadata value will be added. If false,
    * devices that don't match the metadata value will be added.
    * \return DeviceSet containing its current contents plus the added Devices.
    * \sa add(string, string, bool)
    */
    DeviceSet add(string key, regex val, bool isEqual);

    /*!
    * \brief Adds devices based on a parameter comparison function provided by the caller.
    * 
    * One of the more complicated add functions, this one adds devices based on a
    * user-specified comparison function between two LumiverseType pointers. It is up to
    * the user to make sure these types are the same to ensure a good comparison is made.
    * \param key Parameter to look at
    * \param val Target parameter value
    * \param cmp Comparison function
    * \param isEqual If true, the Devices matching the metadata value will be added. If false,
    * devices that don't match the metadata value will be added.
    * \return DeviceSet containing its current contents plus the added Devices.
    * \sa LumiverseType, LumiverseTypeUtils
    */
    DeviceSet add(string key, LumiverseType* val, function<bool(LumiverseType* a, LumiverseType* b)> cmp, bool isEqual);

    /*!
    * \brief Adds devices based on a query string
    *
    * Typically you'd just use the select() function for this.
    * \param query Query string
    * \return DeviceSet containing its current contents plus the added Devices.
    */
    DeviceSet add(string query);

    /*!
    * \brief Removes a device from the set.
    * \param device Device to remove
    * \return DeviceSet containing its current contents without the specified Device.
    */
    DeviceSet remove(Device* device);

    /*!
    * \brief Removes Devices in the specified channel from the set.
    * \param channel The channel to remove from the DeviceSet
    * \return DeviceSet containing its current contents without the specified Devices.
    */
    DeviceSet remove(unsigned int channel);

    /*!
    * \brief Removes Devices in the specified inclusive channel range from the set.
    * \param lower First channel in the range (inclusive)
    * \param upper Last channel in the range (inclusive)
    * \return DeviceSet containing its current contents without the specified Devices..
    */
    DeviceSet remove(unsigned int lower, unsigned int upper);

    /*!
    * \brief Removes Devices matching the specified metadata value to the set.
    *
    * If a Device does not have a value for the key specified, it will not
    * be removed from the set even if isEqual is set to false.
    * \param key Metadata field to look at
    * \param val Target value of the metadata field
    * \param isEqual If true, the Devices matching the metadata value will be removed. If false,
    * devices that don't match the metadata value will be removed.
    * \return DeviceSet containing its current contents without the specified Devices.
    * \sa remove(string, regex, bool)
    */
    DeviceSet remove(string key, string val, bool isEqual);


    /*!
    * \brief Removes Devices matching the specified metadata value from the set.
    *
    * Regex version of the normal metadata remove.
    * If a Device does not have a value for the key specified, it will not
    * be removed from the set even if isEqual is set to false.
    * \param key Metadata field to look at
    * \param val Target value of the metadata field. Since this is a `std::regex`, it is possible
    * to search for metadata using all of the nice regex features you're used to.
    * \param isEqual If true, the Devices matching the metadata value will be removed. If false,
    * devices that don't match the metadata value will be removed.
    * \return DeviceSet containing its current contents without the specified Devices.
    * \sa remove(string, string, bool)
    */
    DeviceSet remove(string key, regex val, bool isEqual);

    /*!
    * \brief Removes devices based on a parameter comparison function provided by the caller.
    *
    * One of the more complicated remove functions, this one removes devices based on a
    * user-specified comparison function between two LumiverseType pointers. It is up to
    * the user to make sure these types are the same to ensure a good comparison is made.
    * \param key Parameter to look at
    * \param val Target parameter value
    * \param cmp Comparison function
    * \param isEqual If true, the Devices matching the metadata value will be removed. If false,
    * devices that don't match the metadata value will be removed.
    * \return DeviceSet containing its current contents without the specified Devices.
    * \sa LumiverseType, LumiverseTypeUtils
    */
    DeviceSet remove(string key, LumiverseType* val, function<bool(LumiverseType* a, LumiverseType* b)> cmp, bool isEqual);

    /*!
    * \brief Removes devices based on a query string
    * 
    * All Devices matching the query string and are in the DeviceSet, will be removed.
    * \param query Query string
    * \return DeviceSet containing its current contents wihtout the specified Devices.
    */
    DeviceSet remove(string query);

    // Inverts the selection.
    //DeviceSet invert();

    /*!
    * \brief Resets all the parameters in each Device in the device set
    * \sa Device::reset()
    */
    void reset();

    // These must mirror the device setparam functions.
    
    /*!
    * \brief Sets the value of a LumiverseFloat parameter on every device in the group
    * 
    * This function will only set the value of an existing parameter. If the
    * parameter doesn't exist, it will not be created by this function.
    * \param param Parameter to modify
    * \param val Value of the parameter
    * \sa Device::setParam(string, float)
    */
    void setParam(string param, float val);

    /*!
    * \brief Sets the value of a LumiverseEnum parameter on every device in the group
    *
    * This function will only set the value of an existing parameter. If the
    * parameter doesn't exist, it will not be created by this function.
    * \param param Parameter to modify
    * \param val Value of the enumeration
    * \param val2 The tweak value of the LumiverseEnum
    * \sa Device::setParam(string, string, float)
    */
    void setParam(string param, string val, float val2 = -1.0f);

    /*!
    * \brief Gets the devices managed by this set.
    * 
    * \return Set of Device* contained by the DeviceSet
    */
    inline const set<Device *>* getDevices() { return &m_workingSet; }

    /*!
    * \brief Gets a list of the IDs contained by this DeviceSet
    */
    vector<string> getIds();

    /*!
    * \brief Gets a set of all the parameters used by devices in this set.
    *
    * Set means no duplicates
    */
    set<string> getAllParams();

    /*!
    * \brief Gets a set of all the metadata keys used by devices in this set.
    */
    set<string> getAllMetadata();

    /*!
    * \brief Returns a string containing info about the DeviceSet.
    *
    * \return DeviceSet as a string. String contains the number of devices contained
    * by the set and the IDs of each device in the set.
    */
    string info();

    /*!
    * \brief Returns the number of devices in the DeviceSet.
    * \return Number of devices in the set.
    */
    inline size_t size() { return m_workingSet.size(); }

  private:
    /*!
    * \brief Adds to the set without returning a new copy.
    *
    * Internal use only.
    * \param device Device to add to the set
    */
    void addDevice(Device* device);

    /*!
    * \brief Removes from the set without returning a new copy.
    *
    * Internal use only.
    * \param device Device to remove from the set.
    */
    void removeDevice(Device* device);

    /*!
    * \brief Adds a set to the current set
    * 
    * Internal set opration. Equivalent to a union.
    * \param otherSet The set to add to the DeviceSet.
    */
    void addSet(DeviceSet otherSet);

    /*!
    * \brief Removes a set from the current set
    *
    * Internal set opration. Equivalent to a set difference.
    * \param otherSet The set to remove fromthe DeviceSet.
    */
    void removeSet(DeviceSet otherSet);

    /*!
    * \brief Set of devices currently contained in the Deviceset
    */
    set<Device *> m_workingSet;

    /*!
    * \brief Pointer to the rig for accessing indexes and devices
    */
    Rig* m_rig;
  };
}

#endif