//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesFrameworkConfigImpl.h"
#include "Utils/CommandLineParser.h"
#include "Utils/LoggingUtils.h"
#include "Utils/Argument.h"
#include "Watchdog/PlatformWatchdog.h"
#include "TransportCommon/EConnectionProtocol.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"

namespace ramses
{
    using namespace ramses_internal;

#if defined(HAS_TCP_COMM)
    static bool gHasTCPComm = true;
#else
    static bool gHasTCPComm = false;
#endif

    struct TCPCommandLineArguments
    {
        explicit TCPCommandLineArguments(const TCPConfig& config)
            : useFakeConnection                  ("fakeConnection", "fakeConnection", false, "fake connection")
            , isRamshEnabled                     ("ramsh", "ramsh", false, "ramsh")
            , enableOffsetPlatformProtocolVersion("pvo", "protocolVersionOffset", false, "protocol version offset")
            , disablePeriodicLogs                ("disablePeriodicLogs", "disablePeriodicLogs", false, "disable periodic logs")
            , userProvidedGuid                   ("guid", "guid", "", "guid")
            , port                               ("myport", "myportnumber", config.getPort(), "my port number")
            , ipAddress                          ("myip", "myipaddress", config.getIPAddress(), "my ip address")
            , daemonIP                           ("i", "daemon-ip", config.getDaemonIPAddress(), "daemon ip")
            , daemonPort                         ("p", "daemon-port", config.getDaemonPort(), "daemon port")
        {
        }

        ArgumentBool   useFakeConnection;
        ArgumentBool   isRamshEnabled;
        ArgumentBool   enableOffsetPlatformProtocolVersion;
        ArgumentBool   disablePeriodicLogs;
        ArgumentString userProvidedGuid;
        ArgumentUInt16 port;
        ArgumentString ipAddress;
        ArgumentString daemonIP;
        ArgumentUInt16 daemonPort;

