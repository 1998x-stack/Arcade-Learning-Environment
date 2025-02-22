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
// $Id: Cart.hxx,v 1.19 2007/06/14 13:47:50 stephena Exp $
//============================================================================

#ifndef CARTRIDGE_HXX
#define CARTRIDGE_HXX

namespace ale {
namespace stella {

class Cartridge;
class System;
class Properties;
class Settings;

}  // namespace stella
}  // namespace ale

#include <fstream>
#include "emucore/Device.hxx"
#include "common/Log.hpp"

namespace ale {
namespace stella {

/**
  A cartridge is a device which contains the machine code for a 
  game and handles any bankswitching performed by the cartridge.
 
  @author  Bradford W. Mott
  @version $Id: Cart.hxx,v 1.19 2007/06/14 13:47:50 stephena Exp $
*/
class Cartridge : public Device
{
  public:
    /**
      Create a new cartridge object allocated on the heap.  The
      type of cartridge created depends on the properties object.

      @param image    A pointer to the ROM image
      @param size     The size of the ROM image 
      @param props    The properties associated with the game
      @param settings The settings associated with the system
      @return   Pointer to the new cartridge object allocated on the heap
    */
    static Cartridge* create(const uint8_t* image, uint32_t size, 
        const Properties& props, const Settings& settings);

    /**
      Create a new cartridge
    */
    Cartridge();
 
    /**
      Destructor
    */
    virtual ~Cartridge();

    /**
      Query some information about this cartridge.
    */
    const std::string& about() const { return myAboutString; }

    /**
      Save the internal (patched) ROM image.

      @param out  The output file stream to save the image
    */
    bool save(std::ofstream& out);

    /** MGB: Added to drop warning on overloaded save() method. */  
    virtual bool save(Serializer& out) = 0; 

    /**
      Lock/unlock bankswitching capability.
    */
    void lockBank()   { bankLocked = true;  }
    void unlockBank() { bankLocked = false; }

  public:
    //////////////////////////////////////////////////////////////////////
    // The following methods are cart-specific and must be implemented
    // in derived classes.
    //////////////////////////////////////////////////////////////////////
    /**
      Set the specified bank.
    */
    virtual void bank(uint16_t bank) = 0;

    /**
      Get the current bank.

      @return  The current bank, or -1 if bankswitching not supported
    */
    virtual int bank() = 0;

    /**
      Query the number of banks supported by the cartridge.
    */
    virtual int bankCount() = 0;

    /**
      Patch the cartridge ROM.

      @param address  The ROM address to patch
      @param value    The value to place into the address
      @return    Success or failure of the patch operation
    */
    virtual bool patch(uint16_t address, uint8_t value) = 0;

    /**
      Access the internal ROM image for this cartridge.

      @param size  Set to the size of the internal ROM image data
      @return  A pointer to the internal ROM image data
    */
    virtual uint8_t* getImage(int& size) = 0;

  protected:
    // If bankLocked is true, ignore attempts at bankswitching. This is used
    // by the debugger, when disassembling/dumping ROM.
    bool bankLocked;

    // Info about this cartridge in string format
    std::string myAboutString;

  private:
    /**
      Try to auto-detect the bankswitching type of the cartridge

      @param image  A pointer to the ROM image
      @param size   The size of the ROM image 
      @return The "best guess" for the cartridge type
    */
    static std::string autodetectType(const uint8_t* image, uint32_t size);

    /**
      Search the image for the specified byte signature

      @param image      A pointer to the ROM image
      @param imagesize  The size of the ROM image 
      @param signature  The byte sequence to search for
      @param sigsize    The number of bytes in the signature
      @param minhits    The minimum number of times a signature is to be found

      @return  True if the signature was found at least 'minhits' time, else false
    */
    static bool searchForBytes(const uint8_t* image, uint32_t imagesize,
                               const uint8_t* signature, uint32_t sigsize,
                               uint32_t minhits);

    /**
      Returns true if the image is probably a SuperChip (256 bytes RAM)
    */
    static bool isProbablySC(const uint8_t* image, uint32_t size);

    /**
      Returns true if the image is probably a 3F bankswitching cartridge
    */
    static bool isProbably3F(const uint8_t* image, uint32_t size);

    /**
      Returns true if the image is probably a 3E bankswitching cartridge
    */
    static bool isProbably3E(const uint8_t* image, uint32_t size);

    /**
      Returns true if the image is probably a E0 bankswitching cartridge
    */
    static bool isProbablyE0(const uint8_t* image, uint32_t size);

    /**
      Returns true if the image is probably a E7 bankswitching cartridge
    */
    static bool isProbablyE7(const uint8_t* image, uint32_t size);

    /**
      Returns true if the image is probably a UA bankswitching cartridge
    */
    static bool isProbablyUA(const uint8_t* image, uint32_t size);

    /**
      Returns true if the image is probably a CV bankswitching cartridge
    */
    static bool isProbablyCV(const uint8_t* image, uint32_t size);

    /**
      Returns true if the image is probably an FE bankswitching cartridge
    */
    static bool isProbablyFE(const uint8_t* image, uint32_t size);

  private:
    // Copy constructor isn't supported by cartridges so make it private
    Cartridge(const Cartridge&);

    // Assignment operator isn't supported by cartridges so make it private
    Cartridge& operator = (const Cartridge&);
};

}  // namespace stella
}  // namespace ale

#endif
