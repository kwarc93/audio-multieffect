/*
 * hal_qspi.cpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#include "hal_nvm.hpp"

using namespace hal;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

nvm::nvm(hal::interface::nvm *interface)
{
    this->interface = interface;
}

nvm::~nvm()
{

}

bool nvm::read(void *data, uint32_t addr, size_t size)
{
    if (this->interface)
        return this->interface->read(data, addr, size);
    else
        return false;
}

bool nvm::write(void *data, uint32_t addr, size_t size)
{
    if (this->interface)
        return this->interface->write(data, addr, size);
    else
        return false;
}

bool nvm::erase(uint32_t addr, size_t size)
{
    if (this->interface)
        return this->interface->erase(addr, size);
    else
        return false;
}

bool nvm::erase(void)
{
    if (this->interface)
        return this->interface->erase();
    else
        return false;
}

hal::interface::nvm::status_t nvm::status(void)
{
    if (this->interface)
        return this->interface->status();
    else
        return hal::interface::nvm::status_t::error;
}

