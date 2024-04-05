#pragma once

#include <library/cpp/lcs/lcs_via_lis.h>

#include <util/generic/algorithm.h>

#include <string_view>
#include <span>

#include <util/stream/output.h>
#include <util/string/split.h>


namespace NDiff {
    template <typename T>
    struct TChunk {

        std::span<const T> Left;
        std::span<const T> Right;
        std::span<const T> Common;

        TChunk() = default;

        TChunk(const std::span<const T>& left, const std::span<const T>& right, const std::span<const T>& common)
            : Left(left)
            , Right(right)
            , Common(common)
        {
        }
    };

    template <typename T>
    size_t InlineDiff(std::vector<TChunk<T>>& chunks, const std::span<const T>& left, const std::span<const T>& right) {
        std::span<const T> s1(left);
        std::span<const T> s2(right);

        bool swapped = false;
        if (s1.size() < s2.size()) {
            // NLCS will silently swap strings if second string is longer
            // So we swap strings here and remember the fact since it is crucial to diff
            DoSwap(s1, s2);
            swapped = true;
        }

        std::vector<T> lcs;
        NLCS::TLCSCtx<T> ctx;
        NLCS::MakeLCS<T>(s1, s2, &lcs, &ctx);

        // Start points of current common and diff parts
        const T* c1 = nullptr;
        const T* c2 = nullptr;
        const T* d1 = s1.data();
        const T* d2 = s2.data();

        // End points of current common parts
        const T* e1 = s1.data();
        const T* e2 = s2.data();

        size_t dist = s1.size() - lcs.size();

        const size_t n = ctx.ResultBuffer.size();
        for (size_t i = 0; i <= n && (e1 != s1.data() + s1.size() || e2 != s2.data() + s2.size());) {
            if (i < n) {
                // Common character exists
                // LCS is marked against positions in s2
                // Save the beginning of common part in s2
                c2 = s2.data() + ctx.ResultBuffer[i]; 
                // Find the beginning of common part in s1
                c1 = Find(e1, s1.data() + s1.size(), *c2);
                // Follow common substring
                for (e1 = c1, e2 = c2; i < n && *e1 == *e2; ++e1, ++e2) {
                    ++i;
                }
            } else {
                // No common character, common part is empty
                c1 = s1.data() + s1.size();
                c2 = s2.data() + s2.size();
                e1 = s1.data() + s1.size();
                e2 = s2.data() + s2.size();
            }

            TChunk<T> chunk(std::span<const T>(d1, c1), std::span<const T>(d2, c2), std::span<const T>(c1, e1));
            if (swapped) {
                DoSwap(chunk.Left, chunk.Right);
                chunk.Common = std::span<const T>(c2, e2);
            }
            chunks.push_back(chunk);

            d1 = e1;
            d2 = e2;
        }

        return dist;
    }

    template <typename TFormatter, typename T>
    void PrintChunks(IOutputStream& out, const TFormatter& fmt, const std::vector<TChunk<T>>& chunks) {
        for (typename std::vector<TChunk<T>>::const_iterator chunk = chunks.begin(); chunk != chunks.end(); ++chunk) {
            if (!chunk->Left.empty() || !chunk->Right.empty()) {
                out << fmt.Special("(");
                out << fmt.Left(chunk->Left);
                out << fmt.Special("|");
                out << fmt.Right(chunk->Right);
                out << fmt.Special(")");
            }
            out << fmt.Common(chunk->Common);
        }
    }

    // Without delimiters calculates character-wise diff
    // With delimiters calculates token-wise diff
    size_t InlineDiff(std::vector<TChunk<char>>& chunks, const std::string_view& left, const std::string_view& right, const std::string& delims = std::string());
    size_t InlineDiff(std::vector<TChunk<wchar16>>& chunks, const std::u16string_view& left, const std::u16string_view& right, const std::u16string& delims = std::u16string());

}
