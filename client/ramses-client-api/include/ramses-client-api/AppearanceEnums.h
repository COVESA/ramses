//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_APPEARANCEENUMS_H
#define RAMSES_APPEARANCEENUMS_H

#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /**
     * @addtogroup CoreAPI
     * @{
     */

    /**
    * Specifies the blending operation.
    * @ingroup CoreAPI
    */
    enum EBlendOperation
    {
        EBlendOperation_Disabled = 0,
        EBlendOperation_Add,
        EBlendOperation_Subtract,
        EBlendOperation_ReverseSubtract,
        EBlendOperation_Min,
        EBlendOperation_Max,
        EBlendOperation_NUMBER_OF_ELEMENTS
    };

    /**
    * Specifies the blending factor used with blending operation.
    * @ingroup CoreAPI
    */
    enum EBlendFactor
    {
        EBlendFactor_Zero = 0,
        EBlendFactor_One,
        EBlendFactor_SrcAlpha,
        EBlendFactor_OneMinusSrcAlpha,
        EBlendFactor_DstAlpha,
        EBlendFactor_OneMinusDstAlpha,
        EBlendFactor_SrcColor,
        EBlendFactor_OneMinusSrcColor,
        EBlendFactor_DstColor,
        EBlendFactor_OneMinusDstColor,
        EBlendFactor_ConstColor,
        EBlendFactor_OneMinusConstColor,
        EBlendFactor_ConstAlpha,
        EBlendFactor_OneMinusConstAlpha,
        EBlendFactor_AlphaSaturate,
        EBlendFactor_NUMBER_OF_ELEMENTS
    };

    /**
    * Specifies the culling mode.
    */
    enum ECullMode
    {
        ECullMode_Disabled = 0,
        ECullMode_FrontFacing,
        ECullMode_BackFacing,
        ECullMode_FrontAndBackFacing,
        ECullMode_NUMBER_OF_ELEMENTS
    };

    /**
    * Specifies the depth write state.
    */
    enum EDepthWrite
    {
        EDepthWrite_Disabled = 0,
        EDepthWrite_Enabled,
        EDepthWrite_NUMBER_OF_ELEMENTS
    };

    /**
    * Specifies the scissor test state.
    */
    enum EScissorTest
    {
        EScissorTest_Disabled = 0,
        EScissorTest_Enabled,
        EScissorTest_NUMBER_OF_ELEMENTS
    };

    /**
    * Specifies the depth function.
    */
    enum EDepthFunc
    {
        EDepthFunc_Disabled = 0,
        EDepthFunc_Greater,
        EDepthFunc_GreaterEqual,
        EDepthFunc_Less,
        EDepthFunc_LessEqual,
        EDepthFunc_Equal,
        EDepthFunc_NotEqual,
        EDepthFunc_Always,
        EDepthFunc_Never,
        EDepthFunc_NUMBER_OF_ELEMENTS
    };

    /**
    * Specifies the stencil function.
    */
    enum EStencilFunc
    {
        EStencilFunc_Disabled = 0,
        EStencilFunc_Never,
        EStencilFunc_Always,
        EStencilFunc_Equal,
        EStencilFunc_NotEqual,
        EStencilFunc_Less,
        EStencilFunc_LessEqual,
        EStencilFunc_Greater,
        EStencilFunc_GreaterEqual,
        EStencilFunc_NUMBER_OF_ELEMENTS
    };

    /**
    * Specifies the stencil operation.
    */
    enum EStencilOperation
    {
        EStencilOperation_Keep = 0,
        EStencilOperation_Zero,
        EStencilOperation_Replace,
        EStencilOperation_Increment,
        EStencilOperation_IncrementWrap,
        EStencilOperation_Decrement,
        EStencilOperation_DecrementWrap,
        EStencilOperation_Invert,
        EStencilOperation_NUMBER_OF_ELEMENTS
    };

    /**
    * Specifies the drawing mode.
    */
    enum EDrawMode
    {
        EDrawMode_Points = 0,
        EDrawMode_Lines,
        EDrawMode_Triangles,
        EDrawMode_TriangleStrip,
        EDrawMode_LineLoop,
        EDrawMode_TriangleFan,
        EDrawMode_LineStrip,
        EDrawMode_NUMBER_OF_ELEMENTS
    };

    /**
     * @brief Returns string representation for blend operation
     * @details Useful for logging, etc.
     *
     * @param blendOperation The enum parameter for which you will get the string
     * @return String representation of the blend operation
     */
    RAMSES_API const char* getBlendOperationString(EBlendOperation blendOperation);

    /**
     * @brief Returns string representation for blend factor
     * @details Useful for logging, etc.
     *
     * @param blendFactor The enum parameter for which you will get the string
     * @return String representation of the blend factor
     */
    RAMSES_API const char* getBlendFactorString(EBlendFactor blendFactor);

    /**
     * @brief Returns string representation for cull mode
     * @details Useful for logging, etc.
     *
     * @param cullMode The enum parameter for which you will get the string
     * @return String representation of the cull mode
     */
    RAMSES_API const char* getCullModeString(ECullMode cullMode);

    /**
     * @brief Returns string representation for depth write
     * @details Useful for logging, etc.
     *
     * @param depthWrite The enum parameter for which you will get the string
     * @return String representation of the depth write
     */
    RAMSES_API const char* getDepthWriteString(EDepthWrite depthWrite);

    /**
     * @brief Returns string representation for scissor test
     * @details Useful for logging, etc.
     *
     * @param scissorTest The enum parameter for which you will get the string
     * @return String representation of the scissor test
     */
    RAMSES_API const char* getScissorTestString(EScissorTest scissorTest);

    /**
     * @brief Returns string representation for depth function
     * @details Useful for logging, etc.
     *
     * @param depthFunc The enum parameter for which you will get the string
     * @return String representation of the depth function
     */
    RAMSES_API const char* getDepthFuncString(EDepthFunc depthFunc);

    /**
     * @brief Returns string representation for stencil function
     * @details Useful for logging, etc.
     *
     * @param stencilFunc The enum parameter for which you will get the string
     * @return String representation of the stencil function
     */
    RAMSES_API const char* getStencilFuncString(EStencilFunc stencilFunc);

    /**
     * @brief Returns string representation for stencil operation
     * @details Useful for logging, etc.
     *
     * @param stencilOp The enum parameter for which you will get the string
     * @return String representation of the stencil operation
     */
    RAMSES_API const char* getStencilOperationString(EStencilOperation stencilOp);

    /**
     * @brief Returns string representation for draw mode
     * @details Useful for logging, etc.
     *
     * @param drawMode The enum parameter for which you will get the string
     * @return String representation of the draw mode
     */
    RAMSES_API const char* getDrawModeString(EDrawMode drawMode);

    /**
     * @}
     */
}

#endif  //RAMSES_APPEARANCEENUMS_H
