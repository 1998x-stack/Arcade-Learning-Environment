//============================================================================
//
// MM     MM  6666  555555  0000   2222
// MMMM MMMM 66  66 55     00  00 22  22
// MM MMM MM 66     55     00  00     22
// MM  M  MM 66666  55555  00  00  22222  --  "A 6502 Microprocessor Emulator"
// MM     MM 66  66     55 00  00 22
// MM     MM 66  66 55  55 00  00 22
// MM     MM  6666   5555   0000  222222
//
// Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
//
// See the file "license" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: System.hxx,v 1.16 2007/01/01 18:04:51 stephena Exp $
//============================================================================

#ifndef SYSTEM_HXX
#define SYSTEM_HXX

namespace ale {
namespace stella {

class Device;
class M6502;
class TIA;
class NullDevice;
class Serializer;
class Deserializer;
class Settings;

}  // namespace stella
}  // namespace ale

#include "emucore/Device.hxx"
#include "emucore/NullDev.hxx"
#include "emucore/Random.hxx"

#include <string>

namespace ale {
namespace stella {

/**
  This class represents a system consisting of a 6502 microprocessor
  and a set of devices.  The devices are mapped into an addressing
  space of 2^n bytes (1 <= n <= 16).  The addressing space is broken
  into 2^m byte pages (1 <= m <= n), where a page is the smallest unit
  a device can use when installing itself in the system.

  In general the addressing space will be 8192 (2^13) bytes for a 
  6507 based system and 65536 (2^16) bytes for a 6502 based system.

  TODO: To allow for dynamic code generation we probably need to
        add a tag to each page that indicates if it is read only
        memory.  We also need to notify the processor anytime a
        page access method is changed so that it can clear the
        dynamic code for that page of memory.

  @author  Bradford W. Mott
  @version $Id: System.hxx,v 1.16 2007/01/01 18:04:51 stephena Exp $
*/
class System
{
  public:
    /**
      Create a new system with an addressing space of 2^13 bytes and
      pages of 2^6 bytes.
    */
    System(Settings& settings);

    /**
      Destructor
    */
    virtual ~System();

  public:
    /**
      Reset the system cycle counter, the attached devices, and the
      attached processor of the system.
    */
    void reset();

    /**
      Saves the current state of this system class to the given Serializer.

      @param out The serializer device to save to.
      @return The result of the save.  True on success, false on failure.
    */
    bool save(Serializer& out);

    /**
      Loads the current state of this system class from the given Deserializer.

      @param in The deserializer device to load from.
      @return The result of the load.  True on success, false on failure.
    */
    bool load(Deserializer& in);

  public:
    /**
      Attach the specified device and claim ownership of it.  The device 
      will be asked to install itself.

      @param device The device to attach to the system
    */
    void attach(Device* device);

    /**
      Attach the specified processor and claim ownership of it.  The
      processor will be asked to install itself.

      @param m6502 The 6502 microprocessor to attach to the system
    */
    void attach(M6502* m6502);

    /**
      Attach the specified TIA device and claim ownership of it.  The device 
      will be asked to install itself.

      @param tia The TIA device to attach to the system
    */
    void attach(TIA* tia);

    /**
      Saves the current state of Stella to the given file.  Calls
      save on every device and CPU attached to this system.

      @param md5sum   MD5 of the current ROM
      @param out      The serializer device to save to

      @return  False on any errors, else true
    */
    bool saveState(const std::string& md5sum, Serializer& out);

    /**
      Loads the current state of Stella from the given file.  Calls
      load on every device and CPU attached to this system.

      @param md5sum   MD5 of the current ROM
      @param in       The deserializer device to load from

      @return  False on any errors, else true
    */
    bool loadState(const std::string& md5sum, Deserializer& in);

  public:
    /**
      Answer the 6502 microprocessor attached to the system.  If a
      processor has not been attached calling this function will fail.

      @return The attached 6502 microprocessor
    */
    M6502& m6502()
    {
      return *myM6502;
    }

    /**
      Answer the TIA device attached to the system.

      @return The attached TIA device
    */
    TIA& tia()
    {
      return *myTIA;
    }

    /**
      Answer the random generator attached to the system.
      @return The random generator
    */
    Random& rng()
    {
      return myRandom;
    }

    /**
      Get the null device associated with the system.  Every system 
      has a null device associated with it that's used by pages which 
      aren't mapped to "real" devices.

      @return The null device associated with the system
    */
    NullDevice& nullDevice()
    {
      return myNullDevice;
    }

    /**
      Get the total number of pages available in the system.

      @return The total number of pages available
    */
    uint16_t numberOfPages() const
    {
      return myNumberOfPages;
    }

    /**
      Get the amount to right shift an address by to obtain its page.

      @return The amount to right shift an address by to get its page
    */
    uint16_t pageShift() const
    {
      return myPageSize;
    }

    /**
      Get the mask to apply to an address to obtain its page offset.

      @return The mask to apply to an address to obtain its page offset
    */
    uint16_t pageMask() const
    {
      return myPageMask;
    }
 
  public:
    /**
      Get the number of system cycles which have passed since the last
      time cycles were reset or the system was reset.

      @return The number of system cycles which have passed
    */
    uint32_t cycles() const 
    { 
      return myCycles; 
    }

