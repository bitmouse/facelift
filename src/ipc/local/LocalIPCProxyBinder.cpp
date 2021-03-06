/**********************************************************************
**
** Copyright (C) 2020 Luxoft Sweden AB
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

#include <QObject>
#include <QTimer>


#include "LocalIPCMessage.h"
#include "FaceliftUtils.h"
#include "LocalIPCProxy.h"
#include "LocalIPC-serialization.h"
#include "LocalIPCRegistry.h"
#include "FaceliftIPCCommon.h"


namespace facelift {
namespace local {

LocalIPCProxyBinder::LocalIPCProxyBinder(InterfaceBase &owner, QObject *parent) :
    IPCProxyBinderBase(owner, parent)
{
}

void LocalIPCProxyBinder::checkServiceAvailability()
{
    if (isReadyToConnect()) {
        auto adapter = LocalIPCRegistry::instance().getAdapter(objectPath());

        if (adapter) {
            if (adapter != m_serviceAdapter) {
                if (m_serviceAdapter) {
                    QObject::disconnect(m_signalConnection);
                }
                if (!isServiceAvailable()) {
                    m_serviceAdapter = adapter;
                    m_signalConnection = QObject::connect(adapter, &LocalIPCServiceAdapterBase::messageSent, this, [this] (LocalIPCMessage &message) {
                                this->onSignalTriggered(message);
                            });
                    requestPropertyValues();
                }
            }
        }
        else {
            if (m_serviceAdapter) {
                m_serviceAdapter = nullptr;
                emit serviceAvailableChanged();
            }
        }
    }
}

void LocalIPCProxyBinder::notifyServiceAvailable()
{
    serviceAvailableChanged();
}

void LocalIPCProxyBinder::setServiceAvailable(bool isRegistered)
{
    Q_UNUSED(isRegistered)

    auto isAvailable = isServiceAvailable();
    if (m_serviceAvailable != isAvailable) {
        m_serviceAvailable = isAvailable;
        emit serviceAvailableChanged();
    }
}

void LocalIPCProxyBinder::setObjectPath(const QString &objectPath) {
    IPCProxyBinderBase::setObjectPath(objectPath);
    LocalIPCRegistry::instance().content().addListener(this->objectPath(), this, &LocalIPCProxyBinder::checkServiceAvailability);
}

void LocalIPCProxyBinder::setServiceName(const QString &name)
{
    m_serviceName = name;
    m_explicitServiceName = true;
    checkInit();
}

void LocalIPCProxyBinder::setInterfaceName(const QString &name)
{
    m_interfaceName = name;
    checkInit();
}

void LocalIPCProxyBinder::onServerNotAvailableError(const char *methodName) const
{
    qCCritical(LogIpc,
            "Error message received when calling method '%s' on service at path '%s'. "
            "This likely indicates that the server you are trying to access is not available yet",
            qPrintable(methodName), qPrintable(objectPath()));
}

void LocalIPCProxyBinder::onPropertiesChanged(LocalIPCMessage &msg)
{
    m_serviceObject->deserializePropertyValues(msg, false);
}

void LocalIPCProxyBinder::onSignalTriggered(LocalIPCMessage &msg)
{
    m_serviceObject->deserializePropertyValues(msg, false);
    m_serviceObject->deserializeSignal(msg);
}

LocalIPCMessage LocalIPCProxyBinder::call(LocalIPCMessage &message) const
{
    Q_UNUSED(message);
    qFatal("Local IPC only used with Async proxies for now");
    return LocalIPCMessage();
}

void LocalIPCProxyBinder::asyncCall(LocalIPCMessage &requestMessage, QObject *context, std::function<void(LocalIPCMessage &message)> callback)
{
    requestMessage.addListener(context, callback);

    // capture the current value of m_serviceAdapter, since m_serviceAdapter might have changed by the time the timer is triggered
    auto serviceAdapter = m_serviceAdapter;
    QTimer::singleShot(0, serviceAdapter, [serviceAdapter, requestMessage]() mutable {
                auto r = serviceAdapter->handleMessage(requestMessage);
                Q_ASSERT(r != facelift::IPCHandlingResult::INVALID);
            });
}

void LocalIPCProxyBinder::requestPropertyValues()
{
    LocalIPCMessage msg(FaceliftIPCCommon::GET_PROPERTIES_MESSAGE_NAME);

    auto replyHandler = [this](LocalIPCMessage &replyMessage) {
                if (replyMessage.isReplyMessage()) {
                    m_serviceObject->deserializePropertyValues(replyMessage, true);
                    m_serviceObject->setServiceRegistered(true);
                    emit serviceAvailableChanged();
                } else {
                    qCDebug(LogIpc) << "Service not yet available : " << objectPath();
                }
            };

    if (isSynchronous()) {
        auto replyMessage = call(msg);
        replyHandler(replyMessage);
    } else {
        asyncCall(msg, this, replyHandler);
    }
}

void LocalIPCProxyBinder::bindToIPC()
{
    checkServiceAvailability();
}

const QString &LocalIPCProxyBinder::serviceName() const
{
    return m_serviceName;
}

const QString &LocalIPCProxyBinder::interfaceName() const
{
    return m_interfaceName;
}

bool LocalIPCProxyBinder::isServiceAvailable() const
{
    return !m_serviceAdapter.isNull();
}

void LocalIPCProxyBinder::setHandler(LocalIPCRequestHandler *handler)
{
    m_serviceObject = handler;
    checkInit();
}

}

}
