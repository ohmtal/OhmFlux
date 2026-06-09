//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "Utility.h"
#include <console/console.h>
#include <core/stringTable.h>


namespace Utility
{

    //-----------------------------------------------------------------------------

    const char* mGetFirstNonWhitespace(const char* inString)
    {
        // Search for first non-whitespace.
        while (*inString == ' ' || *inString == '\n' || *inString == '\t')
            inString++;

        // Return found point.
        return inString;
    }

    //------------------------------------------------------------------------------

    // NOTE:-   You must verify that elements (index/index+1) are valid first!
    Point2F mGetStringElementVector(const char* inString, const U32 index)
    {
        const U32 elementCount = Utility::mGetStringElementCount(inString);

        if ((index + 1) >= elementCount)
        {
            const F32 element = dAtof(mGetStringElement(inString, index));
            return Point2F(element, element);
        }

        // Get String Element Vector.
        return Point2F(dAtof(mGetStringElement(inString, index)), dAtof(Utility::mGetStringElement(inString, index + 1)));
    }

    //------------------------------------------------------------------------------

    // NOTE:-   You must verify that elements (index/index+1/index+2) are valid first!
    Point3F mGetStringElementVector3D(const char* inString, const U32 index)
    {
        if ((index + 2) >= Utility::mGetStringElementCount(inString))
            return Point3F(0.0f, 0.0f, 0.0f);

        // Get String Element Vector.
        return Point3F(dAtof(mGetStringElement(inString, index)), dAtof(Utility::mGetStringElement(inString, index + 1)), dAtof(Utility::mGetStringElement(inString, index + 2)));
    }

    //-----------------------------------------------------------------------------

    const char* mGetStringElement(const char* inString, const U32 index, const bool copyBuffer)
    {
        // Non-whitespace chars.
        static const char* set = " \t\n";

        U32 wordCount = 0;
        U8 search = 0;
        const char* pWordStart = NULL;

        // End of string?
        if (*inString != 0)
        {
            // No, so search string.
            while (*inString)
            {
                // Get string element.
                search = *inString;

                // End of string?
                if (search == 0)
                    break;

                // Move to next element.
                inString++;

                // Search for separators.
                for (U32 i = 0; set[i]; i++)
                {
                    // Found one?
                    if (search == set[i])
                    {
                        // Yes...
                        search = 0;
                        break;
                    }
                }

                // Found a separator?
                if (search == 0)
                    continue;

                // Found are word?
                if (wordCount == index)
                {
                    // Yes, so mark it.
                    pWordStart = inString - 1;
                }

                // We've found a non-separator.
                wordCount++;

                // Search for end of non-separator.
                while (1)
                {
                    // Get string element.
                    search = *inString;

                    // End of string?
                    if (search == 0)
                        break;

                    // Move to next element.
                    inString++;

                    // Search for separators.
                    for (U32 i = 0; set[i]; i++)
                    {
                        // Found one?
                        if (search == set[i])
                        {
                            // Yes...
                            search = 0;
                            break;
                        }
                    }

                    // Found separator?
                    if (search == 0)
                        break;
                }

                // Have we found our word?
                if (pWordStart)
                {
                    // Yes, so return position if not copying buffer.
                    if (!copyBuffer)
                        return pWordStart;

                    // Result Buffer.
                    static char buffer[4096];

                    // Calculate word length.
                    const U32 length = (const U32)(inString - pWordStart - ((*inString) ? 1 : 0));

                    // Copy Word.
                    dStrncpy(buffer, pWordStart, length);
                    buffer[length] = '\0';

                    // Return Word.
                    return buffer;
                }

                // End of string?
                if (*inString == 0)
                {
                    // Bah!
                    break;
                }
            }
        }

        // Sanity!
        AssertFatal(false, "Utility::mGetStringElement() - Couldn't find specified string element!");

        // Didn't find it
        return StringTable->EmptyString();
    }

    //-----------------------------------------------------------------------------

    U32 mGetStringElementCount(const char* inString)
    {
        // Non-whitespace chars.
        static const char* set = " \t\n";

        // End of string.
        if (*inString == 0)
            return 0;

        U32 wordCount = 0;
        U8 search = 0;

        // Search String.
        while (*inString)
        {
            // Get string element.
            search = *inString;

            // End of string?
            if (search == 0)
                break;

            // Move to next element.
            inString++;

            // Search for separators.
            for (U32 i = 0; set[i]; i++)
            {
                // Found one?
                if (search == set[i])
                {
                    // Yes...
                    search = 0;
                    break;
                }
            }

            // Found a separator?
            if (search == 0)
                continue;

            // We've found a non-separator.
            wordCount++;

            // Search for end of non-separator.
            while (1)
            {
                // Get string element.
                search = *inString;

                // End of string?
                if (search == 0)
                    break;

                // Move to next element.
                inString++;

                // Search for separators.
                for (U32 i = 0; set[i]; i++)
                {
                    // Found one?
                    if (search == set[i])
                    {
                        // Yes...
                        search = 0;
                        break;
                    }
                }

                // Found Separator?
                if (search == 0)
                    break;
            }

            // End of string?
            if (*inString == 0)
            {
                // Bah!
                break;
            }
        }

        // We've finished.
        return wordCount;
    }


} // Namespace Utility
