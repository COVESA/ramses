//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAVECTOR4I_H
#define RAMSES_DATAVECTOR4I_H

#include "ramses-client-api/DataObject.h"

namespace ramses
{
    class DataObjectImpl;

    /**
    * @brief The DataVector4i data object stores a vector with 4 integer components within a scene.
    */
    class RAMSES_API DataVector4i : public DataObject
    {
    public:
        /**
         * @brief Sets/updates the stored values of the vector.
         *
         * @param[in] x new value for the first component of the vector.
         * @param[in] y new value for the second component of the vector.
         * @param[in] z new value for the third component of the vector.
         * @param[in] w new value for the fourth component of the vector.
         * @return status == 0 for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t setValue(int32_t x, int32_t y, int32_t z, int32_t w);

        /**
         * @brief Gets all stored values of the vector.
         *
         * @param[out] x value of the first component of the vector.
         * @param[out] y value of the second component of the vector.
         * @param[out] z value of the third component of the vector.
         * @param[out] w value of the fourth component of the vector.
         * @return status == 0 for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t getValue(int32_t& x, int32_t& y, int32_t& z, int32_t& w) const;

    protected:
        /**
         * @brief Scene is the factory for creating DataVector4i instances.
         */
        friend class SceneImpl;

        /**
         * @brief Constructor of DataVector4i
         *
         * @param[in] pimpl Internal data for implementation specifics of a DataObject (sink - instance becomes owner)
         */
        explicit DataVector4i(DataObjectImpl& pimpl);

        /**
         * @brief Destructor of the DataVector4i
         */
        ~DataVector4i() override;
    };
}

#endif
