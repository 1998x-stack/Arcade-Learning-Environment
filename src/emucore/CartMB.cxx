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
// $Id: CartMB.cxx,v 1.11 2007/01/14 16:17:55 stephena Exp $
//============================================================================

#include <cassert>

#include "emucore/System.hxx"
#include "emucore/Serializer.hxx"
#include "emucore/Deserializer.hxx"
#include "emucore/CartMB.hxx"

namespace ale {
namespace stella {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeMB::CartridgeMB(const uint8_t* image)
{
  // Copy the ROM image into my buffer
  for(uint32_t addr = 0; addr < 65536; ++addr)
  {
    myImage[addr] = image[addr];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeMB::~CartridgeMB()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const char* CartridgeMB::name() const
{
  return "CartridgeMB";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::reset()
{
  // Upon reset we switch to bank 1
  myCurrentBank = 0;
  incbank();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::install(System& system)
{
  mySystem = &system;
  uint16_t shift = mySystem->pageShift();
  uint16_t mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Set the page accessing methods for the hot spots
  System::PageAccess access;
  for(uint32_t i = (0x1FF0 & ~mask); i < 0x2000; i += (1 << shift))
  {
    access.directPeekBase = 0;
    access.directPokeBase = 0;
    access.device = this;
    mySystem->setPageAccess(i >> shift, access);
  }

  // Install pages for bank 1
  myCurrentBank = 0;
  incbank();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint8_t CartridgeMB::peek(uint16_t address)
{
  address = address & 0x0FFF;

  // Switch to next bank
  if(address == 0x0FF0) incbank();

  return myImage[myCurrentBank * 4096 + address];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::poke(uint16_t address, uint8_t)
{
  address = address & 0x0FFF;

  // Switch to next bank
  if(address == 0x0FF0) incbank();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::incbank()
{
  if(bankLocked) return;

  // Remember what bank we're in
  myCurrentBank ++;
  myCurrentBank &= 0x0F;
  uint16_t offset = myCurrentBank * 4096;
  uint16_t shift = mySystem->pageShift();
  uint16_t mask = mySystem->pageMask();

  // Setup the page access methods for the current bank
  System::PageAccess access;
  access.device = this;
  access.directPokeBase = 0;

  // Map ROM image into the system
  for(uint32_t address = 0x1000; address < (0x1FF0U & ~mask);
      address += (1 << shift))
  {
    access.directPeekBase = &myImage[offset + (address & 0x0FFF)];
    mySystem->setPageAccess(address >> shift, access);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeMB::save(Serializer& out)
{
  std::string cart = name();

  try
  {
    out.putString(cart);

    out.putInt(myCurrentBank);
  }
  catch(const char* msg)
  {
    ale::Logger::Error << msg << std::endl;
    return false;
  }
  catch(...)
  {
    ale::Logger::Error << "Unknown error in save state for " << cart << std::endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeMB::load(Deserializer& in)
{
  std::string cart = name();

  try
  {
    if(in.getString() != cart)
      return false;

    myCurrentBank = (uint16_t) in.getInt();
  }
  catch(const char* msg)
  {
    ale::Logger::Error << msg << std::endl;
    return false;
  }
  catch(...)
  {
    ale::Logger::Error << "Unknown error in load state for " << cart << std::endl;
    return false;
  }

  // Remember what bank we were in
  --myCurrentBank;
  incbank();

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMB::bank(uint16_t bank)
{
  if(bankLocked) return;

  myCurrentBank = (bank - 1);
  incbank();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int CartridgeMB::bank()
{
  return myCurrentBank;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int CartridgeMB::bankCount()
{
  return 16;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeMB::patch(uint16_t address, uint8_t value)
{
  address = address & 0x0FFF;
  myImage[myCurrentBank * 4096 + address] = value;
  return true;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint8_t* CartridgeMB::getImage(int& size)
{
  size = 65536;
  return &myImage[0];
}

}  // namespace stella
}  // namespace ale