    /**
      Increment the system cycles by the specified number of cycles.

      @param amount The amount to add to the system cycles counter
    */
    void incrementCycles(uint32_t amount) 
    { 
      myCycles += amount; 
    }

    /**
      Reset the system cycle count to zero.  The first thing that
      happens is that all devices are notified of the reset by invoking 
      their systemCyclesReset method then the system cycle count is 
      reset to zero.
    */
    void resetCycles();

  public:
    /**
      Get the current state of the data bus in the system.  The current
      state is the last data that was accessed by the system.

      @return the data bus state
    */  
    uint8_t getDataBusState() const;

    /**
      Get the byte at the specified address.  No masking of the
      address occurs before it's sent to the device mapped at
      the address.

      @return The byte at the specified address
    */
    inline uint8_t peek(uint16_t addr)
    {
      PageAccess& access = myPageAccessTable[(addr & myAddressMask) >> myPageSize];

      uint8_t result;

      // See if this page uses direct accessing or not
      if(access.directPeekBase != 0)
      {
        result = *(access.directPeekBase + (addr & myPageMask));
      }
      else
      {
        result = access.device->peek(addr);
      }

      myDataBusState = result;

      return result;
    }

    /**
      Change the byte at the specified address to the given value.
      No masking of the address occurs before it's sent to the device
      mapped at the address.

      @param addr The address where the value should be stored
      @param value The value to be stored at the address
    */
    inline void poke(uint16_t addr, uint8_t value) {
      PageAccess& access = myPageAccessTable[
          (addr & myAddressMask) >> myPageSize];

      // See if this page uses direct accessing or not
      if(access.directPokeBase != 0)
      {
        *(access.directPokeBase + (addr & myPageMask)) = value;
      }
      else
      {
        access.device->poke(addr, value);
      }

      myDataBusState = value;
    }

    /**
      Lock/unlock the data bus. When the bus is locked, peek() and
      poke() don't update the bus state. The bus should be unlocked
      while the CPU is running (normal emulation, or when the debugger
      is stepping/advancing). It should be locked while the debugger
      is active but not running the CPU. This is so the debugger can
      use System.peek() to examine memory/registers without changing
      the state of the system.
    */
    void lockDataBus();
    void unlockDataBus();

  public:
    /**
      Structure used to specify access methods for a page
    */
    struct PageAccess
    {
      /**
        Pointer to a block of memory or the null pointer.  The null pointer
        indicates that the device's peek method should be invoked for reads
        to this page, while other values are the base address of an array 
        to directly access for reads to this page.
      */
      uint8_t* directPeekBase;

      /**
        Pointer to a block of memory or the null pointer.  The null pointer
        indicates that the device's poke method should be invoked for writes
        to this page, while other values are the base address of an array 
        to directly access for pokes to this page.
      */
      uint8_t* directPokeBase;

      /**
        Pointer to the device associated with this page or to the system's 
        null device if the page hasn't been mapped to a device
      */
      Device* device;
    };

    /**
      Set the page accessing method for the specified page.

      @param page The page accessing methods should be set for
      @param access The accessing methods to be used by the page
    */
    void setPageAccess(uint16_t page, const PageAccess& access);

    /**
      Get the page accessing method for the specified page.

      @param page The page to get accessing methods for
      @return The accessing methods used by the page
    */
    const PageAccess& getPageAccess(uint16_t page);
 
  private:
    // Log base 2 of the addressing space size.
    static constexpr uint16_t myAddressingSpace = 13;

    // Log base 2 of the page size.
    static constexpr uint16_t myPageSize = 6;

    // Mask to apply to an address before accessing memory
    static constexpr uint16_t myAddressMask = (1 << myAddressingSpace) - 1;

    // Mask to apply to an address to obtain its page offset
    static constexpr uint16_t myPageMask = (1 << myPageSize) - 1;
 
    // Number of pages in the system
    static constexpr uint16_t myNumberOfPages = 1 << (myAddressingSpace - myPageSize);

    // Pointer to a dynamically allocated array of PageAccess structures
    PageAccess* myPageAccessTable;

    // Array of all the devices attached to the system
    Device* myDevices[100];

    // Number of devices attached to the system
    uint32_t myNumberOfDevices;

    // 6502 processor attached to the system or the null pointer
    M6502* myM6502;

    // TIA device attached to the system or the null pointer
    TIA* myTIA;

    // Many devices need a source of random numbers, usually for emulating
    // unknown/undefined behaviour
    Random myRandom;

    // Number of system cycles executed since the last reset
    uint32_t myCycles;

    // Null device to use for page which are not installed
    NullDevice myNullDevice; 

    // The current state of the Data Bus
    uint8_t myDataBusState;

    // Whether or not peek() updates the data bus state. This
    // is true during normal emulation, and false when the
    // debugger is active.
    bool myDataBusLocked;

  private:
    // Copy constructor isn't supported by this class so make it private
    System(const System&);

    // Assignment operator isn't supported by this class so make it private
    System& operator = (const System&);
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline uint8_t System::getDataBusState() const
{
  return myDataBusState;
}

}  // namespace stella
}  // namespace ale

#endif
