/*
 * observer.hpp
 *
 *  Created on: 13 wrz 2023
 *      Author: kwarc
 */

#ifndef OBSERVER_HPP_
#define OBSERVER_HPP_

#include <list>

namespace middlewares
{

template<typename T>
class observer
{
public:
    virtual ~observer() {};
    virtual void update(const T &data) = 0;
};

template<typename T>
class subject
{
public:
    virtual ~subject() {};

    void attach(observer<T> *o)
    {
        this->observers.push_back(o);
    }

    void detach(observer<T> *o)
    {
        this->observers.remove(o);
    }

    void notify(const T &data)
    {
        for (const auto &o : this->observers)
            o->update(data);
    }
private:
    std::list<observer<T>*> observers;
};

}

#endif /* OBSERVER_HPP_ */
