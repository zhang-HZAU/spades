#include "read_corrector.hpp"

#include "kmer_data.hpp"
#include "kmer_stat.hpp"
#include "valid_kmer_generator.hpp"

#include <string>
#include <vector>
#include <queue>

using namespace hammer;

struct state {
    state(size_t p, std::string s, double pen, KMer l)
            : pos(p), str(s), penalty(pen), last(l) {}

    size_t pos; std::string str; double penalty; KMer last;
};

std::ostream& operator<<(std::ostream &os, const state &state) {
    os << "[pos: " << state.pos << ", last: " << state.last << " penalty: " << state.penalty << "]";
    return os;
}

namespace std {
template<>
struct less<state> {
    bool operator()(const state &lhs, const state &rhs) const {
        return lhs.penalty < rhs.penalty ||
               (lhs.penalty == rhs.penalty && lhs.pos < rhs.pos);
    }
};
};

std::string ReadCorrector::CorrectReadRight(const std::string &seq, const std::string &qual,
                                            size_t right_pos) {
    size_t read_size = seq.size();
    std::priority_queue<state> corrections;
    corrections.emplace(right_pos, seq, 0.0, KMer(seq, right_pos - K + 1, K, /* raw */ true));
    while (!corrections.empty()) {
        state correction = corrections.top(); corrections.pop();
        changed_reads_ = std::max(changed_reads_, corrections.size());
        //INFO("State: " << correction);
        size_t pos = correction.pos + 1;
        if (pos == read_size) {
            //INFO(seq);
            //INFO(correction.str);
            //std::string qual2 = qual;
            //for (size_t i = 0; i < qual2.size(); ++i)
            //    qual2[i] += 33;
            //INFO(qual2);
            return correction.str;
        }

	if (correction.penalty < 0.0 - read_size * 10 / 100)
	    continue;

        char c = correction.str[pos];
        // See, whether it's enough to perform single nucl extension
        bool extended = false;
        if (is_nucl(c)) {
            KMer last = correction.last << dignucl(c);
            size_t idx = data_.checking_seq_idx(last);
            if (idx != -1ULL) {
                const KMerStat &kmer_data = data_[idx];
                if (kmer_data.isGood()) {
                    corrections.emplace(pos, correction.str, correction.penalty, last);
                    extended = true;
                }
            }
        }

        if (extended)
            continue;

        for (char cc = 0; cc < 4; ++cc) {
            KMer last = correction.last << cc;
            size_t idx = data_.checking_seq_idx(last);
            if (idx == -1ULL)
                continue;

            const KMerStat &kmer_data = data_[idx];
            char ncc = nucl(cc);
            if (c == ncc) {
                corrections.emplace(pos, correction.str, correction.penalty - (kmer_data.isGood() ? 0.0 : 1.0), last);
            } else if (kmer_data.isGood()) {
                std::string corrected = correction.str; corrected[pos] = ncc;
                corrections.emplace(pos, corrected, correction.penalty - 1.0, last);
            }
        }
    }

    #pragma omp atomic
    uncorrected_nucleotides_ += read_size;

    return seq;
}

bool ReadCorrector::CorrectOneRead(Read & r,
                                   bool correct_threshold, bool discard_singletons, bool discard_bad) {
    std::string seq = r.getSequenceString();
    const std::string &qual = r.getQualityString();

    size_t read_size = seq.size();

    // Find the longest "solid island"
    size_t lleft_pos = -1ULL, lright_pos = -1ULL, solid_len = 0;

    ValidKMerGenerator<K> gen(seq.data(), qual.data(), read_size);
    size_t left_pos = 0, right_pos = 0;
    while (gen.HasMore()) {
        size_t read_pos = gen.pos() - 1;
        hammer::KMer kmer = gen.kmer();
        const KMerStat &kmer_data = data_[kmer];

        if (kmer_data.isGood()) {
            if (read_pos != right_pos - K + 2) {
                left_pos = read_pos;
                right_pos = left_pos + K - 1;
            } else
                right_pos += 1;

            if (right_pos - left_pos + 1 > solid_len) {
                lleft_pos = left_pos;
                lright_pos = right_pos;
                solid_len = right_pos - left_pos + 1;
            }
        }

        // INFO("" << left_pos << ":" << right_pos << ":" << read_pos << ", " << lleft_pos << ":" << lright_pos << "(" << solid_len << "), " << (kmer_data.isGood() ? "solid" : "non-solid"));

        gen.Next();
    }

    // Now iterate over all the k-mers of a read trying to make all the stuff solid and good.
    if (solid_len && solid_len != read_size) {
        //std::string seq2 = seq;
        //INFO(seq2.insert(lleft_pos, "[").insert(lright_pos + 2, "]"));

        seq = CorrectReadRight(seq, qual, lright_pos);
        seq = ReverseComplement(CorrectReadRight(ReverseComplement(seq), Reverse(qual),
                                                 read_size - 1 - lleft_pos));

        if (seq.size() != read_size) {
            INFO("Jere");
            return false;
        }

        r.setSequence(seq.data(), /* preserve_trimming */ true);
        return true;
    }

    return solid_len == read_size;
}
