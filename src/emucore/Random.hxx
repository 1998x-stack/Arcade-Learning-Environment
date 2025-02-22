//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: Random.hxx,v 1.4 2007/01/01 18:04:49 stephena Exp $
//============================================================================

#ifndef RANDOM_HXX
#define RANDOM_HXX


namespace ale {
namespace stella {

class Serializer;
class Deserializer;

}  // namespace stella
}  // namespace ale

#include <cstdint>

namespace ale {
namespace stella {

/**
  This Random class uses a Mersenne Twister to provide pseudorandom numbers.
  The class itself is derived from the original 'Random' class by Bradford W. Mott.
*/
class Random
{
  public:
    
    /**
      Class method which allows you to set the seed that'll be used
      for created new instances of this class

      @param value The value to seed the random number generator with
    */
    void seed(uint32_t value);

    /**
      Create a new random number generator
    */
    Random();
   
    ~Random();

    /**
      Answer the next random number from the random number generator

      @return A random number
    */
    uint32_t next();

    /**
      Answer the next random number between 0 and 1 from the random number generator

      @return A random number between 0 and 1
    */
    double nextDouble();

    /**
      Serializes the RNG state.
    */
    bool saveState(Serializer& out);

    /** 
      Deserializes the RNG state.
    */
    bool loadState(Deserializer& in);

    private:
    
    // Actual rng (implementation hidden away from the header to avoid depending on rng libraries). 
    class Impl;
    Impl *m_pimpl;
};

}  // namespace stella
}  // namespace ale

#endif

