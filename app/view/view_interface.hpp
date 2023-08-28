/*
 * view_interface.hpp
 *
 *  Created on: 28 sie 2023
 *      Author: kwarc
 */

#ifndef VIEW_VIEW_INTERFACE_HPP_
#define VIEW_VIEW_INTERFACE_HPP_

namespace mfx
{
    class view_interface
    {
    public:
        virtual ~view_interface() {};

        struct data_holder
        {
            int error_code;
        };

        virtual void update(const data_holder &data) = 0;
    protected:
        data_holder data;
    };
}

#endif /* VIEW_VIEW_INTERFACE_HPP_ */
