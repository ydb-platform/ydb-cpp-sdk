#pragma once

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/generic/fwd.h>
=======
#include <src/util/generic/fwd.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

#include <string>

void ReverseInPlace(std::string& string);

/** NB. UTF-16 is variable-length encoding because of the surrogate pairs.
 * This function takes this into account and treats a surrogate pair as a single symbol.
 * Ex. if [C D] is a surrogate pair,
 * A B [C D] E
 * will become
 * E [C D] B A
 */
void ReverseInPlace(std::u16string& string);

void ReverseInPlace(std::u32string& string);
