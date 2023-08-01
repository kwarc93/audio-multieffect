/*
 * touch_ft5336.hpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#ifndef TOUCH_FT5336_HPP_
#define TOUCH_FT5336_HPP_

#include <hal/hal_interface.hpp>

namespace drivers
{

class touch_ft5336
{
/* TODO */
public:
    touch_ft5336(hal::interface::i2c_device *dev);
private:
    hal::interface::i2c_device *device;
};

}

#endif /* TOUCH_FT5336_HPP_ */
