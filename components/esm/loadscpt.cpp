#include "loadscpt.hpp"

#include "esmreader.hpp"
#include "esmwriter.hpp"
#include "defs.hpp"

#include <iostream>

namespace ESM
{

    unsigned int Script::sRecordId = REC_SCPT;

    void Script::loadSCVR(ESMReader &esm)
    {
        int s = mData.mStringTableSize;

        std::vector<char> tmp (s);
        // not using getHExact, vanilla doesn't seem to mind unused bytes at the end
        esm.getSubHeader();
        int left = esm.getSubSize();
        if (left < s)
            esm.fail("SCVR string list is smaller than specified");
        esm.getExact(&tmp[0], s);
        if (left > s)
            esm.skip(left-s); // skip the leftover junk

        // Set up the list of variable names
        mVarNames.resize(mData.mNumShorts + mData.mNumLongs + mData.mNumFloats);

        // The tmp buffer is a null-byte separated string list, we
        // just have to pick out one string at a time.
        char* str = &tmp[0];
        for (size_t i = 0; i < mVarNames.size(); i++)
        {
            // Support '\r' terminated strings like vanilla.  See Bug #1324.
            char *termsym = strchr(str, '\r');
            if(termsym) *termsym = '\0';
            mVarNames[i] = std::string(str);
            str += mVarNames[i].size() + 1;

            if (str - &tmp[0] > s)
            {
                // Apparently SCVR subrecord is not used and variable names are
                // determined on the fly from the script text.  Therefore don't throw
                // an exeption, just log an error and continue.
                std::stringstream ss;

                ss << "ESM Error: " << "String table overflow";
                ss << "\n  File: " << esm.getName();
                ss << "\n  Record: " << esm.getContext().recName.toString();
                ss << "\n  Subrecord: " << "SCVR";
                ss << "\n  Offset: 0x" << std::hex << esm.getFileOffset();
                std::cerr << ss.str() << std::endl;
                break;
            }

        }
    }

    void Script::load(ESMReader &esm)
    {
        SCHD data;
        esm.getHNT(data, "SCHD", 52);
        mData = data.mData;
        mId = data.mName.toString();

        mVarNames.clear();

        while (esm.hasMoreSubs())
        {
            esm.getSubName();
            uint32_t name = esm.retSubName().val;
            switch (name)
            {
                case ESM::FourCC<'S','C','V','R'>::value:
                    // list of local variables
                    loadSCVR(esm);
                    break;
                case ESM::FourCC<'S','C','D','T'>::value:
                    // compiled script
                    mScriptData.resize(mData.mScriptDataSize);
                    esm.getHExact(&mScriptData[0], mScriptData.size());
                    break;
                case ESM::FourCC<'S','C','T','X'>::value:
                    mScriptText = esm.getHString();
                    break;
                default:
                    esm.fail("Unknown subrecord");
            }
        }
    }

    void Script::save(ESMWriter &esm) const
    {
        std::string varNameString;
        if (!mVarNames.empty())
            for (std::vector<std::string>::const_iterator it = mVarNames.begin(); it != mVarNames.end(); ++it)
                varNameString.append(*it);

        SCHD data;
        memset(&data, 0, sizeof(data));

        data.mData = mData;
        memcpy(data.mName.name, mId.c_str(), mId.size());

        esm.writeHNT("SCHD", data, 52);

        if (!mVarNames.empty())
        {
            esm.startSubRecord("SCVR");
            for (std::vector<std::string>::const_iterator it = mVarNames.begin(); it != mVarNames.end(); ++it)
            {
                esm.writeHCString(*it);
            }
            esm.endRecord("SCVR");
        }

        esm.startSubRecord("SCDT");
        esm.write(reinterpret_cast<const char * >(&mScriptData[0]), mData.mScriptDataSize);
        esm.endRecord("SCDT");

        esm.writeHNOString("SCTX", mScriptText);
    }

    void Script::blank()
    {
        mData.mNumShorts = mData.mNumLongs = mData.mNumFloats = 0;
        mData.mScriptDataSize = 0;
        mData.mStringTableSize = 0;

        mVarNames.clear();
        mScriptData.clear();

        if (mId.find ("::")!=std::string::npos)
            mScriptText = "Begin \"" + mId + "\"\n\nEnd " + mId + "\n";
        else
            mScriptText = "Begin " + mId + "\n\nEnd " + mId + "\n";
    }

}
