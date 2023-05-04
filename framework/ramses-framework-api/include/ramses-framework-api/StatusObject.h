//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STATUSOBJECT_H
#define RAMSES_STATUSOBJECT_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/APIExport.h"
#include "EValidationSeverity.h"
#include <memory>

namespace ramses
{
    class StatusObjectImpl;

    /**
    * @brief The StatusObject provides status message handling
    */
    class StatusObject
    {
    public:
        /**
        * @brief   Generates verbose validation of the object.
        * @details Checks validity of internal values, references, states and performance warnings.
        *          Will validate object it is called on and all its dependencies recursively, e.g. Scene->MeshNode->Appearance->Resource ...
        *          Result can be obtained by calling #getValidationReport.
        *
        * @return StatusOK if no issues found, otherwise an index to a status message
        *         which will be either "Validation warning" or "Validation error" depending on severity of the issues found.
        *         The status message can be obtained by calling #getStatusMessage.
        */
        [[nodiscard]] RAMSES_API status_t validate() const;

        /**
        * @brief Provides verbose report in human readable form generated by #validate.
        *
        * If \c minSeverity is set to #ramses::EValidationSeverity_Error, only objects with errors and their descriptions
        * are returned, if filtering is set to #EValidationSeverity_Warning additionally all objects with warnings and
        * their descriptions are returned. If filtering is set to #EValidationSeverity_Info all errors and warnings are printed
        * and additionally some objects will print more information like number of instances or similar.
        *
        * @param[in] minSeverity Optional filter, only messages with greater or equal severity will be added to report
        * @return Validation string containing human readable status of the object,
        *         if #validate was not called a pointer to an empty string is returned.
        */
        [[nodiscard]] RAMSES_API const char* getValidationReport(EValidationSeverity minSeverity = EValidationSeverity_Info) const;

        /**
        * @brief Get the string description for a status provided by a RAMSES API function
        *
        * @param status Status returned by a RAMSES API function call
        * @return If value refers to an existing status message, the string with text description for status is returned.
        *         If no status message for value is available, unknown status message is returned.
        */
        [[nodiscard]] RAMSES_API const char* getStatusMessage(status_t status) const;

        /**
         * @brief Deleted copy constructor
         */
        StatusObject(const StatusObject&) = delete;

        /**
         * @brief Deleted move constructor
         */
        StatusObject(StatusObject&&) = delete;

        /**
         * @brief Deleted copy assignment
         * @return unused
         */
        StatusObject& operator=(const StatusObject&) = delete;

        /**
         * @brief Deleted move assignment
         * @return unused
         */
        StatusObject& operator=(StatusObject&&) = delete;

    protected:
        /**
        * @brief Constructor for StatusObject.
        *
        * @param[in] impl Internal data for implementation specifics of StatusObject (sink - instance becomes owner)
        */
        explicit StatusObject(std::unique_ptr<StatusObjectImpl> impl);

        /**
        * @brief Destructor of the StatusObject
        */
        virtual ~StatusObject();

        /**
        * Stores internal data for implementation specifics of StatusObject.
        */
        std::unique_ptr<StatusObjectImpl> m_impl;
    };
}

#endif
