#ifndef _CUELIST_H_
#define _CUELIST_H_

#pragma once

#include <LumiverseCore.h>
#include "Cue.h"
#include "Playback.h"

namespace Lumiverse {
namespace ShowControl {

class Playback;

// A cue list is a list of cues. This class maintains the relationships
// between cues and performs update operations.
// This system will optionally track through changes to the cue, meaning
// that if you change a parameter value in a cue, it'll look forward
// through the cue list and update the values of that parameter if
// the parameter was the same in the previous cue.

// these should probably have a name field at some point...
class CueList
{
public:
  /*! \brief Make a new empty cue list */
  CueList(string name, Playback* pb);

  /*! \brief Load a CueList from a JSON node */
  CueList(JSONNode node, Playback* pb);

  // Delete the cue list
  ~CueList();

  // Stores a cue in the list.
  // Returns false if there's already a cue with the given number in the map.
  // Set overwrite to true to force.
  bool storeCue(float num, string cueID, bool overwrite = false);

  // Delets a cue. Does not delete the Cue object from the playback controls by default
  void deleteCue(float num, bool totalDelete = false);

  // Updates a cue and tracks changes.
  // Set track to true if tracking desired.
  // This function is a bit more complicated at the moment
  //void update(float num, Rig* rig, bool track = false);

  // Gets the list of cue numbers
  const map<float, string>& getCueList() { return _cues; }

  /*!
  \brief Gets the number of the first cue. 
  
  If no cues exist, this returns 0.
  */
  float getFirstCueNum();

  string getFirstCue();

  /*!
  \brief Gets the number of the last cue.
  
  If no cues exist, this returns 0.
  */
  float getLastCueNum();

  string getLastCue();

  // Gets a cue and allows user to modify it.
  Cue* getCue(float num);
  string getCueName(float num);

  // Gets the next cue in the list
  // If the given cue number isn't in the list, returns nullptr
  string getNextCue(float num);

  // Same as getNextCue but returns the cue number.
  // If the given cue number isn't in the list, returns -1
  float getNextCueNum(float num);

  /*!
  \brief Gets the previous cue in the list.
  \param num Cue to start search at.
  \return Pointer to previous cue in the list from cue num.
  */
  string getPrevCue(float num);

  /*!
  \brief Gets the number of the previous cue in the list.
  \param num Cue to start search at.
  \return Number of previous cue in the list.
  */
  float getPrevCueNum(float num);
  
  // For those who need to access a cue by indexing into an array
  // we've got you covered.
  float getCueNumAtIndex(int index);

  /*! \brief Sets the name of the cue list. */
  void setName(string name) { _name = name; }

  /*! \brief Gets the name of the cue list. */
  string getName() { return _name; }

  /*!
  \brief Returns the JSON representation of the cue list.
  */
  JSONNode toJSON();

private:
  // List of cues. Cue numbers can be floats.
  map<float, string> _cues;

  /*! \brief Name of the list */
  string _name;

  /*! \brief Pointer to Playback object that contains the actual cue data. */
  Playback* _pb;
};

}
}
#endif