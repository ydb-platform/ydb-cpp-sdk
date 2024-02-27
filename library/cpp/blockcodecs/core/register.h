#pragma once

#include "codecs.h"

namespace NBlockCodecs{

    void RegisterCodec(TCodecPtr codec);
    void RegisterAlias(std::string_view from, std::string_view to);

}
