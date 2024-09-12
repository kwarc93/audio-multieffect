/*
 * hal_qspi.cpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#include "hal_nvm.hpp"

#include <cassert>

using namespace hal;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

nvm::nvm(hal::interface::nvm *interface)
{
    assert(interface);
    this->interface = interface;
}

nvm::~nvm()
{

}

bool nvm::read(std::byte *data, uint32_t addr, size_t size)
{
    return this->interface->read(data, addr, size);
}

bool nvm::write(std::byte *data, uint32_t addr, size_t size)
{
    return this->interface->write(data, addr, size);
}

bool nvm::erase(uint32_t addr, size_t size)
{
    return this->interface->erase(addr, size);
}

bool nvm::erase(void)
{
    return this->interface->erase();
}

hal::interface::nvm::status_t nvm::status(void)
{
    return this->interface->status();
}

