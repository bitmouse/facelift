/**********************************************************************
**
** Copyright (C) 2018 Luxoft Sweden AB
**
** This file is part of the FaceLift project
**
** Permission is hereby granted, free of charge, to any person
** obtaining a copy of this software and associated documentation files
** (the "Software"), to deal in the Software without restriction,
** including without limitation the rights to use, copy, modify, merge,
** publish, distribute, sublicense, and/or sell copies of the Software,
** and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** SPDX-License-Identifier: MIT
**
**********************************************************************/

#pragma once

#include <QObject>

#include "FaceliftModel.h"

namespace facelift {

class PropertyVerifier {

public:

    static bool isEnabled();

    template<typename Getter, typename Signal, typename OwnerType>
    static void checkProperty(OwnerType* object, Getter getter, Signal signal, const char* propertyName = nullptr)
    {
        auto previousValue = (object->*getter)();
        QObject::connect(object, signal, [previousValue, getter, object, propertyName]() mutable {
            auto newValue = (object->*getter)();
            if (previousValue == newValue) {
                qCCritical(LogModel) << "Change signal triggered but the value has not changed" << newValue
                    << object << "propertyName:" << (propertyName ? propertyName : "Unknown");
                onUnexpectedChangeSignalTriggered();
            } else {
                previousValue = newValue;
            }
        });
    }

    template<typename OwnerType, typename PropertyType>
    static void checkProperty(const PropertyInterface<OwnerType, PropertyType>& property, const char* propertyName)
    {
        checkProperty(property.object(), property.getter(), property.signal(), propertyName);
    }

    template<typename OwnerType, typename PropertyType>
    static void checkProperty(const ServicePropertyInterface<OwnerType, PropertyType>& property, const char* propertyName)
    {
        checkProperty(property.object(), property.getter(), property.signal(), propertyName);
    }

    template<typename OwnerType, typename PropertyType>
    static void checkProperty(const ModelPropertyInterface<OwnerType, PropertyType>& property, const char* propertyName)
    {
        M_UNUSED(property, propertyName);    // TODO: implement
    }

private:

    static void onUnexpectedChangeSignalTriggered();

};

}