        void print()
        {
            LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& sos) {
                        sos << "\nTCP arguments:\n";

                        sos << useFakeConnection.getHelpString();
                        sos << isRamshEnabled.getHelpString();
                        sos << enableOffsetPlatformProtocolVersion.getHelpString();
                        sos << disablePeriodicLogs.getHelpString();
                        sos << userProvidedGuid.getHelpString();
                        sos << port.getHelpString();
                        sos << ipAddress.getHelpString();
                        sos << daemonPort.getHelpString();
                        sos << daemonIP.getHelpString();
                    }));

        }
    };

    RamsesFrameworkConfigImpl::RamsesFrameworkConfigImpl(int32_t argc, char const* const* argv)
        : StatusObjectImpl()
        , m_shellType(ERamsesShellType_Default)
        , m_periodicLogsEnabled(true)
        , m_usedProtocol(EConnectionProtocol_Invalid)
        , m_parser(argc, argv)
        , m_dltAppID("RAMS")
        , m_dltAppDescription("RAMS-DESC")
        , m_maximumTotalBytesForAsyncResourceLoading(MAXIMUM_BYTES_FOR_ASYNC_RESOURCE_LOADING)
        , m_enableProtocolVersionOffset(false)
    {
        parseCommandLine();
    }

    RamsesFrameworkConfigImpl::~RamsesFrameworkConfigImpl()
    {
    }

    void RamsesFrameworkConfigImpl::enableProtocolVersionOffset()
    {
        m_enableProtocolVersionOffset = true;
    }

    uint32_t RamsesFrameworkConfigImpl::getProtocolVersion() const
    {
        if (m_enableProtocolVersionOffset)
        {
            const uint32_t protocolVersionOffset = 99u;
            return RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR + protocolVersionOffset;
        }
        else
        {
            return RAMSES_TRANSPORT_PROTOCOL_VERSION_MAJOR;
        }
    }

    const ramses_internal::CommandLineParser& RamsesFrameworkConfigImpl::getCommandLineParser() const
    {
        return m_parser;
    }

    EConnectionProtocol RamsesFrameworkConfigImpl::getUsedProtocol() const
    {
        return m_usedProtocol;
    }

    uint32_t RamsesFrameworkConfigImpl::getWatchdogNotificationInterval(ERamsesThreadIdentifier thread) const
    {
        return m_watchdogConfig.getWatchdogNotificationInterval(thread);
    }

    IThreadWatchdogNotification* RamsesFrameworkConfigImpl::getWatchdogNotificationCallback() const
    {
        return m_watchdogConfig.getCallBack();
    }

    status_t RamsesFrameworkConfigImpl::setRequestedRamsesShellType(ERamsesShellType shellType)
    {
        m_shellType = shellType;

        return StatusOK;
    }

    status_t RamsesFrameworkConfigImpl::setWatchdogNotificationInterval(ERamsesThreadIdentifier thread, uint32_t interval)
    {
        if (0u == interval)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Could not set watchdog notification interval, interval is not valid");
            return addErrorEntry("Could not set watchdog notification interval, interval is not valid");
        }
        if (ERamsesThreadIdentifier_Unknown == thread)
        {
            LOG_ERROR(CONTEXT_CLIENT, "Could not set watchdog notification interval, thread identifier is not valid");
            return addErrorEntry("Could not set watchdog notification interval, thread identifier is not valid");
        }
        m_watchdogConfig.setWatchdogNotificationInterval(thread, interval);
        return StatusOK;
    }

    status_t RamsesFrameworkConfigImpl::setWatchdogNotificationCallBack(IThreadWatchdogNotification* callback)
    {
        m_watchdogConfig.setThreadWatchDogCallback(callback);
        return StatusOK;
    }

    void RamsesFrameworkConfigImpl::parseCommandLine()
    {
        const ArgumentBool useFakeConnection( m_parser, "fakeConnection", "fakeConnection", false);
        const ArgumentBool isRamshEnabled(    m_parser, "ramsh", "ramsh", false);
        const ArgumentBool enableOffsetPlatformProtocolVersion(m_parser, "pvo", "protocolVersionOffset", false);
        const ArgumentBool disablePeriodicLogs(m_parser, "disablePeriodicLogs", "disablePeriodicLogs", false);
        const ArgumentString userProvidedGuid(m_parser, "guid", "guid", "");

        if (enableOffsetPlatformProtocolVersion)
        {
            enableProtocolVersionOffset();
        }
        if (isRamshEnabled)
        {
            m_shellType = ERamsesShellType_Console;
        }

        if (disablePeriodicLogs)
        {
            m_periodicLogsEnabled = false;
        }

        if (useFakeConnection || !gHasTCPComm)
        {
            m_usedProtocol = EConnectionProtocol_Fake;
        }
        else
        {
            m_usedProtocol = EConnectionProtocol_TCP;
            ArgumentUInt16 port(m_parser, "myport", "myportnumber", m_tcpConfig.getPort());
            if( port.wasDefined() )
            {
                m_tcpConfig.setPort(port);
            }

            m_tcpConfig.setIPAddress(ArgumentString(m_parser, "myip", "myipaddress", m_tcpConfig.getIPAddress()));
            m_tcpConfig.setDaemonIPAddress(ArgumentString(m_parser, "i", "daemon-ip", m_tcpConfig.getDaemonIPAddress()));
            m_tcpConfig.setDaemonPort(ArgumentUInt16(m_parser, "p", "daemon-port", m_tcpConfig.getDaemonPort()));
        }

        if (userProvidedGuid.hasValue())
        {
            m_userProvidedGuid = Guid(userProvidedGuid);
        }
    }

    void RamsesFrameworkConfigImpl::setDLTApplicationID(const char* id)
    {
        m_dltAppID = id;
    }

    const char* RamsesFrameworkConfigImpl::getDLTApplicationID() const
    {
        return m_dltAppID.c_str();
    }

    void RamsesFrameworkConfigImpl::setDLTApplicationDescription(const char* description)
    {
        m_dltAppDescription = description;
    }

    const char* RamsesFrameworkConfigImpl::getDLTApplicationDescription() const
    {
        return m_dltAppDescription.c_str();
    }


    uint32_t RamsesFrameworkConfigImpl::getMaximumTotalBytesForAsyncResourceLoading() const
    {
        return m_maximumTotalBytesForAsyncResourceLoading;
    }

    void RamsesFrameworkConfigImpl::setMaximumTotalBytesAllowedForAsyncResourceLoading(uint32_t maximumTotalBytesForAsyncResourceLoading)
    {
        m_maximumTotalBytesForAsyncResourceLoading = maximumTotalBytesForAsyncResourceLoading;
    }

    void RamsesFrameworkConfigImpl::setPeriodicLogsEnabled(bool enabled)
    {
        m_periodicLogsEnabled = enabled;
    }

    ramses_internal::Guid RamsesFrameworkConfigImpl::getUserProvidedGuid() const
    {
        return m_userProvidedGuid;
    }

    void RamsesFrameworkConfigImpl::PrintCommandLineOptions()
    {
        const TCPConfig defaultConfig;
        TCPCommandLineArguments tcpArgs(defaultConfig);
        tcpArgs.print();
    }
}
